#pragma once

#ifdef __linux__ 
#define AB_CNL_DEBUG_HTTP_EXPORT
#elif _WIN32
#ifdef ACCELBYTE_BUILD_SHARED_DEFINE
#ifdef AB_CNL_DEBUG_HTTP
#define AB_CNL_DEBUG_HTTP_EXPORT __declspec(dllexport)
#else
#define AB_CNL_DEBUG_HTTP_EXPORT __declspec(dllimport)
#endif
#else
#define AB_CNL_DEBUG_HTTP_EXPORT
#endif
#else
#endif