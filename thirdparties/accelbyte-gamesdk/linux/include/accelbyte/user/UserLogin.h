// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "User.h"
#include "parameters/LoginWithPassword.h"
#include "parameters/LoginWithPlatform.h"
#include "parameters/LoginWithDeviceId.h"
#include "user_exports.h"

#include <accelbyte/common/Error.h>
#include <accelbyte/memory/memory.h>
#include <functional>

// parameters from iam
#include <accelbyte/iam/o_auth2_0_v4/Verify2faCodeV4.h>
#include <accelbyte/iam/o_auth2_0_v4/9010106bf95ea9cadlessAccountV4.h>
#include <accelbyte/iam/o_auth2_0_v4/AuthenticatePlatformLinkV4.h>
#include <accelbyte/iam/o_auth2_0_v4/GenerateTargetTokenV4.h>

#include <accelbyte/iam/models/LoginQueueTicket.h>

namespace accelbyte {
namespace user {

class UserLogin {
public:
    /**
     * @brief login a client (game)
     *
     */
    ACCELBYTE_USER_API static auto login_client(
        std::function<void(const std::shared_ptr<UserCredential>)> on_success,
        std::function<void(const Error&)> on_error) -> void;
    /**
     * @brief login a publisher (game)
     *
     */
    ACCELBYTE_USER_API static auto login_publisher(
        std::function<void(const std::shared_ptr<UserCredential>)> on_success,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief login a person (user)
     */
    ACCELBYTE_USER_API static auto login_with_password(
        const parameters::LoginWithPassword& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief login a person (user)
     */
    ACCELBYTE_USER_API static auto login_with_password_as_publisher(
        const parameters::LoginWithPassword& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief login a person (user)
     */
    ACCELBYTE_USER_API static auto login_with_platform(
        const parameters::LoginWithPlatform& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief login with steam platform
     */
    ACCELBYTE_USER_API static auto login_with_steam(
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief login with device id
     */
    ACCELBYTE_USER_API static auto login_with_device_id(
        const parameters::LoginWithDeviceId& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief login with device id in publisher namespace
     */
    ACCELBYTE_USER_API static auto login_with_device_id_as_publisher(
        const parameters::LoginWithDeviceId& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief Verify log in with new device when user enabled 2FA
     */
    ACCELBYTE_USER_API static auto verify_login_with_new_device_2fa_enabled(
        iam::o_auth2_0_v4::Verify2faCodeV4& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief Create Headless Account And Login
     */
    ACCELBYTE_USER_API static auto create_headless_account_and_login(
        iam::o_auth2_0_v4::GenerateTokenByHeadlessAccountV4& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief Authentication With Platform Link And Login
     */
    ACCELBYTE_USER_API static auto authenticate_with_platform_link_and_login(
        iam::o_auth2_0_v4::AuthenticatePlatformLinkV4& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief This function for generate publisher user's game token. required a code from request game token
     */
    ACCELBYTE_USER_API static auto generate_game_token_and_login(
        iam::o_auth2_0_v4::GenerateTargetTokenV4& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief login with queue ticket
     */
    ACCELBYTE_USER_API static auto login_with_queue_ticket(
        const memory::SharedPtr<iam::model::LoginQueueTicket>& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief login with queue ticket
     */
    ACCELBYTE_USER_API static auto login_with_queue_ticket_as_publisher(
        const memory::SharedPtr<iam::model::LoginQueueTicket>& parameters,
        std::function<void(const memory::SharedPtr<User>)> on_success,
        std::function<void(const memory::SharedPtr<iam::model::LoginQueueTicket>)> on_waiting_queue,
        std::function<void(const Error&)> on_error) -> void;
};

} // namespace user
} // namespace accelbyte
