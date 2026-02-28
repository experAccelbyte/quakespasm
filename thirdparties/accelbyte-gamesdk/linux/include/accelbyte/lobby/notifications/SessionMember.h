// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../lobby_exports.h"

#include <accelbyte/common/DateTime.h>
#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class SessionMember {
public:
    ACCELBYTE_LOBBY_API SessionMember();
    ACCELBYTE_LOBBY_API ~SessionMember();

    String id{};
    String status{};
    String status_v2{};
    accelbyte::DateTime updated_at{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte