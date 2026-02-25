// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../lobby_exports.h"
#include "SessionTeamParty.h"

#include <accelbyte/common/String.h>
#include <accelbyte/common/Vector.h>

namespace accelbyte {
namespace lobby {
namespace notifications {

class SessionTeam {
public:
    ACCELBYTE_LOBBY_API SessionTeam();
    ACCELBYTE_LOBBY_API ~SessionTeam();

    Vector<String> user_ids{};
    Vector<SessionTeamParty> parties{};
};

} // namespace notifications
} // namespace lobby
} // namespace accelbyte