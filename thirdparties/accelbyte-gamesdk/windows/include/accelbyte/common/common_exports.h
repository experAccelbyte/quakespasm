// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#ifdef ACCELBYTE_GAMESDK_BUILD_SHARED_DEFINE

#    ifndef ACCELBYTE_COMMON
#        ifdef _MSC_VER
#            if defined ACCELBYTE_COMMON_EXPORTS
#                define ACCELBYTE_COMMON __declspec(dllexport)
#            else
#                define ACCELBYTE_COMMON __declspec(dllimport)
#            endif
#        elif __GNUC__ >= 4 || defined(__clang__)
#            define ACCELBYTE_COMMON __attribute__((visibility("default")))
#        else
#            define ACCELBYTE_COMMON
#        endif
#    endif

#else

#    define ACCELBYTE_COMMON

#endif
