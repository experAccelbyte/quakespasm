// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/UpdateSystemMessagesResponse.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace requests {

class UpdateSystemMessages : public RequestWithResponse<responses::UpdateSystemMessagesResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API UpdateSystemMessages();
    ACCELBYTE_CHAT_API virtual ~UpdateSystemMessages();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    struct UpdateSystemMessagesData {
        String inbox_id{};
        String keep{"NONE"}; // "YES", "NO", "NONE"
        String read{"NONE"}; // "YES", "NO", "NONE"
    };

    Vector<UpdateSystemMessagesData> data{};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
