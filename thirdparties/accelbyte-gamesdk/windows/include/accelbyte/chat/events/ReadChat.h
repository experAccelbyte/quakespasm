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

class ReadChat : public Message {
public:
    struct ReadChatResult
    {
        String topic_id{};
        Vector<String> chat_ids{};
    };

    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API ReadChat();
    ACCELBYTE_CHAT_API virtual ~ReadChat();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    Vector<ReadChatResult> read_chats{};
};

} // namespace message
} // namespace chat
} // namespace accelbyte