// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "chat_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {
    
class Message {
public:
    ACCELBYTE_CHAT_API virtual ~Message() {}
    ACCELBYTE_CHAT_API virtual const String& id() const = 0;
};

}
}