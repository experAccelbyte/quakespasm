// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "Message.h"
#include "chat_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {

class Response : public Message {
public:
    String response_message_id;
};

} // namespace chat
} // namespace accelbyte