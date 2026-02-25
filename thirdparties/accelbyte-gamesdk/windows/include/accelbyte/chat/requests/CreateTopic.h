// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/CreateTopicResponse.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace requests {

class CreateTopic : public RequestWithResponse<responses::CreateTopicResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API CreateTopic();
    ACCELBYTE_CHAT_API virtual ~CreateTopic();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String type{"PERSONAL"};

    // GROUP chat
    String topic_id{};
    String name{};
    bool is_joinable{false};
    Vector<String> admins{};
    Vector<String> members{};

    // PERSONAL chat
    String first_member_id{};
    String second_member_id{};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
