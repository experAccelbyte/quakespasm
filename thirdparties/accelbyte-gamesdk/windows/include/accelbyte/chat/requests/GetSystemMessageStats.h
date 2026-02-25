// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../chat_exports.h"
#include "../responses/GetSystemMessageStatsResponse.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace chat {
namespace requests {

class GetSystemMessageStats : public RequestWithResponse<responses::GetSystemMessageStatsResponse> {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API GetSystemMessageStats();
    ACCELBYTE_CHAT_API virtual ~GetSystemMessageStats();
    ACCELBYTE_CHAT_API virtual const String& id() const override;
};

} // namespace requests
} // namespace chat
} // namespace accelbyte
