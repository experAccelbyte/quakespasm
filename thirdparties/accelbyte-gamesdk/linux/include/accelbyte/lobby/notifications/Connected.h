// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class Connected : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API Connected();
    ACCELBYTE_LOBBY_API virtual ~Connected();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String login_type{};
    int reconnect_from_code{0};
    String lobby_session_id{};
};

} // namespace message
} // namespace lobby
} // namespace accelbyte