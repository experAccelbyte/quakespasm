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

class OnSessionInvited : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API OnSessionInvited();
    ACCELBYTE_LOBBY_API virtual ~OnSessionInvited();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String payload{};
    accelbyte::DateTime sent_at{};

    String session_id{};
    String sender_id{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte