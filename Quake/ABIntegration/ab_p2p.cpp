/*
 * AccelByte P2P Connection Layer for QuakeSpasm
 *
 * Architecture:
 *   Host side:
 *     P2PServer listens for incoming ICE connections.
 *     For each accepted P2P client, a proxy thread is spawned that:
 *       - Creates a local UDP socket bound to an ephemeral port
 *       - Forwards data: P2P Connection read() -> UDP sendto(host listen port)
 *       - Forwards data: UDP recvfrom() -> P2P Connection write()
 *     This makes the P2P client appear as a regular UDP client to Quake's server.
 *
 *   Client side:
 *     ClientConnectionFactory creates a Connection to the host (by user ID).
 *     A proxy thread is spawned that:
 *       - Creates a local UDP socket on an ephemeral port (the "proxy port")
 *       - Quake client connects to 127.0.0.1:<proxy_port>
 *       - Forwards data: UDP recvfrom(quake client) -> P2P Connection write()
 *       - Forwards data: P2P Connection read() -> UDP sendto(quake client)
 */

#include "ab_p2p.h"
#include "ab_task_runner.h"

#ifdef DEBUG
#undef DEBUG
#endif

// AccelByte P2P headers
#include <accelbyte/p2p_connection/P2PServer.h>
#include <accelbyte/p2p_connection/P2PServerFactory.h>
#include <accelbyte/p2p_connection/ClientConnectionFactory.h>
#include <accelbyte/p2p_connection/Connection.h>

// AccelByte common
#include <accelbyte/user/User.h>
#include <accelbyte/lobby/LobbyConnection.h>
#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>
#include <accelbyte/common/Error.h>

// Standard library
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>

// Platform sockets
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif

// Quake headers
extern "C" {
#include "quakedef.h"
#include "console.h"
extern int net_hostport;
extern void Cbuf_AddText(const char* text);
}

// Shared state from ab_integration.cpp
extern accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> g_lobby_connection;
extern accelbyte::memory::SharedPtr<accelbyte::user::User> g_current_user;
extern std::mutex g_mutex;
extern ABTaskRunner runner;

//------------------------------------------------------------------------------
// Internal state
//------------------------------------------------------------------------------
static std::mutex s_p2p_mutex;
static unsigned long s_local_addr = INADDR_LOOPBACK; // resolved local IP for proxy sockets (network byte order)

// Host state
static std::shared_ptr<accelbyte::p2p_connection::P2PServer> s_p2p_server;
static std::atomic<bool> s_server_running{false};
static std::thread s_server_accept_thread;

// Per-client proxy on host side
struct HostClientProxy {
	std::shared_ptr<accelbyte::p2p_connection::Connection> connection;
	std::thread proxy_thread;
	std::atomic<bool> running{false};
	SOCKET udp_socket = INVALID_SOCKET;
	std::string peer_id;
};
static std::vector<std::unique_ptr<HostClientProxy>> s_host_proxies;

// Client state
static std::shared_ptr<accelbyte::p2p_connection::Connection> s_client_connection;
static std::thread s_client_connect_thread;
static std::thread s_client_proxy_thread;
static std::atomic<bool> s_client_running{false};
static std::atomic<bool> s_client_connected{false};
static SOCKET s_client_proxy_socket = INVALID_SOCKET;
static std::atomic<int> s_client_proxy_port{0};

//------------------------------------------------------------------------------
// Resolve local IP address (same logic as Quake's WINS_GetLocalAddress)
//------------------------------------------------------------------------------
static void resolve_local_addr(void)
{
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) != 0)
	{
		s_local_addr = htonl(INADDR_LOOPBACK);
		return;
	}
	hostname[sizeof(hostname) - 1] = 0;

	struct hostent* host = gethostbyname(hostname);
	if (host && host->h_addr_list[0])
	{
		s_local_addr = *(unsigned long*)host->h_addr_list[0];
	}
	else
	{
		s_local_addr = htonl(INADDR_LOOPBACK);
	}
}

