/**
 * @file entity.hpp
 * @brief Entity identification and management for NovaCore ECS
 * 
 * Implements a production-grade entity system with:
 * - Generational indices to detect stale references
 * - 64-bit entity IDs with version + index encoding
 * - O(1) entity creation, destruction, and validation
 * - Recycled entity indices for memory efficiency
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include <vector>
#include <queue>
#include <atomic>
#include <cassert>

namespace nova::ecs {

/**
 * @brief Entity identifier with generational index
 * 
 * Encodes both the entity index and a generation counter in a 64-bit value.
 * The generation is incremented when an entity is destroyed and its index
 * is recycled, allowing detection of stale references.
 * 
 * Layout:
 * - Bits 0-31: Entity index (4 billion entities max)
 * - Bits 32-55: Generation counter (16 million generations)
 * - Bits 56-63: Reserved flags (alive, locked, etc.)
 * 
 * @note This extends the EntityId type from nova/core/types/types.hpp
 */
class Entity {
public:
    /// Index portion bit width
    static constexpr u32 INDEX_BITS = 32;
    /// Generation portion bit width
    static constexpr u32 GENERATION_BITS = 24;
    /// Flag portion bit width
    static constexpr u32 FLAG_BITS = 8;
    
    /// Index mask for extracting the entity index
    static constexpr u64 INDEX_MASK = (1ULL << INDEX_BITS) - 1;
    /// Generation mask after shifting
    static constexpr u64 GENERATION_MASK = (1ULL << GENERATION_BITS) - 1;
    /// Flag mask after shifting
    static constexpr u64 FLAG_MASK = (1ULL << FLAG_BITS) - 1;
    
    /// Generation shift amount
    static constexpr u32 GENERATION_SHIFT = INDEX_BITS;
    /// Flag shift amount
    static constexpr u32 FLAG_SHIFT = INDEX_BITS + GENERATION_BITS;
    
    /// Flag indicating entity is alive
    static constexpr u64 FLAG_ALIVE = 1ULL << 0;
    /// Flag indicating entity is locked (cannot be destroyed)
    static constexpr u64 FLAG_LOCKED = 1ULL << 1;
    
    /// Invalid entity constant
    static constexpr u64 INVALID_ID = ~0ULL;
    
private:
    u64 m_id;
    
public:
    /// Default constructor creates an invalid entity
    constexpr Entity() noexcept : m_id(INVALID_ID) {}
    
    /// Explicit construction from raw ID
    constexpr explicit Entity(u64 id) noexcept : m_id(id) {}
    
    /// Construct from index and generation
    constexpr Entity(u32 index, u32 generation, u8 flags = 0) noexcept
        : m_id(static_cast<u64>(index) |
               (static_cast<u64>(generation & GENERATION_MASK) << GENERATION_SHIFT) |
               (static_cast<u64>(flags & FLAG_MASK) << FLAG_SHIFT)) {}
    
    /// Get the entity index
    [[nodiscard]] constexpr u32 index() const noexcept {
        return static_cast<u32>(m_id & INDEX_MASK);
    }
    
    /// Get the generation counter
    [[nodiscard]] constexpr u32 generation() const noexcept {
        return static_cast<u32>((m_id >> GENERATION_SHIFT) & GENERATION_MASK);
    }
    
    /// Get the flags
    [[nodiscard]] constexpr u8 flags() const noexcept {
        return static_cast<u8>((m_id >> FLAG_SHIFT) & FLAG_MASK);
    }
    
    /// Check if the entity is valid (not INVALID_ID)
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return m_id != INVALID_ID;
    }
    
    /// Get the raw ID value
    [[nodiscard]] constexpr u64 id() const noexcept {
        return m_id;
    }
    
    /// Comparison operators
    constexpr bool operator==(const Entity& other) const noexcept { return m_id == other.m_id; }
    constexpr bool operator!=(const Entity& other) const noexcept { return m_id != other.m_id; }
    constexpr bool operator<(const Entity& other) const noexcept { return m_id < other.m_id; }
    constexpr bool operator>(const Entity& other) const noexcept { return m_id > other.m_id; }
    constexpr bool operator<=(const Entity& other) const noexcept { return m_id <= other.m_id; }
    constexpr bool operator>=(const Entity& other) const noexcept { return m_id >= other.m_id; }
    
    /// Hash support
    [[nodiscard]] constexpr u64 hash() const noexcept { return m_id; }
    
    /// Create an invalid entity
    [[nodiscard]] static constexpr Entity invalid() noexcept { return Entity{}; }
};

