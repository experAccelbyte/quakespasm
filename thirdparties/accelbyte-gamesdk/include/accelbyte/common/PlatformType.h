// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_GAME_SDK_CPP_GAME_SDK_COMMON_INCLUDE_ACCELBYTE_COMMON_PLATFORM_TYPE_H_
#define ACCELBYTE_GAME_SDK_CPP_GAME_SDK_COMMON_INCLUDE_ACCELBYTE_COMMON_PLATFORM_TYPE_H_
#include "common_exports.h"

#include <accelbyte/common/String.h>

namespace accelbyte {
namespace common {
class PlatformType {
public:
    enum platform_type {
        STEAM,
        FACEBOOK,
        GOOGLE,
        OCULUS,
        TWITCH,
        DISCORD,
        APPLE,
        JUSTICE,
        EPIC_GAMES,
        PS4,
        PS5,
        NINTENDO,
        AWSCOGNITO,
        LIVE,
        XBLWEB,
        NETFLIX,
        SNAPCHAT
    };

    ACCELBYTE_COMMON static accelbyte::String to_platform_id(const platform_type& from);
};
} // namespace common
} // namespace accelbyte

#endif