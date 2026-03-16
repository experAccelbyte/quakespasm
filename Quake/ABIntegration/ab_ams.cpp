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

#include "ab_ams.h"

#include <cstdlib>
#include <map>

// Quake headers
extern "C" {
#include "quakedef.h"
extern double Sys_DoubleTime(void);
extern int COM_CheckParm(const char* parm);
}

static const double AMS_HEARTBEAT_INTERVAL = 15.0;
static const char* AMS_DEFAULT_WATCHDOG_URL = "ws://172.27.160.1:5555/watchdog";

AB_AMS::AB_AMS()
    : ws_(nullptr)
    , initialized_(false)
    , draining_(false)
    , ready_sent_(false)
    , last_heartbeat_time_(0.0)
{
}

AB_AMS::~AB_AMS()
{
    Shutdown();
}

void AB_AMS::Init()
{
    if (initialized_)
        return;

    // Get DS_ID from command line (-dsid <id>) or environment variable DS_ID
    int dsid_arg = COM_CheckParm("-dsid");
    if (dsid_arg && dsid_arg + 1 < com_argc) {
        dsid_ = com_argv[dsid_arg + 1];
    }
    else {
        const char* env_dsid = getenv("DS_ID");
        if (env_dsid && env_dsid[0])
            dsid_ = env_dsid;
    }

    if (dsid_.empty()) {
        // No DS_ID — not running under AMS. Normal for client builds and
        // servers not managed by AccelByte AMS.
        return;
    }

    const char* env_url = getenv("WATCHDOG_URL");
    if (env_url && env_url[0])
        watchdog_url_ = env_url;
    else
        watchdog_url_ = AMS_DEFAULT_WATCHDOG_URL;

    Con_Printf("AMS: DS_ID = %s\n", dsid_.c_str());
    Con_Printf("AMS: Watchdog URL = %s\n", watchdog_url_.c_str());

    std::map<std::string, std::string> headers;
    headers["ams-dsid"] = dsid_;

    ws_ = easywsclient::WebSocket::from_url(watchdog_url_, headers);
    if (!ws_ || ws_->getReadyState() == easywsclient::WebSocket::CLOSED) {
        Con_Printf("AMS: Failed to connect to watchdog. Continuing without AMS.\n");
        if (ws_) {
            delete ws_;
            ws_ = nullptr;
        }
        return;
    }

    initialized_ = true;
    draining_ = false;
    ready_sent_ = false;
    last_heartbeat_time_ = Sys_DoubleTime();

    Con_Printf("AMS: Connected to watchdog\n");

    ws_->send("{\"heartbeat\":{}}");
}

void AB_AMS::Shutdown()
{
    if (!initialized_)
        return;

    if (ws_) {
        ws_->close();
        ws_->poll(); // flush the close frame
        delete ws_;
        ws_ = nullptr;
        Con_Printf("AMS: Disconnected from watchdog\n");
    }

    initialized_ = false;
    draining_ = false;
    ready_sent_ = false;
    dsid_.clear();
    watchdog_url_.clear();
}

void AB_AMS::Update()
{
    if (!initialized_ || !ws_)
        return;

    if (ws_->getReadyState() == easywsclient::WebSocket::CLOSED) {
        Con_Printf("AMS: Lost connection to watchdog\n");
        initialized_ = false;
        delete ws_;
        ws_ = nullptr;
        return;
    }

    // Auto-send ready once the server is active and accepting connections
    if (!ready_sent_ && sv.active)
        Ready();

    ws_->poll(0);
    ws_->dispatch([this](const std::string& msg) { ProcessMessage(msg); });

    double now = Sys_DoubleTime();
    if (now - last_heartbeat_time_ >= AMS_HEARTBEAT_INTERVAL) {
        ws_->send("{\"heartbeat\":{}}");
        last_heartbeat_time_ = now;
    }
}

void AB_AMS::Ready()
{
    if (!initialized_ || !ws_) {
        Con_Printf("AMS: Cannot send ready — not connected to watchdog\n");
        return;
    }

    if (ready_sent_) {
        Con_Printf("AMS: Ready already sent\n");
        return;
    }

    std::string msg = "{\"ready\":{\"dsid\":\"" + dsid_ + "\"}}";
    ws_->send(msg);
    ready_sent_ = true;
    Con_Printf("AMS: Ready signal sent\n");
}

bool AB_AMS::IsDraining() const
{
    return draining_;
}

void AB_AMS::ProcessMessage(const std::string& msg)
{
    if (msg.find("\"drain\"") != std::string::npos) {
        if (!draining_) {
            draining_ = true;
            Con_Printf("AMS: Drain received — server will shut down when all players leave\n");

            if (net_activeconnections == 0) {
                Con_Printf("AMS: No players connected, shutting down now\n");
                Cbuf_AddText("quit\n");
            }
        }
    }
}
