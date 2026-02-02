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
#include <accelbyte/utils/JsonObjectString.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class OnGameSessionUpdated : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API OnGameSessionUpdated();
    ACCELBYTE_LOBBY_API virtual ~OnGameSessionUpdated();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String payload{};
    accelbyte::DateTime sent_at{};

    // payload datas
    String session_id{};
    String leader_id{};
    Vector<SessionMember> members{};
    Vector<SessionTeam> teams{};
    String ab_namespace{};
    accelbyte::DateTime created_at{};
    accelbyte::DateTime updated_at{};
    String created_by{};
    int version{};
    bool is_full{};
    String code{};
    SessionConfiguration configuration{};
    utils::JsonObjectString attributes{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte