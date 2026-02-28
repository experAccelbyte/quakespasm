// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_SET_H
#define ACCELBYTE_SET_H

#include "accelbyte/memory/memory.h"
#include "exports.h"

// STL
#include <set>
#include <unordered_set>

namespace accelbyte {

template<typename Key, typename Compare = std::less<Key>>
using Set = std::set<Key, Compare, memory::MemoryManagerAllocator<Key>>;

template<typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using UnorderedSet = std::unordered_set<Key, Hash, KeyEqual, memory::MemoryManagerAllocator<Key>>;

} // namespace accelbyte

#endif