// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../lobby_exports.h"
#include "../responses/RefreshTokenResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace requests {

class RefreshToken : public RequestWithResponse<responses::RefreshTokenResponse> {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API RefreshToken();
    ACCELBYTE_LOBBY_API virtual ~RefreshToken();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String token{};
};

} // namespace requests
} // namespace lobby
} // namespace accelbyte