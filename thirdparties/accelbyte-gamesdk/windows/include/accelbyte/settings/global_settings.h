// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "Settings.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace settings {

namespace internal {
extern ACCELBYTE_SETTINGS_API Settings* global_settings;
} // namespace details

/**
 * @brief Get the default global settings.
 * 
 * @return Settings& 
 */
auto ACCELBYTE_SETTINGS_API get_default_global_settings() noexcept -> Settings&;

/**
 * @brief Set the global settings object
 * 
 * @param value 
 */
void ACCELBYTE_SETTINGS_API set_global_settings(Settings& value) noexcept;

/**
 * @brief Get the global settings.
 * 
 * @return Settings& 
 */
auto ACCELBYTE_SETTINGS_API get_global_settings() noexcept -> Settings&;

/**
 * @brief Get the project version.
 *
 * @return String&
 */
auto ACCELBYTE_SETTINGS_API get_project_version() noexcept -> const String&;

/**
 * @brief log all the endpoint usage to a given folder.
 *
 * @return void&
 */
auto ACCELBYTE_SETTINGS_API log_endpoint_usage(const String& folder) noexcept -> const void;

} // namespace settings
} // namespace accelbyte