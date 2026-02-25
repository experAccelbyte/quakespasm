// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../lobby_exports.h"
#include "../responses/UnblockPlayerResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace requests {

class UnblockPlayer : public RequestWithResponse<responses::UnblockPlayerResponse> {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API UnblockPlayer();
    ACCELBYTE_LOBBY_API virtual ~UnblockPlayer();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String user_id{};
    String unblocked_user_id{};
    String ab_namespace{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte