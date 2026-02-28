// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/AddUserToTopicResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace chat {
namespace requests {

class AddUserToTopic : public RequestWithResponse<responses::AddUserToTopicResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API AddUserToTopic();
    ACCELBYTE_CHAT_API virtual ~AddUserToTopic();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String topic_id{};
    String user_id{};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte