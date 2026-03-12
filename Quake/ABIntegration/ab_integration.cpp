/*
 * AccelByte SDK Integration for QuakeSpasm
 * C API wrappers — delegates to ABInstance
 */

#include "ab_integration.h"
#include "ab_p2p.h"

#include "ab_instance.h"

#include <accelbyte/curl_http_executor/CurlRequestExecutorFactory.h>
#include <accelbyte/http/RequestExecutorFactory.h>
#include <accelbyte/settings/InMemorySettings.h>
#include <accelbyte/settings/global_settings.h>

#include <accelbyte/social/UserStatistic.h>
#include <accelbyte/social/user_statistic/UpdateUserStatItemValueV2.h>
#include <accelbyte/social/models/UpdateStatItem.h>

// Lobby and WebSocket headers
#include <accelbyte/lobby/Lobby.h>
#include <accelbyte/lobby/LobbyConnection.h>
#include <accelbyte/cpp_web_socket/CppWebSocketFactory.h>
#include <accelbyte/web_socket/WebSocketFactory.h>

// P2P connection headers
#include <accelbyte/p2p_connection/P2PConnectionFactory.h>
#include <accelbyte/libjuice_p2p_connection/LibjuiceP2PConnectionFactory.h>

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
#include <accelbyte/lobby/notifications/OnDSStatusChanged.h>
#include "ab_task_runner.h"

// Standard library
#include <string>
#include <mutex>

extern "C" {
#include "quakedef.h"
}

static cvar_t ab_server_url    = {"ab_server_url",    "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_id     = {"ab_client_id",     "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_secret = {"ab_client_secret", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};

static cvar_t ab_match_pool = {"ab_match_pool", "quake_ffa", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_match_map = {"ab_match_map", "start", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_device_id = {"ab_device_id", "", CVAR_NONE, 0.0f, NULL, NULL, NULL};

std::mutex g_mutex;

static accelbyte::settings::InMemorySettings settings;

static void InitHttpExecutor()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        auto executor = std::make_shared<accelbyte::http::CurlRequestExecutorFactory>();
        accelbyte::http::RequestExecutorFactory::set_executor_factory(executor);
    });
}

static ABInstance* cast(ab_instance_t* h)             { return reinterpret_cast<ABInstance*>(h); }
static const ABInstance* cast(const ab_instance_t* h) { return reinterpret_cast<const ABInstance*>(h); }

extern "C" {

ab_instance_t* ab_create(void)
{
    InitHttpExecutor();
    static bool cvars_registered = false;
    if (!cvars_registered) {
        Cvar_RegisterVariable(&ab_server_url);
        Cvar_RegisterVariable(&ab_client_id);
        Cvar_RegisterVariable(&ab_client_secret);
        cvars_registered = true;
    }
    return reinterpret_cast<ab_instance_t*>(new ABInstance());
}

void ab_destroy(ab_instance_t* instance)
{
    delete cast(instance);
}

void ab_set_server_url(ab_instance_t* instance, const char* url)       { cast(instance)->SetServerUrl(url); }
void ab_set_client_id(ab_instance_t* instance, const char* id)         { cast(instance)->SetClientId(id); }
void ab_set_client_secret(ab_instance_t* instance, const char* secret) { cast(instance)->SetClientSecret(secret); }

void ab_login_with_device_id(ab_instance_t* instance, ab_login_success_callback_t on_success, void* userdata)
{
    ABInstance* inst = cast(instance);

    /* Cvars take precedence over values set via ab_set_*(). */
    const char* url    = ab_server_url.string[0]    ? ab_server_url.string    : inst->GetServerUrl();
    const char* id     = ab_client_id.string[0]     ? ab_client_id.string     : inst->GetClientId();
    const char* secret = ab_client_secret.string[0] ? ab_client_secret.string : inst->GetClientSecret();

    if (url    && url[0])    settings.set_server_url(url);
    if (id     && id[0])     settings.set_client_id(id);
    if (secret && secret[0]) settings.set_client_secret(secret);
    accelbyte::settings::set_global_settings(settings);

    inst->GetLogin().LoginWithDeviceId(on_success, userdata);
}

void ab_update(ab_instance_t* instance)               { cast(instance)->Update(); }

ab_login_status_t ab_get_login_status (const ab_instance_t* instance) { return cast(instance)->GetLogin().GetStatus(); }
const char*       ab_get_user_id      (const ab_instance_t* instance) { return cast(instance)->GetLogin().GetUserId(); }
const char*       ab_get_display_name (const ab_instance_t* instance) { return cast(instance)->GetLogin().GetDisplayName(); }
const char*       ab_get_error_message(const ab_instance_t* instance) { return cast(instance)->GetLogin().GetErrorMessage(); }

void ab_stat_update(ab_instance_t* instance, const char* stat_code, float value, ab_stat_strategy_t strategy)
{
    ABInstance* inst = cast(instance);
    inst->GetStatistic().UpdateStat(inst->GetCurrentUser(), stat_code, value, (int)strategy);
}

void ab_stat_fetch(ab_instance_t* instance, const char* const* stat_codes, int count)
{
    ABInstance* inst = cast(instance);
    inst->GetStatistic().FetchStats(inst->GetCurrentUser(), stat_codes, count);
}

void ab_stat_bulk_update(ab_instance_t* instance, const char* const* stat_codes, const float* values, int count, ab_stat_strategy_t strategy)
{
    ABInstance* inst = cast(instance);
    inst->GetStatistic().BulkUpdateStats(inst->GetCurrentUser(), stat_codes, values, count, (int)strategy);
}

int ab_stat_get_cached(const ab_instance_t* instance, const char* stat_code, float* out_value)
{
    return cast(instance)->GetStatistic().GetCachedValue(stat_code, out_value) ? 1 : 0;
}

void ab_stat_invalidate_cache(ab_instance_t* instance)
{
    cast(instance)->GetStatistic().InvalidateCache();
}

void ab_cycle_fetch_items(ab_instance_t* instance, const char* cycle_id,
                           const char* const* stat_codes, int count)
{
    ABInstance* inst = cast(instance);
    inst->GetCycle().FetchCycleItems(inst->GetCurrentUser(), cycle_id, stat_codes, count);
}

int ab_cycle_get_cached(const ab_instance_t* instance, const char* cycle_id,
                         const char* stat_code, float* out_value)
{
    return cast(instance)->GetCycle().GetCachedValue(cycle_id, stat_code, out_value) ? 1 : 0;
}

void ab_cycle_invalidate_cache(ab_instance_t* instance)
{
    cast(instance)->GetCycle().InvalidateCache();
}

void ab_leaderboard_fetch_rankings(ab_instance_t* instance,
    const char* leaderboard_code, int limit, int offset)
{
    ABInstance* inst = cast(instance);
    inst->GetLeaderboard().FetchRankings(inst->GetCurrentUser(), leaderboard_code, limit, offset);
}

void ab_leaderboard_fetch_cycle_rankings(ab_instance_t* instance,
    const char* leaderboard_code, const char* cycle_id, int limit, int offset)
{
    ABInstance* inst = cast(instance);
    inst->GetLeaderboard().FetchCycleRankings(
        inst->GetCurrentUser(), leaderboard_code, cycle_id, limit, offset);
}

void ab_leaderboard_fetch_user_rank(ab_instance_t* instance, const char* leaderboard_code)
{
    ABInstance* inst = cast(instance);
    inst->GetLeaderboard().FetchUserRank(inst->GetCurrentUser(), leaderboard_code);
}

int ab_leaderboard_get_rankings(const ab_instance_t* instance,
    const char* leaderboard_code, ab_rank_entry_t* out, int max_entries)
{
    return cast(instance)->GetLeaderboard().GetCachedRankings(leaderboard_code, out, max_entries);
}

int ab_leaderboard_get_cycle_rankings(const ab_instance_t* instance,
    const char* leaderboard_code, const char* cycle_id,
    ab_rank_entry_t* out, int max_entries)
{
    return cast(instance)->GetLeaderboard().GetCachedCycleRankings(
        leaderboard_code, cycle_id, out, max_entries);
}

int ab_leaderboard_get_user_rank(const ab_instance_t* instance,
    const char* leaderboard_code, long* out_rank, float* out_point)
{
    return cast(instance)->GetLeaderboard().GetCachedUserRank(
        leaderboard_code, out_rank, out_point) ? 1 : 0;
}

int ab_leaderboard_get_user_cycle_rank(const ab_instance_t* instance,
    const char* leaderboard_code, const char* cycle_id,
    long* out_rank, float* out_point)
{
    return cast(instance)->GetLeaderboard().GetCachedUserCycleRank(
        leaderboard_code, cycle_id, out_rank, out_point) ? 1 : 0;
}

void ab_leaderboard_invalidate_cache(ab_instance_t* instance)
{
    cast(instance)->GetLeaderboard().InvalidateCache();
}

void ab_update_user_stat(ab_instance_t* instance, const char* stat_code, float value, int strategy)
{
    ab_stat_update(instance, stat_code, value, (ab_stat_strategy_t)strategy);
}


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
accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> g_lobby_connection;

// Async futures storage
static std::future<void> g_matchmake_future;

ABTaskRunner runner;

//------------------------------------------------------------------------------
// AB_TryConnectFromDSInfo — extract IP/port from DS info and connect
//------------------------------------------------------------------------------
static bool AB_TryConnectFromDSInfo(const accelbyte::session::model::GameSession& session)
{
    const auto& ds_info = session.ds_information;

    if (!ds_info.server.has_value())
        return false;

    const auto& server = ds_info.server.value();
    if (!server.ip.has_value() || !server.port.has_value())
        return false;

    std::string ip = server.ip.value().c_str();
    int port = server.port.value();
    std::string address = ip + ":" + std::to_string(port);

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_host_address = address;
        g_matchmake_status = AB_MM_CONNECTING;
    }

    runner.queue_task([address](){
        Con_Printf("AccelByte: DS available at %s, connecting...\n", address.c_str());
        Cbuf_AddText(va("connect \"%s\"\n", address.c_str()));
        key_dest = key_game;
        m_state = m_none;
    });

    return true;
}

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
// DSStatusChangedHandler — receives DS status updates for DS sessions
//------------------------------------------------------------------------------
class DSStatusChangedHandler : public accelbyte::lobby::TypedMessageHandler<accelbyte::lobby::notifications::OnDSStatusChanged>
{
public:
    void handle(const accelbyte::lobby::notifications::OnDSStatusChanged& message) override
    {
        std::string session_id;
        accelbyte::memory::SharedPtr<accelbyte::user::User> user;

        {
            std::lock_guard<std::mutex> lock(g_mutex);
            if (g_matchmake_status != AB_MM_WAITING_FOR_DS)
                return;
            session_id = g_session_id;
            user = g_current_user;
        }

        // Fetch full session to get typed ds_information
        accelbyte::session::game_session::GetGameSession request;
        request.session_id = session_id.c_str();
        const accelbyte::tls::SecurityAuthorization& auth = *user;

        accelbyte::session::GameSession::get_game_session(
            auth, request,
            [](const accelbyte::session::model::GameSession& session) {
                if (!AB_TryConnectFromDSInfo(session))
                {
                    runner.queue_task([](){
                        Con_Printf("AccelByte: DS status changed but not yet available, continuing to wait...\n");
                    });
                }
            },
            [](const accelbyte::Error& error) {
                std::string err = error.what().c_str();
                runner.queue_task([err](){
                    Con_Printf("AccelByte: Failed to fetch session: %s\n", err.c_str());
                });
            }
        );
    }
};

