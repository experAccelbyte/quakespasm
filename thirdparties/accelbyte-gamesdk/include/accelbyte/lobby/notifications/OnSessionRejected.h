// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"
#include "SessionMember.h"

#include <accelbyte/common/DateTime.h>
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class OnSessionRejected : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API OnSessionRejected();
    ACCELBYTE_LOBBY_API virtual ~OnSessionRejected();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String payload{};
    accelbyte::DateTime sent_at{};

    // payload datas
    String session_id{};
    Vector<SessionMember> members{};
    String rejected_id{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte