/*
 * AccelByte SDK Integration for QuakeSpasm
 * C++ implementation
 */

#include "ab_integration.h"

#ifdef DEBUG
#undef DEBUG
#endif

// AccelByte SDK headers
#include <accelbyte/common/String.h>
#include <accelbyte/user/UserLogin.h>
#include <accelbyte/user/User.h>
#include <accelbyte/user/parameters/LoginWithDeviceId.h>
#include <accelbyte/settings/InMemorySettings.h>
#include <accelbyte/settings/global_settings.h>
#include <accelbyte/common/Error.h>
#include <accelbyte/iam/models/LoginQueueTicket.h>
#include <accelbyte/curl_http_executor/CurlRequestExecutorFactory.h>
#include <accelbyte/http/RequestExecutorFactory.h>
#include <accelbyte/social/UserStatistic.h>
#include <accelbyte/social/user_statistic/UpdateUserStatItemValueV2.h>
#include <accelbyte/social/models/UpdateStatItem.h>

// Lobby and WebSocket headers
#include <accelbyte/lobby/Lobby.h>
#include <accelbyte/lobby/LobbyConnection.h>
#include <accelbyte/cpp_web_socket/CppWebSocketFactory.h>
#include <accelbyte/web_socket/WebSocketFactory.h>

// Session headers
#include <accelbyte/session/SessionService.h>
#include <accelbyte/session/GameSession.h>

// Match2 headers
#include <accelbyte/match2/Match2Service.h>
#include <accelbyte/match2/MatchTickets.h>

// Lobby notification headers
#include <accelbyte/lobby/TypedMessageHandler.h>
#include <accelbyte/lobby/notifications/OnMatchFound.h>
#include <accelbyte/lobby/notifications/OnGameSessionUpdated.h>
#include "ab_task_runner.h"

// Standard library
#include <string>
#include <mutex>
#include <future>

// Quake headers (C linkage)
extern "C" {
#include "quakedef.h"
#include "console.h"
}

// Forward declarations for Quake C functions
extern "C" {
    extern cvar_t* Cvar_FindVar(const char* var_name);
    extern void Cvar_RegisterVariable(cvar_t* variable);
    extern double Sys_DoubleTime(void);
    extern void Cbuf_AddText(const char* text);
    extern char my_tcpip_address[];
    extern int net_hostport;
}

// C++ forward declarations
static void AB_JoinSession(void);
static void AB_StartHosting(void);
static void AB_PatchSessionWithHostIP(void);

