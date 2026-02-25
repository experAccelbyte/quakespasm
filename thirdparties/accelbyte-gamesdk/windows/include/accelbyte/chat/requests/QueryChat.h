// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// cpp-game-sdk
#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/QueryChatResponse.h"

// CNL
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {
namespace requests {

class QueryChat : public RequestWithResponse<responses::QueryChatResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API QueryChat();
    ACCELBYTE_CHAT_API virtual ~QueryChat();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String topic_id{};
    unsigned int limit{10};
    String last_chat_created_at{"0"};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
