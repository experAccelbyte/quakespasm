// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "settings_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace settings {

/**
 * @brief Settings is an interface that provides settings for the SDK.
 */
class Settings {
public:
    /**
     * @brief Get the server url.
     * 
     * @return String 
     */
    virtual auto server_url() const -> String = 0;

    /**
     * @brief Get the lobby url.
     *
     * @return String
     */
    virtual auto lobby_url() const -> String = 0;
    
    /**
     * @brief Get the chat url.
     *
     * @return String
     */
    virtual auto chat_url() const -> String = 0;

    /**
     * @brief Get the client id.
     * 
     * @return String 
     */
    virtual auto client_id() const -> String = 0;

    /**
     * @brief Get the client secret.
     * 
     * @return String 
     */
    virtual auto client_secret() const -> String = 0;

    /**
     * @brief Get the publisher id.
     *
     * @return String
     */
    virtual auto publisher_id() const -> String = 0;

    /**
     * @brief Get the publisher secret.
     *
     * @return String
     */
    virtual auto publisher_secret() const -> String = 0;

    /**
     * @brief treat_warning_as_error.
     *
     * @return bool
     */
    virtual auto warning_as_error() const -> bool = 0;

    /**
     * @brief log the endpoint usage.
     *
     * @return bool
     */
    virtual auto log_endpoint_usage() const -> bool = 0;
};

} // namespace settings
} // namespace accelbyte