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

class DeleteSystemMessagesResponse : public Response {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API DeleteSystemMessagesResponse();
    ACCELBYTE_CHAT_API virtual ~DeleteSystemMessagesResponse();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String processed{};
};

} // namespace responses
} // namespace chat
} // namespace accelbyte
