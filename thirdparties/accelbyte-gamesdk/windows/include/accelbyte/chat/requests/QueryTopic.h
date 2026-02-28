// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

// cpp-game-sdk
#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/QueryTopicResponse.h"

// CNL
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {
namespace requests {

class QueryTopic : public RequestWithResponse<responses::QueryTopicResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API QueryTopic();
    ACCELBYTE_CHAT_API virtual ~QueryTopic();
    ACCELBYTE_CHAT_API const String& id() const override;

    String ab_namespace{};
    String keyword{};
    unsigned int offset{0};
    unsigned int limit{10};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
