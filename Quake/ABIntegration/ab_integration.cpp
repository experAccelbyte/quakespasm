/*
 * AccelByte SDK Integration for QuakeSpasm
 * C API wrappers — delegates to ABInstance
 */

#include "ab_integration.h"

#include "ab_instance.h"

#include <accelbyte/curl_http_executor/CurlRequestExecutorFactory.h>
#include <accelbyte/http/RequestExecutorFactory.h>
#include <accelbyte/settings/InMemorySettings.h>
#include <accelbyte/settings/global_settings.h>
#include <accelbyte/social/UserStatistic.h>
#include <accelbyte/social/models/UpdateStatItem.h>
#include <accelbyte/social/user_statistic/UpdateUserStatItemValueV2.h>

// WebSocket factory header
#include <accelbyte/cpp_web_socket/CppWebSocketFactory.h>
#include <accelbyte/web_socket/WebSocketFactory.h>

// P2P connection factory header
#include <accelbyte/libjuice_p2p_connection/LibjuiceP2PConnectionFactory.h>
#include <accelbyte/p2p_connection/P2PConnectionFactory.h>

// Session and Match2 service initialization
#include <accelbyte/match2/Match2Service.h>
#include <accelbyte/session/SessionService.h>

extern "C" {
#include "quakedef.h"
}

//------------------------------------------------------------------------------
// Cvar declarations
//------------------------------------------------------------------------------
static cvar_t ab_server_url = {"ab_server_url", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_id = {"ab_client_id", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_secret = {"ab_client_secret", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_device_id = {"ab_device_id", "", CVAR_NONE, 0.0f, NULL, NULL, NULL};

//------------------------------------------------------------------------------
// Module-level state
//------------------------------------------------------------------------------

static accelbyte::settings::InMemorySettings settings;

static void InitHttpExecutor()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        auto executor = std::make_shared<accelbyte::http::CurlRequestExecutorFactory>();
        accelbyte::http::RequestExecutorFactory::set_executor_factory(executor);
    });
}

static ABInstance* cast(ab_instance_t* h)
{
    return reinterpret_cast<ABInstance*>(h);
}
static const ABInstance* cast(const ab_instance_t* h)
{
    return reinterpret_cast<const ABInstance*>(h);
}

