// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/utils/utils_exports.h"

//
#include "accelbyte/common/String.h"

namespace accelbyte {
namespace utils {

class JsonObjectString {
public:
    AB_CNL_UTILS_EXPORT JsonObjectString() = default;
    AB_CNL_UTILS_EXPORT explicit JsonObjectString(const char* s);
    AB_CNL_UTILS_EXPORT explicit JsonObjectString(const String& str);
    AB_CNL_UTILS_EXPORT explicit JsonObjectString(String&& str) noexcept;
    AB_CNL_UTILS_EXPORT JsonObjectString(const String& str, size_t pos, size_t len = String::npos);
    AB_CNL_UTILS_EXPORT JsonObjectString(const char* s, size_t n);
    AB_CNL_UTILS_EXPORT JsonObjectString(size_t n, char c);
    AB_CNL_UTILS_EXPORT JsonObjectString(std::initializer_list<char> il);

    template<class InputIterator>
    JsonObjectString(InputIterator first, InputIterator last)
        : value_(first, last)
    {
    }

    AB_CNL_UTILS_EXPORT auto operator=(const String& other) -> JsonObjectString&;
    AB_CNL_UTILS_EXPORT auto operator==(const String& other) const -> bool;
    AB_CNL_UTILS_EXPORT operator String() const;

private:
    String value_;
};

} // namespace utils
} // namespace accelbyte