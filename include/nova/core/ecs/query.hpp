/**
 * @file query.hpp
 * @brief Query system for efficient entity iteration
 * 
 * Provides type-safe, cache-efficient queries for iterating over
 * entities with specific component combinations.
 * 
 * Features:
 * - Compile-time query validation
 * - Optional and excluded component filters
 * - Parallel iteration support
 * - Query caching for repeated queries
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "entity.hpp"
#include "component.hpp"
#include "archetype.hpp"
#include "nova/core/types/types.hpp"
#include <vector>
#include <functional>
#include <type_traits>

namespace nova::ecs {

// Forward declarations
class World;

/**
 * @brief Query filter specifying required, optional, and excluded components
 */
template<typename... Required>
struct With {};

template<typename... Excluded>
struct Without {};

template<typename... Optional>
struct Maybe {};

/**
 * @brief Query descriptor for caching matched archetypes
 */
class QueryDescriptor {
private:
    ComponentMask m_required;
    ComponentMask m_excluded;
    ComponentMask m_optional;
    u64 m_hash = 0;
    
public:
    /// Default constructor
    QueryDescriptor() = default;
    
    /// Construct from component masks
    QueryDescriptor(ComponentMask required, ComponentMask excluded = {}, ComponentMask optional = {})
        : m_required(required), m_excluded(excluded), m_optional(optional) {
        // Compute hash
        m_hash = m_required.hash() ^ (m_excluded.hash() * 31) ^ (m_optional.hash() * 127);
    }
    
    /// Get required component mask
    [[nodiscard]] const ComponentMask& required() const noexcept { return m_required; }
    
    /// Get excluded component mask
    [[nodiscard]] const ComponentMask& excluded() const noexcept { return m_excluded; }
    
    /// Get optional component mask
    [[nodiscard]] const ComponentMask& optional() const noexcept { return m_optional; }
    
    /// Get hash for caching
    [[nodiscard]] u64 hash() const noexcept { return m_hash; }
    
    /// Check if an archetype matches this query
    [[nodiscard]] bool matches(const Archetype& archetype) const noexcept {
        return archetype.mask().containsAll(m_required) &&
               !archetype.mask().containsAny(m_excluded);
    }
    
    /// Create from component types
    template<typename... Components>
    static QueryDescriptor create() {
        return QueryDescriptor(ComponentMask::create<Components...>());
    }
    
    /// Create with excluded components
    template<typename... Required, typename... Excluded>
    static QueryDescriptor createWithExcluded() {
        return QueryDescriptor(
            ComponentMask::create<Required...>(),
            ComponentMask::create<Excluded...>()
        );
    }
};

/**
 * @brief Query result providing iteration over matched entities
 */
class QueryResult {
private:
    std::vector<Archetype*> m_archetypes;
    
public:
    /// Default constructor
    QueryResult() = default;
    
    /// Construct from archetype list
    explicit QueryResult(std::vector<Archetype*> archetypes)
        : m_archetypes(std::move(archetypes)) {}
    
    /// Get matched archetypes
    [[nodiscard]] const std::vector<Archetype*>& archetypes() const noexcept {
        return m_archetypes;
    }
    
    /// Get total entity count across all matched archetypes
    [[nodiscard]] u32 count() const noexcept {
        u32 total = 0;
        for (const Archetype* arch : m_archetypes) {
            total += arch->entityCount();
        }
        return total;
    }
    
    /// Check if result is empty
    [[nodiscard]] bool empty() const noexcept {
        return count() == 0;
    }
    
    /**
     * @brief Iterate over all matching entities with component access
     * @tparam Components Component types to access
     * @param callback Function(Components&...) called for each entity
     */
    template<typename... Components, typename Func>
    void each(Func&& callback) {
        for (Archetype* archetype : m_archetypes) {
            for (usize chunkIdx = 0; chunkIdx < archetype->chunkCount(); ++chunkIdx) {
                Chunk* chunk = archetype->getChunk(chunkIdx);
                u32 entityCount = chunk->count();
                
                // Get component arrays
                auto arrays = std::make_tuple(
                    archetype->getComponentArray<Components>(static_cast<u32>(chunkIdx))...
                );
                
                // Iterate entities
                for (u32 row = 0; row < entityCount; ++row) {
                    callback(std::get<decltype(archetype->getComponentArray<Components>(0))>(arrays)[row]...);
                }
            }
        }
    }
    
