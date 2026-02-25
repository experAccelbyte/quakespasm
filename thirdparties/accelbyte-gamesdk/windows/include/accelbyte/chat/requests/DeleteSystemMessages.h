// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/DeleteSystemMessagesResponse.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace requests {

class DeleteSystemMessages : public RequestWithResponse<responses::DeleteSystemMessagesResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API DeleteSystemMessages();
    ACCELBYTE_CHAT_API virtual ~DeleteSystemMessages();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    Vector<String> message_ids{};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
