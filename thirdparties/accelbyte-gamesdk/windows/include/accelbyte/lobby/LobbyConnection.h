// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "lobby_exports.h"
#include "MessageHandler.h"
#include "Request.h"
#include <accelbyte/memory/memory.h>
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
    
class LobbyConnection {
public:
    ACCELBYTE_LOBBY_API virtual ~LobbyConnection() = default;

    ACCELBYTE_LOBBY_API virtual void add_message_handler(const std::weak_ptr<MessageHandler>& handler) = 0;
    ACCELBYTE_LOBBY_API virtual void remove_message_handler(const std::weak_ptr<MessageHandler>& handler) = 0;
    ACCELBYTE_LOBBY_API virtual bool connect() = 0;
    ACCELBYTE_LOBBY_API virtual void disconnect() = 0;
    ACCELBYTE_LOBBY_API virtual bool is_connected() const = 0;
    ACCELBYTE_LOBBY_API virtual bool read() = 0;
    ACCELBYTE_LOBBY_API virtual bool wait_for_message(int timout = -1) = 0;
    ACCELBYTE_LOBBY_API virtual void stop_waiting() = 0;
    ACCELBYTE_LOBBY_API virtual String send_message(const Request& message) = 0;
    ACCELBYTE_LOBBY_API virtual String send_message(const Request& message, const memory::SharedPtr<MessageHandler>& response_handler) = 0;
};

}
}