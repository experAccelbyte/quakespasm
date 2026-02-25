// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/UpdateTopicResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {
namespace requests {

class UpdateTopic : public RequestWithResponse<responses::UpdateTopicResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API UpdateTopic();
    ACCELBYTE_CHAT_API virtual ~UpdateTopic();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String topic_id{};
    String name{};
    bool is_joinable{false};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