    /**
     * @brief Iterate with entity access
     */
    template<typename... Components, typename Func>
    void eachWithEntity(Func&& callback) {
        for (Archetype* archetype : m_archetypes) {
            for (usize chunkIdx = 0; chunkIdx < archetype->chunkCount(); ++chunkIdx) {
                Chunk* chunk = archetype->getChunk(chunkIdx);
                u32 entityCount = chunk->count();
                
                auto arrays = std::make_tuple(
                    archetype->getComponentArray<Components>(static_cast<u32>(chunkIdx))...
                );
                
                for (u32 row = 0; row < entityCount; ++row) {
                    Entity entity = chunk->getEntity(row);
                    callback(entity, std::get<decltype(archetype->getComponentArray<Components>(0))>(arrays)[row]...);
                }
            }
        }
    }
};

/**
 * @brief Type-safe query builder
 * 
 * Example usage:
 * @code
 * auto query = Query<Position, Velocity>::create(world);
 * query.each([](Position& pos, Velocity& vel) {
 *     pos.x += vel.x * dt;
 *     pos.y += vel.y * dt;
 * });
 * @endcode
 */
template<typename... Components>
class Query {
private:
    QueryDescriptor m_descriptor;
    QueryResult m_result;
    
public:
    /// Create query from world
    static Query create(World& world) {
        Query query;
        query.m_descriptor = QueryDescriptor::create<Components...>();
        query.refresh(world);
        return query;
    }
    
    /// Create query with excluded components
    template<typename... Excluded>
    static Query createWithout(World& world) {
        Query query;
        query.m_descriptor = QueryDescriptor::createWithExcluded<Components..., Excluded...>();
        query.refresh(world);
        return query;
    }
    
    /// Refresh the query (re-match archetypes)
    void refresh(World& world) {
        ComponentMask required = ComponentMask::create<Components...>();
        auto archetypes = world.archetypeManager().query(
            m_descriptor.required(),
            m_descriptor.excluded()
        );
        m_result = QueryResult(std::move(archetypes));
    }
    
    /// Get the query result
    [[nodiscard]] QueryResult& result() noexcept { return m_result; }
    [[nodiscard]] const QueryResult& result() const noexcept { return m_result; }
    
    /// Get entity count
    [[nodiscard]] u32 count() const noexcept { return m_result.count(); }
    
    /// Check if empty
    [[nodiscard]] bool empty() const noexcept { return m_result.empty(); }
    
    /**
     * @brief Iterate over matching entities
     */
    template<typename Func>
    void each(Func&& callback) {
        m_result.template each<Components...>(std::forward<Func>(callback));
    }
    
    /**
     * @brief Iterate with entity access
     */
    template<typename Func>
    void eachWithEntity(Func&& callback) {
        m_result.template eachWithEntity<Components...>(std::forward<Func>(callback));
    }
};

/**
 * @brief Query cache for efficient repeated queries
 */
class QueryCache {
private:
    std::unordered_map<u64, QueryResult> m_cache;
    u64 m_version = 0;
    
public:
    /// Clear the cache (call when archetypes change)
    void invalidate() {
        m_cache.clear();
        ++m_version;
    }
    
    /// Get cache version
    [[nodiscard]] u64 version() const noexcept { return m_version; }
    
    /// Get or create cached query result
    QueryResult& getOrCreate(const QueryDescriptor& descriptor, ArchetypeManager& archetypes) {
        u64 hash = descriptor.hash();
        
        auto it = m_cache.find(hash);
        if (it != m_cache.end()) {
            return it->second;
        }
        
        // Create new cached result
        auto matchedArchetypes = archetypes.query(
            descriptor.required(),
            descriptor.excluded()
        );
        
        m_cache[hash] = QueryResult(std::move(matchedArchetypes));
        return m_cache[hash];
    }
};

} // namespace nova::ecs