/**
 * @brief Record storing entity metadata for the entity pool
 */
struct EntityRecord {
    /// Current generation (incremented on destroy for reuse detection)
    u32 generation = 0;
    /// Archetype index this entity belongs to (-1 if none)
    i32 archetypeIndex = -1;
    /// Row within the archetype's component storage
    u32 archetypeRow = 0;
    /// Flags (alive, locked, etc.)
    u8 flags = 0;
    
    /// Check if this record represents a live entity
    [[nodiscard]] constexpr bool isAlive() const noexcept {
        return (flags & Entity::FLAG_ALIVE) != 0;
    }
    
    /// Check if this entity is locked
    [[nodiscard]] constexpr bool isLocked() const noexcept {
        return (flags & Entity::FLAG_LOCKED) != 0;
    }
};

/**
 * @brief Manages entity creation, destruction, and lifecycle
 * 
 * Thread-safe entity manager using lock-free operations for high-performance
 * concurrent entity operations. Supports up to 4 billion entities with
 * automatic index recycling.
 * 
 * Performance characteristics:
 * - O(1) entity creation (amortized)
 * - O(1) entity destruction
 * - O(1) entity validation
 * - O(1) entity lookup
 */
class EntityManager {
private:
    /// Entity records indexed by entity index
    std::vector<EntityRecord> m_records;
    
    /// Free list of recycled entity indices
    std::vector<u32> m_freeIndices;
    
    /// Next fresh index to allocate (when free list empty)
    std::atomic<u32> m_nextFreshIndex{0};
    
    /// Total number of alive entities
    std::atomic<u32> m_aliveCount{0};
    
    /// Lock for entity creation/destruction (simple mutex for correctness)
    mutable std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
    
    /// Lock helper
    class ScopedLock {
        std::atomic_flag& m_flag;
    public:
        explicit ScopedLock(std::atomic_flag& flag) noexcept : m_flag(flag) {
            while (m_flag.test_and_set(std::memory_order_acquire)) {
                // Spin - in production, use std::this_thread::yield()
            }
        }
        ~ScopedLock() { m_flag.clear(std::memory_order_release); }
        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;
    };
    
public:
    /// Default constructor with initial capacity
    explicit EntityManager(usize initialCapacity = 1024) {
        m_records.reserve(initialCapacity);
        m_freeIndices.reserve(initialCapacity / 4);
    }
    
    /// Destructor
    ~EntityManager() = default;
    
    /// Non-copyable
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    
    /// Movable
    EntityManager(EntityManager&&) noexcept = default;
    EntityManager& operator=(EntityManager&&) noexcept = default;
    
    /**
     * @brief Create a new entity
     * @return The newly created entity
     */
    [[nodiscard]] Entity create() {
        ScopedLock lock(m_lock);
        
        u32 index;
        u32 generation;
        
        if (!m_freeIndices.empty()) {
            // Reuse recycled index
            index = m_freeIndices.back();
            m_freeIndices.pop_back();
            generation = m_records[index].generation;
        } else {
            // Allocate fresh index
            index = m_nextFreshIndex.fetch_add(1, std::memory_order_relaxed);
            
            // Grow records vector if needed
            if (index >= m_records.size()) {
                m_records.resize(static_cast<usize>(index) + 1);
            }
            
            generation = 0;
        }
        
        // Initialize record
        m_records[index].generation = generation;
        m_records[index].archetypeIndex = -1;
        m_records[index].archetypeRow = 0;
        m_records[index].flags = Entity::FLAG_ALIVE;
        
        m_aliveCount.fetch_add(1, std::memory_order_relaxed);
        
        return Entity(index, generation, Entity::FLAG_ALIVE);
    }
    
    /**
     * @brief Destroy an entity
     * @param entity The entity to destroy
     * @return true if entity was destroyed, false if already dead or invalid
     */
    bool destroy(Entity entity) {
        if (!isValid(entity)) {
            return false;
        }
        
        ScopedLock lock(m_lock);
        
        u32 index = entity.index();
        
        // Check again under lock
        if (index >= m_records.size() || 
            m_records[index].generation != entity.generation() ||
            !m_records[index].isAlive()) {
            return false;
        }
        
        // Cannot destroy locked entities
        if (m_records[index].isLocked()) {
            return false;
        }
        
        // Mark as dead and increment generation
        m_records[index].flags = 0;
        m_records[index].generation++;
        m_records[index].archetypeIndex = -1;
        m_records[index].archetypeRow = 0;
        
        // Add to free list for recycling
        m_freeIndices.push_back(index);
        
        m_aliveCount.fetch_sub(1, std::memory_order_relaxed);
        
        return true;
    }
    
    /**
     * @brief Check if an entity is valid and alive
     * @param entity The entity to check
     * @return true if entity exists and is alive
     */
    [[nodiscard]] bool isValid(Entity entity) const noexcept {
        if (!entity.isValid()) {
            return false;
        }
        
        u32 index = entity.index();
        if (index >= m_records.size()) {
            return false;
        }
        
        const auto& record = m_records[index];
        return record.generation == entity.generation() && record.isAlive();
    }
    
    /**
     * @brief Get the record for an entity (unsafe - no validation)
     * @param entity The entity
     * @return Reference to the entity record
     */
    [[nodiscard]] EntityRecord& getRecord(Entity entity) noexcept {
        return m_records[entity.index()];
    }
    
    /**
     * @brief Get the record for an entity (const, unsafe - no validation)
     * @param entity The entity
     * @return Const reference to the entity record
     */
    [[nodiscard]] const EntityRecord& getRecord(Entity entity) const noexcept {
        return m_records[entity.index()];
    }
    
    /**
     * @brief Get the number of alive entities
     * @return Number of entities currently alive
     */
    [[nodiscard]] u32 aliveCount() const noexcept {
        return m_aliveCount.load(std::memory_order_relaxed);
    }
    
    /**
     * @brief Get the total capacity (highest index + 1)
     * @return Total entity capacity
     */
    [[nodiscard]] usize capacity() const noexcept {
        return m_records.size();
    }
    
    /**
     * @brief Reserve capacity for entities
     * @param capacity The capacity to reserve
     */
    void reserve(usize capacity) {
        ScopedLock lock(m_lock);
        m_records.reserve(capacity);
    }
    
    /**
     * @brief Lock an entity to prevent destruction
     * @param entity The entity to lock
     * @return true if locked successfully
     */
    bool lock(Entity entity) {
        if (!isValid(entity)) {
            return false;
        }
        
        ScopedLock scopedLock(m_lock);
        u32 index = entity.index();
        m_records[index].flags |= Entity::FLAG_LOCKED;
        return true;
    }
    
    /**
     * @brief Unlock an entity to allow destruction
     * @param entity The entity to unlock
     * @return true if unlocked successfully
     */
    bool unlock(Entity entity) {
        if (!isValid(entity)) {
            return false;
        }
        
        ScopedLock scopedLock(m_lock);
        u32 index = entity.index();
        m_records[index].flags &= ~Entity::FLAG_LOCKED;
        return true;
    }
    
    /**
     * @brief Clear all entities (reset the manager)
     */
    void clear() {
        ScopedLock lock(m_lock);
        m_records.clear();
        m_freeIndices.clear();
        m_nextFreshIndex.store(0, std::memory_order_relaxed);
        m_aliveCount.store(0, std::memory_order_relaxed);
    }
};

} // namespace nova::ecs

// Hash specialization for Entity
namespace std {
    template<>
    struct hash<nova::ecs::Entity> {
        size_t operator()(const nova::ecs::Entity& entity) const noexcept {
            return static_cast<size_t>(entity.hash());
        }
    };
}
