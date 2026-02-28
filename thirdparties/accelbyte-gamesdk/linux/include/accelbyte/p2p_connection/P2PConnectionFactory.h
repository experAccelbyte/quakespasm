// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/p2p_connection/P2PConnection.h"
#include "accelbyte/p2p_connection/p2p_connection_exports.h"
#include "accelbyte/memory/memory.h"

#include <mutex>

namespace accelbyte {
namespace p2p_connection {

/**
 * @brief interfaces to a p2p connection factory.
 **/
class AB_CNL_P2P_CONNECTION_EXPORT P2PConnectionFactory {
public:
    /**
     * @brief default detructor.
     **/
    virtual ~P2PConnectionFactory() = default;

    /**
     * @brief create a wp2p connection
     **/
    virtual memory::SharedPtr<P2PConnection>
    create_connection(const String& server, uint16_t server_port, const String& id, const String& pwd) const = 0;

    /**
     * @brief get the last set web socket factory. If none was set return nullptr.
     **/
    static const memory::SharedPtr<P2PConnectionFactory> get_p2p_connection_factory();

    /**
     * @brief set web socket to use.
     **/
    static void set_p2p_connection_factory(const memory::SharedPtr<P2PConnectionFactory>& factory);

private:
    static std::mutex singleton_mutex;
};

} // namespace p2p_connection
} // namespace accelbyte