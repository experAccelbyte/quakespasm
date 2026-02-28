// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_MEMORY_MEMORY_EXPORTS_H
#define ACCELBYTE_MEMORY_MEMORY_EXPORTS_H

#ifdef ACCELBYTE_BUILD_SHARED_DEFINE

#  ifndef ACCELBYTE_MEMORY_API
#    ifdef _MSC_VER
#        if defined ACCELBYTE_MEMORY_EXPORTS
#            define ACCELBYTE_MEMORY_API __declspec(dllexport)
#        else
#            define ACCELBYTE_MEMORY_API __declspec(dllimport)
#        endif
#    elif __GNUC__ >= 4 || defined(__clang__)
#        define ACCELBYTE_MEMORY_API __attribute__((visibility("default")))
#    else
#        define ACCELBYTE_MEMORY_API
#    endif
#  endif

#  ifndef ACCELBYTE_MEMORY_API_CALL
#    ifdef _MSC_VER
#      define ACCELBYTE_MEMORY_API_CALL(rtype) ACCELBYTE_MEMORY_API rtype __cdecl
#    else
#      define ACCELBYTE_MEMORY_API_CALL(rtype) ACCELBYTE_MEMORY_API rtype
#    endif
#  endif

#else

#  define ACCELBYTE_MEMORY_API
#  define ACCELBYTE_MEMORY_API_CALL(rtype) rtype

#endif

#endif /* ACCELBYTE_MEMORY_EXPORT_H */