//------------------------------------------------------------------------------
// Utility: create a non-blocking UDP socket bound to local IP on an ephemeral port
//------------------------------------------------------------------------------
static SOCKET create_local_udp_socket(int* out_port)
{
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
		return INVALID_SOCKET;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = s_local_addr;
	addr.sin_port = 0; // ephemeral

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	// Retrieve assigned port
	socklen_t addrlen = sizeof(addr);
	if (getsockname(sock, (struct sockaddr*)&addr, &addrlen) == SOCKET_ERROR)
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	if (out_port)
		*out_port = ntohs(addr.sin_port);

	// Set non-blocking
#ifdef _WIN32
	u_long mode = 1;
	ioctlsocket(sock, FIONBIO, &mode);
#else
	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif

	return sock;
}

//------------------------------------------------------------------------------
// Stream framing protocol for P2P data channel
//
// The P2P Connection::read() returns all data accumulated since the last read,
// concatenated into a single buffer. We cannot rely on message boundaries.
//
// Protocol: each UDP datagram is sent as a length-prefixed message:
//   [2 bytes] length (network byte order) — size of the UDP datagram
//   [N bytes] payload — the raw UDP datagram
//
// The receiver maintains a stream buffer, appends each read() result,
// and extracts complete length-prefixed messages from it.
//------------------------------------------------------------------------------

// Max bytes per P2P write call — must be under TURN relay MTU (~1200).
// Leave headroom for TURN framing overhead.
static const int P2P_WRITE_CHUNK_SIZE = 1100;

static void p2p_send_framed(
	std::shared_ptr<accelbyte::p2p_connection::Connection>& conn,
	const char* data, int len)
{
	// Build the framed message: 2-byte length header + payload
	std::vector<char> msg(2 + len);
	uint16_t net_len = htons((uint16_t)len);
	memcpy(msg.data(), &net_len, 2);
	memcpy(msg.data() + 2, data, len);

	// Send in chunks to stay within TURN relay MTU
	size_t offset = 0;
	size_t total = msg.size();
	while (offset < total)
	{
		size_t chunk = total - offset;
		if (chunk > (size_t)P2P_WRITE_CHUNK_SIZE)
			chunk = (size_t)P2P_WRITE_CHUNK_SIZE;
		conn->write(msg.data() + offset, chunk);
		offset += chunk;
	}
}

// Stream buffer that accumulates P2P reads and extracts framed messages
struct StreamBuffer {
	std::vector<char> buf;

	void append(const char* data, size_t len)
	{
		buf.insert(buf.end(), data, data + len);
	}

	// Extract the next complete message. Returns size, or 0 if incomplete.
	// On success, copies the payload into out_data and removes it from the buffer.
	int extract(char* out_data, int out_max)
	{
		if (buf.size() < 2)
			return 0;

		uint16_t net_len;
		memcpy(&net_len, buf.data(), 2);
		int msg_len = ntohs(net_len);

		if (msg_len <= 0 || msg_len > out_max)
		{
			// Invalid length — discard the whole buffer to resync
			buf.clear();
			return 0;
		}

		if ((int)buf.size() < 2 + msg_len)
			return 0; // incomplete, wait for more data

		memcpy(out_data, buf.data() + 2, msg_len);
		buf.erase(buf.begin(), buf.begin() + 2 + msg_len);
		return msg_len;
	}
};

//------------------------------------------------------------------------------
// Host: proxy thread for a single P2P client
// Bridges P2P read/write <-> UDP to Quake's listen server port
//------------------------------------------------------------------------------
// Quake net protocol constants (from net_defs.h)
#define NETFLAG_CTL		0x80000000
#define CCREP_ACCEPT	0x81

// Read a big-endian 32-bit int from a buffer (for packet header)
static int32_t read_big_long(const char* buf)
{
	const unsigned char* p = (const unsigned char*)buf;
	return ((int32_t)p[0] << 24) | ((int32_t)p[1] << 16) | ((int32_t)p[2] << 8) | p[3];
}

// Read a little-endian 32-bit int from a buffer (for MSG_WriteLong payload)
static int32_t read_little_long(const char* buf)
{
	const unsigned char* p = (const unsigned char*)buf;
	return p[0] | ((int32_t)p[1] << 8) | ((int32_t)p[2] << 16) | ((int32_t)p[3] << 24);
}

