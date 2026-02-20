/*
 * AMS Watchdog Protocol for QuakeSpasm Dedicated Server
 *
 * Implements the AccelByte Multiplayer Servers (AMS) watchdog protocol.
 * The dedicated server connects to the local watchdog via WebSocket,
 * sends heartbeats every 15 seconds, signals readiness, and handles
 * drain notifications for graceful shutdown.
 *
 * Uses easywsclient for the WebSocket connection (no AccelByte SDK dependency).
 *
 * Protocol reference:
 *   https://docs.accelbyte.io/gaming-services/modules/multiplayer/
 *   multiplayer-servers/ams-watchdog-protocol/
 */

#include "ams.h"
#include "easywsclient.hpp"

// Standard library
#include <string>
#include <map>
#include <cstdlib>

// Quake headers (C linkage)
extern "C" {
#include "quakedef.h"
}

extern "C" {
	extern double Sys_DoubleTime(void);
	extern int COM_CheckParm(const char *parm);
}

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
static const double AMS_HEARTBEAT_INTERVAL = 15.0; // seconds
static const char *AMS_DEFAULT_WATCHDOG_URL = "ws://127.0.0.1:5555/watchdog";

//------------------------------------------------------------------------------
// Internal state
//------------------------------------------------------------------------------
static easywsclient::WebSocket::pointer g_ws = nullptr;
static std::string g_dsid;
static std::string g_watchdog_url;
static bool g_ams_initialized = false;
static bool g_ams_draining = false;
static bool g_ams_ready_sent = false;
static double g_last_heartbeat_time = 0.0;

//------------------------------------------------------------------------------
// Helper: process incoming messages (look for "drain")
//------------------------------------------------------------------------------
static void AMS_ProcessMessage(const std::string& msg)
{
	if (msg.find("\"drain\"") != std::string::npos)
	{
		if (!g_ams_draining)
		{
			g_ams_draining = true;
			Con_Printf("AMS: Drain received — server will shut down after current sessions\n");
		}
	}
}

//------------------------------------------------------------------------------
// Public C API
//------------------------------------------------------------------------------
extern "C" {

void AMS_Init(void)
{
	if (g_ams_initialized)
		return;

	// Get DS_ID from environment or command line
	// Command line: -dsid <id>
	int dsid_arg = COM_CheckParm("-dsid");
	if (dsid_arg && dsid_arg + 1 < com_argc)
	{
		g_dsid = com_argv[dsid_arg + 1];
	}
	else
	{
		// Fall back to environment variable
		const char *env_dsid = getenv("DS_ID");
		if (env_dsid && env_dsid[0])
			g_dsid = env_dsid;
	}

	if (g_dsid.empty())
	{
		Con_Printf("AMS: No DS_ID found (set DS_ID env var or use -dsid). Watchdog disabled.\n");
		return;
	}

	// Get watchdog URL from environment or use default
	const char *env_url = getenv("WATCHDOG_URL");
	if (env_url && env_url[0])
		g_watchdog_url = env_url;
	else
		g_watchdog_url = AMS_DEFAULT_WATCHDOG_URL;

	Con_Printf("AMS: DS_ID = %s\n", g_dsid.c_str());
	Con_Printf("AMS: Watchdog URL = %s\n", g_watchdog_url.c_str());

	// Connect with ams-dsid header
	std::map<std::string, std::string> headers;
	headers["ams-dsid"] = g_dsid;

	g_ws = easywsclient::WebSocket::from_url(g_watchdog_url, headers);
	if (!g_ws || g_ws->getReadyState() == easywsclient::WebSocket::CLOSED)
	{
		Con_Printf("AMS: Failed to connect to watchdog. Continuing without AMS.\n");
		if (g_ws)
		{
			delete g_ws;
			g_ws = nullptr;
		}
		return;
	}

	g_ams_initialized = true;
	g_ams_draining = false;
	g_ams_ready_sent = false;
	g_last_heartbeat_time = Sys_DoubleTime();

	Con_Printf("AMS: Connected to watchdog\n");

	// Send initial heartbeat
	g_ws->send("{\"heartbeat\":{}}");
}

void AMS_Shutdown(void)
{
	if (!g_ams_initialized)
		return;

	if (g_ws)
	{
		g_ws->close();
		// Poll once to flush the close frame
		g_ws->poll();
		delete g_ws;
		g_ws = nullptr;
		Con_Printf("AMS: Disconnected from watchdog\n");
	}

	g_ams_initialized = false;
	g_ams_draining = false;
	g_ams_ready_sent = false;
	g_dsid.clear();
	g_watchdog_url.clear();
}

void AMS_Update(void)
{
	if (!g_ams_initialized || !g_ws)
		return;

	if (g_ws->getReadyState() == easywsclient::WebSocket::CLOSED)
	{
		Con_Printf("AMS: Lost connection to watchdog\n");
		g_ams_initialized = false;
		delete g_ws;
		g_ws = nullptr;
		return;
	}

	// Auto-send ready once the server is active and accepting connections
	if (!g_ams_ready_sent && sv.active)
		AMS_Ready();

	// Poll for incoming messages (non-blocking)
	g_ws->poll(0);
	g_ws->dispatch(AMS_ProcessMessage);

	// Send heartbeat every 15 seconds
	double now = Sys_DoubleTime();
	if (now - g_last_heartbeat_time >= AMS_HEARTBEAT_INTERVAL)
	{
		g_ws->send("{\"heartbeat\":{}}");
		g_last_heartbeat_time = now;
	}
}

void AMS_Ready(void)
{
	if (!g_ams_initialized || !g_ws)
	{
		Con_Printf("AMS: Cannot send ready — not connected to watchdog\n");
		return;
	}

	if (g_ams_ready_sent)
	{
		Con_Printf("AMS: Ready already sent\n");
		return;
	}

	std::string msg = "{\"ready\":{\"dsid\":\"" + g_dsid + "\"}}";
	g_ws->send(msg);
	g_ams_ready_sent = true;
	Con_Printf("AMS: Ready signal sent\n");
}

int AMS_IsDraining(void)
{
	return g_ams_draining ? 1 : 0;
}

} // extern "C"
