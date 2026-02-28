// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class MatchmakingNotification : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API MatchmakingNotification();
    ACCELBYTE_LOBBY_API virtual ~MatchmakingNotification();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String status{};
    String match_id{};
    String ticket_id{};
    Vector<String> party_member_ids{};
    Vector<String> counter_party_member_ids{};
    String game_mode{};
    String deployment{};
    String client_version{};
    String joinable{};
    String matching_allies{};
    String ready_duration{};
    String message{};
    String error_code{0};
};

} // namespace message
} // namespace lobby
} // namespace accelbyte