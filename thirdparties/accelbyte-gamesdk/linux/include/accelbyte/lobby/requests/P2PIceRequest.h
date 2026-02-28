// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "P2PBaseRequest.h"
#include "../lobby_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace requests {

class P2PIceRequest : public P2PBaseRequest {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API P2PIceRequest();
    ACCELBYTE_LOBBY_API virtual ~P2PIceRequest();
    ACCELBYTE_LOBBY_API virtual const String& type() const override;

    String data{};
    String host{};
    int port{-1};
    String user_id{};
    String password{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte