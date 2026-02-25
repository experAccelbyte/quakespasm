// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../lobby_exports.h"
#include "../responses/ListOfFriendsResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace requests {

class ListFriends : public RequestWithResponse<responses::ListOfFriendsResponse> {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API ListFriends();
    ACCELBYTE_LOBBY_API virtual ~ListFriends();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;
};

} // namespace requests
} // namespace lobby
} // namespace accelbyte