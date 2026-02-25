// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#ifndef AB_CNL_LINUX_NETWORK_UTILITY_EXPORT
#    if (__GNUC__ >= 4 || defined(__clang__)) && defined(ACCELBYTE_BUILD_SHARED_DEFINE)
#        define AB_CNL_LINUX_NETWORK_UTILITY_EXPORT __attribute__((visibility("default")))
#    else
#        define AB_CNL_LINUX_NETWORK_UTILITY_EXPORT
#    endif
#endif
