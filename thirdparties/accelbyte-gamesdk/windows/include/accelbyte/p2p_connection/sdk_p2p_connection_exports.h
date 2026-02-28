// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#ifdef ACCELBYTE_GAMESDK_BUILD_SHARED_DEFINE

#   ifndef ACCELBYTE_CPP_P2P_CONNECTION_API
#        ifdef _MSC_VER
#            if defined ACCELBYTE_CPP_P2P_CONNECTION_EXPORTS
#                define ACCELBYTE_CPP_P2P_CONNECTION_API __declspec(dllexport)
#            else
#                define ACCELBYTE_CPP_P2P_CONNECTION_API __declspec(dllimport)
#            endif
#        elif __GNUC__ >= 4 || defined(__clang__)
#            define ACCELBYTE_CPP_P2P_CONNECTION_API __attribute__((visibility("default")))
#        else
#            define ACCELBYTE_CPP_P2P_CONNECTION_API
#        endif
#    endif

#else

#    define ACCELBYTE_CPP_P2P_CONNECTION_API

#endif
