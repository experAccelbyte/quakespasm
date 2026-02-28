// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"

#include <accelbyte/common/DateTime.h>
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class FriendsPresenceNotif : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API FriendsPresenceNotif();
    ACCELBYTE_LOBBY_API virtual ~FriendsPresenceNotif();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    FriendsPresenceNotif(
        const String& user_id,
        const String& availability, 
        const String& activity, 
        const String& platform,
        const accelbyte::DateTime& last_seen_at);

    String user_id{};
    String availability{};
    String activity{};
    String platform{};
    accelbyte::DateTime last_seen_at{};
};

} // namespace message
} // namespace lobby
} // namespace accelbyte