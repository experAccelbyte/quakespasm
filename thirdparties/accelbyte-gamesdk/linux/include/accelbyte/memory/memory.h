// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#ifndef ACCELBYTE_MEMORY_MEMORY_H
#define ACCELBYTE_MEMORY_MEMORY_H

#include "memory_exports.h"

//
#include "MemoryResourceManager.h"
#include "NewDeleteMemoryManager.h"

// STL
#include <memory>
#include <type_traits>

namespace accelbyte {
namespace memory {

namespace details {
extern ACCELBYTE_MEMORY_API MemoryResourceManager* global_memory_resource_manager;

template<typename T, typename Return = void>
using enable_if_not_array_t = std::enable_if_t<!std::is_array<T>::value, Return>;
} // namespace details

auto ACCELBYTE_MEMORY_API get_default_memory_manager() noexcept -> MemoryResourceManager*;

void ACCELBYTE_MEMORY_API set_default_memory_manager(MemoryResourceManager* memory_manager) noexcept;

template<typename T>
class MemoryManagerAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = T const*;
    using reference = T&;
    using const_reference = T const&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using ValueType = value_type;

    using propagate_on_container_move_assignment = std::true_type;

    template<typename U>
    struct rebind {
        using other = MemoryManagerAllocator<U>;
    };

    MemoryManagerAllocator() noexcept
        : MemoryManagerAllocator{get_default_memory_manager()}
    {
    }

    MemoryManagerAllocator(MemoryManagerAllocator const& other) noexcept
        : MemoryManagerAllocator{other.resource()}
    {
    }

    template<typename U>
    MemoryManagerAllocator(MemoryManagerAllocator<U> const& other) noexcept
        : MemoryManagerAllocator{other.resource()}
    {
    }

    MemoryManagerAllocator(MemoryResourceManager* memory_resource_manager) noexcept
        : memory_resource_manager_(memory_resource_manager)
    {
    }

    MemoryManagerAllocator(MemoryManagerAllocator&& other) noexcept
        : memory_resource_manager_{other.resource()}
    {
        other.memory_resource_manager_ = nullptr;
    }

    auto operator=(MemoryManagerAllocator const& other) noexcept -> MemoryManagerAllocator&
    {
        if (this == &other) {
            return *this;
        }
        memory_resource_manager_ = other.resource();
        return *this;
    }

    auto operator=(MemoryManagerAllocator&& other) noexcept -> MemoryManagerAllocator&
    {
        memory_resource_manager_ = other.resource();
        other.memory_resource_manager_ = nullptr;
        return *this;
    }

    auto allocate(size_t size) -> ValueType*
    {
        if (!memory_resource_manager_) {
            return nullptr;
        }
        return static_cast<ValueType*>(memory_resource_manager_->allocate(size * sizeof(ValueType)));
    }

    void deallocate(ValueType* p, size_t size)
    {
        if (!memory_resource_manager_) {
            return;
        }
        memory_resource_manager_->deallocate(p, size * sizeof(ValueType));
    }

    auto resource() const noexcept -> MemoryResourceManager* { return memory_resource_manager_; }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        ::new (p) U(std::forward<Args>(args)...);
    }

    friend auto operator==(const MemoryManagerAllocator& lhs, const MemoryManagerAllocator& rhs) noexcept -> bool
    {
        return *lhs.resource() == *rhs.resource();
    }

    friend auto operator!=(const MemoryManagerAllocator& lhs, const MemoryManagerAllocator& rhs) noexcept -> bool
    {
        return !(lhs == rhs);
    }

private:
    MemoryResourceManager* memory_resource_manager_{};
};

template<typename T1, typename T2>
auto operator==(const MemoryManagerAllocator<T1>& lhs, const MemoryManagerAllocator<T2>& rhs) noexcept -> bool
{
    return *lhs.resource() == *rhs.resource();
}

