// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_QUEUE_H
#define ACCELBYTE_QUEUE_H

#include "accelbyte/memory/memory.h"
#include "exports.h"

// STL
#include <queue>

namespace accelbyte {

template<typename T>
using Queue = std::queue<T, std::deque<T, memory::MemoryManagerAllocator<T>>>;

} // namespace accelbyte

#endif