//------------------------------------------------------------------------------
// Instance-based C API
//------------------------------------------------------------------------------
extern "C" {

ab_instance_t* ab_create(void)
{
    InitHttpExecutor();

    static bool cvars_registered = false;
    if (!cvars_registered) {
        Cvar_RegisterVariable(&ab_server_url);
        Cvar_RegisterVariable(&ab_client_id);
        Cvar_RegisterVariable(&ab_client_secret);
        Cvar_RegisterVariable(&ab_device_id);
        cvars_registered = true;
    }

    // Register AccelByte SDK factories once.  The WebSocket factory is needed
    // for lobby connections; the P2P factory for ICE/TURN P2P connections.
    static bool factories_registered = false;
    if (!factories_registered) {
        auto wsFactory = accelbyte::memory::make_shared_ptr<accelbyte::cpp_web_socket::CppWebSocketFactory>();
        accelbyte::web_socket::WebSocketFactory::set_web_socket_factory(wsFactory);
        Con_Printf("AccelByte: WebSocket factory registered\n");

        auto p2pFactory =
            accelbyte::memory::make_shared_ptr<accelbyte::libjuice_p2p_connection::LibjuiceP2PConnectionFactory>();
        accelbyte::p2p_connection::P2PConnectionFactory::set_p2p_connection_factory(p2pFactory);
        Con_Printf("AccelByte: P2P connection factory registered\n");

        factories_registered = true;
    }

    ABInstance* inst = new ABInstance();

    // Apply -device_id command-line override if present.  Must happen before
    // LoginWithDeviceId so the overridden ID is used in the login request.
    int i = COM_CheckParm("-device_id");
    if (i && i + 1 < com_argc) {
        inst->GetLogin().SetDeviceId(com_argv[i + 1]);
    }

    inst->GetP2P().Init();
    inst->GetMatchmaking().Init();
    inst->GetAMS().Init();

    Con_Printf("AccelByte: SDK created\n");
    return reinterpret_cast<ab_instance_t*>(inst);
}

void ab_destroy(ab_instance_t* instance)
{
    ABInstance* inst = cast(instance);
    if (inst) {
        inst->GetMatchmaking().Shutdown();
        inst->GetP2P().Shutdown();
    }

    delete inst;
    Con_Printf("AccelByte: SDK destroyed\n");
}

void ab_set_server_url(ab_instance_t* instance, const char* url)
{
    ABInstance* inst = cast(instance);
    if (inst) {
        inst->SetServerUrl(url);
    }
}

void ab_set_client_id(ab_instance_t* instance, const char* id)
{
    ABInstance* inst = cast(instance);
    if (inst) {
        inst->SetClientId(id);
    }
}

void ab_set_client_secret(ab_instance_t* instance, const char* secret)
{
    ABInstance* inst = cast(instance);
    if (inst) {
        inst->SetClientSecret(secret);
    }
}

void ab_login_with_device_id(ab_instance_t* instance, ab_login_success_callback_t on_success, void* userdata)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }

    /* Cvars take precedence over values set via ab_set_*(). */
    const char* url = ab_server_url.string[0] ? ab_server_url.string : inst->GetServerUrl();
    const char* id = ab_client_id.string[0] ? ab_client_id.string : inst->GetClientId();
    const char* secret = ab_client_secret.string[0] ? ab_client_secret.string : inst->GetClientSecret();

    if (url && url[0]) {
        settings.set_server_url(url);
    }
    if (id && id[0]) {
        settings.set_client_id(id);
    }
    if (secret && secret[0]) {
        settings.set_client_secret(secret);
    }

    // Derive WebSocket lobby URL from the HTTP server URL and set it in
    // settings so the lobby connection uses the correct endpoint.
    if (url && url[0]) {
        std::string server_url_str(url);
        std::string lobby_url;
        if (server_url_str.find("https://") == 0) {
            lobby_url = "wss://" + server_url_str.substr(8);
        }
        else if (server_url_str.find("http://") == 0) {
            lobby_url = "ws://" + server_url_str.substr(7);
        }
        else {
            lobby_url = server_url_str;
        }

        if (!lobby_url.empty() && lobby_url.back() == '/') {
            lobby_url.pop_back();
        }
        lobby_url += "/lobby/";

        settings.set_lobby_url(lobby_url.c_str());
        Con_Printf("AccelByte: Lobby URL: %s\n", lobby_url.c_str());
    }

    accelbyte::settings::set_global_settings(settings);

    // Initialize session and matchmaking services.
    if (url && url[0]) {
        if (!accelbyte::session::SessionService::initialized()) {
            accelbyte::session::SessionService::initialize(url);
            Con_Printf("AccelByte: SessionService initialized\n");
        }
        if (!accelbyte::match2::Match2Service::initialized()) {
            accelbyte::match2::Match2Service::initialize(url);
            Con_Printf("AccelByte: Match2Service initialized\n");
        }
    }

    // Apply ab_device_id cvar override if set (checked at login time so the
    // cvar can be set from the config file which is loaded after ab_create).
    if (ab_device_id.string[0]) {
        inst->GetLogin().SetDeviceId(ab_device_id.string);
    }

    // Register the post-login hook that connects the lobby WebSocket and
    // registers notification handlers.  Runs on the login background thread
    // so it completes before the C on_success callback fires.
    inst->GetLogin().SetPostLoginHook([inst](accelbyte::memory::SharedPtr<accelbyte::user::User> user) {
        const char* uid_cstr = inst->GetLogin().GetUserId();
        std::string uid = uid_cstr ? uid_cstr : "";
        inst->GetMatchmaking().SetupLobbyAfterLogin(user, uid);
    });

    inst->GetLogin().LoginWithDeviceId(on_success, userdata);
}

void ab_update(ab_instance_t* instance)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }

    // Drain ABInstance's own task runner (login, stats, leaderboard callbacks).
    inst->Update();

    // Poll the lobby WebSocket and drain matchmaking task queue.
    inst->GetMatchmaking().Update();

    // Poll P2P proxy state (clean up dead connections).
    inst->GetP2P().Update();
}

ab_login_status_t ab_get_login_status(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLogin().GetStatus() : AB_LOGIN_IDLE;
}

const char* ab_get_user_id(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLogin().GetUserId() : NULL;
}

const char* ab_get_display_name(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLogin().GetDisplayName() : NULL;
}

const char* ab_get_error_message(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLogin().GetErrorMessage() : NULL;
}

void ab_stat_update(ab_instance_t* instance, const char* stat_code, float value, ab_stat_strategy_t strategy)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetStatistic().UpdateStat(inst->GetCurrentUser(), stat_code, value, (int)strategy);
}

void ab_stat_fetch(ab_instance_t* instance, const char* const* stat_codes, int count)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetStatistic().FetchStats(inst->GetCurrentUser(), stat_codes, count);
}

void ab_stat_bulk_update(
    ab_instance_t* instance, const char* const* stat_codes, const float* values, int count, ab_stat_strategy_t strategy)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetStatistic().BulkUpdateStats(inst->GetCurrentUser(), stat_codes, values, count, (int)strategy);
}

int ab_stat_get_cached(const ab_instance_t* instance, const char* stat_code, float* out_value)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetStatistic().GetCachedValue(stat_code, out_value) : 0;
}

void ab_stat_invalidate_cache(ab_instance_t* instance)
{
    ABInstance* inst = cast(instance);
    if (inst) {
        inst->GetStatistic().InvalidateCache();
    }
}