static std::shared_ptr<DSStatusChangedHandler> g_ds_status_handler;

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

            g_ds_status_handler = std::make_shared<DSStatusChangedHandler>();
            connection->add_message_handler(g_ds_status_handler);

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
    Cvar_RegisterVariable(&ab_device_id);

    // Use custom device ID if provided on command line, otherwise generate one
    int i = COM_CheckParm("-device_id");
    if (i && i < com_argc - 1)
        g_device_id = com_argv[i + 1];
    else
        g_device_id = GenerateDeviceId();

    Con_Printf("AccelByte: SDK initialized\n");
    Con_Printf("AccelByte: Device ID: %s\n", g_device_id.c_str());

    g_initialized = true;

    ABP2P_Init();

    // CURL HTTP EXECUTOR
    auto curlExecutor = std::make_shared<accelbyte::http::CurlRequestExecutorFactory>();
    accelbyte::http::RequestExecutorFactory::set_executor_factory(curlExecutor);

    // WebSocket factory — needed for lobby connections
    auto wsFactory = accelbyte::memory::make_shared_ptr<accelbyte::cpp_web_socket::CppWebSocketFactory>();
    accelbyte::web_socket::WebSocketFactory::set_web_socket_factory(wsFactory);
    Con_Printf("AccelByte: WebSocket factory registered\n");

    // P2P connection factory — needed for ICE/TURN P2P connections
    auto p2pFactory = accelbyte::memory::make_shared_ptr<accelbyte::libjuice_p2p_connection::LibjuiceP2PConnectionFactory>();
    accelbyte::p2p_connection::P2PConnectionFactory::set_p2p_connection_factory(p2pFactory);
    Con_Printf("AccelByte: P2P connection factory registered\n");
}

