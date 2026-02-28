// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_STRINGSTREAM_H
#define ACCELBYTE_STRINGSTREAM_H

#include "accelbyte/memory/memory.h"
#include "exports.h"

// STL
#include <sstream>

namespace accelbyte {

using IStringStream = std::basic_istringstream<char, std::char_traits<char>, memory::MemoryManagerAllocator<char>>;

using OStringStream = std::basic_ostringstream<char, std::char_traits<char>, memory::MemoryManagerAllocator<char>>;

using StringStream = std::basic_stringstream<char, std::char_traits<char>, memory::MemoryManagerAllocator<char>>;

} // namespace accelbyte

#endif