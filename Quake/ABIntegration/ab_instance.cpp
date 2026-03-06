#include "ab_instance.h"

#include <accelbyte/user/UserLogin.h>
#include <accelbyte/user/parameters/LoginWithDeviceId.h>
#include <accelbyte/settings/global_settings.h>
#include <accelbyte/crypto/md5.h>

#include <future>

extern "C" {
#include "quakedef.h"
#include "console.h"
    extern double Sys_DoubleTime(void);
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static std::string GenerateDeviceId()
{
    std::string combined;
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS)
    {
        char guid[256] = {0};
        DWORD size = sizeof(guid);
        if (RegQueryValueExA(hKey, "MachineGuid", NULL, NULL, (LPBYTE)guid, &size) == ERROR_SUCCESS)
            combined += guid;
        RegCloseKey(hKey);
    }
    DWORD serial = 0;
    if (GetVolumeInformationA("C:\\", NULL, 0, &serial, NULL, NULL, NULL, 0))
        combined += std::to_string(serial);
    if (combined.empty())
        combined = "quakespasm-default-device";
    return accelbyte::crypto::md5(accelbyte::String(combined.c_str())).c_str();
}
#else
#include <fstream>

static std::string GenerateDeviceId()
{
    std::string machine_id;
    std::ifstream file("/etc/machine-id");
    if (file.is_open()) { std::getline(file, machine_id); }
    if (machine_id.empty()) {
        std::ifstream dbus_file("/var/lib/dbus/machine-id");
        if (dbus_file.is_open()) { std::getline(dbus_file, machine_id); }
    }
    if (machine_id.empty())
        machine_id = "quakespasm-default-device";
    return accelbyte::crypto::md5(accelbyte::String(machine_id.c_str())).c_str();
}
#endif

ABInstance::ABInstance(cvar_t* cvar_url, cvar_t* cvar_client_id, cvar_t* cvar_client_secret)
    : cvar_server_url_(cvar_url)
    , cvar_client_id_(cvar_client_id)
    , cvar_client_secret_(cvar_client_secret)
{
    device_id_ = GenerateDeviceId();
    Con_Printf("AccelByte: Instance created. Device ID: %s\n", device_id_.c_str());

    statistic_.SetTaskRunner(task_runner_);
    cycle_.SetTaskRunner(task_runner_);
    leaderboard_.SetTaskRunner(task_runner_);
}

void ABInstance::SetServerUrl(const char* url)  { settings_.set_server_url(url); }
void ABInstance::SetClientId(const char* id)    { settings_.set_client_id(id); }
void ABInstance::SetClientSecret(const char* s) { settings_.set_client_secret(s); }

void ABInstance::LoginWithDeviceId()
{
    if (cvar_server_url_ && cvar_server_url_->string && cvar_server_url_->string[0])
        settings_.set_server_url(cvar_server_url_->string);
    if (cvar_client_id_ && cvar_client_id_->string && cvar_client_id_->string[0])
        settings_.set_client_id(cvar_client_id_->string);
    if (cvar_client_secret_ && cvar_client_secret_->string && cvar_client_secret_->string[0])
        settings_.set_client_secret(cvar_client_secret_->string);

    accelbyte::settings::set_global_settings(settings_);

    Con_Printf("AccelByte: Logging in with device ID...\n");

    accelbyte::user::parameters::LoginWithDeviceId params;
    params.device_id = device_id_.c_str();
    params.create_headless = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        login_status_ = AB_LOGIN_IN_PROGRESS;
    }

    login_future_ = std::async(std::launch::async, [params, this]() {
        accelbyte::user::UserLogin::login_with_device_id(
            params,
            [this](accelbyte::memory::SharedPtr<accelbyte::user::User> user) { OnLoginSuccess(user); },
            [this](accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> ticket) { OnLoginQueued(ticket); },
            [this](const accelbyte::Error& error) { OnLoginError(error); }
        );
    });
}

AB_Statistic& ABInstance::GetStatistic()             { return statistic_; }
const AB_Statistic& ABInstance::GetStatistic() const  { return statistic_; }
AB_Cycle&       ABInstance::GetCycle()                { return cycle_; }
const AB_Cycle& ABInstance::GetCycle() const          { return cycle_; }
AB_Leaderboard&       ABInstance::GetLeaderboard()       { return leaderboard_; }
const AB_Leaderboard& ABInstance::GetLeaderboard() const { return leaderboard_; }

accelbyte::memory::SharedPtr<accelbyte::user::User> ABInstance::GetCurrentUser() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return current_user_;
}

void ABInstance::Update()
{
    task_runner_.execute_task_queue();
}

ab_login_status_t ABInstance::GetLoginStatus() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return login_status_;
}

const char* ABInstance::GetUserId() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return user_id_.empty() ? nullptr : user_id_.c_str();
}

const char* ABInstance::GetDisplayName() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return display_name_.empty() ? nullptr : display_name_.c_str();
}

const char* ABInstance::GetErrorMessage() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return error_message_.empty() ? nullptr : error_message_.c_str();
}

void ABInstance::OnLoginSuccess(accelbyte::memory::SharedPtr<accelbyte::user::User> user)
{
    std::lock_guard<std::mutex> lock(mutex_);
    current_user_ = user;
    user_id_ = user->user_id().c_str();
    display_name_ = user->display_name().c_str();
    login_status_ = AB_LOGIN_SUCCESS;
    queue_ticket_ = nullptr;

    task_runner_.queue_task([](const accelbyte::String& token) {
        Con_Printf("AccelByte: Login successful! Token: %s\n", token.c_str());
    }, user->credential()->access_token().value());
}

void ABInstance::OnLoginQueued(accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> ticket)
{
    std::lock_guard<std::mutex> lock(mutex_);
    login_status_ = AB_LOGIN_QUEUED;
    queue_ticket_ = ticket;
    last_queue_poll_ = Sys_DoubleTime();
    Con_Printf("AccelByte: In login queue...\n");
}

void ABInstance::OnLoginError(const accelbyte::Error& error)
{
    std::lock_guard<std::mutex> lock(mutex_);
    error_message_ = error.what().c_str();
    login_status_ = AB_LOGIN_FAILED;
    queue_ticket_ = nullptr;
    Con_Printf("AccelByte: Login failed - %s\n", error_message_.c_str());
}
