// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_STRING_H
#define ACCELBYTE_STRING_H

#include "accelbyte/memory/memory.h"
#include "exports.h"

// STL
#include <string>

namespace accelbyte {

using String = std::basic_string<char, std::char_traits<char>, memory::MemoryManagerAllocator<char>>;

auto ACCELBYTE_COMMON_API to_string(int value) -> String;

auto ACCELBYTE_COMMON_API to_string(long value) -> String;

auto ACCELBYTE_COMMON_API to_string(long long value) -> String;

auto ACCELBYTE_COMMON_API to_string(unsigned value) -> String;

auto ACCELBYTE_COMMON_API to_string(unsigned long value) -> String;

auto ACCELBYTE_COMMON_API to_string(unsigned long long value) -> String;

auto ACCELBYTE_COMMON_API to_string(float value) -> String;

auto ACCELBYTE_COMMON_API to_string(double value) -> String;

auto ACCELBYTE_COMMON_API to_string(long double value) -> String;

using WString = std::basic_string<wchar_t, std::char_traits<wchar_t>, memory::MemoryManagerAllocator<wchar_t>>;

} // namespace accelbyte

#if defined(__linux__) && defined(__GLIBCXX__)
namespace std {
template<typename Alloc>
struct hash<std::basic_string<char, std::char_traits<char>, Alloc>> {
    auto operator()(std::basic_string<char, std::char_traits<char>, Alloc> const& str) const -> size_t
    {
        return std::_Hash_impl::hash(str.data(), str.length());
    }
};

template<typename Alloc>
struct hash<std::basic_string<wchar_t, std::char_traits<wchar_t>, Alloc>> {
    auto operator()(std::basic_string<wchar_t, std::char_traits<wchar_t>, Alloc> const& str) const -> size_t
    {
        return std::_Hash_impl::hash(str.data(), str.length() * sizeof(wchar_t));
    }
};

} // namespace std
#endif
#endif