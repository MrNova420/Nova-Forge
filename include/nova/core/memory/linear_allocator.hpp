// =============================================================================
// NovaCore Engine - Linear Allocator (Scratch/Frame Allocator)
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Ultra-fast bump allocator for per-frame temporary allocations.
// Characteristics:
// - O(1) allocation (pointer bump)
// - O(1) reset (pointer reset)
// - No individual deallocation
// - Perfect for frame-scoped data
// =============================================================================

#pragma once

#include "nova/core/memory/allocator.hpp"

namespace nova::memory {

/// @brief Linear/Bump allocator for fast temporary allocations
/// @note Memory is freed all at once via reset(), not individual deallocate()
class LinearAllocator final : public Allocator {
public:
    /// @brief Construct linear allocator with given buffer
    /// @param buffer Pre-allocated memory buffer
    /// @param size Buffer size in bytes
    /// @param name Optional name for debugging
    LinearAllocator(void* buffer, usize size, const char* name = "LinearAllocator") noexcept
        : m_buffer(static_cast<u8*>(buffer))
        , m_size(size)
        , m_offset(0)
        , m_name(name)
    {
        m_stats.reset();
    }
    
    /// @brief Allocate memory (bump pointer)
    /// @param size Number of bytes
    /// @param alignment Memory alignment
    /// @return Pointer to allocated memory, or nullptr if out of space
    [[nodiscard]] void* allocate(usize size, usize alignment = DEFAULT_ALIGNMENT) noexcept override {
        // Calculate aligned offset
        usize alignedOffset = alignUp(m_offset, alignment);
        
        // Check if we have space
        if (alignedOffset + size > m_size) {
            return nullptr; // Out of memory
        }
        
        void* ptr = m_buffer + alignedOffset;
        m_offset = alignedOffset + size;
        
        // Update stats
        m_stats.totalAllocated += size;
        m_stats.allocationCount++;
        m_stats.totalAllocationCount++;
        if (m_stats.totalAllocated > m_stats.peakAllocated) {
            m_stats.peakAllocated = m_stats.totalAllocated;
        }
        
        return ptr;
    }
    
    /// @brief Deallocate does nothing for linear allocator
    /// @note Use reset() to free all memory
    void deallocate([[maybe_unused]] void* ptr) noexcept override {
        // Linear allocator doesn't support individual deallocation
        // Memory is freed via reset()
    }
    
    /// @brief Reset allocator - frees all allocations
    void reset() noexcept {
        m_stats.totalFreed += m_stats.totalAllocated;
        m_stats.totalFreeCount += m_stats.allocationCount;
        m_stats.totalAllocated = 0;
        m_stats.allocationCount = 0;
        m_offset = 0;
    }
    
    /// @brief Get marker for current position (for partial reset)
    [[nodiscard]] usize getMarker() const noexcept {
        return m_offset;
    }
    
    /// @brief Reset to a previous marker position
    /// @param marker Marker from getMarker()
    void resetToMarker(usize marker) noexcept {
        if (marker <= m_offset) {
            usize freedBytes = m_offset - marker;
            m_stats.totalFreed += freedBytes;
            m_stats.totalAllocated -= freedBytes;
            m_offset = marker;
        }
    }
    
    /// @brief Get total buffer size
    [[nodiscard]] usize getCapacity() const noexcept {
        return m_size;
    }
    
    /// @brief Get remaining free space
    [[nodiscard]] usize getFreeSpace() const noexcept {
        return m_size - m_offset;
    }
    
    // Allocator interface implementation
    [[nodiscard]] usize getAllocatedSize() const noexcept override {
        return m_offset;
    }
    
    [[nodiscard]] AllocationStats getStats() const noexcept override {
        return m_stats;
    }
    
    [[nodiscard]] bool owns(const void* ptr) const noexcept override {
        auto p = static_cast<const u8*>(ptr);
        return p >= m_buffer && p < m_buffer + m_size;
    }
    
    [[nodiscard]] const char* getName() const noexcept override {
        return m_name;
    }
    
private:
    u8* m_buffer;
    usize m_size;
    usize m_offset;
    const char* m_name;
    AllocationStats m_stats;
};

// =============================================================================
// Scoped Linear Allocator Reset
// =============================================================================

/// @brief RAII helper to automatically reset linear allocator on scope exit
class ScopedLinearReset {
public:
    explicit ScopedLinearReset(LinearAllocator& allocator) noexcept
        : m_allocator(allocator)
        , m_marker(allocator.getMarker()) {}
    
    ~ScopedLinearReset() {
        m_allocator.resetToMarker(m_marker);
    }
    
    // Non-copyable
    ScopedLinearReset(const ScopedLinearReset&) = delete;
    ScopedLinearReset& operator=(const ScopedLinearReset&) = delete;
    
private:
    LinearAllocator& m_allocator;
    usize m_marker;
};

} // namespace nova::memory

namespace nova {
    using LinearAllocator = memory::LinearAllocator;
}
