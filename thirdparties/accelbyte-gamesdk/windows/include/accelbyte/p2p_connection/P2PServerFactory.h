// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "sdk_p2p_connection_exports.h"
#include "P2PServer.h"

#include "accelbyte/user/User.h"
#include "accelbyte/lobby/LobbyConnection.h"

namespace accelbyte {
namespace p2p_connection {

/**
* @brief a class that will manage the connection between peers
*/
class P2PServerFactory {
public:
    ACCELBYTE_CPP_P2P_CONNECTION_API static std::shared_ptr<P2PServer> create_server(
        const std::shared_ptr<user::User>& user, 
        const std::shared_ptr<lobby::LobbyConnection>& connection);
};

} // namespace session
} // namespace accelbyte