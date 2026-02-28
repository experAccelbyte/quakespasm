// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"
#include "SessionMember.h"
#include "SessionConfiguration.h"
#include "SessionTeam.h"

#include <accelbyte/common/DateTime.h>
#include <accelbyte/common/Optional.h>
#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class OnSessionMembersChanged : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API OnSessionMembersChanged();
    ACCELBYTE_LOBBY_API virtual ~OnSessionMembersChanged();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String payload{};
    accelbyte::DateTime sent_at{};

    // payload datas
    String session_id{};
    String leader_id{};
    Optional<String> joiner_id{};
    Vector<SessionMember> members{};
    Vector<SessionTeam> teams{};
    bool text_chat{};
    String ab_namespace{};
    accelbyte::DateTime created_at{};
    accelbyte::DateTime updated_at{};
    String created_by{};
    int version{};
    bool is_full{};
    String code{};
    SessionConfiguration configuration{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte