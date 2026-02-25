// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"
#include "FreeformNotificationMessage.h"

#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class OnMatchmakingTicketCanceled : public FreeformNotificationMessage {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API OnMatchmakingTicketCanceled();
    ACCELBYTE_LOBBY_API virtual ~OnMatchmakingTicketCanceled();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    //payload data
    String party_id;
    String ab_namespace;
    Vector<String> user_ids;
    String reason;

};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte