// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../lobby_exports.h"
#include "../responses/RequestFriendsResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace requests {

class RequestFriend : public RequestWithResponse<responses::RequestFriendsResponse> {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API RequestFriend();
    ACCELBYTE_LOBBY_API virtual ~RequestFriend();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String friend_id{};
};

} // namespace requests
} // namespace lobby
} // namespace accelbyte