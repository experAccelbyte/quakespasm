// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// cpp-game-sdk
#include "User.h"
#include "user_exports.h"

// CNL
#include <accelbyte/common/Error.h>
#include <accelbyte/common/String.h>

#include <accelbyte/iam/users_v4/CreateTestUserV4.h>
#include <accelbyte/iam/users_v4/CreateUserV4.h>
#include <accelbyte/iam/models/AccountCreateUserV4.h>

namespace accelbyte {
namespace user {

class UserManagement {
public:
    /**
     * @brief create a user
     *
     * @param client_credential the client credential created using game client id and secret
     * @param parameters the request parameter
     * @param on_sucess the on sucess callback
     * @param on_error the on error callback
     */
    ACCELBYTE_USER_API static auto create_user(
        accelbyte::iam::users_v4::CreateUserV4& parameters,
        std::function<void(const accelbyte::iam::model::AccountCreateUserV4&)> on_success,
        std::function<void(const Error&)> on_error) -> void;
    /**
     * @brief create a publisher user
     *
     * @param client_credential the client credential created using game client id and secret
     * @param parameters the request parameter
     * @param on_sucess the on sucess callback
     * @param on_error the on error callback
     */
    ACCELBYTE_USER_API static auto create_publisher_user(
        accelbyte::iam::users_v4::CreateUserV4& parameters,
        std::function<void(const accelbyte::iam::model::AccountCreateUserV4&)> on_success,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief create a test user (that does not required email confirmation)
     *
     * @param client_credential the client credential created using game client id and secret
     * @param parameters the request parameter
     * @param on_sucess the on sucess callback
     * @param on_error the on error callback
     */
    ACCELBYTE_USER_API static auto create_test_user(
        accelbyte::iam::users_v4::CreateTestUserV4& parameters,
        std::function<void(const accelbyte::iam::model::AccountCreateUserV4&)> on_success,
        std::function<void(const Error&)> on_error) -> void;

    /**
     * @brief create a test user at publisher level (that does not required email confirmation)
     *
     * @param client_credential the client credential created using game client id and secret
     * @param parameters the request parameter
     * @param on_sucess the on sucess callback
     * @param on_error the on error callback
     */
    ACCELBYTE_USER_API static auto create_publisher_test_user(
        accelbyte::iam::users_v4::CreateTestUserV4& parameters,
        std::function<void(const accelbyte::iam::model::AccountCreateUserV4&)> on_success,
        std::function<void(const Error&)> on_error) -> void;
};

} // namespace user
} // namespace accelbyte
