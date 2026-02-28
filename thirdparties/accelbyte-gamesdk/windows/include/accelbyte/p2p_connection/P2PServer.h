// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "sdk_p2p_connection_exports.h"
#include "Connection.h"

#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace p2p_connection {

/**
* @brief an interface to a class that will let you manage a p2p server
*/
class P2PServer {
public:
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual ~P2PServer() = default;

    
    /**
     * @brief set connection accepted to the value
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual void set_connection_accepted(bool accepted, bool force_relay = false) = 0;

    /**
    * @brief wait for a connection to be requested
    */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual void wait_for_connection_request(int timout = -1) = 0;

    /**
     * @brief return the connection that have been requested so far
     */
    ACCELBYTE_CPP_P2P_CONNECTION_API virtual Vector<std::shared_ptr<Connection>> requested_connections() = 0;
};

} // namespace session
} // namespace accelbyte