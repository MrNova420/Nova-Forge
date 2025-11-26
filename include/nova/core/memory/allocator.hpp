// =============================================================================
// NovaCore Engine - Memory Allocator Interface
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Base allocator interface for all NovaCore memory allocators.
// Designed for:
// - Custom memory allocation strategies
// - Memory tracking and debugging
// - Cache-friendly allocation patterns
// - Mobile-optimized memory budgets
// =============================================================================

#pragma once

#include <cstddef>
#include <new>

#include "nova/core/types/types.hpp"
#include "nova/core/types/result.hpp"

namespace nova::memory {

// =============================================================================
// Forward Declarations
// =============================================================================
class Allocator;

// =============================================================================
// Memory Alignment Constants
// =============================================================================

/// Default alignment for general allocations
inline constexpr usize DEFAULT_ALIGNMENT = alignof(std::max_align_t);

/// Alignment for SIMD operations (16 bytes for SSE/NEON)
inline constexpr usize SIMD_ALIGNMENT = 16;

/// Cache line size for cache-aligned allocations
inline constexpr usize CACHE_LINE_SIZE = 64;

// =============================================================================
// Memory Block Info
// =============================================================================

/// Information about an allocated memory block
struct AllocationInfo {
    void* address;          ///< Block address
    usize size;             ///< Allocated size
    usize alignment;        ///< Alignment used
    usize actualSize;       ///< Actual size including alignment overhead
};

// =============================================================================
// Allocation Statistics
// =============================================================================

/// Memory allocation statistics for debugging and profiling
struct AllocationStats {
    usize totalAllocated;       ///< Total bytes currently allocated
    usize totalFreed;           ///< Total bytes freed over lifetime
    usize peakAllocated;        ///< Peak allocation in bytes
    usize allocationCount;      ///< Number of active allocations
    usize totalAllocationCount; ///< Total allocations over lifetime
    usize totalFreeCount;       ///< Total frees over lifetime
    
    /// Reset statistics
    constexpr void reset() noexcept {
        totalAllocated = 0;
        totalFreed = 0;
        peakAllocated = 0;
        allocationCount = 0;
        totalAllocationCount = 0;
        totalFreeCount = 0;
    }
};

// =============================================================================
// Allocator Interface
// =============================================================================

/// @brief Abstract base class for memory allocators
/// @note All allocators in NovaCore inherit from this interface
class Allocator {
public:
    virtual ~Allocator() = default;
    
    /// @brief Allocate memory with specified size and alignment
    /// @param size Number of bytes to allocate
    /// @param alignment Memory alignment (must be power of 2)
    /// @return Pointer to allocated memory, or nullptr on failure
    [[nodiscard]] virtual void* allocate(usize size, usize alignment = DEFAULT_ALIGNMENT) noexcept = 0;
    
    /// @brief Deallocate previously allocated memory
    /// @param ptr Pointer to memory block (can be nullptr)
    virtual void deallocate(void* ptr) noexcept = 0;
    
    /// @brief Reallocate memory block with new size
    /// @param ptr Pointer to existing block (can be nullptr)
    /// @param newSize New size in bytes
    /// @param alignment Memory alignment
    /// @return Pointer to reallocated memory, or nullptr on failure
    [[nodiscard]] virtual void* reallocate(void* ptr, usize newSize, usize alignment = DEFAULT_ALIGNMENT) noexcept {
        if (ptr == nullptr) {
            return allocate(newSize, alignment);
        }
        
        if (newSize == 0) {
            deallocate(ptr);
            return nullptr;
        }
        
        // Default implementation: allocate new, copy, free old
        void* newPtr = allocate(newSize, alignment);
        if (newPtr) {
            // Note: This is a fallback - subclasses should override with more efficient implementations
            // that know the original allocation size
            deallocate(ptr);
        }
        return newPtr;
    }
    
    /// @brief Get total memory currently allocated
    [[nodiscard]] virtual usize getAllocatedSize() const noexcept = 0;
    
    /// @brief Get allocation statistics
    [[nodiscard]] virtual AllocationStats getStats() const noexcept = 0;
    
    /// @brief Check if allocator owns a pointer
    [[nodiscard]] virtual bool owns(const void* ptr) const noexcept = 0;
    
    /// @brief Get allocator name for debugging
    [[nodiscard]] virtual const char* getName() const noexcept = 0;
    
protected:
    Allocator() = default;
    
