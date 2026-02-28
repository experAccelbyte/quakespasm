// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"
#include "SessionMember.h"
#include "SessionConfiguration.h"

#include <accelbyte/common/DateTime.h>
#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class OnPartyUpdated : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API OnPartyUpdated();
    ACCELBYTE_LOBBY_API virtual ~OnPartyUpdated();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String payload{};
    accelbyte::DateTime sent_at{};

    // payload datas
    String party_id{};
    String leader_id{};
    Vector<SessionMember> members{};

    accelbyte::DateTime created_at{};
    accelbyte::DateTime updated_at{};
    String created_by{};
    int version{};
    bool is_full{};
    String code{};
    String ab_namespace{};

    SessionConfiguration configuration{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte