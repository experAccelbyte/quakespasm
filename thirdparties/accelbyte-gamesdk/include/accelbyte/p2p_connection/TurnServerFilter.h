// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace p2p_connection {

class TurnServerFilter
{
public:
    virtual ~TurnServerFilter() = default;
    virtual auto filter_turn_server(const String& ip, const String& region) const -> bool = 0;
};

}
} // namespace accelbyte