void ab_cycle_fetch_items(ab_instance_t* instance, const char* cycle_id, const char* const* stat_codes, int count)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetCycle().FetchCycleItems(inst->GetCurrentUser(), cycle_id, stat_codes, count);
}

int ab_cycle_get_cached(const ab_instance_t* instance, const char* cycle_id, const char* stat_code, float* out_value)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetCycle().GetCachedValue(cycle_id, stat_code, out_value) : 0;
}

void ab_cycle_invalidate_cache(ab_instance_t* instance)
{
    ABInstance* inst = cast(instance);
    if (inst) {
        inst->GetCycle().InvalidateCache();
    }
}

void ab_leaderboard_fetch_rankings(ab_instance_t* instance, const char* leaderboard_code, int limit, int offset)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetLeaderboard().FetchRankings(inst->GetCurrentUser(), leaderboard_code, limit, offset);
}

void ab_leaderboard_fetch_cycle_rankings(
    ab_instance_t* instance, const char* leaderboard_code, const char* cycle_id, int limit, int offset)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetLeaderboard().FetchCycleRankings(inst->GetCurrentUser(), leaderboard_code, cycle_id, limit, offset);
}

void ab_leaderboard_fetch_user_rank(ab_instance_t* instance, const char* leaderboard_code)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetLeaderboard().FetchUserRank(inst->GetCurrentUser(), leaderboard_code);
}

int ab_leaderboard_get_rankings(
    const ab_instance_t* instance, const char* leaderboard_code, ab_rank_entry_t* out, int max_entries)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLeaderboard().GetCachedRankings(leaderboard_code, out, max_entries) : 0;
}

int ab_leaderboard_get_cycle_rankings(
    const ab_instance_t* instance,
    const char* leaderboard_code,
    const char* cycle_id,
    ab_rank_entry_t* out,
    int max_entries)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLeaderboard().GetCachedCycleRankings(leaderboard_code, cycle_id, out, max_entries) : 0;
}

int ab_leaderboard_get_user_rank(
    const ab_instance_t* instance, const char* leaderboard_code, long* out_rank, float* out_point)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLeaderboard().GetCachedUserRank(leaderboard_code, out_rank, out_point) : 0;
}

int ab_leaderboard_get_user_cycle_rank(
    const ab_instance_t* instance, const char* leaderboard_code, const char* cycle_id, long* out_rank, float* out_point)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLeaderboard().GetCachedUserCycleRank(leaderboard_code, cycle_id, out_rank, out_point) : 0;
}

void ab_leaderboard_invalidate_cache(ab_instance_t* instance)
{
    ABInstance* inst = cast(instance);
    if (inst) {
        inst->GetLeaderboard().InvalidateCache();
    }
}

void ab_update_user_stat(ab_instance_t* instance, const char* stat_code, float value, int strategy)
{
    if (!cast(instance)) {
        return;
    }
    ab_stat_update(instance, stat_code, value, (ab_stat_strategy_t)strategy);
}

void AB_CreateMatchTicket(ab_instance_t* instance)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        Con_Printf("AccelByte: SDK not initialized\n");
        return;
    }
    inst->GetMatchmaking().CreateMatchTicket();
}

void AB_CancelMatchTicket(ab_instance_t* instance)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetMatchmaking().CancelMatchTicket();
}

ab_matchmake_status_t AB_GetMatchmakingStatus(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetMatchmaking().GetStatus() : AB_MM_IDLE;
}

void AB_SetMatchmakingStatus(ab_instance_t* instance, ab_matchmake_status_t status)
{
    ABInstance* inst = cast(instance);
    if (inst) {
        inst->GetMatchmaking().SetStatus(status);
    }
}

const char* AB_GetMatchmakingErrorMessage(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetMatchmaking().GetErrorMessage() : NULL;
}

const char* AB_GetMatchTicketId(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetMatchmaking().GetTicketId() : NULL;
}

const char* AB_GetMatchId(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetMatchmaking().GetMatchId() : NULL;
}

const char* AB_GetMatchPoolName(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetMatchmaking().GetMatchPoolName() : NULL;
}

int AB_GetMatchNumPlayers(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetMatchmaking().GetNumPlayers() : 0;
}

int AB_GetMatchNumTeams(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetMatchmaking().GetNumTeams() : 0;
}

int AB_IsSessionLeader(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return (inst && inst->GetMatchmaking().IsSessionLeader()) ? 1 : 0;
}

const char* AB_GetErrorMessage(const ab_instance_t* instance)
{
    const ABInstance* inst = cast(instance);
    return inst ? inst->GetLogin().GetErrorMessage() : NULL;
}

void AB_UpdateUserStatItemValue(ab_instance_t* instance, const char* stat_code, float value, int strategy)
{
    ABInstance* inst = cast(instance);
    if (!inst) {
        return;
    }
    inst->GetStatistic().UpdateStat(inst->GetCurrentUser(), stat_code, value, strategy);
}

} // extern "C"
