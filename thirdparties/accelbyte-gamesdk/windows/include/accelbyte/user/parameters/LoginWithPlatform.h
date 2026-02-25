// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../user_exports.h"
#include <accelbyte/common/String.h>
#include <accelbyte/common/Optional.h>
#include <accelbyte/common/PlatformType.h>

namespace accelbyte {
namespace user {
namespace parameters {

struct LoginWithPlatform {
public:
    using platform_type = common::PlatformType::platform_type;

    platform_type platform;
    String platform_token;
    Optional<bool> create_headless;
    Optional<bool> skip_set_cookie;
    Optional<String> additional_data;
};

} // namespace parameters
} // namespace user
} // namespace accelbyte