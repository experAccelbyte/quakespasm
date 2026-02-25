// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_VECTOR_H
#define ACCELBYTE_VECTOR_H

#include "accelbyte/memory/memory.h"
#include "exports.h"

// STL
#include <vector>

namespace accelbyte {

template<typename T>
using Vector = std::vector<T, memory::MemoryManagerAllocator<T>>;

} // namespace accelbyte

#endif