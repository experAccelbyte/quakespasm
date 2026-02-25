// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "Settings.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace settings {

/**
 * @brief InMemorySettings is a class that provides settings for the SDK.
 * 
 */
class InMemorySettings : public Settings {
public:

    /**
     * @brief Get the server url.
     * 
     * @return String 
     */
    ACCELBYTE_SETTINGS_API auto server_url() const -> String override;

    /**
     * @brief Get the lobby url.
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API virtual auto lobby_url() const -> String override;

    /**
     * @brief Get the lobby url.
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API virtual auto chat_url() const -> String override;

    /**
     * @brief Get the client id.
     * 
     * @return String 
     */
    ACCELBYTE_SETTINGS_API auto client_id() const -> String override;

    /**
     * @brief Get the client secret.
     * 
     * @return String 
     */
    ACCELBYTE_SETTINGS_API auto client_secret() const -> String override;

    /**
     * @brief Get the publisher id.
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto publisher_id() const -> String override;

    /**
     * @brief Get the publisher secret.
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto publisher_secret() const -> String override;

    /**
     * @brief treat_warning_as_error.
     *
     * @return bool
     */
    ACCELBYTE_SETTINGS_API auto warning_as_error() const -> bool override;

    /**
     * @brief log the endpoint usage.
     *
     * @return bool
     */
    ACCELBYTE_SETTINGS_API auto log_endpoint_usage() const -> bool override;


    /**
     * @brief Set the server url object
     * 
     * @param value 
     */
    ACCELBYTE_SETTINGS_API void set_server_url(const String& value);

    /**
     * @brief Set the chat url object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_chat_url(const String& value);

    /**
     * @brief Set the lobby url object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_lobby_url(const String& value);

    /**
     * @brief Set the client id object
     * 
     * @param value 
     */
    ACCELBYTE_SETTINGS_API void set_client_id(const String& value);

    /**
     * @brief Set the client secret object
     * 
     * @param value 
     */
    ACCELBYTE_SETTINGS_API void set_client_secret(const String& value);

    /**
     * @brief Set the client id object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_publisher_id(const String& value);

    /**
     * @brief Set the client secret object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_publisher_secret(const String& value);

    /**
     * @brief Set the warning as error
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_warning_as_error(bool value);

    /**
     * @brief Set the log endpoint usage
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_log_endpoint_usage(bool value);

    /**
     * @brief virtual destructor
     * 
     */
    ACCELBYTE_SETTINGS_API virtual ~InMemorySettings() noexcept = default;

private:
    String server_url_{};

    String chat_url_{};

    String lobby_url_{};

    String client_id_{};

    String client_secret_{};

    String publisher_id_{};

    String publisher_secret_{};

    bool warning_as_error_{false};

    bool log_endpoint_usage_{false};
};

} // namespace settings
} // namespace accelbyte