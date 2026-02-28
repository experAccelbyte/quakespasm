// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Response.h"
#include "../chat_exports.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace responses {

class GetSystemMessageStatsResponse : public Response {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API GetSystemMessageStatsResponse();
    ACCELBYTE_CHAT_API virtual ~GetSystemMessageStatsResponse();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String processed{};
    String oldest_unread{};
    unsigned int unread{0};
};

} // namespace responses
} // namespace chat
} // namespace accelbyte