//------------------------------------------------------------------------------
// CVars for AccelByte configuration
//------------------------------------------------------------------------------
static cvar_t ab_server_url = {"ab_server_url", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_id = {"ab_client_id", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_secret = {"ab_client_secret", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_match_pool = {"ab_match_pool", "quake_ffa", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_match_map = {"ab_match_map", "start", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};

//------------------------------------------------------------------------------
// Internal state
//------------------------------------------------------------------------------
static std::mutex g_mutex;
static ab_login_status_t g_login_status = AB_LOGIN_IDLE;
static std::string g_user_id;
static std::string g_display_name;
static std::string g_error_message;
static std::string g_device_id;
static bool g_initialized = false;

// Login queue handling
static accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> g_queue_ticket;
static double g_last_queue_poll = 0.0;

// User credential storage
static accelbyte::memory::SharedPtr<accelbyte::user::User> g_current_user;

// Settings instance
static accelbyte::settings::InMemorySettings g_settings;

// Matchmaking state
static ab_matchmake_status_t g_matchmake_status = AB_MM_IDLE;
static std::string g_match_ticket_id;
static std::string g_matchmake_error;
static std::string g_match_id;
static std::string g_match_pool_name;
static int g_match_num_players = 0;
static int g_match_num_teams = 0;

// Session state
static std::string g_session_id;
static std::string g_session_leader_id;
static bool g_is_session_leader = false;
static std::string g_host_address;
static int g_session_version = 0;

// Lobby state
static std::shared_ptr<accelbyte::lobby::Lobby> g_lobby;
static accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> g_lobby_connection;

// Async futures storage
static std::future<void> g_matchmake_future;

static ABTaskRunner runner;

//------------------------------------------------------------------------------
// MatchFoundHandler — receives OnMatchFound from lobby WebSocket
//------------------------------------------------------------------------------
class MatchFoundHandler : public accelbyte::lobby::TypedMessageHandler<accelbyte::lobby::notifications::OnMatchFound>
{
public:
    void handle(const accelbyte::lobby::notifications::OnMatchFound& message) override
    {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_matchmake_status = AB_MM_FOUND;
        g_match_id = message.match_id.c_str();
        g_match_pool_name = message.match_pool.c_str();

        // Count teams and total players across all teams
        g_match_num_teams = (int)message.teams.size();
        g_match_num_players = 0;
        for (size_t i = 0; i < message.teams.size(); i++)
        {
            g_match_num_players += (int)message.teams[i].size();
        }

        int num_players = g_match_num_players;
        int num_teams = g_match_num_teams;
        std::string match_id = g_match_id;

        runner.queue_task([match_id, num_players, num_teams](){
            Con_Printf("AccelByte: Match found! ID: %s, Players: %d, Teams: %d\n",
                match_id.c_str(), num_players, num_teams);
            AB_JoinSession();
        });
    }
};

static std::shared_ptr<MatchFoundHandler> g_match_found_handler;

//------------------------------------------------------------------------------
// GameSessionUpdatedHandler — receives session updates (client path)
//------------------------------------------------------------------------------
class GameSessionUpdatedHandler : public accelbyte::lobby::TypedMessageHandler<accelbyte::lobby::notifications::OnGameSessionUpdated>
{
public:
    void handle(const accelbyte::lobby::notifications::OnGameSessionUpdated& message) override
    {
        std::lock_guard<std::mutex> lock(g_mutex);

        // Only care about updates when we're waiting for the host IP
        if (g_matchmake_status != AB_MM_JOINED_AS_CLIENT)
            return;

        // Parse host_ip from the attributes JSON
        accelbyte::String ab_attrs = message.attributes;
        std::string attrs(ab_attrs.c_str());
        std::string host_ip;

        // Simple JSON parse: find "host_ip":"<value>"
        size_t pos = attrs.find("\"host_ip\"");
        if (pos != std::string::npos)
        {
            pos = attrs.find(':', pos);
            if (pos != std::string::npos)
            {
                size_t start = attrs.find('"', pos + 1);
                if (start != std::string::npos)
                {
                    size_t end = attrs.find('"', start + 1);
                    if (end != std::string::npos)
                    {
                        host_ip = attrs.substr(start + 1, end - start - 1);
                    }
                }
            }
        }

        if (!host_ip.empty())
        {
            g_host_address = host_ip;
            g_matchmake_status = AB_MM_CONNECTING;

            runner.queue_task([host_ip](){
                Con_Printf("AccelByte: Host IP received: %s, connecting...\n", host_ip.c_str());
                Cbuf_AddText(va("connect \"%s\"\n", host_ip.c_str()));
                key_dest = key_game;
                m_state = m_none;
            });
        }
    }
};

static std::shared_ptr<GameSessionUpdatedHandler> g_session_updated_handler;
//------------------------------------------------------------------------------
// Device ID generation (Windows)
//------------------------------------------------------------------------------
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <accelbyte/crypto/md5.h>

static std::string GenerateDeviceId()
{
    std::string combined;

    // Get machine GUID from registry
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
    {
        char guid[256] = {0};
        DWORD size = sizeof(guid);
        if (RegQueryValueExA(hKey, "MachineGuid", NULL, NULL, (LPBYTE)guid, &size) == ERROR_SUCCESS)
        {
            combined += guid;
        }
        RegCloseKey(hKey);
    }

    // Get volume serial number of C:
    DWORD serial = 0;
    if (GetVolumeInformationA("C:\\", NULL, 0, &serial, NULL, NULL, NULL, 0))
    {
        combined += std::to_string(serial);
    }

    // If we couldn't get any system info, use a fallback
    if (combined.empty())
    {
        combined = "quakespasm-default-device";
    }

    // Hash with MD5 for a consistent format
    accelbyte::String ab_combined(combined.c_str());
    return accelbyte::crypto::md5(ab_combined).c_str();
}
#else
// Unix/Linux implementation
#include <fstream>
#include <accelbyte/crypto/md5.h>

static std::string GenerateDeviceId()
{
    std::string machine_id;

    // Try to read /etc/machine-id (systemd)
    std::ifstream file("/etc/machine-id");
    if (file.is_open())
    {
        std::getline(file, machine_id);
        file.close();
    }

    // Fallback to /var/lib/dbus/machine-id
    if (machine_id.empty())
    {
        std::ifstream dbus_file("/var/lib/dbus/machine-id");
        if (dbus_file.is_open())
        {
            std::getline(dbus_file, machine_id);
            dbus_file.close();
        }
    }

    // If we couldn't get any system info, use a fallback
    if (machine_id.empty())
    {
        machine_id = "quakespasm-default-device";
    }

    // Hash with MD5 for a consistent format
    accelbyte::String ab_machine_id(machine_id.c_str());
    return accelbyte::crypto::md5(ab_machine_id).c_str();
}
#endif

//------------------------------------------------------------------------------
// Callback handlers
//------------------------------------------------------------------------------
static void OnLoginSuccess(const accelbyte::memory::SharedPtr<accelbyte::user::User> user)
{
    {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_current_user = user;
        g_user_id = user->user_id().c_str();
        g_display_name = user->display_name().c_str();
        g_login_status = AB_LOGIN_SUCCESS;
        g_queue_ticket = nullptr;
    }

    runner.queue_task([](const accelbyte::String& access_token){
        Con_Printf("AccelByte: Login successful! Token: %s\n", access_token.c_str());
    }, user->credential()->access_token().value());

    // Connect to lobby WebSocket
    try
    {
        auto lobby = std::make_shared<accelbyte::lobby::Lobby>();
        auto connection = lobby->create_connection(*user);

        bool connected = connection->connect();

        std::lock_guard<std::mutex> lock(g_mutex);
        if (connected)
        {
            g_lobby = lobby;
            g_lobby_connection = connection;

            // Register lobby message handlers
            g_match_found_handler = std::make_shared<MatchFoundHandler>();
            connection->add_message_handler(g_match_found_handler);

            g_session_updated_handler = std::make_shared<GameSessionUpdatedHandler>();
            connection->add_message_handler(g_session_updated_handler);

            runner.queue_task([](){
                Con_Printf("AccelByte: Connected to lobby successfully\n");
            });
        }
        else
        {
            runner.queue_task([](){
                Con_Printf("AccelByte: Failed to connect to lobby\n");
            });
        }
    }
    catch (const std::exception& e)
    {
        std::string err = e.what();
        runner.queue_task([err](){
            Con_Printf("AccelByte: Lobby connection error: %s\n", err.c_str());
        });
    }
}

static void OnLoginQueued(const accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> ticket)
{
    std::lock_guard<std::mutex> lock(g_mutex);

    g_login_status = AB_LOGIN_QUEUED;
    g_queue_ticket = ticket;
    g_last_queue_poll = Sys_DoubleTime();

    Con_Printf("AccelByte: In login queue...\n");
}

static void OnLoginError(const accelbyte::Error& error)
{
    std::lock_guard<std::mutex> lock(g_mutex);

    g_error_message = error.what().c_str();
    g_login_status = AB_LOGIN_FAILED;
    g_queue_ticket = nullptr;

    Con_Printf("AccelByte: Login failed - %s\n", g_error_message.c_str());
}

//------------------------------------------------------------------------------
// Public C API implementation
//------------------------------------------------------------------------------
extern "C" {

void AB_Init(void)
{
    if (g_initialized)
    {
        return;
    }

    // Register cvars
    Cvar_RegisterVariable(&ab_server_url);
    Cvar_RegisterVariable(&ab_client_id);
    Cvar_RegisterVariable(&ab_client_secret);
    Cvar_RegisterVariable(&ab_match_pool);
    Cvar_RegisterVariable(&ab_match_map);

    // Generate device ID
    g_device_id = GenerateDeviceId();

    Con_Printf("AccelByte: SDK initialized\n");
    Con_Printf("AccelByte: Device ID: %s\n", g_device_id.c_str());

    g_initialized = true;

    // CURL HTTP EXECUTOR
    auto curlExecutor = std::make_shared<accelbyte::http::CurlRequestExecutorFactory>();
    accelbyte::http::RequestExecutorFactory::set_executor_factory(curlExecutor);

    // WebSocket factory — needed for lobby connections
    auto wsFactory = accelbyte::memory::make_shared_ptr<accelbyte::cpp_web_socket::CppWebSocketFactory>();
    accelbyte::web_socket::WebSocketFactory::set_web_socket_factory(wsFactory);
    Con_Printf("AccelByte: WebSocket factory registered\n");
}

void AB_Shutdown(void)
{
    if (!g_initialized)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(g_mutex);

    // Disconnect lobby
    if (g_lobby_connection)
    {
        g_lobby_connection->disconnect();
        g_lobby_connection = nullptr;
        Con_Printf("AccelByte: Lobby disconnected\n");
    }
    g_lobby = nullptr;
    g_match_found_handler = nullptr;
    g_session_updated_handler = nullptr;

    // Reset match state
    g_matchmake_status = AB_MM_IDLE;
    g_match_ticket_id.clear();
    g_matchmake_error.clear();
    g_match_id.clear();
    g_match_pool_name.clear();
    g_match_num_players = 0;
    g_match_num_teams = 0;

    // Reset session state
    g_session_id.clear();
    g_session_leader_id.clear();
    g_is_session_leader = false;
    g_host_address.clear();
    g_session_version = 0;

    g_current_user = nullptr;
    g_queue_ticket = nullptr;
    g_login_status = AB_LOGIN_IDLE;
    g_user_id.clear();
    g_display_name.clear();
    g_error_message.clear();
    g_initialized = false;

    Con_Printf("AccelByte: SDK shutdown\n");
}

std::future<void> g_dummy_future;

void AB_LoginWithDeviceId(void)
{
    if (!g_initialized)
    {
        Con_Printf("AccelByte: SDK not initialized\n");
        return;
    }

    // Check if cvars are configured
    const char* server_url = ab_server_url.string;
    const char* client_id = ab_client_id.string;
    const char* client_secret = ab_client_secret.string;

    if (!server_url || !server_url[0])
    {
        Con_Printf("AccelByte: ab_server_url not configured\n");
        return;
    }
    if (!client_id || !client_id[0])
    {
        Con_Printf("AccelByte: ab_client_id not configured\n");
        return;
    }
    // client_secret is optional for game SDK clients using public client auth

    // Derive lobby URL from server URL: https:// -> wss://, http:// -> ws://
    std::string server_url_str(server_url);
    std::string lobby_url;
    if (server_url_str.find("https://") == 0)
        lobby_url = "wss://" + server_url_str.substr(8);
    else if (server_url_str.find("http://") == 0)
        lobby_url = "ws://" + server_url_str.substr(7);
    else
        lobby_url = server_url_str;

    // Strip trailing slash and append /lobby/
    if (!lobby_url.empty() && lobby_url.back() == '/')
        lobby_url.pop_back();
    lobby_url += "/lobby/";

    Con_Printf("AccelByte: Lobby URL: %s\n", lobby_url.c_str());

    // Configure settings
    g_settings.set_server_url(server_url);
    g_settings.set_client_id(client_id);
    // g_settings.set_client_secret(client_secret); // Game SDK clients use public client auth
    g_settings.set_lobby_url(lobby_url.c_str());

    // Set as global settings
    accelbyte::settings::set_global_settings(g_settings);

    // Initialize SessionService if not already done (needed later for joining sessions)
    if (!accelbyte::session::SessionService::initialized())
    {
        accelbyte::session::SessionService::initialize(server_url);
        Con_Printf("AccelByte: SessionService initialized\n");
    }

    // Initialize Match2Service for matchmaking
    if (!accelbyte::match2::Match2Service::initialized())
    {
        accelbyte::match2::Match2Service::initialize(server_url);
        Con_Printf("AccelByte: Match2Service initialized\n");
    }

    // Login with device ID
    Con_Printf("AccelByte: Logging in with device ID...\n");

    accelbyte::user::parameters::LoginWithDeviceId params;
    params.device_id = g_device_id.c_str();
    params.create_headless = true;

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_login_status = AB_LOGIN_IN_PROGRESS;
    }
    
    g_dummy_future = std::async(std::launch::async, [params](){
        accelbyte::user::UserLogin::login_with_device_id(
        params,
        OnLoginSuccess,
        OnLoginQueued,
        OnLoginError
    );
    });
}

void AB_Update(void)
{
    if (!g_initialized)
    {
        return;
    }

    // Poll lobby WebSocket — grab the connection pointer under the mutex,
    // then call read() outside the lock. Message handlers will need to
    // acquire the mutex themselves, so holding it during read() would deadlock.
    accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> conn;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        conn = g_lobby_connection;
    }

    if (conn)
    {
        try
        {
            conn->read();
        }
        catch (const std::exception& e)
        {
            (void)e; // Silently ignore read errors for now
        }
    }

    runner.execute_task_queue();
}

ab_login_status_t AB_GetLoginStatus(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_login_status;
}

const char* AB_GetUserId(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_login_status == AB_LOGIN_SUCCESS && !g_user_id.empty())
    {
        return g_user_id.c_str();
    }
    return NULL;
}

const char* AB_GetDisplayName(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_login_status == AB_LOGIN_SUCCESS && !g_display_name.empty())
    {
        return g_display_name.c_str();
    }
    return NULL;
}

const char* AB_GetErrorMessage(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_login_status == AB_LOGIN_FAILED && !g_error_message.empty())
    {
        return g_error_message.c_str();
    }
    return NULL;
}

