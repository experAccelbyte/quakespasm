// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// CNL
#include "accelbyte/network_utility/NetworkUtilityFactory.h"
#include "accelbyte/windows_network_utility/windows_network_utility_exports.h"

// AccelByte
#include "accelbyte/common/Map.h"
#include "accelbyte/common/String.h"
#include "accelbyte/memory/memory.h"

namespace accelbyte {
namespace windows_network_utility {

/**
 * @brief a windows network utility factory using winsock2
 **/
class AB_CNL_WINDOWS_NETWORK_UTILITY_EXPORT WindowsNetworkUtilityFactory
    : public accelbyte::network_utility::NetworkUtilityFactory {
public:
    /**
     * @brief default destructor
     */
    ~WindowsNetworkUtilityFactory() override = default;

    /**
     * @brief create a windows network utility
     */
    auto create_network(
        const String& address,
        const uint16_t port,
        const accelbyte::network_utility::NetworkUtility::SocketProtocol& socket_protocol,
        const accelbyte::network_utility::NetworkUtility::NetworkType& network_type) const
        -> memory::SharedPtr<network_utility::NetworkUtility> override;
};
} // namespace windows_network_utility
} // namespace accelbyte
