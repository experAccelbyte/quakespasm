#pragma once

#include <accelbyte/user/User.h>
#include <accelbyte/iam/models/LoginQueueTicket.h>
#include <accelbyte/common/Error.h>
#include "ab_integration.h"
#include "ab_task_runner.h"

#include <functional>
#include <string>
#include <mutex>
#include <future>

class AB_Login {
public:
    AB_Login();
    ~AB_Login() = default;

    void SetTaskRunner(ABTaskRunner& tr);

    // Override the generated device ID (must be called before LoginWithDeviceId).
    void SetDeviceId(const char* id);

    // Register a C++ hook invoked on the login thread immediately after a
    // successful login.  Use this to perform async setup (e.g. lobby connect)
    // that needs the full User object before the C callback fires.
    void SetPostLoginHook(std::function<void(accelbyte::memory::SharedPtr<accelbyte::user::User>)> hook);

    void LoginWithDeviceId(ab_login_success_callback_t on_success, void* userdata);

    ab_login_status_t GetStatus()       const;
    const char*       GetUserId()       const;
    const char*       GetDisplayName()  const;
    const char*       GetErrorMessage() const;

    accelbyte::memory::SharedPtr<accelbyte::user::User> GetCurrentUser() const;

private:
    void OnLoginSuccess(accelbyte::memory::SharedPtr<accelbyte::user::User> user,
                        ab_login_success_callback_t on_success, void* userdata);
    void OnLoginQueued(accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> ticket);
    void OnLoginError(const accelbyte::Error& error);

    mutable std::mutex mutex_;
    ab_login_status_t  status_          = AB_LOGIN_IDLE;
    std::string        user_id_;
    std::string        display_name_;
    std::string        error_message_;
    std::string        device_id_;
    double             last_queue_poll_ = 0.0;

    accelbyte::memory::SharedPtr<accelbyte::user::User>                    current_user_;
    accelbyte::memory::SharedPtr<accelbyte::iam::model::LoginQueueTicket> queue_ticket_;
    std::future<void>  login_future_;

    std::function<void(accelbyte::memory::SharedPtr<accelbyte::user::User>)> post_login_hook_;

    ABTaskRunner* task_runner_ = nullptr;
};