void AB_UpdateUserStatItemValue(const char* stat_code, float value, int strategy)
{
    if (!g_initialized)
    {
        Con_Printf("AccelByte: SDK not initialized\n");
        return;
    }

    if (!stat_code || !stat_code[0])
    {
        Con_Printf("AccelByte: stat_code is empty\n");
        return;
    }

    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_login_status != AB_LOGIN_SUCCESS || !g_current_user)
    {
        Con_Printf("AccelByte: Not logged in, cannot update stat\n");
        return;
    }

    using UpdateStrategy = accelbyte::social::model::UpdateStatItem::UpdateStrategy;
    UpdateStrategy update_strategy;
    switch (strategy)
    {
    case 0:  update_strategy = UpdateStrategy::OVERRIDE;  break;
    case 1:  update_strategy = UpdateStrategy::INCREMENT;  break;
    case 2:  update_strategy = UpdateStrategy::MAX;        break;
    case 3:  update_strategy = UpdateStrategy::MIN;        break;
    default:
        Con_Printf("AccelByte: Invalid strategy %d (use 0=OVERRIDE, 1=INCREMENT, 2=MAX, 3=MIN)\n", strategy);
        return;
    }

    accelbyte::social::user_statistic::UpdateUserStatItemValueV2 request;
    request.stat_code = stat_code;
    request.user_id = g_user_id.c_str();
    request.body.update_strategy = update_strategy;
    request.body.value = value;

    const accelbyte::tls::SecurityAuthorization& authorization = *g_current_user;

    std::string stat_code_copy(stat_code);

    accelbyte::social::UserStatistic::update_user_stat_item_value_v2(
        authorization,
        request,
        [stat_code_copy](const accelbyte::social::model::StatItemInc& result) {
            Con_Printf("AccelByte: Stat '%s' updated, current value: %f\n",
                stat_code_copy.c_str(), result.current_value);
        },
        [stat_code_copy](const accelbyte::Error& error) {
            Con_Printf("AccelByte: Failed to update stat '%s' - %s\n",
                stat_code_copy.c_str(), error.what().c_str());
        }
    );
}

} // close extern "C" temporarily for static C++ functions

