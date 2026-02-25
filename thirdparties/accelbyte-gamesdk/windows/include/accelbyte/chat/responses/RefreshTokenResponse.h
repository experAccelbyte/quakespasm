// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Response.h"
#include "../chat_exports.h"

#include <accelbyte/common/String.h>

#include <cstdint>

namespace accelbyte {
namespace chat {
namespace responses {

class RefreshTokenResponse : public Response {
public:
    ACCELBYTE_CHAT_API static const String& message_id();

    ACCELBYTE_CHAT_API RefreshTokenResponse();
    ACCELBYTE_CHAT_API virtual ~RefreshTokenResponse();
    ACCELBYTE_CHAT_API virtual const String& id() const override;

    std::int64_t code{};
};

} // namespace message
} // namespace chat
} // namespace accelbyte