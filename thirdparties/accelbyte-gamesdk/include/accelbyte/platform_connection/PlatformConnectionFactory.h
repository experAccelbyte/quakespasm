// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/platform_connection/PlatformConnection.h"
#include "accelbyte/platform_connection/platform_connection_exports.h"
#include "accelbyte/memory/memory.h"

namespace accelbyte {
namespace platform_connection {

/**
 * @brief interfaces to a p2p connection factory.
 **/
class AB_CNL_PLATFORM_CONNECTION_EXPORT PlatformConnectionFactory {
public:

    enum platform_type {
        STEAM,
        XBOX
    };

    /**
     * @brief default detructor.
     **/
    virtual ~PlatformConnectionFactory() = default;

    /**
     * @brief return the type of platform this factory will create connection to
     **/
    virtual platform_type platform() const = 0;

    /**
     * @brief create a platform comnection
     **/
    virtual memory::SharedPtr<PlatformConnection> create_connection() const = 0;

    /**
     * @brief get the platform connection factory set for the platform
     **/
    static const memory::SharedPtr<PlatformConnectionFactory> get_platform_connection_factory(platform_type platform);

    /**
     * @brief set a platform connection factory to use for a given platform
     **/
    static void set_platform_connection_factory(const memory::SharedPtr<PlatformConnectionFactory>& factory);
};

} // namespace steam_connection
} // namespace accelbyte