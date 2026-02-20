/*
 * AMS Watchdog Protocol for QuakeSpasm Dedicated Server
 * Implements the AccelByte AMS watchdog WebSocket protocol so the DS
 * can signal readiness and receive drain notifications.
 */

#ifndef AMS_H
#define AMS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize AMS watchdog connection.
 * Reads WATCHDOG_URL and DS_ID from environment variables (or command line).
 * Connects the WebSocket and starts sending heartbeats.
 * Call once during Host_Init() for dedicated servers.
 */
void AMS_Init(void);

/*
 * Shutdown AMS watchdog connection.
 * Disconnects the WebSocket cleanly.
 * Call during Host_Shutdown().
 */
void AMS_Shutdown(void);

/*
 * Per-frame update. Sends heartbeats and polls for incoming messages.
 * Call each frame in _Host_Frame().
 */
void AMS_Update(void);

/*
 * Signal to AMS that this dedicated server is ready to accept players.
 * Sends the "ready" message over the watchdog WebSocket.
 */
void AMS_Ready(void);

/*
 * Returns non-zero if a drain message has been received from AMS.
 * The dedicated server should finish current sessions and shut down.
 */
int AMS_IsDraining(void);

#ifdef __cplusplus
}
#endif

#endif /* AMS_H */