//------------------------------------------------------------------------------
// AB_PatchSessionWithHostIP — leader publishes their IP to the session
//------------------------------------------------------------------------------
static void AB_PatchSessionWithHostIP(void)
{
    std::string session_id;
    int version;
    accelbyte::memory::SharedPtr<accelbyte::user::User> user;

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        session_id = g_session_id;
        version = g_session_version;
        user = g_current_user;
    }

    // Build host address from Quake's network layer
    std::string host_addr;
    if (my_tcpip_address[0])
        host_addr = std::string(my_tcpip_address) + ":" + std::to_string(net_hostport);
    else
        host_addr = "127.0.0.1:" + std::to_string(net_hostport);

    Con_Printf("AccelByte: Patching session with host IP: %s\n", host_addr.c_str());

    std::string attrs_json = "{\"host_ip\":\"" + host_addr + "\"}";

    g_matchmake_future = std::async(std::launch::async, [user, session_id, version, attrs_json](){
        accelbyte::session::game_session::PatchGameSession request;
        request.session_id = session_id.c_str();
        request.body.attributes = accelbyte::utils::JsonObjectString(attrs_json.c_str());
        request.body.version = version;

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::session::GameSession::patch_game_session(
            auth,
            request,
            [](const accelbyte::session::model::GameSession& session) {
                runner.queue_task([](){
                    Con_Printf("AccelByte: Session patched with host IP\n");
                });
            },
            [](const accelbyte::Error& error) {
                std::string err = error.what().c_str();
                runner.queue_task([err](){
                    Con_Printf("AccelByte: Failed to patch session - %s\n", err.c_str());
                });
            }
        );
    });
}

//------------------------------------------------------------------------------
// AB_StartHosting — leader starts a Quake listen server
//------------------------------------------------------------------------------
static void AB_StartHosting(void)
{
    std::string map_name;
    int num_players;

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_matchmake_status = AB_MM_HOSTING;
        num_players = g_match_num_players;
    }

    // Read map name from cvar
    map_name = ab_match_map.string;
    if (map_name.empty())
        map_name = "start";

    Con_Printf("AccelByte: Starting host — map: %s, maxplayers: %d\n", map_name.c_str(), num_players);

    // Issue Quake console commands to start a listen server
    Cbuf_AddText("disconnect\n");
    Cbuf_AddText("listen 0\n");
    Cbuf_AddText(va("maxplayers %d\n", num_players));
    Cbuf_AddText(va("map %s\n", map_name.c_str()));

    // Dismiss the menu
    key_dest = key_game;
    m_state = m_none;

    // Publish our host IP to the session so other players can connect
    AB_PatchSessionWithHostIP();
}

//------------------------------------------------------------------------------
// AB_JoinSession — join the AccelByte game session after match found
//------------------------------------------------------------------------------
static void AB_JoinSession(void)
{
    std::string match_id;
    accelbyte::memory::SharedPtr<accelbyte::user::User> user;
    std::string our_user_id;

    {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (g_matchmake_status != AB_MM_FOUND)
        {
            Con_Printf("AccelByte: Cannot join session — not in FOUND state\n");
            return;
        }

        match_id = g_match_id;
        user = g_current_user;
        our_user_id = g_user_id;
        g_matchmake_status = AB_MM_JOINING;
    }

    Con_Printf("AccelByte: Joining session %s...\n", match_id.c_str());

    g_matchmake_future = std::async(std::launch::async, [user, match_id, our_user_id](){
        accelbyte::session::game_session::JoinGameSession request;
        request.session_id = match_id.c_str();

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::session::GameSession::join_game_session(
            auth,
            request,
            [our_user_id](const accelbyte::session::model::GameSession& session) {
                std::string session_id = session.id.c_str();
                std::string leader_id = session.leader_id.c_str();
                int version = session.version;
                bool is_leader = (leader_id == our_user_id);

                {
                    std::lock_guard<std::mutex> lock(g_mutex);
                    g_session_id = session_id;
                    g_session_leader_id = leader_id;
                    g_session_version = version;
                    g_is_session_leader = is_leader;

                    if (is_leader)
                        g_matchmake_status = AB_MM_JOINED_AS_LEADER;
                    else
                        g_matchmake_status = AB_MM_JOINED_AS_CLIENT;
                }

                if (is_leader)
                {
                    runner.queue_task([session_id](){
                        Con_Printf("AccelByte: Joined session %s as LEADER — starting host\n", session_id.c_str());
                        AB_StartHosting();
                    });
                }
                else
                {
                    runner.queue_task([session_id, leader_id](){
                        Con_Printf("AccelByte: Joined session %s as CLIENT — waiting for host (leader: %s)\n",
                            session_id.c_str(), leader_id.c_str());
                    });
                }
            },
            [](const accelbyte::Error& error) {
                std::string err = error.what().c_str();

                std::lock_guard<std::mutex> lock(g_mutex);
                g_matchmake_status = AB_MM_ERROR;
                g_matchmake_error = err;

                runner.queue_task([err](){
                    Con_Printf("AccelByte: Failed to join session - %s\n", err.c_str());
                });
            }
        );
    });
}