void AB_Shutdown(void)
{
    if (!g_initialized)
    {
        return;
    }

    ABP2P_Shutdown();

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
    g_ds_status_handler = nullptr;

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

    // Re-check device ID cvar at login time (config may have set it after init)
    if (ab_device_id.string[0])
        g_device_id = ab_device_id.string;

    // Login with device ID
    Con_Printf("AccelByte: Logging in with device ID: %s\n", g_device_id.c_str());

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

    ABP2P_Update();

    runner.execute_task_queue();
}

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

    // Start P2P server so other players can connect via AccelByte P2P
    ABP2P_StartServer();
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

                // Determine session type
                std::string session_type = session.configuration.type.c_str();

                {
                    std::lock_guard<std::mutex> lock(g_mutex);
                    g_session_id = session_id;
                    g_session_leader_id = leader_id;
                    g_session_version = version;
                    g_is_session_leader = is_leader;
                }

                if (session_type == "DS")
                {
                    // DS session — connect from DS info or wait
                    if (AB_TryConnectFromDSInfo(session))
                    {
                        runner.queue_task([session_id](){
                            Con_Printf("AccelByte: Joined session %s — DS available, connecting\n", session_id.c_str());
                        });
                    }
                    else
                    {
                        {
                            std::lock_guard<std::mutex> lock(g_mutex);
                            g_matchmake_status = AB_MM_WAITING_FOR_DS;
                        }
                        runner.queue_task([session_id](){
                            Con_Printf("AccelByte: Joined session %s — waiting for DS\n", session_id.c_str());
                        });
                    }
                }
                else
                {
                    // P2P session — use AccelByte P2P for connection
                    {
                        std::lock_guard<std::mutex> lock(g_mutex);
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
                            Con_Printf("AccelByte: Joined session %s as CLIENT — connecting to host via P2P (leader: %s)\n",
                                session_id.c_str(), leader_id.c_str());
                            ABP2P_ConnectToHost(leader_id.c_str());
                        });
                    }
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

} // extern "C"
