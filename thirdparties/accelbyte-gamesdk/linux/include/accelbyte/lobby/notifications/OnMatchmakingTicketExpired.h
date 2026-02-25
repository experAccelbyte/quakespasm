// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"
#include "FreeformNotificationMessage.h"


namespace accelbyte {
namespace lobby {
namespace notifications {

class OnMatchmakingTicketExpired : public FreeformNotificationMessage {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API OnMatchmakingTicketExpired();
    ACCELBYTE_LOBBY_API virtual ~OnMatchmakingTicketExpired();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    //payload data
    String ticket_id;
    String ab_namespace;
    String created_at;
    String match_pool;
    String reason;
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte