// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../lobby_exports.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class SessionTeamParty {
public:
    ACCELBYTE_LOBBY_API SessionTeamParty();
    ACCELBYTE_LOBBY_API ~SessionTeamParty();

    Vector<String> user_ids{};
    String party_id{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte