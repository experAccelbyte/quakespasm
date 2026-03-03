#pragma once

#include <accelbyte/user/User.h>
#include <accelbyte/iam/models/LoginQueueTicket.h>
#include <accelbyte/settings/InMemorySettings.h>
#include <accelbyte/common/Error.h>
#include "ab_integration.h"
#include "ab_task_runner.h"

extern "C" {
#include "quakedef.h"
}

#include <string>
#include <mutex>
#include <future>

class ABInstance {
public:
    ABInstance(cvar_t* cvar_url, cvar_t* cvar_client_id, cvar_t* cvar_client_secret);
    ~ABInstance() = default;

    void SetServerUrl(const char* url);
    void SetClientId(const char* id);
    void SetClientSecret(const char* secret);

    void LoginWithDeviceId();
    void Update();
    void UpdateUserStat(const char* stat_code, float value, int strategy);

    ab_login_status_t GetLoginStatus() const;
    const char* GetUserId() const;
    const char* GetDisplayName() const;
    const char* GetErrorMessage() const;

private:
    void OnLoginSuccess(accelbyte::memory::SharedPtr<accelbyte::user::User> user);
    void OnLoginQueued(accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> ticket);
    void OnLoginError(const accelbyte::Error& error);

    mutable std::mutex mutex_;
    ab_login_status_t login_status_ = AB_LOGIN_IDLE;
    std::string user_id_;
    std::string display_name_;
    std::string error_message_;
    std::string device_id_;
    double last_queue_poll_ = 0.0;

    accelbyte::memory::SharedPtr<accelbyte::user::User> current_user_;
    accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> queue_ticket_;
    accelbyte::settings::InMemorySettings settings_;
    ABTaskRunner task_runner_;
    std::future<void> login_future_;

    cvar_t* cvar_server_url_    = nullptr;
    cvar_t* cvar_client_id_     = nullptr;
    cvar_t* cvar_client_secret_ = nullptr;
};
