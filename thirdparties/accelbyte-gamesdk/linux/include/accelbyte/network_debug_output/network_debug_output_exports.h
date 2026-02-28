// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#ifdef __linux__ 
#define AB_CNL_NETWORK_DEBUG_OUTPUT_EXPORT
#elif _WIN32
#ifdef ACCELBYTE_BUILD_SHARED_DEFINE
#ifdef AB_CNL_NETWORK_DEBUG_OUTPUT
#define AB_CNL_NETWORK_DEBUG_OUTPUT_EXPORT __declspec(dllexport)
#else
#define AB_CNL_NETWORK_DEBUG_OUTPUT_EXPORT   __declspec( dllimport )
#endif
#else
#define AB_CNL_NETWORK_DEBUG_OUTPUT_EXPORT
#endif
#else
#endif