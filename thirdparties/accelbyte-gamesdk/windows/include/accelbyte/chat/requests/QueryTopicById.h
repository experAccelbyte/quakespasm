// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// cpp-game-sdk
#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/QueryTopicByIdResponse.h"

// CNL
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {
namespace requests {

class QueryTopicById : public RequestWithResponse<responses::QueryTopicByIdResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API QueryTopicById();
    ACCELBYTE_CHAT_API virtual ~QueryTopicById();
    ACCELBYTE_CHAT_API const String& id() const override;

    String topic_id{};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
