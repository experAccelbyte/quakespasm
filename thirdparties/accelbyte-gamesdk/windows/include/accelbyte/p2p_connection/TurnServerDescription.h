// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace p2p_connection {

struct TurnServerDescription {
    String url_{};
    int port_{0};
    String user_name_{};
    String password_{};
};

}
} // namespace accelbyte