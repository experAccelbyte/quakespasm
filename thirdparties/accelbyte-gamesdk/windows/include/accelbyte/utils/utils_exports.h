#pragma once

#ifdef __linux__ 
#define AB_CNL_UTILS_EXPORT
#elif _WIN32
#ifdef ACCELBYTE_BUILD_SHARED_DEFINE
#ifdef AB_CNL_UTILS
#define AB_CNL_UTILS_EXPORT   __declspec( dllexport )
#else
#define AB_CNL_UTILS_EXPORT   __declspec( dllimport )
#endif
#else
#define AB_CNL_UTILS_EXPORT
#endif
#else
#endif