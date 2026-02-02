// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../Message.h"
#include "../lobby_exports.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class DSNotification : public Message {
public:
    ACCELBYTE_LOBBY_API static const String& message_id();

    ACCELBYTE_LOBBY_API DSNotification();
    ACCELBYTE_LOBBY_API virtual ~DSNotification();
    ACCELBYTE_LOBBY_API virtual const String& id() const override;

    String match_id{}; 
    String ab_namespace{};
    String pod_name{};
    String region{};
    String image_version{};
    String ip{};
    Vector<String> alternate_ips{};
    int port{}; 
    Vector<int> ports{};
    String protocol{};
    String game_version{};
    String provider{};
    String status_text{};
    String deployment{};
    String custom_attribute{};
    String ok{};
    String message{};
};

} // namespace message
} // namespace lobby
} // namespace accelbyte