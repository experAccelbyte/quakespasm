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

class QueryPersonalTopicResponse : public Response {
public:
    struct QueryTopicResponseData {
        String topic_id{};
        String type{};
        String updated_at{};
        String name{};
        unsigned int unread_chats{0};
        Vector<String> members{};
    };

    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API QueryPersonalTopicResponse();
    ACCELBYTE_CHAT_API virtual ~QueryPersonalTopicResponse();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String processed{};
    Vector<QueryTopicResponseData> data{};
};

} // namespace responses
} // namespace chat
} // namespace accelbyte
