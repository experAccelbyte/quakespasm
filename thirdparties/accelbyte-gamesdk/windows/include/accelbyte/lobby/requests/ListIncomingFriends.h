// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../lobby_exports.h"
#include "../responses/ListIncomingFriendsResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace requests {

class ListIncomingFriends : public RequestWithResponse<responses::ListIncomingFriendsResponse> {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API ListIncomingFriends();
    ACCELBYTE_LOBBY_API virtual ~ListIncomingFriends();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;
};

} // namespace requests
} // namespace lobby
} // namespace accelbyte