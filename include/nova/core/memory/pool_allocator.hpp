// =============================================================================
// NovaCore Engine - Pool Allocator
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Fixed-size block pool allocator for efficient allocation of same-sized objects.
// Characteristics:
// - O(1) allocation and deallocation
// - No fragmentation
// - Perfect for game objects, components, particles
// - Cache-friendly allocation patterns
// =============================================================================

#pragma once

#include "nova/core/memory/allocator.hpp"

namespace nova::memory {

/// @brief Pool allocator for fixed-size objects
/// @note Best for frequently allocated/deallocated objects of the same size
class PoolAllocator final : public Allocator {
public:
    /// @brief Construct pool allocator
    /// @param buffer Pre-allocated memory buffer
    /// @param bufferSize Total buffer size
    /// @param blockSize Size of each block (must be >= sizeof(void*))
    /// @param name Optional name for debugging
    PoolAllocator(void* buffer, usize bufferSize, usize blockSize, const char* name = "PoolAllocator") noexcept
        : m_buffer(static_cast<u8*>(buffer))
        , m_bufferSize(bufferSize)
        , m_blockSize(std::max(blockSize, sizeof(void*)))  // Minimum size for free list pointer
        , m_freeList(nullptr)
        , m_name(name)
    {
        m_stats.reset();
        
        // Calculate number of blocks
        m_blockCount = m_bufferSize / m_blockSize;
        
        // Initialize free list
        initializeFreeList();
    }
    
    /// @brief Allocate one block
    /// @param size Must be <= block size
    /// @param alignment Ignored (blocks are naturally aligned)
    /// @return Pointer to allocated block, or nullptr if pool exhausted
    [[nodiscard]] void* allocate(usize size, [[maybe_unused]] usize alignment = DEFAULT_ALIGNMENT) noexcept override {
        if (size > m_blockSize || m_freeList == nullptr) {
            return nullptr;
        }
        
        // Pop from free list
        void* block = m_freeList;
        m_freeList = *static_cast<void**>(m_freeList);
        
        // Update stats
        m_stats.totalAllocated += m_blockSize;
        m_stats.allocationCount++;
        m_stats.totalAllocationCount++;
        if (m_stats.totalAllocated > m_stats.peakAllocated) {
            m_stats.peakAllocated = m_stats.totalAllocated;
        }
        
        return block;
    }
    
    /// @brief Return block to pool
    /// @param ptr Pointer to block (must be from this pool)
    void deallocate(void* ptr) noexcept override {
        if (ptr == nullptr || !owns(ptr)) {
            return;
        }
        
        // Push to free list
        *static_cast<void**>(ptr) = m_freeList;
        m_freeList = ptr;
        
        // Update stats
        m_stats.totalAllocated -= m_blockSize;
        m_stats.totalFreed += m_blockSize;
        m_stats.allocationCount--;
        m_stats.totalFreeCount++;
    }
    
    /// @brief Get block size
    [[nodiscard]] usize getBlockSize() const noexcept {
        return m_blockSize;
    }
    
    /// @brief Get total block count
    [[nodiscard]] usize getBlockCount() const noexcept {
        return m_blockCount;
    }
    
    /// @brief Get number of free blocks
    [[nodiscard]] usize getFreeBlockCount() const noexcept {
        return m_blockCount - (m_stats.allocationCount);
    }
    
    /// @brief Reset pool - returns all blocks to free list
    void reset() noexcept {
        m_stats.totalFreed += m_stats.totalAllocated;
        m_stats.totalFreeCount += m_stats.allocationCount;
        m_stats.totalAllocated = 0;
        m_stats.allocationCount = 0;
        initializeFreeList();
    }
    
    // Allocator interface implementation
    [[nodiscard]] usize getAllocatedSize() const noexcept override {
        return m_stats.totalAllocated;
    }
    
    [[nodiscard]] AllocationStats getStats() const noexcept override {
        return m_stats;
    }
    
    [[nodiscard]] bool owns(const void* ptr) const noexcept override {
        auto p = static_cast<const u8*>(ptr);
        return p >= m_buffer && p < m_buffer + m_bufferSize;
    }
    
    [[nodiscard]] const char* getName() const noexcept override {
        return m_name;
    }
    
private:
    void initializeFreeList() noexcept {
        m_freeList = nullptr;
        
        // Build free list in reverse order for cache-friendly allocation
        for (usize i = m_blockCount; i > 0; --i) {
            void* block = m_buffer + (i - 1) * m_blockSize;
            *static_cast<void**>(block) = m_freeList;
            m_freeList = block;
        }
    }
    
    u8* m_buffer;
    usize m_bufferSize;
    usize m_blockSize;
    usize m_blockCount;
    void* m_freeList;
    const char* m_name;
    AllocationStats m_stats;
};

// =============================================================================
// Typed Pool Allocator
// =============================================================================

/// @brief Type-safe pool allocator for a specific object type
template<typename T>
class TypedPoolAllocator {
public:
    /// Construct with buffer
    TypedPoolAllocator(void* buffer, usize bufferSize, const char* name = "TypedPool") noexcept
        : m_pool(buffer, bufferSize, sizeof(T), name) {}
    
    /// Allocate and construct object
    template<typename... Args>
    [[nodiscard]] T* create(Args&&... args) noexcept {
        void* memory = m_pool.allocate(sizeof(T));
        if (!memory) return nullptr;
        return new (memory) T(static_cast<Args&&>(args)...);
    }
    
    /// Destroy and deallocate object
    void destroy(T* ptr) noexcept {
        if (ptr) {
            ptr->~T();
            m_pool.deallocate(ptr);
        }
    }
    
    /// Get number of free slots
    [[nodiscard]] usize getFreeCount() const noexcept {
        return m_pool.getFreeBlockCount();
    }
    
    /// Get total capacity
    [[nodiscard]] usize getCapacity() const noexcept {
        return m_pool.getBlockCount();
    }
    
    /// Reset pool
    void reset() noexcept {
        m_pool.reset();
    }
    
    /// Get underlying pool allocator
    [[nodiscard]] PoolAllocator& getPool() noexcept { return m_pool; }
    [[nodiscard]] const PoolAllocator& getPool() const noexcept { return m_pool; }
    
private:
    PoolAllocator m_pool;
};

} // namespace nova::memory

// Bring common items into nova namespace
namespace nova {
    using PoolAllocator = memory::PoolAllocator;
}
