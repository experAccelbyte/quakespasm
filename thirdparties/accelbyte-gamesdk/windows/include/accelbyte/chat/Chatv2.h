// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "ChatConnection.h"
#include "accelbyte/user/User.h"
#include "chat_exports.h"

// CNL
#include <accelbyte/common/Map.h>
#include <accelbyte/memory/memory.h>

namespace accelbyte {
namespace chat {

class Chat {
public:
    ACCELBYTE_CHAT_API Chat();
    ACCELBYTE_CHAT_API ~Chat();

    ACCELBYTE_CHAT_API memory::SharedPtr<ChatConnection> create_connection(user::User& credential);
    ACCELBYTE_CHAT_API memory::SharedPtr<ChatConnection> create_connection(user::User& credential, const String& entitlement_token);
};

} // namespace chat
} // namespace accelbyte
