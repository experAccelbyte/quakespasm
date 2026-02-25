// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../user_exports.h"
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace user {
namespace parameters {

struct LoginWithDeviceId {
public:
    String device_id{};
    bool create_headless{true};
};

} // namespace parameters
} // namespace user
} // namespace accelbyte