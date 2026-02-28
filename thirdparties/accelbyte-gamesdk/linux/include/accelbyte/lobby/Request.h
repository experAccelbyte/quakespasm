// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "lobby_exports.h"
#include "Message.h"
#include "Response.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {

class Request: public Message {
public:
    mutable String request_message_id;
};

}
}