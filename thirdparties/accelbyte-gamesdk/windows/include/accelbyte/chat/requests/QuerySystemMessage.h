// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/QuerySystemMessageResponse.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace requests {

class QuerySystemMessage : public RequestWithResponse<responses::QuerySystemMessageResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API QuerySystemMessage();
    ACCELBYTE_CHAT_API virtual ~QuerySystemMessage();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    bool unread_only{false};
    unsigned int start_created_at{0};
    unsigned int end_created_at{0};
    String category{};
    unsigned int offset{0};
    unsigned int limit{10};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
