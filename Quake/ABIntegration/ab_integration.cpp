/*
 * AccelByte SDK Integration for QuakeSpasm
 * C API wrappers — delegates to ABInstance
 */

#include "ab_integration.h"
#include "ab_instance.h"

#include <accelbyte/curl_http_executor/CurlRequestExecutorFactory.h>
#include <accelbyte/http/RequestExecutorFactory.h>

#include <mutex>

extern "C" {
#include "quakedef.h"
}

static cvar_t ab_server_url    = {"ab_server_url",    "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_id     = {"ab_client_id",     "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_secret = {"ab_client_secret", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};

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
    return reinterpret_cast<ab_instance_t*>(new ABInstance(
        &ab_server_url, &ab_client_id, &ab_client_secret
    ));
}

void ab_destroy(ab_instance_t* instance)
{
    delete cast(instance);
}

void ab_set_server_url(ab_instance_t* instance, const char* url)       { cast(instance)->SetServerUrl(url); }
void ab_set_client_id(ab_instance_t* instance, const char* id)         { cast(instance)->SetClientId(id); }
void ab_set_client_secret(ab_instance_t* instance, const char* secret) { cast(instance)->SetClientSecret(secret); }

void ab_login_with_device_id(ab_instance_t* instance) { cast(instance)->LoginWithDeviceId(); }
void ab_update(ab_instance_t* instance)               { cast(instance)->Update(); }

ab_login_status_t ab_get_login_status (const ab_instance_t* instance) { return cast(instance)->GetLoginStatus(); }
const char*       ab_get_user_id      (const ab_instance_t* instance) { return cast(instance)->GetUserId(); }
const char*       ab_get_display_name (const ab_instance_t* instance) { return cast(instance)->GetDisplayName(); }
const char*       ab_get_error_message(const ab_instance_t* instance) { return cast(instance)->GetErrorMessage(); }

void ab_update_user_stat(ab_instance_t* instance, const char* stat_code, float value, int strategy)
{
    cast(instance)->UpdateUserStat(stat_code, value, strategy);
}

} // extern "C"
