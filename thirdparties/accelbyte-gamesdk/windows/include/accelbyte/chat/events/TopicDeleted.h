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

class TopicDeleted : public Message {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API TopicDeleted();
    ACCELBYTE_CHAT_API virtual ~TopicDeleted();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String name{};
    String topic_id{};
    String sender_id{};
};

} // namespace events
} // namespace chat
} // namespace accelbyte
