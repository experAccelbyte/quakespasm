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

class QueryPublicTopicResponse : public Response {
public:
    struct QueryPublicTopicResponseData {
        String topic_id{};
        String name{};
        unsigned int number_of_members{0};
    };

    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API QueryPublicTopicResponse();
    ACCELBYTE_CHAT_API virtual ~QueryPublicTopicResponse();
    ACCELBYTE_CHAT_API const String& id() const override;
    
    String processed{};
    Vector<QueryPublicTopicResponseData> data{};
};

} // namespace responses
} // namespace chat
} // namespace accelbyte
