// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_SETTINGS_ENVIRONMENTVARIABLESETTINGS_H_
#define ACCELBYTE_SETTINGS_ENVIRONMENTVARIABLESETTINGS_H_

#include "Settings.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace settings {

/**
 * @brief EnvironmentVariableSettings is a Settings implementation that reads settings from environment variables.
 *
 */
class EnvironmentVariableSettings : public Settings {
public:
    /**
     * @brief Construct a new Environment Variable Settings object
     *
     */
    ACCELBYTE_SETTINGS_API EnvironmentVariableSettings();

    /**
     * @brief Get the server url from environment variable.
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto server_url() const -> String override;

    /**
     * @brief Get the loby url from environment variable.
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto lobby_url() const -> String override;
    

    /**
     * @brief Get the loby url from environment variable.
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto chat_url() const -> String override;

    /**
     * @brief Get the client id from environment variable.
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto client_id() const -> String override;

    /**
     * @brief Get the client secret from environment variable.
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
     * @brief Set the server url env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_server_url_env_name(const String& value);

    /**
     * @brief Get the server url env name object
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto get_server_url_env_name() const -> String;

    /**
     * @brief Set the chat url env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_chat_url_env_name(const String& value);

    /**
     * @brief Get the chat url env name object
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto get_chat_url_env_name() const -> String;

    /**
     * @brief Set the lobby url env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_lobby_url_env_name(const String& value);

    /**
     * @brief Get the lobby url env name object
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto get_lobby_url_env_name() const -> String;

    /**
     * @brief Set the client id env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_client_id_env_name(const String& value);

    /**
     * @brief Get the client id env name object
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto get_client_id_env_name() const -> String;

    /**
     * @brief Set the client secret env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_client_secret_env_name(const String& value);

    /**
     * @brief Get the client secret env name object
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto get_client_secret_env_name() const -> String;

    /**
     * @brief Set the publisher id env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_publisher_id_env_name(const String& value);

    /**
     * @brief Get the publisher id env name object
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto get_publisher_id_env_name() const -> String;

    /**
     * @brief Set the publisher secret env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_publisher_secret_env_name(const String& value);

    /**
     * @brief Get the publisher secret env name object
     *
     * @return String
     */
    ACCELBYTE_SETTINGS_API auto get_publisher_secret_env_name() const -> String;

    /**
     * @brief Set the warning as error env name object, var value need to be equal to "TRUE" to be activated 
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_warning_as_error_env_name(const String& value);

    /**
     * @brief Get the warning as error env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API auto get_warning_as_error_env_name() const -> String;

    /**
     * @brief Set the log endpoint usage env name object, var value need to be equal to "TRUE" to be activated 
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API void set_log_endpoint_usage_env_name(const String& value);

    /**
     * @brief Get the log endpoint usage env name object
     *
     * @param value
     */
    ACCELBYTE_SETTINGS_API auto get_log_endpoint_usage_env_name() const -> String;

    /**
     * @brief vitual destructor
     */
    ACCELBYTE_SETTINGS_API virtual ~EnvironmentVariableSettings() noexcept = default;

private:
    String server_url_env_name_;
    String chat_url_env_name_;
    String lobby_url_env_name_;
    String client_id_env_name_;
    String client_secret_env_name_;
    String publisher_id_env_name_;
    String publisher_secret_env_name_;
    String warning_as_error_env_name_;
    String log_endpoint_usage_env_name_;
};

} // namespace settings
} // namespace accelbyte

#endif // !ACCELBYTE_SETTINGS_ENVIRONMENTVARIABLESETTINGS_H_
