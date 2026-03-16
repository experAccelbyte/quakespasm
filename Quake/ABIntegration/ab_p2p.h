/*
 * AccelByte P2P Connection Layer for QuakeSpasm
 *
 * Provides P2P connectivity using AccelByte's ICE/TURN infrastructure.
 * The host creates a P2PServer and accepts incoming connections.
 * Clients connect via ClientConnectionFactory using the host's user ID.
 * All Quake UDP traffic is proxied through local loopback sockets
 * that bridge to the P2P data channel.
 */

#pragma once

#include <accelbyte/user/User.h>
#include <accelbyte/lobby/LobbyConnection.h>
#include <accelbyte/p2p_connection/P2PServer.h>
#include <accelbyte/p2p_connection/Connection.h>
#include "ab_task_runner.h"

#include <cstdint>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <string>

class AB_P2P {
public:
    // Per-client proxy state on the host side.
    // Uses uintptr_t for the UDP socket handle to avoid pulling
    // platform socket headers (<winsock2.h> / <sys/socket.h>) into this header.
    // Sentinel for "no socket": ~uintptr_t(0)  (== INVALID_SOCKET on all platforms).
    struct HostClientProxy {
        std::shared_ptr<accelbyte::p2p_connection::Connection> connection;
        std::thread       proxy_thread;
        std::atomic<bool> running{false};
        uintptr_t         udp_socket{~uintptr_t(0)};
        std::string       peer_id;
    };

    AB_P2P();
    ~AB_P2P();

    AB_P2P(const AB_P2P&) = delete;
    AB_P2P& operator=(const AB_P2P&) = delete;

    void SetTaskRunner(ABTaskRunner& tr);

    // Call after a successful login with the user and lobby connection.
    // StartServer / ConnectToHost will use these without needing them as parameters.
    void SetLobbyContext(
        accelbyte::memory::SharedPtr<accelbyte::user::User>             user,
        accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> lobby_conn);

    // Lifecycle — called from ABInstance (wired in ab_create / ab_destroy / ab_update).
    void Init();
    void Shutdown();
    void Update();

    // Host path: start / stop listening for incoming P2P connections.
    void StartServer();
    void StopServer();

    // Client path: connect to the session leader by AccelByte user ID.
    void ConnectToHost(const char* peer_id);
    void Disconnect();

    int GetClientProxyPort() const;
    int IsClientConnected()  const;

private:
    void ResolveLocalAddr();
    void ServerAcceptThreadBody();
    void HostProxyThreadBody(HostClientProxy* proxy);
    void ClientProxyThreadBody();

    // Lobby context (set once after login, read by Start/Connect)
    mutable std::mutex                                              ctx_mutex_;
    accelbyte::memory::SharedPtr<accelbyte::user::User>             current_user_;
    accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> lobby_conn_;

    // Guards host_proxies_ and p2p_server_
    std::mutex    p2p_mutex_;
    unsigned long local_addr_{0}; // network-byte-order local IP for proxy sockets

    // Host state
    std::shared_ptr<accelbyte::p2p_connection::P2PServer> p2p_server_;
    std::atomic<bool>  server_running_{false};
    std::thread        server_accept_thread_;
    std::vector<std::unique_ptr<HostClientProxy>> host_proxies_;

    // Client state
    std::shared_ptr<accelbyte::p2p_connection::Connection> client_connection_;
    std::thread        client_connect_thread_;
    std::atomic<bool>  client_running_{false};
    std::atomic<bool>  client_connected_{false};
    uintptr_t          client_proxy_socket_{~uintptr_t(0)};
    std::atomic<int>   client_proxy_port_{0};

    ABTaskRunner* task_runner_ = nullptr;
};
