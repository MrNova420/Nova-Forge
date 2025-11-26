// =============================================================================
// NovaCore Engine - Stack Allocator (Double-Ended)
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// LIFO stack allocator with optional double-ended support.
// Characteristics:
// - O(1) allocation
// - O(1) deallocation (in LIFO order)
// - Perfect for scoped/hierarchical allocations
// - Double-ended mode for temporary data from both ends
// =============================================================================

#pragma once

#include "nova/core/memory/allocator.hpp"

namespace nova::memory {

/// @brief Stack allocator with LIFO deallocation
/// @note Deallocations must be in reverse order of allocations
class StackAllocator final : public Allocator {
public:
    /// @brief Construct stack allocator
    /// @param buffer Pre-allocated memory buffer
    /// @param size Buffer size in bytes
    /// @param name Optional name for debugging
    StackAllocator(void* buffer, usize size, const char* name = "StackAllocator") noexcept
        : m_buffer(static_cast<u8*>(buffer))
        , m_size(size)
        , m_topOffset(0)
        , m_bottomOffset(size)  // Double-ended: grows from both ends
        , m_name(name)
    {
        m_stats.reset();
    }
    
    /// @brief Allocate from top of stack
    /// @param size Number of bytes
    /// @param alignment Memory alignment
    /// @return Pointer to allocated memory, or nullptr if out of space
    [[nodiscard]] void* allocate(usize size, usize alignment = DEFAULT_ALIGNMENT) noexcept override {
        return allocateTop(size, alignment);
    }
    
    /// @brief Allocate from top of stack
    [[nodiscard]] void* allocateTop(usize size, usize alignment = DEFAULT_ALIGNMENT) noexcept {
        // Store header before allocation for deallocation
        usize headerSize = sizeof(AllocationHeader);
        usize totalSize = headerSize + size;
        
        // Calculate aligned offset (after header)
        usize alignedHeaderEnd = alignUp(m_topOffset + headerSize, alignment);
        usize alignedEnd = alignedHeaderEnd + size;
        
        // Check if we have space
        if (alignedEnd > m_bottomOffset) {
            return nullptr; // Out of memory
        }
        
        // Write header just before the aligned user data
        AllocationHeader* header = reinterpret_cast<AllocationHeader*>(m_buffer + alignedHeaderEnd - headerSize);
        header->previousOffset = m_topOffset;
        header->size = size;
        
        void* ptr = m_buffer + alignedHeaderEnd;
        m_topOffset = alignedEnd;
        
        // Update stats
        m_stats.totalAllocated += size;
        m_stats.allocationCount++;
        m_stats.totalAllocationCount++;
        if (m_stats.totalAllocated > m_stats.peakAllocated) {
            m_stats.peakAllocated = m_stats.totalAllocated;
        }
        
        return ptr;
    }
    
    /// @brief Allocate from bottom of stack (grows downward)
    [[nodiscard]] void* allocateBottom(usize size, usize alignment = DEFAULT_ALIGNMENT) noexcept {
        // Align the end position
        usize alignedStart = alignDown(m_bottomOffset - size, alignment);
        
        // Reserve space for header
        usize headerSize = sizeof(AllocationHeader);
        usize headerOffset = alignedStart - headerSize;
        
        // Check if we have space
        if (headerOffset < m_topOffset) {
            return nullptr; // Out of memory
        }
        
        // Write header
        AllocationHeader* header = reinterpret_cast<AllocationHeader*>(m_buffer + headerOffset);
        header->previousOffset = m_bottomOffset;
        header->size = size;
        
        m_bottomOffset = headerOffset;
        
        void* ptr = m_buffer + alignedStart;
        
        // Update stats
        m_stats.totalAllocated += size;
        m_stats.allocationCount++;
        m_stats.totalAllocationCount++;
        if (m_stats.totalAllocated > m_stats.peakAllocated) {
            m_stats.peakAllocated = m_stats.totalAllocated;
        }
        
        return ptr;
    }
    
    /// @brief Deallocate from top (must be the last top allocation)
    /// @note Passing wrong pointer results in undefined behavior
    void deallocate(void* ptr) noexcept override {
        deallocateTop(ptr);
    }
    
