// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "user_exports.h"

#include <chrono>
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace user {

/**
 * @brief Token class to hold the token value and the expiration time
 *
 */
class Token {
public:
    /**
     * @brief Construct a new Token object
     *
     * @param value
     * @param expires_in
     */
    ACCELBYTE_USER_API Token(String value, int expires_in);

    /**
     * @brief Construct a new Token object
     *
     * @param value
     * @param expires_in
     */
    ACCELBYTE_USER_API Token(String value, std::chrono::seconds expires_in);

    /**
     * @brief Construct a new Token object
     *
     * @param value
     * @param expires_in
     * @param current_time
     */
    ACCELBYTE_USER_API Token(String value, std::chrono::seconds expires_in, std::chrono::system_clock::time_point current_time);

    /**
     * @brief the token value
     *
     * @return const String&
     */
    ACCELBYTE_USER_API auto value() const->const String&;

    /**
     * @brief get the information if the token is expired or not
     *
     * @return true if the token is expired
     */
    ACCELBYTE_USER_API auto is_expired() const->bool;

    /**
     * @brief get the remain duration before expired
     *
     * @return std::chrono::seconds
     */
    ACCELBYTE_USER_API auto expired_in() const->std::chrono::seconds;

    /**
     * @brief the token is convertible to string
     *
     * @return String
     */
    ACCELBYTE_USER_API operator String() const;

private:
    String value_{};

    std::chrono::system_clock::time_point current_time_;

    std::chrono::seconds expires_duration_;
};

} // namespace ags
} // namespace accelbyte