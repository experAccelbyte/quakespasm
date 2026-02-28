// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/platform_connection/platform_connection_exports.h"
#include "accelbyte/platform_connection/PlatformConnectionParameters.h"

#include <accelbyte/common/Error.h>
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace platform_connection {

/**
 * @brief interface for a platform connection
 **/
class AB_CNL_PLATFORM_CONNECTION_EXPORT PlatformConnection {
public:
    virtual ~PlatformConnection() = default;

    /** 
    * @brief return a tocken to the platform or an empty string if the tocken grab failed
    */
    virtual String get_platform_token(const PlatformConnectionParameters& parameters, String& error) = 0;
};

} // namespace steam_connection
} // namespace accelbyte