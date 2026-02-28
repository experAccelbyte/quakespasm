// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_MEMORY_MEMORY_RESOURCE_MANAGER_H
#define ACCELBYTE_MEMORY_MEMORY_RESOURCE_MANAGER_H

#include "memory_exports.h"

#include <cstddef>

namespace accelbyte {
namespace memory {

class ACCELBYTE_MEMORY_API MemoryResourceManager {
public:
    auto allocate(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) -> void*;

    void deallocate(void* ptr, std::size_t bytes, std::size_t alignment = alignof(std::max_align_t));

    auto is_equal(const MemoryResourceManager& other) const noexcept -> bool;

    virtual ~MemoryResourceManager() noexcept = default;

private:
    virtual auto do_allocate(std::size_t size, std::size_t alignment) -> void* = 0;

    virtual void do_deallocate(void* ptr, std::size_t size, std::size_t alignment) = 0;

    virtual auto do_is_equal(const MemoryResourceManager& other) const noexcept -> bool;
};

auto ACCELBYTE_MEMORY_API operator==(const MemoryResourceManager& lhs, const MemoryResourceManager& rhs) noexcept
    -> bool;

auto ACCELBYTE_MEMORY_API operator!=(const MemoryResourceManager& lhs, const MemoryResourceManager& rhs) noexcept
    -> bool;

} // namespace memory
} // namespace accelbyte

#endif