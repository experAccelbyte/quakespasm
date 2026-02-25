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

class OnMatchFound : public FreeformNotificationMessage {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API OnMatchFound();
    ACCELBYTE_LOBBY_API virtual ~OnMatchFound();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    //payload data
    String match_id;
    String ab_namespace;
    String created_at;
    String match_pool;
    Vector<Vector<String>> teams;
    Vector<String> tickets;
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte