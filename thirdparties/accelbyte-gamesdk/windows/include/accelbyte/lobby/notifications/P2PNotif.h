// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class P2PNotif : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API P2PNotif();
    ACCELBYTE_LOBBY_API virtual ~P2PNotif();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String destination_id{};
    String message{};

    String type{};
    int channel{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte