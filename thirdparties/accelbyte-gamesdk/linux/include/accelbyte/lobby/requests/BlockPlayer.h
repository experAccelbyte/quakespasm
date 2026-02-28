// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../lobby_exports.h"
#include "../responses/BlockPlayerResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace requests {

class BlockPlayer : public RequestWithResponse<responses::BlockPlayerResponse> {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API BlockPlayer();
    ACCELBYTE_LOBBY_API virtual ~BlockPlayer();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String user_id{};
    String blocked_user_id{};
    String ab_namespace{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte