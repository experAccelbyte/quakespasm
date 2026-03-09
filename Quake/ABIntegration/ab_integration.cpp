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

#include <mutex>

extern "C" {
#include "quakedef.h"
}

static cvar_t ab_server_url    = {"ab_server_url",    "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_id     = {"ab_client_id",     "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_secret = {"ab_client_secret", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
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

} // extern "C"
