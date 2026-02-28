// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "sdk_p2p_connection_exports.h"
#include "Connection.h"

#include "accelbyte/user/User.h"
#include "accelbyte/lobby/LobbyConnection.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace p2p_connection {

/**
 * @brief a class that will manage the connection between peers
 */
class ClientConnectionFactory {
public:
    ACCELBYTE_CPP_P2P_CONNECTION_API static std::shared_ptr<Connection>
    create(const String& peer_id, const std::shared_ptr<user::User>& user, const std::shared_ptr<lobby::LobbyConnection>& connection, bool force_relay = false);
};

} // namespace p2p_connection_handler
} // namespace accelbyte