    // Non-copyable, non-movable
    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(Allocator&&) = delete;
};

// =============================================================================
// Allocation Helpers
// =============================================================================

/// @brief Allocate and construct an object
/// @tparam T Type to construct
/// @tparam Args Constructor argument types
/// @param allocator Allocator to use
/// @param args Constructor arguments
/// @return Pointer to constructed object, or nullptr on failure
template<typename T, typename... Args>
[[nodiscard]] T* allocateObject(Allocator& allocator, Args&&... args) noexcept {
    void* memory = allocator.allocate(sizeof(T), alignof(T));
    if (!memory) {
        return nullptr;
    }
    return new (memory) T(static_cast<Args&&>(args)...);
}

/// @brief Destroy and deallocate an object
/// @tparam T Type to destroy
/// @param allocator Allocator that allocated the object
/// @param ptr Pointer to object
template<typename T>
void deallocateObject(Allocator& allocator, T* ptr) noexcept {
    if (ptr) {
        ptr->~T();
        allocator.deallocate(ptr);
    }
}

/// @brief Allocate array of objects (default constructed)
/// @tparam T Element type
/// @param allocator Allocator to use
/// @param count Number of elements
/// @return Pointer to array, or nullptr on failure
template<typename T>
[[nodiscard]] T* allocateArray(Allocator& allocator, usize count) noexcept {
    if (count == 0) return nullptr;
    
    void* memory = allocator.allocate(sizeof(T) * count, alignof(T));
    if (!memory) {
        return nullptr;
    }
    
    T* arr = static_cast<T*>(memory);
    for (usize i = 0; i < count; ++i) {
        new (arr + i) T();
    }
    return arr;
}

/// @brief Destroy and deallocate array of objects
/// @tparam T Element type
/// @param allocator Allocator that allocated the array
/// @param ptr Pointer to array
/// @param count Number of elements
template<typename T>
void deallocateArray(Allocator& allocator, T* ptr, usize count) noexcept {
    if (ptr) {
        for (usize i = count; i > 0; --i) {
            ptr[i - 1].~T();
        }
        allocator.deallocate(ptr);
    }
}

// =============================================================================
// Scoped Allocation RAII Wrapper
// =============================================================================

/// @brief RAII wrapper for allocator-managed memory
/// @tparam T Type of managed object
template<typename T>
class ScopedAlloc {
public:
    /// Construct from allocator and object
    ScopedAlloc(Allocator& alloc, T* ptr) noexcept
        : m_allocator(&alloc), m_ptr(ptr) {}
    
    /// No copy
    ScopedAlloc(const ScopedAlloc&) = delete;
    ScopedAlloc& operator=(const ScopedAlloc&) = delete;
    
    /// Move constructor
    ScopedAlloc(ScopedAlloc&& other) noexcept
        : m_allocator(other.m_allocator), m_ptr(other.m_ptr) {
        other.m_ptr = nullptr;
    }
    
    /// Move assignment
    ScopedAlloc& operator=(ScopedAlloc&& other) noexcept {
        if (this != &other) {
            reset();
            m_allocator = other.m_allocator;
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }
    
    /// Destructor - deallocates
    ~ScopedAlloc() {
        reset();
    }
    
    /// Access pointer
    [[nodiscard]] T* get() const noexcept { return m_ptr; }
    [[nodiscard]] T* operator->() const noexcept { return m_ptr; }
    [[nodiscard]] T& operator*() const noexcept { return *m_ptr; }
    
    /// Check if valid
    [[nodiscard]] explicit operator bool() const noexcept { return m_ptr != nullptr; }
    
    /// Release ownership
    [[nodiscard]] T* release() noexcept {
        T* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }
    
    /// Reset and deallocate
    void reset() noexcept {
        if (m_ptr) {
            deallocateObject(*m_allocator, m_ptr);
            m_ptr = nullptr;
        }
    }
    
private:
    Allocator* m_allocator;
    T* m_ptr;
};

/// Create ScopedAlloc with constructed object
template<typename T, typename... Args>
[[nodiscard]] ScopedAlloc<T> makeScopedAlloc(Allocator& allocator, Args&&... args) noexcept {
    return ScopedAlloc<T>(allocator, allocateObject<T>(allocator, static_cast<Args&&>(args)...));
}

} // namespace nova::memory

namespace nova {
    using Allocator = memory::Allocator;
    using AllocationStats = memory::AllocationStats;
}
