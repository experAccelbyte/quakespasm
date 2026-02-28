// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// cpp-game-sdk
#include "../Response.h"
#include "../chat_exports.h"

// CNL
#include <accelbyte/common/Map.h>
#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace responses {

class QueryChatResponse : public Response {
public:
    struct QueryTopicResponseData {
        String chat_id{};
        String topic_id{};
        String message{};
        String created_at{};
        String from{};
        String read_at{};
    };

    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API QueryChatResponse();
    ACCELBYTE_CHAT_API virtual ~QueryChatResponse();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String processed{};
    Vector<QueryTopicResponseData> data{};
};

} // namespace responses
} // namespace chat
} // namespace accelbyte
