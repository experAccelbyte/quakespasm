// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "lobby_exports.h"
#include "LobbyConnection.h"
#include "accelbyte/user/User.h"

#include <accelbyte/memory/memory.h>
#include <accelbyte/common/Map.h>


namespace accelbyte {
namespace lobby {
    
class Lobby {
public:
    ACCELBYTE_LOBBY_API Lobby();
    ACCELBYTE_LOBBY_API ~Lobby();

    ACCELBYTE_LOBBY_API memory::SharedPtr<LobbyConnection> create_connection(user::User& credential);
    ACCELBYTE_LOBBY_API memory::SharedPtr<LobbyConnection> create_connection(user::User& credential, const String& entitlement_token);

    ACCELBYTE_LOBBY_API static std::shared_ptr<Message> decode_message(const String& message);
};

}
}