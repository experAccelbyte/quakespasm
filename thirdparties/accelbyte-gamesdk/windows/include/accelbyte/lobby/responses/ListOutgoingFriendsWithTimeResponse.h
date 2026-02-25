// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Response.h"
#include "../lobby_exports.h"

#include <accelbyte/common/DateTime.h>

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace responses {

class ListOutgoingFriendsWithTimeResponse : public Response {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API ListOutgoingFriendsWithTimeResponse();
    ACCELBYTE_LOBBY_API virtual ~ListOutgoingFriendsWithTimeResponse();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    int code{0};
    Vector<std::pair<String, accelbyte::DateTime>> outgoing_friends{};
};

} // namespace message
} // namespace lobby
} // namespace accelbyte