// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../lobby_exports.h"

#include "../Message.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class AcceptFriendsNotif : public Message
{
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API AcceptFriendsNotif();
    ACCELBYTE_LOBBY_API virtual ~AcceptFriendsNotif();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String friend_id{};
};

} // namespace message
} // namespace lobby
} // namespace accelbyte