// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// CNL
#include "accelbyte/linux_network_utility/linux_network_utility_exports.h"
#include "accelbyte/network_utility/NetworkUtilityFactory.h"

// AccelByte
#include "accelbyte/common/Map.h"
#include "accelbyte/common/String.h"
#include "accelbyte/memory/memory.h"

namespace accelbyte {
namespace linux_network_utility {

/**
 * @brief a Linux network utility factory
 **/
class AB_CNL_LINUX_NETWORK_UTILITY_EXPORT LinuxNetworkUtilityFactory : public network_utility::NetworkUtilityFactory {
public:
    /**
     * @brief default constructor
     *
     * @return
     */
    LinuxNetworkUtilityFactory() = default;

    // Uncopyable and unmoveable
    LinuxNetworkUtilityFactory(LinuxNetworkUtilityFactory const&) = delete;
    LinuxNetworkUtilityFactory(LinuxNetworkUtilityFactory&&) = delete;
    auto operator=(LinuxNetworkUtilityFactory const&) -> LinuxNetworkUtilityFactory& = delete;
    auto operator=(LinuxNetworkUtilityFactory&&) -> LinuxNetworkUtilityFactory& = delete;

    /**
     * @brief default destructor
     */
    ~LinuxNetworkUtilityFactory() override = default;

    /**
     * @brief create a windows network utility
     */
    auto create_network(
        const String& address,
        const uint16_t port,
        const network_utility::NetworkUtility::SocketProtocol& socket_protocol,
        const network_utility::NetworkUtility::NetworkType& network_type) const
        -> memory::SharedPtr<network_utility::NetworkUtility> override;
};
} // namespace linux_network_utility
} // namespace accelbyte
