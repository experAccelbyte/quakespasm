// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "P2PNotif.h"
#include "../lobby_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class P2PHostingReplyNotif : public P2PNotif {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API P2PHostingReplyNotif();
    ACCELBYTE_LOBBY_API virtual ~P2PHostingReplyNotif();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    bool hosting{false};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte