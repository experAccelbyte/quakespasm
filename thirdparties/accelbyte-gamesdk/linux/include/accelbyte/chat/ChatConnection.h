// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "chat_exports.h"

#include "MessageHandler.h"
#include "Request.h"

#include <accelbyte/memory/memory.h>

namespace accelbyte {
namespace chat {

class ChatConnection {
public:
    ACCELBYTE_CHAT_API virtual ~ChatConnection() = default;

    ACCELBYTE_CHAT_API virtual void add_message_handler(const memory::SharedPtr<MessageHandler>& handler) = 0;
    ACCELBYTE_CHAT_API virtual bool connect() = 0;
    ACCELBYTE_CHAT_API virtual void disconnect() = 0;
    ACCELBYTE_CHAT_API virtual bool is_connected() const = 0;
    ACCELBYTE_CHAT_API virtual bool read() = 0;
    ACCELBYTE_CHAT_API virtual bool wait_for_message(int timout = -1) = 0;
    ACCELBYTE_CHAT_API virtual String send_request(const Request& request) = 0;
};

}
}
