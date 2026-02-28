// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../user_exports.h"
#include <accelbyte/common/String.h>

namespace accelbyte {
namespace user {
namespace parameters {

struct LoginWithPassword {
public:
    String username{};
    String password{};
    bool remember_me{};
};

} // namespace parameters
} // namespace user
} // namespace accelbyte