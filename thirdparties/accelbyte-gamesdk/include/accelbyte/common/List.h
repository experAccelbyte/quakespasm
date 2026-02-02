// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_LIST_H
#define ACCELBYTE_LIST_H

#include "accelbyte/memory/memory.h"
#include "exports.h"

// STL
#include <list>

namespace accelbyte {

template<typename T>
using List = std::list<T, memory::MemoryManagerAllocator<T>>;

} // namespace accelbyte

#endif