/*
 * AccelByte SDK Integration for QuakeSpasm
 * C++ implementation
 */

#include "ab_integration.h"

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
}

//------------------------------------------------------------------------------
// CVars for AccelByte configuration
//------------------------------------------------------------------------------
static cvar_t ab_server_url = {"ab_server_url", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_id = {"ab_client_id", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};
static cvar_t ab_client_secret = {"ab_client_secret", "", CVAR_ARCHIVE, 0.0f, NULL, NULL, NULL};

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

// Lobby state
static std::shared_ptr<accelbyte::lobby::Lobby> g_lobby;
static accelbyte::memory::SharedPtr<accelbyte::lobby::LobbyConnection> g_lobby_connection;

static ABTaskRunner runner;
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

int AB_IsInitialized(void)
{
    return g_initialized ? 1 : 0;
}

void* get_current_user(void)
{
    return nullptr;
}

} // extern "C"