template<typename T1, typename T2>
auto operator!=(const MemoryManagerAllocator<T1>& lhs, const MemoryManagerAllocator<T2>& rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template<typename T, typename Alloc, typename... Args, details::enable_if_not_array_t<T>* = nullptr>
auto allocate_new_ptr(const Alloc& /* alloc */, Args&&... args) -> T*
{
    using AllocatorTraits = typename std::allocator_traits<Alloc>::template rebind_traits<T>;
    using OtherAllocator = typename AllocatorTraits::allocator_type;
    auto other_alloc = OtherAllocator{};
    auto ptr = AllocatorTraits::allocate(other_alloc, 1);
    AllocatorTraits::construct(other_alloc, ptr, std::forward<Args>(args)...);
    return ptr;
}

template<typename Alloc, typename T, details::enable_if_not_array_t<T>* = nullptr>
void deallocate_delete_ptr(const Alloc& /* alloc */, T* ptr)
{
    using AllocatorTraits = typename std::allocator_traits<Alloc>::template rebind_traits<T>;
    using OtherAllocator = typename AllocatorTraits::allocator_type;

    auto other_alloc = OtherAllocator{};
    AllocatorTraits::destroy(other_alloc, ptr);
    AllocatorTraits::deallocate(other_alloc, ptr, 1);
}

template<typename T, typename... Args, details::enable_if_not_array_t<T>* = nullptr>
auto new_raw_ptr(Args&&... args) -> T*
{
    using Allocator = MemoryManagerAllocator<T>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    auto alloc = Allocator{};
    auto ptr = AllocatorTraits::allocate(alloc, 1);
    AllocatorTraits::construct(alloc, ptr, std::forward<Args>(args)...);
    return ptr;
}

template<typename T, details::enable_if_not_array_t<T>* = nullptr>
void delete_raw_ptr(T* ptr)
{
    using Allocator = MemoryManagerAllocator<T>;
    using AllocatorTraits = std::allocator_traits<Allocator>;
    auto alloc = Allocator{};
    AllocatorTraits::destroy(alloc, ptr);
    AllocatorTraits::deallocate(alloc, ptr, 1);
}

// SMART POINTER

template<typename Alloc>
class AllocationDeleter {
public:
    using Pointer = typename std::allocator_traits<Alloc>::pointer;

    AllocationDeleter() noexcept
        : AllocationDeleter{Alloc{}}
    {
    }

    explicit AllocationDeleter(const Alloc& allocator) noexcept
        : allocator_{allocator}
    {
    }

    AllocationDeleter(AllocationDeleter const&) = default;
    auto operator=(AllocationDeleter const&) -> AllocationDeleter& = default;

    AllocationDeleter(AllocationDeleter&&) noexcept = default;
    auto operator=(AllocationDeleter&&) -> AllocationDeleter& = default;

    ~AllocationDeleter() = default;

    void operator()(Pointer ptr) const { deallocate_delete_ptr(allocator_, ptr); }

private:
    Alloc allocator_;
};

template<typename T>
using UniquePtr = std::unique_ptr<T, AllocationDeleter<MemoryManagerAllocator<T>>>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T, typename Alloc, typename... Args>
auto allocate_shared_ptr(const Alloc& alloc, Args&&... args) -> SharedPtr<T>
{
    return std::allocate_shared<T>(alloc, std::forward<Args>(args)...);
}

template<typename T, typename... Args>
auto make_shared_ptr(Args&&... args) -> SharedPtr<T>
{
    return allocate_shared_ptr<T>(MemoryManagerAllocator<T>{}, std::forward<Args>(args)...);
}

template<typename T, typename Alloc, typename... Args, details::enable_if_not_array_t<T>* = nullptr>
auto allocate_unique_ptr(const Alloc& /* alloc */, Args&&... args)
    -> std::unique_ptr<T, AllocationDeleter<typename std::allocator_traits<Alloc>::template rebind_alloc<T>>>
{
    using OtherAllocator = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
    auto other_alloc = OtherAllocator{};
    auto ptr = allocate_new_ptr<T>(other_alloc, std::forward<Args>(args)...);
    auto other_alloc_deleter = AllocationDeleter<OtherAllocator>();
    return std::unique_ptr<T, AllocationDeleter<OtherAllocator>>(ptr, other_alloc_deleter);
}

template<typename T, typename... Args, details::enable_if_not_array_t<T>* = nullptr>
auto make_unique_ptr(Args&&... args) -> UniquePtr<T>
{
    auto ptr = new_raw_ptr<T>(std::forward<Args>(args)...);
    return UniquePtr<T>{ptr};
}

} // namespace memory
} // namespace accelbyte

#endif