    /// @brief Deallocate from top of stack
    void deallocateTop(void* ptr) noexcept {
        if (ptr == nullptr) return;
        
        // Get header
        usize headerSize = sizeof(AllocationHeader);
        AllocationHeader* header = reinterpret_cast<AllocationHeader*>(static_cast<u8*>(ptr) - headerSize);
        
        // Update stats
        m_stats.totalAllocated -= header->size;
        m_stats.totalFreed += header->size;
        m_stats.allocationCount--;
        m_stats.totalFreeCount++;
        
        // Pop
        m_topOffset = header->previousOffset;
    }
    
    /// @brief Deallocate from bottom of stack
    void deallocateBottom(void* ptr) noexcept {
        if (ptr == nullptr) return;
        
        // Header is at m_bottomOffset
        AllocationHeader* header = reinterpret_cast<AllocationHeader*>(m_buffer + m_bottomOffset);
        
        // Update stats
        m_stats.totalAllocated -= header->size;
        m_stats.totalFreed += header->size;
        m_stats.allocationCount--;
        m_stats.totalFreeCount++;
        
        // Pop
        m_bottomOffset = header->previousOffset;
    }
    
    /// @brief Get marker for current top position
    [[nodiscard]] usize getTopMarker() const noexcept {
        return m_topOffset;
    }
    
    /// @brief Get marker for current bottom position
    [[nodiscard]] usize getBottomMarker() const noexcept {
        return m_bottomOffset;
    }
    
    /// @brief Reset top to marker
    void resetTopToMarker(usize marker) noexcept {
        if (marker <= m_topOffset) {
            usize freed = m_topOffset - marker;
            m_stats.totalFreed += freed;
            m_stats.totalAllocated -= freed;
            m_topOffset = marker;
        }
    }
    
    /// @brief Reset bottom to marker
    void resetBottomToMarker(usize marker) noexcept {
        if (marker >= m_bottomOffset) {
            usize freed = marker - m_bottomOffset;
            m_stats.totalFreed += freed;
            m_stats.totalAllocated -= freed;
            m_bottomOffset = marker;
        }
    }
    
    /// @brief Reset entire stack
    void reset() noexcept {
        m_stats.totalFreed += m_stats.totalAllocated;
        m_stats.totalFreeCount += m_stats.allocationCount;
        m_stats.totalAllocated = 0;
        m_stats.allocationCount = 0;
        m_topOffset = 0;
        m_bottomOffset = m_size;
    }
    
    /// @brief Get capacity
    [[nodiscard]] usize getCapacity() const noexcept {
        return m_size;
    }
    
    /// @brief Get free space
    [[nodiscard]] usize getFreeSpace() const noexcept {
        return m_bottomOffset - m_topOffset;
    }
    
    // Allocator interface implementation
    [[nodiscard]] usize getAllocatedSize() const noexcept override {
        return m_topOffset + (m_size - m_bottomOffset);
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
    struct AllocationHeader {
        usize previousOffset;
        usize size;
    };
    
    u8* m_buffer;
    usize m_size;
    usize m_topOffset;
    usize m_bottomOffset;
    const char* m_name;
    AllocationStats m_stats;
};

// =============================================================================
// Scoped Stack Allocation
// =============================================================================

/// @brief RAII helper for stack allocator scope
class ScopedStackReset {
public:
    explicit ScopedStackReset(StackAllocator& allocator) noexcept
        : m_allocator(allocator)
        , m_topMarker(allocator.getTopMarker())
        , m_bottomMarker(allocator.getBottomMarker()) {}
    
    ~ScopedStackReset() {
        m_allocator.resetTopToMarker(m_topMarker);
        m_allocator.resetBottomToMarker(m_bottomMarker);
    }
    
    // Non-copyable
    ScopedStackReset(const ScopedStackReset&) = delete;
    ScopedStackReset& operator=(const ScopedStackReset&) = delete;
    
private:
    StackAllocator& m_allocator;
    usize m_topMarker;
    usize m_bottomMarker;
};

} // namespace nova::memory

namespace nova {
    using StackAllocator = memory::StackAllocator;
}
