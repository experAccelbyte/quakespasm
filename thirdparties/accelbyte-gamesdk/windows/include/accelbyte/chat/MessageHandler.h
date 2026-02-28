// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "Message.h"
#include "chat_exports.h"

#include <accelbyte/common/Set.h>
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {

class MessageHandler {
public:
    ACCELBYTE_CHAT_API virtual ~MessageHandler() {}
    ACCELBYTE_CHAT_API virtual Set<String> handled_messages() const = 0;
    ACCELBYTE_CHAT_API virtual void handle_message(const Message& message) = 0;
};

} // namespace chat
} // namespace accelbyte