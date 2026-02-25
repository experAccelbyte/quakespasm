// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/p2p_connection/P2PConnectionFactory.h"
#include "accelbyte/libjuice_p2p_connection/libjuice_p2p_connection_exports.h"
#include "accelbyte/memory/memory.h"

#include <functional>

namespace accelbyte {
namespace libjuice_p2p_connection {

/**
 * @brief a p2p connection factory using libjuice.
 **/
class AB_CNL_LIBJUICE_P2P_CONNECTION_EXPORT LibjuiceP2PConnectionFactory : public p2p_connection::P2PConnectionFactory {
public:
    /**
     * @brief default detructor.
     **/
    virtual ~LibjuiceP2PConnectionFactory() = default;

    /**
     * @brief create a wp2p connection
     **/
    virtual memory::SharedPtr<p2p_connection::P2PConnection> create_connection(const String& server, uint16_t server_port, const String& id, const String& pwd) const override;


    enum log_level {
        LOG_LEVEL_VERBOSE = 0,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_FATAL,
        LOG_LEVEL_NONE
    };

    /**
    * @brief set the log level that will be used for all the connection created by this factory
    */
    static auto set_log_level(log_level level) -> void;

    /**
     * @brief set the log handler function
     */
    static auto set_log_handler(std::function<void(log_level, String)> handler) -> void;
};

} // namespace p2p_connection
} // namespace accelbyte