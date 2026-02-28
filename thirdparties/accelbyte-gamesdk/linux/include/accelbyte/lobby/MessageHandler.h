// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "lobby_exports.h"
#include "Message.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Set.h>

namespace accelbyte {
namespace lobby {
    
class MessageHandler {
public:
  ACCELBYTE_LOBBY_API virtual ~MessageHandler(){}
  virtual Set<String> handled_messages() const = 0;
  virtual void handle_message(const Message &message) = 0;
};

}
}