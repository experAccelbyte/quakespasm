// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#ifndef AB_CNL_NETWORK_UTILITY_EXPORT
#    ifdef _MSC_VER
#        ifdef ACCELBYTE_BUILD_SHARED_DEFINE
#            ifdef AB_CNL_NETWORK_UTILITY
#                define AB_CNL_NETWORK_UTILITY_EXPORT __declspec(dllexport)
#            else
#                define AB_CNL_NETWORK_UTILITY_EXPORT __declspec(dllimport)
#            endif
#        else
#            define AB_CNL_NETWORK_UTILITY_EXPORT
#        endif
#    elif (__GNUC__ >= 4 || defined(__clang__)) && defined(ACCELBYTE_BUILD_SHARED_DEFINE)
#        define AB_CNL_NETWORK_UTILITY_EXPORT __attribute__((visibility("default")))
#    else
#        define AB_CNL_NETWORK_UTILITY_EXPORT
#    endif
#endif
