// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#ifdef ACCELBYTE_GAMESDK_BUILD_SHARED_DEFINE

#    ifndef ACCELBYTE_USER_API
#        ifdef _MSC_VER
#            if defined ACCELBYTE_USER_EXPORTS
#                define ACCELBYTE_USER_API __declspec(dllexport)
#            else
#                define ACCELBYTE_USER_API __declspec(dllimport)
#            endif
#        elif __GNUC__ >= 4 || defined(__clang__)
#            define ACCELBYTE_USER_API __attribute__((visibility("default")))
#        else
#            define ACCELBYTE_USER_API
#        endif
#    endif

#else

#    define ACCELBYTE_USER_API

#endif

