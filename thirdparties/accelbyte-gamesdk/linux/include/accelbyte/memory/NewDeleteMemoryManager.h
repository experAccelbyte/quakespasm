// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_MEMORY_NEW_DELETE_MEMORY_MANAGER_H
#define ACCELBYTE_MEMORY_NEW_DELETE_MEMORY_MANAGER_H

#include "accelbyte/memory/MemoryResourceManager.h"
#include "memory_exports.h"

#include <cstddef>

namespace accelbyte {
namespace memory {

class ACCELBYTE_MEMORY_API NewDeleteMemoryManager : public MemoryResourceManager {
protected:
    auto do_allocate(std::size_t size, std::size_t alignment) -> void* override;

    void do_deallocate(void* ptr, std::size_t size, std::size_t alignment) override;
};

auto ACCELBYTE_MEMORY_API get_new_delete_memory_manager() -> NewDeleteMemoryManager*;

} // namespace memory
} // namespace accelbyte

#endif