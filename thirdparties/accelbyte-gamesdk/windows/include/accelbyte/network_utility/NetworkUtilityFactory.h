// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// CNL
#include "accelbyte/network_utility/NetworkUtility.h"
#include "accelbyte/network_utility/network_utility_exports.h"

// AccelByte
#include "accelbyte/common/String.h"
#include "accelbyte/memory/memory.h"

#include <mutex>

namespace accelbyte {
namespace network_utility {

/**
 * @brief interface for a network utility factory
 **/
class AB_CNL_NETWORK_UTILITY_EXPORT NetworkUtilityFactory {
public:
    NetworkUtilityFactory() = default;

    NetworkUtilityFactory(NetworkUtilityFactory const&) = delete;
    NetworkUtilityFactory(NetworkUtilityFactory&&) = delete;
    auto operator=(NetworkUtilityFactory const&) -> NetworkUtilityFactory& = delete;
    auto operator=(NetworkUtilityFactory&&) -> NetworkUtilityFactory& = delete;

    /**
     * @brief default destructor
     */
    virtual ~NetworkUtilityFactory() = default;

    /**
     * @brief create a network utility
     */
    virtual auto create_network(
        const String& address,
        const uint16_t port,
        const accelbyte::network_utility::NetworkUtility::SocketProtocol& socket_protocol,
        const accelbyte::network_utility::NetworkUtility::NetworkType& network_type) const
        -> memory::SharedPtr<network_utility::NetworkUtility> = 0;

    /**
     * @brief get the last network utility factory. If none, return nullptr
     */
    static auto get_network_utility_factory() -> const memory::SharedPtr<NetworkUtilityFactory>;

    /**
     * @brief set the network utility to be use
     */
    static auto set_network_utility_factory(const memory::SharedPtr<NetworkUtilityFactory>& factory) -> void;

private:
    static std::mutex singleton_mutex;
};
} // namespace network_utility
} // namespace accelbyte
