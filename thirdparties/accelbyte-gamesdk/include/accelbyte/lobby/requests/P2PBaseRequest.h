// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../RequestWithResponse.h"
#include "../lobby_exports.h"
#include "../responses/P2PResponse.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace requests {

class P2PBaseRequest : public RequestWithResponse<responses::P2PResponse> {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API P2PBaseRequest();
    ACCELBYTE_LOBBY_API virtual ~P2PBaseRequest();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;
    ACCELBYTE_LOBBY_API virtual const String& type() const;

    String destination_id{};
    int channel{-1};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte