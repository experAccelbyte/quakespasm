// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Response.h"
#include "../lobby_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace responses {

class CancelFriendsResponse : public Response {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API CancelFriendsResponse();
    ACCELBYTE_LOBBY_API virtual ~CancelFriendsResponse();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    int code{0};
};

} // namespace message
} // namespace lobby
} // namespace accelbyte