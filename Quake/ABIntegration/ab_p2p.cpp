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

#ifdef DEBUG
#undef DEBUG
#endif

// AccelByte P2P headers
#include <accelbyte/p2p_connection/P2PServerFactory.h>
#include <accelbyte/p2p_connection/ClientConnectionFactory.h>

// AccelByte common
#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>
#include <accelbyte/common/Error.h>

// Standard library
#include <chrono>
#include <cstring>

// Platform sockets
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>
typedef int socklen_t;
#  define P2P_INVALID_SOCKET INVALID_SOCKET
#  define p2p_closesocket closesocket
static inline SOCKET p2p_sock(uintptr_t h) { return (SOCKET)h; }
#else
#  include <sys/socket.h>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <errno.h>
typedef int SOCKET;
#  define P2P_INVALID_SOCKET ((SOCKET)(-1))
#  define p2p_closesocket close
static inline SOCKET p2p_sock(uintptr_t h) { return (SOCKET)(intptr_t)h; }
#endif

// Quake headers
extern "C" {
#include "quakedef.h"
#include "console.h"
extern int  net_hostport;
extern void Cbuf_AddText(const char* text);
}

//------------------------------------------------------------------------------
// Utility: create a non-blocking UDP socket bound to local IP on an ephemeral port
//------------------------------------------------------------------------------
static SOCKET create_local_udp_socket(unsigned long local_addr, int* out_port)
{
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == P2P_INVALID_SOCKET)
		return P2P_INVALID_SOCKET;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = local_addr;
	addr.sin_port        = 0; // ephemeral

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0)
	{
		p2p_closesocket(sock);
		return P2P_INVALID_SOCKET;
	}

	socklen_t addrlen = sizeof(addr);
	if (getsockname(sock, (struct sockaddr*)&addr, &addrlen) != 0)
	{
		p2p_closesocket(sock);
		return P2P_INVALID_SOCKET;
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
// P2P Connection::read() returns all data accumulated since the last read,
// concatenated into a single buffer.  We cannot rely on message boundaries.
//
// Protocol: each UDP datagram is sent as a length-prefixed message:
//   [2 bytes] length (network byte order) — size of the UDP datagram
//   [N bytes] payload — the raw UDP datagram
//
// The receiver maintains a stream buffer, appends each read() result,
// and extracts complete length-prefixed messages from it.
//------------------------------------------------------------------------------

static const int P2P_WRITE_CHUNK_SIZE = 1100; // stay under TURN relay MTU

static void p2p_send_framed(
	std::shared_ptr<accelbyte::p2p_connection::Connection>& conn,
	const char* data, int len)
{
	std::vector<char> msg(2 + len);
	uint16_t net_len = htons((uint16_t)len);
	memcpy(msg.data(), &net_len, 2);
	memcpy(msg.data() + 2, data, len);

	size_t offset = 0;
	size_t total  = msg.size();
	while (offset < total)
	{
		size_t chunk = total - offset;
		if (chunk > (size_t)P2P_WRITE_CHUNK_SIZE)
			chunk = (size_t)P2P_WRITE_CHUNK_SIZE;
		conn->write(msg.data() + offset, chunk);
		offset += chunk;
	}
}

struct StreamBuffer {
	std::vector<char> buf;

	void append(const char* data, size_t len)
	{
		buf.insert(buf.end(), data, data + len);
	}

	// Returns the next complete payload length, or 0 if incomplete.
	// Copies payload into out_data and removes the message from the buffer.
	int extract(char* out_data, int out_max)
	{
		if (buf.size() < 2)
			return 0;

		uint16_t net_len;
		memcpy(&net_len, buf.data(), 2);
		int msg_len = ntohs(net_len);

		if (msg_len <= 0 || msg_len > out_max)
		{
			buf.clear(); // invalid length — discard to resync
			return 0;
		}

		if ((int)buf.size() < 2 + msg_len)
			return 0; // incomplete

		memcpy(out_data, buf.data() + 2, msg_len);
		buf.erase(buf.begin(), buf.begin() + 2 + msg_len);
		return msg_len;
	}
};

//------------------------------------------------------------------------------
// Packet header helpers (for CCREP_ACCEPT interception)
//------------------------------------------------------------------------------
#define NETFLAG_CTL  0x80000000
#define CCREP_ACCEPT 0x81

static int32_t read_big_long(const char* buf)
{
	const unsigned char* p = (const unsigned char*)buf;
	return ((int32_t)p[0] << 24) | ((int32_t)p[1] << 16) | ((int32_t)p[2] << 8) | p[3];
}

static int32_t read_little_long(const char* buf)
{
	const unsigned char* p = (const unsigned char*)buf;
	return p[0] | ((int32_t)p[1] << 8) | ((int32_t)p[2] << 16) | ((int32_t)p[3] << 24);
}

static void write_little_long(char* buf, int32_t val)
{
	unsigned char* p = (unsigned char*)buf;
	p[0] = val & 0xff;
	p[1] = (val >> 8) & 0xff;
	p[2] = (val >> 16) & 0xff;
	p[3] = (val >> 24) & 0xff;
}

//------------------------------------------------------------------------------
// AB_P2P implementation
//------------------------------------------------------------------------------

AB_P2P::AB_P2P()
{
}

AB_P2P::~AB_P2P()
{
	Shutdown();
}

void AB_P2P::SetTaskRunner(ABTaskRunner& tr)
{
	task_runner_ = &tr;
}

void AB_P2P::SetLobbyContext(
	accelbyte::memory::SharedPtr<accelbyte::user::User>             user,
	accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> lobby_conn)
{
	std::lock_guard<std::mutex> lock(ctx_mutex_);
	current_user_ = std::move(user);
	lobby_conn_   = std::move(lobby_conn);
}

//------------------------------------------------------------------------------
// ResolveLocalAddr — same logic as Quake's WINS_GetLocalAddress
//------------------------------------------------------------------------------
void AB_P2P::ResolveLocalAddr()
{
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) != 0)
	{
		local_addr_ = htonl(INADDR_LOOPBACK);
		return;
	}
	hostname[sizeof(hostname) - 1] = 0;

	struct hostent* host = gethostbyname(hostname);
	if (host && host->h_addr_list[0])
		local_addr_ = *(unsigned long*)host->h_addr_list[0];
	else
		local_addr_ = htonl(INADDR_LOOPBACK);
}

void AB_P2P::Init()
{
	ResolveLocalAddr();

	struct in_addr a;
	a.s_addr = local_addr_;
	Con_Printf("ABP2P: Local address for proxying: %s\n", inet_ntoa(a));
}

void AB_P2P::Shutdown()
{
	StopServer();
	Disconnect();
}

void AB_P2P::Update()
{
	// Auto-join a client thread that exited naturally (e.g., remote P2P disconnect).
	// Update() and Disconnect() both run on the main thread, so there is no race.
	if (!client_running_ && client_connect_thread_.joinable())
	{
		client_connect_thread_.join();

		SOCKET sock = p2p_sock(client_proxy_socket_);
		if (sock != P2P_INVALID_SOCKET)
		{
			p2p_closesocket(sock);
			client_proxy_socket_ = ~uintptr_t(0);
		}
		client_proxy_port_ = 0;
		client_connection_.reset();

		if (task_runner_)
			task_runner_->queue_task([]() {
				Con_Printf("ABP2P: Client P2P connection dropped (remote disconnect)\n");
			});
	}

	// Clean up dead host proxies (connections that dropped).
	std::lock_guard<std::mutex> lock(p2p_mutex_);
	for (auto it = host_proxies_.begin(); it != host_proxies_.end(); )
	{
		auto& proxy = *it;
		if (proxy->running && proxy->connection && !proxy->connection->is_connected())
		{
			proxy->running = false;
			if (proxy->proxy_thread.joinable())
				proxy->proxy_thread.join();

			std::string peer_id = proxy->peer_id;
			if (task_runner_)
				task_runner_->queue_task([peer_id]() {
					Con_Printf("ABP2P: Peer %s disconnected\n", peer_id.c_str());
				});

			it = host_proxies_.erase(it);
		}
		else
		{
			++it;
		}
	}
}

//------------------------------------------------------------------------------
// Host: proxy thread for a single P2P client
//------------------------------------------------------------------------------
void AB_P2P::HostProxyThreadBody(HostClientProxy* proxy)
{
	const int QUAKE_SERVER_PORT = net_hostport;

	int local_port = 0;
	SOCKET sock = create_local_udp_socket(local_addr_, &local_port);
	proxy->udp_socket = (uintptr_t)sock;

	if (sock == P2P_INVALID_SOCKET)
	{
		if (task_runner_)
			task_runner_->queue_task([peer_id = proxy->peer_id]() {
				Con_Printf("ABP2P: Failed to create proxy socket for peer %s\n", peer_id.c_str());
			});
		proxy->running = false;
		return;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = local_addr_;
	server_addr.sin_port        = htons((u_short)QUAKE_SERVER_PORT);

	if (task_runner_)
		task_runner_->queue_task([peer_id = proxy->peer_id, local_port]() {
			Con_Printf("ABP2P: Host proxy for peer %s on local port %d\n", peer_id.c_str(), local_port);
		});

	char udp_buf[NET_MAXMESSAGE];
	StreamBuffer stream;

	while (proxy->running)
	{
		bool did_work = false;

		// P2P -> UDP: read from P2P, extract framed messages, forward to Quake server
		accelbyte::Vector<char> p2p_data;
		auto err = proxy->connection->read(p2p_data);
		if (err.type() == accelbyte::Error::ok && p2p_data.size() > 0)
		{
			stream.append(p2p_data.data(), p2p_data.size());
			int msg_len;
			while ((msg_len = stream.extract(udp_buf, sizeof(udp_buf))) > 0)
			{
				sendto(sock, udp_buf, msg_len, 0,
					(struct sockaddr*)&server_addr, sizeof(server_addr));
			}
			did_work = true;
		}

		// UDP -> P2P: read from Quake server, intercept CCREP_ACCEPT, frame and send
		struct sockaddr_in from_addr;
		socklen_t from_len = sizeof(from_addr);
		int recv_len = recvfrom(sock, udp_buf, sizeof(udp_buf), 0,
			(struct sockaddr*)&from_addr, &from_len);
		if (recv_len > 0)
		{
			if (recv_len >= 9)
			{
				int32_t header   = read_big_long(udp_buf);
				int     pkt_len  = header & 0x0000ffff;
				int     pkt_flags = header & 0xffff0000;

				if ((pkt_flags & NETFLAG_CTL) && (unsigned char)udp_buf[4] == CCREP_ACCEPT && pkt_len >= 9)
				{
					int32_t server_game_port = read_little_long(udp_buf + 5);

					if (task_runner_)
						task_runner_->queue_task([peer_id = proxy->peer_id, server_game_port, local_port]() {
							Con_Printf("ABP2P: Intercepted CCREP_ACCEPT for peer %s: server game port %d, rewriting to proxy port %d\n",
								peer_id.c_str(), server_game_port, local_port);
						});

					server_addr.sin_port = htons((u_short)server_game_port);
					write_little_long(udp_buf + 5, local_port);
				}
			}

			p2p_send_framed(proxy->connection, udp_buf, recv_len);
			did_work = true;
		}

		if (!did_work)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	if (sock != P2P_INVALID_SOCKET)
	{
		p2p_closesocket(sock);
		proxy->udp_socket = ~uintptr_t(0);
	}
}

//------------------------------------------------------------------------------
// Host: accept thread — polls for incoming P2P connections
//------------------------------------------------------------------------------
void AB_P2P::ServerAcceptThreadBody()
{
	p2p_server_->set_connection_accepted(true, /*force_relay=*/true);

	if (task_runner_)
		task_runner_->queue_task([]() {
			Con_Printf("ABP2P: Server accept thread started, accepting connections\n");
		});

	while (server_running_)
	{
		if (!p2p_server_)
			break;

		p2p_server_->wait_for_connection_request(100);

		auto connections = p2p_server_->requested_connections();
		for (size_t i = 0; i < connections.size(); i++)
		{
			auto& conn = connections[i];
			std::string peer_id = conn->peer_id().c_str();

			auto err = conn->connect();
			if (err.type() != accelbyte::Error::ok)
			{
				std::string err_msg = err.what().c_str();
				if (task_runner_)
					task_runner_->queue_task([peer_id, err_msg]() {
						Con_Printf("ABP2P: Failed to connect peer %s: %s\n",
							peer_id.c_str(), err_msg.c_str());
					});
				continue;
			}

			if (task_runner_)
				task_runner_->queue_task([peer_id]() {
					Con_Printf("ABP2P: Peer %s connected via P2P\n", peer_id.c_str());
				});

			auto proxy = std::make_unique<HostClientProxy>();
			proxy->connection = conn;
			proxy->peer_id    = peer_id;
			proxy->running    = true;

			HostClientProxy* proxy_raw = proxy.get();
			proxy->proxy_thread = std::thread([this, proxy_raw]() {
				HostProxyThreadBody(proxy_raw);
			});

			std::lock_guard<std::mutex> lock(p2p_mutex_);
			host_proxies_.push_back(std::move(proxy));
		}
	}

	if (task_runner_)
		task_runner_->queue_task([]() {
			Con_Printf("ABP2P: Server accept thread stopped\n");
		});
}

void AB_P2P::StartServer()
{
	if (server_running_)
	{
		Con_Printf("ABP2P: Server already running\n");
		return;
	}

	accelbyte::memory::SharedPtr<accelbyte::user::User>             user;
	accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> lobby_conn;
	{
		std::lock_guard<std::mutex> lock(ctx_mutex_);
		user      = current_user_;
		lobby_conn = lobby_conn_;
	}

	if (!user || !lobby_conn)
	{
		Con_Printf("ABP2P: Cannot start P2P server — not logged in or no lobby connection\n");
		return;
	}

	// P2PServerFactory expects std::shared_ptr; wrap the accelbyte::memory::SharedPtr
	// with a no-op deleter so ownership stays with the AB SDK.
	auto user_ptr = std::shared_ptr<accelbyte::user::User>(
		user.get(), [user](accelbyte::user::User*) { (void)user; });
	auto lobby_ptr = std::shared_ptr<accelbyte::lobby::LobbyConnection>(
		lobby_conn.get(), [lobby_conn](accelbyte::lobby::LobbyConnection*) { (void)lobby_conn; });

	p2p_server_ = accelbyte::p2p_connection::P2PServerFactory::create_server(user_ptr, lobby_ptr);
	if (!p2p_server_)
	{
		Con_Printf("ABP2P: Failed to create P2P server\n");
		return;
	}

	Con_Printf("ABP2P: P2P server created, accepting connections...\n");

	server_running_ = true;
	server_accept_thread_ = std::thread([this]() { ServerAcceptThreadBody(); });
}

void AB_P2P::StopServer()
{
	server_running_ = false;

	if (server_accept_thread_.joinable())
		server_accept_thread_.join();

	{
		std::lock_guard<std::mutex> lock(p2p_mutex_);
		for (auto& proxy : host_proxies_)
		{
			proxy->running = false;
			if (proxy->proxy_thread.joinable())
				proxy->proxy_thread.join();
		}
		host_proxies_.clear();
	}

	p2p_server_.reset();
}

//------------------------------------------------------------------------------
// Client: proxy thread
//------------------------------------------------------------------------------
void AB_P2P::ClientProxyThreadBody()
{
	char udp_buf[NET_MAXMESSAGE];
	struct sockaddr_in client_addr;
	bool have_client_addr = false;
	StreamBuffer stream;

	SOCKET sock      = p2p_sock(client_proxy_socket_);
	int    proxy_port = client_proxy_port_.load();

	if (task_runner_)
		task_runner_->queue_task([proxy_port]() {
			Con_Printf("ABP2P: Client proxy thread started on port %d\n", proxy_port);
		});

	while (client_running_)
	{
		bool did_work = false;

		// P2P -> UDP: read from host, extract framed messages, forward to Quake client
		accelbyte::Vector<char> p2p_data;
		auto err = client_connection_->read(p2p_data);
		if (err.type() == accelbyte::Error::ok && p2p_data.size() > 0)
		{
			stream.append(p2p_data.data(), p2p_data.size());
			int msg_len;
			while ((msg_len = stream.extract(udp_buf, sizeof(udp_buf))) > 0)
			{
				// Intercept CCREP_ACCEPT: rewrite advertised port to our proxy port
				if (msg_len >= 9)
				{
					int32_t header    = read_big_long(udp_buf);
					int     pkt_flags = header & 0xffff0000;
					int     pkt_len   = header & 0x0000ffff;

					if ((pkt_flags & NETFLAG_CTL) && (unsigned char)udp_buf[4] == CCREP_ACCEPT && pkt_len >= 9)
					{
						int32_t advertised_port = read_little_long(udp_buf + 5);
						if (task_runner_)
							task_runner_->queue_task([advertised_port, proxy_port]() {
								Con_Printf("ABP2P: Client proxy intercepted CCREP_ACCEPT: host advertised port %d, rewriting to proxy port %d\n",
									advertised_port, proxy_port);
							});
						write_little_long(udp_buf + 5, proxy_port);
					}
				}

				if (have_client_addr)
				{
					sendto(sock, udp_buf, msg_len, 0,
						(struct sockaddr*)&client_addr, sizeof(client_addr));
				}
			}
			did_work = true;
		}
		else if (!client_connection_->is_connected())
		{
			break; // remote side dropped — exit proxy loop
		}

		// UDP -> P2P: read from Quake client, frame and forward to host
		struct sockaddr_in from_addr;
		socklen_t from_len = sizeof(from_addr);
		int recv_len = recvfrom(sock, udp_buf, sizeof(udp_buf), 0,
			(struct sockaddr*)&from_addr, &from_len);
		if (recv_len > 0)
		{
			if (!have_client_addr)
			{
				client_addr     = from_addr;
				have_client_addr = true;
			}

			p2p_send_framed(client_connection_, udp_buf, recv_len);
			did_work = true;
		}

		if (!did_work)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	if (task_runner_)
		task_runner_->queue_task([]() {
			Con_Printf("ABP2P: Client proxy thread stopped\n");
		});
}

void AB_P2P::ConnectToHost(const char* peer_id)
{
	if (client_running_)
	{
		Con_Printf("ABP2P: Client already connecting/connected\n");
		return;
	}

	// Join and release any previous connection that ended naturally (remote disconnect).
	// This must happen here — we cannot rely on Update() having run since the last
	// disconnect.  Assigning to a joinable std::thread calls std::terminate(), so this
	// join is safety-critical, not just a cleanup nicety.
	if (client_connect_thread_.joinable())
		client_connect_thread_.join();

	{
		SOCKET old_sock = p2p_sock(client_proxy_socket_);
		if (old_sock != P2P_INVALID_SOCKET)
		{
			p2p_closesocket(old_sock);
			client_proxy_socket_ = ~uintptr_t(0);
		}
	}
	client_proxy_port_ = 0;
	client_connection_.reset();

	accelbyte::memory::SharedPtr<accelbyte::user::User>             user;
	accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> lobby_conn;
	{
		std::lock_guard<std::mutex> lock(ctx_mutex_);
		user      = current_user_;
		lobby_conn = lobby_conn_;
	}

	if (!user || !lobby_conn)
	{
		Con_Printf("ABP2P: Cannot connect — not logged in or no lobby connection\n");
		return;
	}

	int    proxy_port = 0;
	SOCKET sock       = create_local_udp_socket(local_addr_, &proxy_port);
	if (sock == P2P_INVALID_SOCKET)
	{
		Con_Printf("ABP2P: Failed to create client proxy socket\n");
		return;
	}

	client_proxy_socket_ = (uintptr_t)sock;
	client_proxy_port_   = proxy_port;

	std::string peer_id_str(peer_id);
	Con_Printf("ABP2P: Connecting to host %s via P2P (proxy port %d)...\n", peer_id, proxy_port);

	client_running_   = true;
	client_connected_ = false;

	// Spawn connection thread — ICE negotiation is synchronous, may take a few seconds.
	// On success, the proxy loop runs on this same thread.
	client_connect_thread_ = std::thread([this, peer_id_str, user, lobby_conn]() {
		auto user_ptr = std::shared_ptr<accelbyte::user::User>(
			user.get(), [user](accelbyte::user::User*) { (void)user; });
		auto lobby_ptr = std::shared_ptr<accelbyte::lobby::LobbyConnection>(
			lobby_conn.get(), [lobby_conn](accelbyte::lobby::LobbyConnection*) { (void)lobby_conn; });

		client_connection_ = accelbyte::p2p_connection::ClientConnectionFactory::create(
			peer_id_str.c_str(), user_ptr, lobby_ptr, /*force_relay=*/true);

		if (!client_connection_)
		{
			if (task_runner_)
				task_runner_->queue_task([]() {
					Con_Printf("ABP2P: Failed to create P2P connection\n");
				});
			client_running_ = false;
			return;
		}

		auto err = client_connection_->connect();
		if (err.type() != accelbyte::Error::ok)
		{
			std::string err_msg = err.what().c_str();
			if (task_runner_)
				task_runner_->queue_task([err_msg]() {
					Con_Printf("ABP2P: P2P connection failed: %s\n", err_msg.c_str());
				});
			client_connection_.reset();
			client_running_ = false;
			return;
		}

		bool using_relay = client_connection_->is_using_relay();
		int  port        = client_proxy_port_.load();

		if (task_runner_)
		{
			// Capture local_addr_ by value — this lambda may outlive AB_P2P if it
			// sits in the task runner queue when the instance is being destroyed.
			unsigned long addr_copy = local_addr_;
			task_runner_->queue_task([using_relay, port, addr_copy]() {
				Con_Printf("ABP2P: P2P connection established! (relay: %s)\n",
					using_relay ? "yes" : "no");

				struct in_addr a;
				a.s_addr = addr_copy;
				const char* ip = inet_ntoa(a);
				Con_Printf("ABP2P: Connecting Quake client to %s:%d\n", ip, port);

				Cbuf_AddText(va("connect \"%s:%d\"\n", ip, port));
				key_dest = key_game;
				m_state  = m_none;
			});
		}

		client_connected_ = true;
		ClientProxyThreadBody();

		// Proxy exited (remote disconnect or explicit Disconnect()).
		// Clear flags so Update() can join and ConnectToHost() can proceed.
		client_running_   = false;
		client_connected_ = false;
	});
}

void AB_P2P::Disconnect()
{
	client_running_   = false;
	client_connected_ = false;

	if (client_connect_thread_.joinable())
		client_connect_thread_.join();

	SOCKET sock = p2p_sock(client_proxy_socket_);
	if (sock != P2P_INVALID_SOCKET)
	{
		p2p_closesocket(sock);
		client_proxy_socket_ = ~uintptr_t(0);
	}

	client_proxy_port_ = 0;
	client_connection_.reset();
}

int AB_P2P::GetClientProxyPort() const
{
	return client_proxy_port_.load();
}

int AB_P2P::IsClientConnected() const
{
	return client_connected_ ? 1 : 0;
}
