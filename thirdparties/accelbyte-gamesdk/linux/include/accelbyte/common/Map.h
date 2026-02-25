// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_MAP_H
#define ACCELBYTE_MAP_H

#include "accelbyte/memory/memory.h"
#include "exports.h"

// STL
#include <map>
#include <unordered_map>

namespace accelbyte {

template<typename Key, typename T, typename Compare = std::less<Key>>
using Map = std::map<Key, T, Compare, memory::MemoryManagerAllocator<std::pair<const Key, T>>>;

template<typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using UnorderedMap =
    std::unordered_map<Key, T, Hash, KeyEqual, memory::MemoryManagerAllocator<std::pair<const Key, T>>>;

} // namespace accelbyte

#endif