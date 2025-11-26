// =============================================================================
// NovaCore Engine - Memory Management
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Comprehensive memory management system designed for:
// - Mobile-first (memory budget aware)
// - High performance (O(1) allocators)
// - Cache efficiency
// - Debugging and profiling support
// =============================================================================

#pragma once

// All allocator types
#include "nova/core/memory/allocator.hpp"
#include "nova/core/memory/linear_allocator.hpp"
#include "nova/core/memory/stack_allocator.hpp"
#include "nova/core/memory/pool_allocator.hpp"

namespace nova::memory {

// =============================================================================
// Memory System Constants
// =============================================================================

/// Default frame allocator size (16 MB for mobile)
inline constexpr usize DEFAULT_FRAME_ALLOCATOR_SIZE = 16 * 1024 * 1024;

/// Default scratch allocator size (4 MB)
inline constexpr usize DEFAULT_SCRATCH_ALLOCATOR_SIZE = 4 * 1024 * 1024;

// =============================================================================
// Global Memory Functions
// =============================================================================

/// @brief Allocate aligned memory using system allocator
/// @param size Size in bytes
/// @param alignment Alignment (must be power of 2)
/// @return Pointer to allocated memory, or nullptr on failure
[[nodiscard]] inline void* alignedAlloc(usize size, usize alignment) noexcept {
    // Ensure alignment is at least sizeof(void*) and power of 2
    alignment = std::max(alignment, alignof(std::max_align_t));
    
#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#else
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
    return ptr;
#endif
}

/// @brief Free aligned memory
/// @param ptr Pointer to aligned memory
inline void alignedFree(void* ptr) noexcept {
#if defined(_MSC_VER)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

// =============================================================================
// Memory Utility Functions
// =============================================================================

/// @brief Zero-fill memory
/// @param ptr Pointer to memory
/// @param size Size in bytes
inline void zeroMemory(void* ptr, usize size) noexcept {
    std::memset(ptr, 0, size);
}

/// @brief Copy memory
/// @param dst Destination pointer
/// @param src Source pointer
/// @param size Size in bytes
inline void copyMemory(void* dst, const void* src, usize size) noexcept {
    std::memcpy(dst, src, size);
}

/// @brief Move memory (handles overlapping regions)
/// @param dst Destination pointer
/// @param src Source pointer
/// @param size Size in bytes
inline void moveMemory(void* dst, const void* src, usize size) noexcept {
    std::memmove(dst, src, size);
}

/// @brief Compare memory
/// @param a First pointer
/// @param b Second pointer
/// @param size Size in bytes
/// @return 0 if equal, negative if a < b, positive if a > b
[[nodiscard]] inline i32 compareMemory(const void* a, const void* b, usize size) noexcept {
    return std::memcmp(a, b, size);
}

/// @brief Fill memory with a byte value
/// @param ptr Pointer to memory
/// @param value Byte value
/// @param size Size in bytes
inline void fillMemory(void* ptr, u8 value, usize size) noexcept {
    std::memset(ptr, value, size);
}

} // namespace nova::memory

// Bring common items into nova namespace
namespace nova {
    using memory::alignedAlloc;
    using memory::alignedFree;
    using memory::zeroMemory;
    using memory::copyMemory;
    using memory::moveMemory;
}
