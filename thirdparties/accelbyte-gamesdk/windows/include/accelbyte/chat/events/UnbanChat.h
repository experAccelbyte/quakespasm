// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../chat_exports.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace events {

class UnbanChat : public Message {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API UnbanChat();
    ACCELBYTE_CHAT_API virtual ~UnbanChat();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String user_id{};
    String name_space{};
    String ban{};
    String end_date{};
    String reason{};
    bool enable{false};
};

} // namespace events
} // namespace chat
} // namespace accelbyte
