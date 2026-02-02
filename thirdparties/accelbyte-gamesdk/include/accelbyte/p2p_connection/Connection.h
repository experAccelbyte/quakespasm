// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/p2p_connection/TurnServerDescription.h"
#include "accelbyte/p2p_connection/TurnServerFilter.h"
#include "sdk_p2p_connection_exports.h"

#include <accelbyte/common/Vector.h>
#include <accelbyte/common/Error.h>

namespace accelbyte {
namespace p2p_connection {

/**
 * @brief a class that will manage the connection between peers
 */
class Connection {
public:
    /**
     * @brief return the user id  of the peer we are connected with
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual String peer_id() const = 0;

    /**
     * @brief try and connect to peer. This method is synchronous .
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Error connect() = 0;

    /**
     * @brief try and connect to peer. This method is synchronous .
     * @param server_to_use the server to use for this connection
     * NB: since the used turn server is determined by the client this has no effect on a "Server" connection
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Error connect(const TurnServerDescription& server_to_use) = 0;

    /**
     * @brief try and connect to peer. This method is synchronous .
     * @param filter a filter to apply at turn server selection
     * NB: since the used turn server is determined by the client this has no effect on a "Server" connection
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Error connect(const TurnServerFilter& filter) = 0;

    /**
     * @brief return true if the connection has been established.
     * NB: the conneciton is UDP so is_connected only means that the ICE protocol
     * went through not that the connection is still alive.
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual bool is_connected() const = 0;

    /**
     * @brief write the content of data to the socket
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Error write(const Vector<char>& data) = 0;

    /**
     * @brief write the content of data to the socket
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Error write(const char* data, size_t size) = 0;

    /**
     * @brief read the aggregated data received by the connection since the last call to read
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Error read(Vector<char>& data) = 0;

    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Error wait_for_data(int timeout = -1) = 0;

    ACCELBYTE_CPP_P2P_CONNECTION_API virtual bool is_using_relay() = 0;

    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Error selected_candidates(String& remote, String& local) = 0;
};

} // namespace p2p_connection_handler
} // namespace accelbyte