extern "C" {

void AB_CreateMatchTicket(void)
{
    if (!g_initialized)
    {
        Con_Printf("AccelByte: SDK not initialized\n");
        return;
    }

    const char* pool = ab_match_pool.string;
    if (!pool || !pool[0])
    {
        Con_Printf("AccelByte: ab_match_pool not configured\n");
        return;
    }

    accelbyte::memory::SharedPtr<accelbyte::user::User> user;
    {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (g_login_status != AB_LOGIN_SUCCESS || !g_current_user)
        {
            Con_Printf("AccelByte: Not logged in, cannot create match ticket\n");
            return;
        }

        // Reset match state
        g_matchmake_status = AB_MM_SEARCHING;
        g_match_ticket_id.clear();
        g_matchmake_error.clear();
        g_match_id.clear();
        g_match_pool_name.clear();
        g_match_num_players = 0;
        g_match_num_teams = 0;

        user = g_current_user;
    }

    std::string pool_copy(pool);
    Con_Printf("AccelByte: Creating match ticket for pool '%s'...\n", pool);

    g_matchmake_future = std::async(std::launch::async, [user, pool_copy](){
        accelbyte::match2::match_tickets::CreateMatchTicket request;
        request.body.match_pool = pool_copy.c_str();

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::match2::MatchTickets::create_match_ticket(
            auth,
            request,
            [](const accelbyte::match2::model::MatchTicket& ticket) {
                std::string ticket_id = ticket.match_ticket_id.c_str();

                std::lock_guard<std::mutex> lock(g_mutex);
                g_match_ticket_id = ticket_id;

                runner.queue_task([ticket_id](){
                    Con_Printf("AccelByte: Match ticket created: %s\n", ticket_id.c_str());
                });
            },
            [](const accelbyte::Error& error) {
                std::string err = error.what().c_str();

                std::lock_guard<std::mutex> lock(g_mutex);
                g_matchmake_status = AB_MM_ERROR;
                g_matchmake_error = err;

                runner.queue_task([err](){
                    Con_Printf("AccelByte: Failed to create match ticket - %s\n", err.c_str());
                });
            }
        );
    });
}

void AB_CancelMatchTicket(void)
{
    std::string ticket_id;
    accelbyte::memory::SharedPtr<accelbyte::user::User> user;

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        ticket_id = g_match_ticket_id;
        g_matchmake_status = AB_MM_CANCELLED;
        user = g_current_user;
    }

    if (ticket_id.empty())
    {
        Con_Printf("AccelByte: No match ticket to cancel\n");
        return;
    }

    Con_Printf("AccelByte: Cancelling match ticket %s...\n", ticket_id.c_str());

    g_matchmake_future = std::async(std::launch::async, [user, ticket_id](){
        accelbyte::match2::match_tickets::DeleteMatchTicket request;
        request.ticketid = ticket_id.c_str();

        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::match2::MatchTickets::delete_match_ticket(
            auth,
            request,
            [ticket_id]() {
                runner.queue_task([ticket_id](){
                    Con_Printf("AccelByte: Match ticket %s cancelled\n", ticket_id.c_str());
                });
            },
            [](const accelbyte::Error& error) {
                std::string err = error.what().c_str();
                runner.queue_task([err](){
                    Con_Printf("AccelByte: Failed to cancel match ticket - %s\n", err.c_str());
                });
            }
        );
    });
}

ab_matchmake_status_t AB_GetMatchmakingStatus(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_matchmake_status;
}

void AB_SetMatchmakingStatus(ab_matchmake_status_t status)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_matchmake_status = status;
}

const char* AB_GetMatchmakingErrorMessage(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_matchmake_error.empty() ? NULL : g_matchmake_error.c_str();
}

const char* AB_GetMatchTicketId(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_match_ticket_id.empty() ? NULL : g_match_ticket_id.c_str();
}

const char* AB_GetMatchId(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_match_id.empty() ? NULL : g_match_id.c_str();
}

const char* AB_GetMatchPoolName(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_match_pool_name.empty() ? NULL : g_match_pool_name.c_str();
}

int AB_GetMatchNumPlayers(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_match_num_players;
}

int AB_GetMatchNumTeams(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_match_num_teams;
}

int AB_IsSessionLeader(void)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_is_session_leader ? 1 : 0;
}

int AB_IsInitialized(void)
{
    return g_initialized ? 1 : 0;
}

void* get_current_user(void)
{
    return nullptr;
}

} // extern "C"
