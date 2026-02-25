// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../lobby_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class SessionConfiguration {
public:
    ACCELBYTE_LOBBY_API SessionConfiguration();
    ACCELBYTE_LOBBY_API ~SessionConfiguration();

    String name{};
    String joinability{};
    int min_player{};
    int max_player{};
    String client_version{};
    String type{};
    int invite_timeout{};
    int inactive_timeout{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte