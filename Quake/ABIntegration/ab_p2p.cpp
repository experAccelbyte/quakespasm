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
// Utility: create a non-blocking UDP socket bound to 127.0.0.1 on an ephemeral port
//------------------------------------------------------------------------------
static SOCKET create_local_udp_socket(int* out_port)
{
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
		return INVALID_SOCKET;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
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
// Host: proxy thread for a single P2P client
// Bridges P2P read/write <-> UDP to Quake's listen server port
//------------------------------------------------------------------------------
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

	// Target: Quake listen server on localhost
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_addr.sin_port = htons((u_short)QUAKE_SERVER_PORT);

	runner.queue_task([peer_id = proxy->peer_id, local_port](){
		Con_Printf("ABP2P: Host proxy for peer %s on local port %d\n", peer_id.c_str(), local_port);
	});

	char udp_buf[NET_MAXMESSAGE];

	while (proxy->running)
	{
		bool did_work = false;

		// P2P -> UDP: read from P2P connection, send to Quake server
		accelbyte::Vector<char> p2p_data;
		auto err = proxy->connection->read(p2p_data);
		if (err.type() == accelbyte::Error::ok && p2p_data.size() > 0)
		{
			sendto(proxy->udp_socket, p2p_data.data(), (int)p2p_data.size(), 0,
				(struct sockaddr*)&server_addr, sizeof(server_addr));
			did_work = true;
		}

		// UDP -> P2P: read from Quake server response, write to P2P connection
		struct sockaddr_in from_addr;
		socklen_t from_len = sizeof(from_addr);
		int recv_len = recvfrom(proxy->udp_socket, udp_buf, sizeof(udp_buf), 0,
			(struct sockaddr*)&from_addr, &from_len);
		if (recv_len > 0)
		{
			proxy->connection->write(udp_buf, (size_t)recv_len);
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

	runner.queue_task([port = s_client_proxy_port.load()](){
		Con_Printf("ABP2P: Client proxy thread started on port %d\n", port);
	});

	while (s_client_running)
	{
		bool did_work = false;

		// P2P -> UDP: read from P2P connection (from host), forward to Quake client
		accelbyte::Vector<char> p2p_data;
		auto err = s_client_connection->read(p2p_data);
		if (err.type() == accelbyte::Error::ok && p2p_data.size() > 0 && have_client_addr)
		{
			sendto(s_client_proxy_socket, p2p_data.data(), (int)p2p_data.size(), 0,
				(struct sockaddr*)&client_addr, sizeof(client_addr));
			did_work = true;
		}

		// UDP -> P2P: read from Quake client, forward to host via P2P
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

			s_client_connection->write(udp_buf, (size_t)recv_len);
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
	// Nothing to do yet — state is initialized statically
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
			Con_Printf("ABP2P: Connecting Quake client to 127.0.0.1:%d\n", port);

			// Tell Quake to connect through our proxy
			Cbuf_AddText(va("connect \"127.0.0.1:%d\"\n", port));
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
