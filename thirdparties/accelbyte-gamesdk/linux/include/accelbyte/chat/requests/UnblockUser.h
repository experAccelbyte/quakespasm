// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/UnblockUserResponse.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace requests {

class UnblockUser : public RequestWithResponse<responses::UnblockUserResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API UnblockUser();
    ACCELBYTE_CHAT_API virtual ~UnblockUser();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    String user_id{};
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
