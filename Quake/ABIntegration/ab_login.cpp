#include "ab_login.h"

#include <accelbyte/crypto/md5.h>
#include <accelbyte/user/UserLogin.h>
#include <accelbyte/user/parameters/LoginWithDeviceId.h>

extern "C" {
#include "quakedef.h"
//
#include "console.h"
extern double Sys_DoubleTime(void);
}
// STL
#include <future>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <windows.h>

static std::string GenerateDeviceId()
{
    std::string combined;
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &hKey) ==
        ERROR_SUCCESS) {
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
#    include <fstream>

static std::string GenerateDeviceId()
{
    std::string machine_id;
    std::ifstream file("/etc/machine-id");
    if (file.is_open()) {
        std::getline(file, machine_id);
    }
    if (machine_id.empty()) {
        std::ifstream dbus_file("/var/lib/dbus/machine-id");
        if (dbus_file.is_open()) {
            std::getline(dbus_file, machine_id);
        }
    }
    if (machine_id.empty())
        machine_id = "quakespasm-default-device";
    return accelbyte::crypto::md5(accelbyte::String(machine_id.c_str())).c_str();
}
#endif

AB_Login::AB_Login()
{
    device_id_ = GenerateDeviceId();
    Con_Printf("AccelByte: Device ID: %s\n", device_id_.c_str());
}

void AB_Login::SetTaskRunner(ABTaskRunner& tr)
{
    task_runner_ = &tr;
}

void AB_Login::LoginWithDeviceId()
{
    Con_Printf("AccelByte: Logging in with device ID...\n");

    accelbyte::user::parameters::LoginWithDeviceId params;
    params.device_id = device_id_.c_str();
    params.create_headless = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        status_ = AB_LOGIN_IN_PROGRESS;
    }

    login_future_ = std::async(std::launch::async, [params, this]() {
        accelbyte::user::UserLogin::login_with_device_id(
            params,
            [this](accelbyte::memory::SharedPtr<accelbyte::user::User> user) { OnLoginSuccess(user); },
            [this](accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> ticket) {
                OnLoginQueued(ticket);
            },
            [this](const accelbyte::Error& error) { OnLoginError(error); });
    });
}

ab_login_status_t AB_Login::GetStatus() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return status_;
}

const char* AB_Login::GetUserId() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return user_id_.empty() ? nullptr : user_id_.c_str();
}

const char* AB_Login::GetDisplayName() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return display_name_.empty() ? nullptr : display_name_.c_str();
}

const char* AB_Login::GetErrorMessage() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return error_message_.empty() ? nullptr : error_message_.c_str();
}

accelbyte::memory::SharedPtr<accelbyte::user::User> AB_Login::GetCurrentUser() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return current_user_;
}

void AB_Login::OnLoginSuccess(accelbyte::memory::SharedPtr<accelbyte::user::User> user)
{
    std::lock_guard<std::mutex> lock(mutex_);
    current_user_ = user;
    user_id_ = user->user_id().c_str();
    display_name_ = user->display_name().c_str();
    status_ = AB_LOGIN_SUCCESS;
    queue_ticket_ = nullptr;

    if (task_runner_)
        task_runner_->queue_task(
            [](const accelbyte::String& token) {
                Con_Printf("AccelByte: Login successful! Token: %s\n", token.c_str());
            },
            user->credential()->access_token().value());
}

void AB_Login::OnLoginQueued(accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> ticket)
{
    std::lock_guard<std::mutex> lock(mutex_);
    status_ = AB_LOGIN_QUEUED;
    queue_ticket_ = ticket;
    last_queue_poll_ = Sys_DoubleTime();

    if (task_runner_)
        task_runner_->queue_task([]() { Con_Printf("AccelByte: In login queue...\n"); });
}

void AB_Login::OnLoginError(const accelbyte::Error& error)
{
    std::lock_guard<std::mutex> lock(mutex_);
    error_message_ = error.what().c_str();
    status_ = AB_LOGIN_FAILED;
    queue_ticket_ = nullptr;

    if (task_runner_)
        task_runner_->queue_task(
            [](const std::string& msg) { Con_Printf("AccelByte: Login failed - %s\n", msg.c_str()); }, error_message_);
}
