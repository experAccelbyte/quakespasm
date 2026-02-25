// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "Token.h"
#include "TokenUpdateListener.h"
#include "user_exports.h"

//
#include <accelbyte/tls/SecurityAuthorization.h>

//
#include <accelbyte/common/Error.h>
#include <accelbyte/common/Map.h>
#include <accelbyte/common/String.h>

// STL
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

namespace accelbyte {
namespace user {

class UserCredential {
public:
    /**
     * @brief Construct a new User Credential object
     *
     */
    ACCELBYTE_USER_API UserCredential();

    /**
     * @brief Construct a new User Credential object
     *
     * @param access_token
     * @param refresh_token
     */
    ACCELBYTE_USER_API UserCredential(Token access_token, Token refresh_token, const String& nmsp);

    /**
     * @brief get the current Access Token. The token can be expired
     */
    ACCELBYTE_USER_API auto access_token() const noexcept -> Token;

    /**
     * @brief get the Access Token. The token will refreshed if near expired
     */
    ACCELBYTE_USER_API auto get_valid_access_token()->Token;

    /**
     * @brief refresh the credential. The access token will be refreshed
     *
     * @return UserCredential& self object with updated credential
     */
    ACCELBYTE_USER_API auto refresh() -> void;

    /**
     * @brief Get the game client secret
     */
    ACCELBYTE_USER_API auto security_authorization() const
        -> const memory::SharedPtr<accelbyte::tls::SecurityAuthorization>&;

    ACCELBYTE_USER_API auto add_token_listener(const std::shared_ptr<TokenUpdateListener>& listener) -> void;
    ACCELBYTE_USER_API auto remove_token_listener(const std::shared_ptr<TokenUpdateListener>& listener) -> void;

private:
    void on_token_refreshed(const UserCredential& refreshed);
    void on_refresh_failed(const Error& error);

    Token access_token_;
    Token refresh_token_;

    memory::SharedPtr<accelbyte::tls::SecurityAuthorization> security_authorization_;

    auto is_access_token_near_expired() -> bool;

    mutable std::mutex token_mutex;
    mutable std::condition_variable cv;
    mutable std::mutex cv_m;
    bool refreshed_ = false;

    mutable std::mutex token_listener_mutex;
    Map<TokenUpdateListener*, std::weak_ptr<TokenUpdateListener>> token_listeners_;
};

} // namespace user
} // namespace accelbyte