// Write a little-endian 32-bit int to a buffer (for MSG_ReadLong payload)
static void write_little_long(char* buf, int32_t val)
{
	unsigned char* p = (unsigned char*)buf;
	p[0] = val & 0xff;
	p[1] = (val >> 8) & 0xff;
	p[2] = (val >> 16) & 0xff;
	p[3] = (val >> 24) & 0xff;
}

static void host_proxy_thread_func(HostClientProxy* proxy)
{
	const int QUAKE_SERVER_PORT = net_hostport;

	// Create a local UDP socket to talk to Quake's listen server
	int local_port = 0;
	proxy->udp_socket = create_local_udp_socket(&local_port);
	if (proxy->udp_socket == INVALID_SOCKET)
	{
		runner.queue_task([peer_id = proxy->peer_id](){
			Con_Printf("ABP2P: Failed to create proxy socket for peer %s\n", peer_id.c_str());
		});
		proxy->running = false;
		return;
	}

	// Target: Quake listen server on local IP
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = s_local_addr;
	server_addr.sin_port = htons((u_short)QUAKE_SERVER_PORT);

	runner.queue_task([peer_id = proxy->peer_id, local_port](){
		Con_Printf("ABP2P: Host proxy for peer %s on local port %d\n", peer_id.c_str(), local_port);
	});

	char udp_buf[NET_MAXMESSAGE];
	StreamBuffer stream;

	while (proxy->running)
	{
		bool did_work = false;

		// P2P -> UDP: read stream data from P2P, extract framed messages, send to Quake server
		accelbyte::Vector<char> p2p_data;
		auto err = proxy->connection->read(p2p_data);
		if (err.type() == accelbyte::Error::ok && p2p_data.size() > 0)
		{
			stream.append(p2p_data.data(), p2p_data.size());
			int msg_len;
			while ((msg_len = stream.extract(udp_buf, sizeof(udp_buf))) > 0)
			{
				sendto(proxy->udp_socket, udp_buf, msg_len, 0,
					(struct sockaddr*)&server_addr, sizeof(server_addr));
			}
			did_work = true;
		}

		// UDP -> P2P: read from Quake server, intercept CCREP_ACCEPT, frame and send via P2P
		struct sockaddr_in from_addr;
		socklen_t from_len = sizeof(from_addr);
		int recv_len = recvfrom(proxy->udp_socket, udp_buf, sizeof(udp_buf), 0,
			(struct sockaddr*)&from_addr, &from_len);
		if (recv_len > 0)
		{
			// Intercept CCREP_ACCEPT to track server's new game port and rewrite it
			// Packet format: [4 bytes: BigLong(NETFLAG_CTL|len)] [1 byte: CCREP_ACCEPT] [4 bytes: BigLong(port)]
			if (recv_len >= 9)
			{
				int32_t header = read_big_long(udp_buf);
				int pkt_len = header & 0x0000ffff;  // low 16 bits = length
				int pkt_flags = header & 0xffff0000; // high 16 bits = flags

				if ((pkt_flags & NETFLAG_CTL) && (unsigned char)udp_buf[4] == CCREP_ACCEPT && pkt_len >= 9)
				{
					// Read the server's new game port (little-endian, written by MSG_WriteLong)
					int32_t server_game_port = read_little_long(udp_buf + 5);

					runner.queue_task([peer_id = proxy->peer_id, server_game_port, local_port](){
						Con_Printf("ABP2P: Intercepted CCREP_ACCEPT for peer %s: server game port %d, rewriting to proxy port %d\n",
							peer_id.c_str(), server_game_port, local_port);
					});

					// Update server_addr to point to the server's new game socket
					server_addr.sin_port = htons((u_short)server_game_port);

					// Rewrite the port in the packet to be our proxy's local port
					// so the client on the other end keeps talking to the proxy
					write_little_long(udp_buf + 5, local_port);
				}
			}

			p2p_send_framed(proxy->connection, udp_buf, recv_len);
			did_work = true;
		}

		if (!did_work)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	if (proxy->udp_socket != INVALID_SOCKET)
	{
		closesocket(proxy->udp_socket);
		proxy->udp_socket = INVALID_SOCKET;
	}
}

//------------------------------------------------------------------------------
// Host: server accept thread — polls for incoming P2P connections
//------------------------------------------------------------------------------
static void server_accept_thread_func()
{
	// Signal that we accept connections upfront (force relay for testing)
	s_p2p_server->set_connection_accepted(true, /*force_relay=*/true);

	runner.queue_task([](){
		Con_Printf("ABP2P: Server accept thread started, accepting connections\n");
	});

	while (s_server_running)
	{
		if (!s_p2p_server)
			break;

		// Wait briefly for connection requests
		s_p2p_server->wait_for_connection_request(100); // 100ms timeout

		auto connections = s_p2p_server->requested_connections();
		if (connections.size() > 0)
		{
			for (size_t i = 0; i < connections.size(); i++)
			{
				auto& conn = connections[i];
				std::string peer_id = conn->peer_id().c_str();

				// Connect the peer (this does the ICE negotiation)
				auto err = conn->connect();
				if (err.type() != accelbyte::Error::ok)
				{
					std::string err_msg = err.what().c_str();
					runner.queue_task([peer_id, err_msg](){
						Con_Printf("ABP2P: Failed to connect peer %s: %s\n",
							peer_id.c_str(), err_msg.c_str());
					});
					continue;
				}

				runner.queue_task([peer_id](){
					Con_Printf("ABP2P: Peer %s connected via P2P\n", peer_id.c_str());
				});

				// Spawn a proxy thread for this client
				auto proxy = std::make_unique<HostClientProxy>();
				proxy->connection = conn;
				proxy->peer_id = peer_id;
				proxy->running = true;
				proxy->proxy_thread = std::thread(host_proxy_thread_func, proxy.get());

				std::lock_guard<std::mutex> lock(s_p2p_mutex);
				s_host_proxies.push_back(std::move(proxy));
			}
		}
	}

	runner.queue_task([](){
		Con_Printf("ABP2P: Server accept thread stopped\n");
	});
}

//------------------------------------------------------------------------------
// Client: proxy thread
// Bridges Quake client UDP <-> P2P connection to host
//------------------------------------------------------------------------------
static void client_proxy_thread_func()
{
	char udp_buf[NET_MAXMESSAGE];
	struct sockaddr_in client_addr;
	bool have_client_addr = false;
	StreamBuffer stream;

	runner.queue_task([port = s_client_proxy_port.load()](){
		Con_Printf("ABP2P: Client proxy thread started on port %d\n", port);
	});

	int proxy_port = s_client_proxy_port.load();

	while (s_client_running)
	{
		bool did_work = false;

		// P2P -> UDP: read stream data from P2P (from host), extract framed messages, forward to Quake client
		accelbyte::Vector<char> p2p_data;
		auto err = s_client_connection->read(p2p_data);
		if (err.type() == accelbyte::Error::ok && p2p_data.size() > 0)
		{
			stream.append(p2p_data.data(), p2p_data.size());
			int msg_len;
			while ((msg_len = stream.extract(udp_buf, sizeof(udp_buf))) > 0)
			{
				// Intercept CCREP_ACCEPT: rewrite port to our proxy port
				// so the Quake client keeps sending to this proxy
				if (msg_len >= 9)
				{
					int32_t header = read_big_long(udp_buf);
					int pkt_flags = header & 0xffff0000;
					int pkt_len = header & 0x0000ffff;

					if ((pkt_flags & NETFLAG_CTL) && (unsigned char)udp_buf[4] == CCREP_ACCEPT && pkt_len >= 9)
					{
						int32_t advertised_port = read_little_long(udp_buf + 5);
						runner.queue_task([advertised_port, proxy_port](){
							Con_Printf("ABP2P: Client proxy intercepted CCREP_ACCEPT: host advertised port %d, rewriting to proxy port %d\n",
								advertised_port, proxy_port);
						});
						write_little_long(udp_buf + 5, proxy_port);
					}
				}

				if (have_client_addr)
				{
					sendto(s_client_proxy_socket, udp_buf, msg_len, 0,
						(struct sockaddr*)&client_addr, sizeof(client_addr));
				}
			}
			did_work = true;
		}

		// UDP -> P2P: read from Quake client, frame and forward to host via P2P
		struct sockaddr_in from_addr;
		socklen_t from_len = sizeof(from_addr);
		int recv_len = recvfrom(s_client_proxy_socket, udp_buf, sizeof(udp_buf), 0,
			(struct sockaddr*)&from_addr, &from_len);
		if (recv_len > 0)
		{
			// Remember the Quake client's address so we can send responses back
			if (!have_client_addr)
			{
				client_addr = from_addr;
				have_client_addr = true;
			}

			p2p_send_framed(s_client_connection, udp_buf, recv_len);
			did_work = true;
		}

		if (!did_work)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	runner.queue_task([](){
		Con_Printf("ABP2P: Client proxy thread stopped\n");
	});
}

//------------------------------------------------------------------------------
// Public C API
//------------------------------------------------------------------------------
extern "C" {

void ABP2P_Init(void)
{
	resolve_local_addr();

	struct in_addr a;
	a.s_addr = s_local_addr;
	Con_Printf("ABP2P: Local address for proxying: %s\n", inet_ntoa(a));
}

void ABP2P_Shutdown(void)
{
	ABP2P_StopServer();
	ABP2P_Disconnect();
}

void ABP2P_Update(void)
{
	// Clean up dead host proxies (connections that dropped)
	std::lock_guard<std::mutex> lock(s_p2p_mutex);
	for (auto it = s_host_proxies.begin(); it != s_host_proxies.end(); )
	{
		auto& proxy = *it;
		if (proxy->running && proxy->connection && !proxy->connection->is_connected())
		{
			proxy->running = false;
			if (proxy->proxy_thread.joinable())
				proxy->proxy_thread.join();

			std::string peer_id = proxy->peer_id;
			runner.queue_task([peer_id](){
				Con_Printf("ABP2P: Peer %s disconnected\n", peer_id.c_str());
			});

			it = s_host_proxies.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void ABP2P_StartServer(void)
{
	if (s_server_running)
	{
		Con_Printf("ABP2P: Server already running\n");
		return;
	}

	// Grab user and lobby connection
	accelbyte::memory::SharedPtr<accelbyte::user::User> user;
	accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> lobby_conn;
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		user = g_current_user;
		lobby_conn = g_lobby_connection;
	}

	if (!user || !lobby_conn)
	{
		Con_Printf("ABP2P: Cannot start P2P server — not logged in or no lobby connection\n");
		return;
	}

	// Create P2P server — need std::shared_ptr versions
	// P2PServerFactory expects std::shared_ptr<User> and std::shared_ptr<LobbyConnection>
	// We have accelbyte::memory::SharedPtr, which should be compatible
	auto user_ptr = std::shared_ptr<accelbyte::user::User>(user.get(), [user](accelbyte::user::User*){
		// prevent double-delete — the accelbyte::memory::SharedPtr still owns it
		(void)user;
	});
	auto lobby_ptr = std::shared_ptr<accelbyte::lobby::LobbyConnection>(lobby_conn.get(), [lobby_conn](accelbyte::lobby::LobbyConnection*){
		(void)lobby_conn;
	});

	s_p2p_server = accelbyte::p2p_connection::P2PServerFactory::create_server(user_ptr, lobby_ptr);
	if (!s_p2p_server)
	{
		Con_Printf("ABP2P: Failed to create P2P server\n");
		return;
	}

	Con_Printf("ABP2P: P2P server created, accepting connections...\n");

	s_server_running = true;
	s_server_accept_thread = std::thread(server_accept_thread_func);
}

void ABP2P_StopServer(void)
{
	s_server_running = false;

	if (s_server_accept_thread.joinable())
		s_server_accept_thread.join();

	// Stop all client proxies
	{
		std::lock_guard<std::mutex> lock(s_p2p_mutex);
		for (auto& proxy : s_host_proxies)
		{
			proxy->running = false;
			if (proxy->proxy_thread.joinable())
				proxy->proxy_thread.join();
		}
		s_host_proxies.clear();
	}

	s_p2p_server.reset();
}

void ABP2P_ConnectToHost(const char* peer_id)
{
	if (s_client_running)
	{
		Con_Printf("ABP2P: Client already connecting/connected\n");
		return;
	}

	accelbyte::memory::SharedPtr<accelbyte::user::User> user;
	accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> lobby_conn;
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		user = g_current_user;
		lobby_conn = g_lobby_connection;
	}

	if (!user || !lobby_conn)
	{
		Con_Printf("ABP2P: Cannot connect — not logged in or no lobby connection\n");
		return;
	}

	// Create the proxy socket first so we know the port
	int proxy_port = 0;
	s_client_proxy_socket = create_local_udp_socket(&proxy_port);
	if (s_client_proxy_socket == INVALID_SOCKET)
	{
		Con_Printf("ABP2P: Failed to create client proxy socket\n");
		return;
	}
	s_client_proxy_port = proxy_port;

	std::string peer_id_str(peer_id);

	Con_Printf("ABP2P: Connecting to host %s via P2P (proxy port %d)...\n", peer_id, proxy_port);

	s_client_running = true;
	s_client_connected = false;

	// Spawn connection thread — ICE negotiation is synchronous and may take a few seconds
	s_client_connect_thread = std::thread([peer_id_str, user, lobby_conn](){
		// Create the connection to the peer
		auto user_ptr = std::shared_ptr<accelbyte::user::User>(user.get(), [user](accelbyte::user::User*){
			(void)user;
		});
		auto lobby_ptr = std::shared_ptr<accelbyte::lobby::LobbyConnection>(lobby_conn.get(), [lobby_conn](accelbyte::lobby::LobbyConnection*){
			(void)lobby_conn;
		});

		s_client_connection = accelbyte::p2p_connection::ClientConnectionFactory::create(
			peer_id_str.c_str(), user_ptr, lobby_ptr, /*force_relay=*/true);

		if (!s_client_connection)
		{
			runner.queue_task([](){
				Con_Printf("ABP2P: Failed to create P2P connection\n");
			});
			s_client_running = false;
			return;
		}

		// Connect (synchronous ICE negotiation)
		auto err = s_client_connection->connect();
		if (err.type() != accelbyte::Error::ok)
		{
			std::string err_msg = err.what().c_str();
			runner.queue_task([err_msg](){
				Con_Printf("ABP2P: P2P connection failed: %s\n", err_msg.c_str());
			});
			s_client_connection.reset();
			s_client_running = false;
			return;
		}

		bool using_relay = s_client_connection->is_using_relay();
		int port = s_client_proxy_port.load();

		runner.queue_task([using_relay, port](){
			Con_Printf("ABP2P: P2P connection established! (relay: %s)\n",
				using_relay ? "yes" : "no");

			struct in_addr a;
			a.s_addr = s_local_addr;
			const char* ip = inet_ntoa(a);
			Con_Printf("ABP2P: Connecting Quake client to %s:%d\n", ip, port);

			Cbuf_AddText(va("connect \"%s:%d\"\n", ip, port));
			key_dest = key_game;
			m_state = m_none;
		});

		s_client_connected = true;

		// Start the proxy thread on this same thread
		client_proxy_thread_func();
	});
}

void ABP2P_Disconnect(void)
{
	s_client_running = false;
	s_client_connected = false;

	if (s_client_connect_thread.joinable())
		s_client_connect_thread.join();

	if (s_client_proxy_socket != INVALID_SOCKET)
	{
		closesocket(s_client_proxy_socket);
		s_client_proxy_socket = INVALID_SOCKET;
	}

	s_client_proxy_port = 0;
	s_client_connection.reset();
}

int ABP2P_GetClientProxyPort(void)
{
	return s_client_proxy_port.load();
}

int ABP2P_IsClientConnected(void)
{
	return s_client_connected ? 1 : 0;
}

} // extern "C"
