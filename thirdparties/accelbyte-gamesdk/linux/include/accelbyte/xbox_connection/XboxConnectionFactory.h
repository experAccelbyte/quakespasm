// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/platform_connection/PlatformConnectionFactory.h"
#include "accelbyte/xbox_connection/xbox_connection_exports.h"

namespace accelbyte {
namespace xbox_connection {

/**
 * @brief steam implementation of the platform connection factory
 **/
class AB_CNL_XBOX_CONNECTION_EXPORT XboxConnectionFactory : public platform_connection::PlatformConnectionFactory
{
public:
    /**
     * @brief default detructor.
     **/
    virtual ~XboxConnectionFactory() = default;

    /**
     * @brief return the type of platform this factory will create connection to
     **/
    virtual platform_type platform() const override;

};

} // namespace xbox_connection
} // namespace accelbyte