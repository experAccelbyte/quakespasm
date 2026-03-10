/*
 * AccelByte P2P Connection Layer for QuakeSpasm
 *
 * Provides P2P connectivity using AccelByte's ICE/TURN infrastructure.
 * The host creates a P2PServer and accepts incoming connections.
 * Clients connect via ClientConnectionFactory using the host's user ID.
 * All Quake UDP traffic is proxied through local loopback sockets
 * that bridge to the P2P data channel.
 */

#ifndef AB_P2P_H
#define AB_P2P_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize P2P subsystem. Called once during AB_Init().
 */
void ABP2P_Init(void);

/*
 * Shutdown P2P subsystem and clean up all connections/threads.
 */
void ABP2P_Shutdown(void);

/*
 * Poll P2P state — accept new connections on host, check connection status, etc.
 * Called each frame from AB_Update().
 */
void ABP2P_Update(void);

/*
 * Host: create P2P server and start accepting connections.
 * Called after the listen server is up.
 */
void ABP2P_StartServer(void);

/*
 * Host: stop P2P server and disconnect all P2P clients.
 */
void ABP2P_StopServer(void);

/*
 * Client: initiate P2P connection to the session leader.
 * peer_id is the AccelByte user ID of the host.
 * On success, returns the local proxy port to connect to via "connect 127.0.0.1:<port>".
 * Returns 0 on failure.
 */
void ABP2P_ConnectToHost(const char* peer_id);

/*
 * Client: disconnect from P2P host.
 */
void ABP2P_Disconnect(void);

/*
 * Returns the local proxy port for the client to connect to.
 * Returns 0 if not connected yet.
 */
int ABP2P_GetClientProxyPort(void);

/*
 * Returns 1 if the client P2P connection is established, 0 otherwise.
 */
int ABP2P_IsClientConnected(void);

#ifdef __cplusplus
}
#endif

#endif /* AB_P2P_H */
