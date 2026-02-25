// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../chat_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {
namespace events {

class NewChat : public Message {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API NewChat();
    ACCELBYTE_CHAT_API virtual ~NewChat();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String chat_id{};
    String message{};
    String created_at{};
    String topic_id{};
    String from_user_id{};
    String sender_type{};
};

} // namespace message
} // namespace chat
} // namespace accelbyte