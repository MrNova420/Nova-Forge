/**
 * @file world.hpp
 * @brief World class - the main container for all ECS data
 * 
 * The World is the central hub of the NovaCore ECS system. It manages:
 * - Entity creation and destruction
 * - Component storage via archetypes
 * - System scheduling and execution
 * - Entity queries
 * 
 * Performance targets:
 * - 10M entities @ 60 FPS on mid-range mobile
 * - O(1) entity creation and destruction
 * - Cache-efficient iteration via archetype queries
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "entity.hpp"
#include "component.hpp"
#include "archetype.hpp"
#include "nova/core/types/types.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace nova::ecs {

/**
 * @brief The World is the main container for all ECS data
 * 
 * Thread safety:
 * - Entity creation/destruction is thread-safe
 * - Component modification should be done from a single thread or properly synchronized
 * - System execution can be parallelized via the scheduler
 */
class World {
private:
    /// Entity manager for entity lifecycle
    EntityManager m_entityManager;
    
    /// Archetype manager for component storage
    ArchetypeManager m_archetypeManager;
    
    /// Map from entity to archetype location (for fast lookup)
    struct EntityLocation {
        Archetype* archetype = nullptr;
        u32 chunkIndex = 0;
        u32 row = 0;
    };
    std::vector<EntityLocation> m_entityLocations;
    
    /// Pending entity destructions (deferred to end of frame)
    std::vector<Entity> m_pendingDestructions;
    
    /// Frame counter for deterministic simulation
    u64 m_frameCount = 0;
    
    /// Delta time for the current frame
    f32 m_deltaTime = 0.0f;
    
public:
    /// Default constructor
    World() {
        m_entityLocations.reserve(1024);
    }
    
    /// Destructor
    ~World() = default;
    
    /// Non-copyable
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    
    /// Movable
    World(World&&) noexcept = default;
    World& operator=(World&&) noexcept = default;
    
    // =========================================================================
    // Entity Management
    // =========================================================================
    
    /**
     * @brief Create a new entity with no components
     * @return The newly created entity
     */
    [[nodiscard]] Entity createEntity() {
        Entity entity = m_entityManager.create();
        ensureLocationCapacity(entity.index() + 1);
        m_entityLocations[entity.index()] = {}; // No archetype yet
        return entity;
    }
    
    /**
     * @brief Create a new entity with initial components
     * @tparam Components The component types to add
     * @param components The component values
     * @return The newly created entity
     */
    template<typename... Components>
    [[nodiscard]] Entity createEntity(Components&&... components) {
        Entity entity = createEntity();
        (addComponent<std::decay_t<Components>>(entity, std::forward<Components>(components)), ...);
        return entity;
    }
    
    /**
     * @brief Destroy an entity
     * @param entity The entity to destroy
     * @param immediate If true, destroy immediately. If false, defer to end of frame.
     * @return true if entity was marked for destruction
     */
    bool destroyEntity(Entity entity, bool immediate = true) {
        if (!m_entityManager.isValid(entity)) {
            return false;
        }
        
        if (immediate) {
            return destroyEntityImmediate(entity);
        } else {
            m_pendingDestructions.push_back(entity);
            return true;
        }
    }
    
    /**
     * @brief Check if an entity is valid (exists and is alive)
     * @param entity The entity to check
     * @return true if entity is valid
     */
    [[nodiscard]] bool isValid(Entity entity) const noexcept {
        return m_entityManager.isValid(entity);
    }
    
    /**
     * @brief Get the number of alive entities
     * @return Entity count
     */
    [[nodiscard]] u32 entityCount() const noexcept {
        return m_entityManager.aliveCount();
    }
    
    // =========================================================================
    // Component Management
    // =========================================================================
    
    /**
     * @brief Add a component to an entity
     * @tparam T The component type
     * @param entity The entity
     * @param component The component value
     * @return Reference to the added component
     */
    template<typename T>
    T& addComponent(Entity entity, T&& component = T{}) {
        static_assert(!std::is_reference_v<T>, "Component type cannot be a reference");
        
        // Register component type if not already registered
        NOVA_REGISTER_COMPONENT(std::decay_t<T>);
        
        ComponentId cid = componentId<std::decay_t<T>>();
        
        // Get current location
        EntityLocation& loc = m_entityLocations[entity.index()];
        
        // Build new component mask
        ComponentMask newMask;
        if (loc.archetype) {
            newMask = loc.archetype->mask();
            
            // If already has this component, just update it
            if (newMask.test(cid)) {
                T* existing = loc.archetype->getComponent<std::decay_t<T>>(loc.chunkIndex, loc.row);
                *existing = std::forward<T>(component);
                return *existing;
            }
        }
        newMask.set(cid);
        
        // Migrate to new archetype
        return migrateEntity<std::decay_t<T>>(entity, loc, newMask, std::forward<T>(component));
    }
    
    /**
     * @brief Remove a component from an entity
     * @tparam T The component type
     * @param entity The entity
     * @return true if component was removed
     */
    template<typename T>
    bool removeComponent(Entity entity) {
        if (!m_entityManager.isValid(entity)) {
            return false;
        }
        
        EntityLocation& loc = m_entityLocations[entity.index()];
        if (!loc.archetype) {
            return false;
        }
        
        ComponentId cid = componentId<T>();
        if (!loc.archetype->hasComponent(cid)) {
            return false;
        }
        
        // Build new mask without this component
        ComponentMask newMask = loc.archetype->mask();
        newMask.clear(cid);
        
        if (newMask.empty()) {
            // No components left - remove from archetype
            removeFromArchetype(entity, loc);
            loc = {};
        } else {
            // Migrate to archetype without this component
            migrateEntityRemove(entity, loc, newMask, cid);
        }
        
        return true;
    }
    
    /**
     * @brief Get a component from an entity
     * @tparam T The component type
     * @param entity The entity
     * @return Pointer to the component, or nullptr if not found
     */
    template<typename T>
    [[nodiscard]] T* getComponent(Entity entity) noexcept {
        if (!m_entityManager.isValid(entity)) {
            return nullptr;
        }
        
        const EntityLocation& loc = m_entityLocations[entity.index()];
        if (!loc.archetype) {
            return nullptr;
        }
        
        return loc.archetype->getComponent<T>(loc.chunkIndex, loc.row);
    }
    
    /**
     * @brief Get a const component from an entity
     */
    template<typename T>
    [[nodiscard]] const T* getComponent(Entity entity) const noexcept {
        return const_cast<World*>(this)->getComponent<T>(entity);
    }
    
    /**
     * @brief Check if an entity has a component
     * @tparam T The component type
     * @param entity The entity
     * @return true if entity has the component
     */
    template<typename T>
    [[nodiscard]] bool hasComponent(Entity entity) const noexcept {
        if (!m_entityManager.isValid(entity)) {
            return false;
        }
        
        const EntityLocation& loc = m_entityLocations[entity.index()];
        return loc.archetype && loc.archetype->hasComponent<T>();
    }
    
    /**
     * @brief Check if an entity has all specified components
     */
    template<typename... Components>
    [[nodiscard]] bool hasComponents(Entity entity) const noexcept {
        return (hasComponent<Components>(entity) && ...);
    }
    
    // =========================================================================
    // Querying
    // =========================================================================
    
    /**
     * @brief Iterate over all entities with specific components
     * @tparam Components The required component types
     * @param callback Function to call for each matching entity
     */
    template<typename... Components, typename Func>
    void each(Func&& callback) {
        ComponentMask required = ComponentMask::create<Components...>();
        
        auto archetypes = m_archetypeManager.query(required);
        
        for (Archetype* archetype : archetypes) {
            iterateArchetype<Components...>(archetype, std::forward<Func>(callback));
        }
    }
    
    /**
     * @brief Iterate over all entities with specific components (with entity access)
     * @tparam Components The required component types
     * @param callback Function(Entity, Components&...) to call for each matching entity
     */
    template<typename... Components, typename Func>
    void eachWithEntity(Func&& callback) {
        ComponentMask required = ComponentMask::create<Components...>();
        
        auto archetypes = m_archetypeManager.query(required);
        
        for (Archetype* archetype : archetypes) {
            iterateArchetypeWithEntity<Components...>(archetype, std::forward<Func>(callback));
        }
    }
    
    /**
     * @brief Count entities matching a component query
     */
    template<typename... Components>
    [[nodiscard]] u32 count() const {
        ComponentMask required = ComponentMask::create<Components...>();
        
        auto archetypes = const_cast<ArchetypeManager&>(m_archetypeManager).query(required);
        
        u32 total = 0;
        for (Archetype* archetype : archetypes) {
            total += archetype->entityCount();
        }
        return total;
    }
    
    // =========================================================================
    // Frame Management
    // =========================================================================
    
    /**
     * @brief Begin a new frame
     * @param deltaTime Time since last frame
     */
    void beginFrame(f32 deltaTime) {
        m_deltaTime = deltaTime;
        ++m_frameCount;
    }
    
    /**
     * @brief End the current frame
     * 
     * Processes deferred entity destructions
     */
    void endFrame() {
        // Process pending destructions
        for (Entity entity : m_pendingDestructions) {
            destroyEntityImmediate(entity);
        }
        m_pendingDestructions.clear();
    }
    
    /**
     * @brief Get the current delta time
     */
    [[nodiscard]] f32 deltaTime() const noexcept { return m_deltaTime; }
    
    /**
     * @brief Get the current frame count
     */
    [[nodiscard]] u64 frameCount() const noexcept { return m_frameCount; }
    
    // =========================================================================
    // Utility
    // =========================================================================
    
    /**
     * @brief Clear all entities and reset the world
     */
    void clear() {
        m_archetypeManager.clear();
        m_entityManager.clear();
        m_entityLocations.clear();
        m_pendingDestructions.clear();
        m_frameCount = 0;
        m_deltaTime = 0.0f;
    }
    
    /**
     * @brief Reserve capacity for entities
     */
    void reserve(usize capacity) {
        m_entityManager.reserve(capacity);
        m_entityLocations.reserve(capacity);
    }
    
    /**
     * @brief Get the entity manager
     */
    [[nodiscard]] EntityManager& entityManager() noexcept { return m_entityManager; }
    [[nodiscard]] const EntityManager& entityManager() const noexcept { return m_entityManager; }
    
    /**
     * @brief Get the archetype manager
     */
    [[nodiscard]] ArchetypeManager& archetypeManager() noexcept { return m_archetypeManager; }
    [[nodiscard]] const ArchetypeManager& archetypeManager() const noexcept { return m_archetypeManager; }
    
private:
    /// Ensure location vector has capacity
    void ensureLocationCapacity(usize capacity) {
        if (m_entityLocations.size() < capacity) {
            m_entityLocations.resize(capacity);
        }
    }
    
    /// Destroy entity immediately
    bool destroyEntityImmediate(Entity entity) {
        if (!m_entityManager.isValid(entity)) {
            return false;
        }
        
        EntityLocation& loc = m_entityLocations[entity.index()];
        
        // Remove from archetype if present
        if (loc.archetype) {
            removeFromArchetype(entity, loc);
        }
        
        // Destroy entity
        loc = {};
        return m_entityManager.destroy(entity);
    }
    
    /// Remove entity from its current archetype
    void removeFromArchetype(Entity entity, EntityLocation& loc) {
        if (!loc.archetype) return;
        
        // Remove from archetype (may swap with another entity)
        Entity movedEntity = loc.archetype->removeEntity(loc.chunkIndex, loc.row);
        
        // Update location of moved entity
        if (movedEntity.isValid()) {
            m_entityLocations[movedEntity.index()].row = loc.row;
        }
    }
    
    /// Migrate entity to new archetype with added component
    template<typename T>
    T& migrateEntity(Entity entity, EntityLocation& loc, const ComponentMask& newMask, T&& newComponent) {
        // Get or create new archetype
        Archetype* newArchetype = m_archetypeManager.getOrCreate(newMask);
        
        // Allocate in new archetype
        auto [newChunkIndex, newRow] = newArchetype->allocateEntity(entity);
        
        // Copy existing components
        if (loc.archetype) {
            for (ComponentId cid : loc.archetype->componentIds()) {
                if (newArchetype->hasComponent(cid)) {
                    const ComponentInfo* info = ComponentRegistry::instance().getInfo(cid);
                    if (info) {
                        usize oldIdx = loc.archetype->getComponentIndex(cid);
                        usize newIdx = newArchetype->getComponentIndex(cid);
                        
                        Chunk* oldChunk = loc.archetype->getChunk(loc.chunkIndex);
                        Chunk* newChunk = newArchetype->getChunk(newChunkIndex);
                        
                        u8* src = static_cast<u8*>(oldChunk->getComponentArray(oldIdx)) + loc.row * info->size;
                        u8* dst = static_cast<u8*>(newChunk->getComponentArray(newIdx)) + newRow * info->size;
                        
                        if (info->moveConstruct) {
                            info->destruct(dst); // Destruct default-constructed
                            info->moveConstruct(dst, src);
                        } else if (info->isTrivial) {
                            std::memcpy(dst, src, info->size);
                        }
                    }
                }
            }
            
            // Remove from old archetype
            removeFromArchetype(entity, loc);
        }
        
        // Store new component
        T* componentPtr = newArchetype->getComponent<T>(newChunkIndex, newRow);
        *componentPtr = std::forward<T>(newComponent);
        
        // Update entity record
        auto& record = m_entityManager.getRecord(entity);
        record.archetypeIndex = static_cast<i32>(newArchetype->id());
        record.archetypeRow = newRow;
        
        // Update location
        loc.archetype = newArchetype;
        loc.chunkIndex = newChunkIndex;
        loc.row = newRow;
        
        return *componentPtr;
    }
    
    /// Migrate entity to archetype without a component
    void migrateEntityRemove(Entity entity, EntityLocation& loc, const ComponentMask& newMask, ComponentId removedId) {
        // Get or create new archetype
        Archetype* newArchetype = m_archetypeManager.getOrCreate(newMask);
        
        // Allocate in new archetype
        auto [newChunkIndex, newRow] = newArchetype->allocateEntity(entity);
        
        // Copy remaining components
        for (ComponentId cid : loc.archetype->componentIds()) {
            if (cid != removedId && newArchetype->hasComponent(cid)) {
                const ComponentInfo* info = ComponentRegistry::instance().getInfo(cid);
                if (info) {
                    usize oldIdx = loc.archetype->getComponentIndex(cid);
                    usize newIdx = newArchetype->getComponentIndex(cid);
                    
                    Chunk* oldChunk = loc.archetype->getChunk(loc.chunkIndex);
                    Chunk* newChunk = newArchetype->getChunk(newChunkIndex);
                    
                    u8* src = static_cast<u8*>(oldChunk->getComponentArray(oldIdx)) + loc.row * info->size;
                    u8* dst = static_cast<u8*>(newChunk->getComponentArray(newIdx)) + newRow * info->size;
                    
                    if (info->moveConstruct) {
                        info->destruct(dst);
                        info->moveConstruct(dst, src);
                    } else if (info->isTrivial) {
                        std::memcpy(dst, src, info->size);
                    }
                }
            }
        }
        
        // Remove from old archetype
        removeFromArchetype(entity, loc);
        
        // Update entity record
        auto& record = m_entityManager.getRecord(entity);
        record.archetypeIndex = static_cast<i32>(newArchetype->id());
        record.archetypeRow = newRow;
        
        // Update location
        loc.archetype = newArchetype;
        loc.chunkIndex = newChunkIndex;
        loc.row = newRow;
    }
    
    /// Iterate over an archetype with component callback
    template<typename... Components, typename Func>
    void iterateArchetype(Archetype* archetype, Func&& callback) {
        for (usize chunkIdx = 0; chunkIdx < archetype->chunkCount(); ++chunkIdx) {
            Chunk* chunk = archetype->getChunk(chunkIdx);
            u32 count = chunk->count();
            
            // Get component arrays
            auto arrays = std::make_tuple(archetype->getComponentArray<Components>(static_cast<u32>(chunkIdx))...);
            
            // Iterate entities in chunk
            for (u32 row = 0; row < count; ++row) {
                callback(std::get<decltype(archetype->getComponentArray<Components>(0))>(arrays)[row]...);
            }
        }
    }
    
    /// Iterate over an archetype with entity and component callback
    template<typename... Components, typename Func>
    void iterateArchetypeWithEntity(Archetype* archetype, Func&& callback) {
        for (usize chunkIdx = 0; chunkIdx < archetype->chunkCount(); ++chunkIdx) {
            Chunk* chunk = archetype->getChunk(chunkIdx);
            u32 count = chunk->count();
            
            // Get component arrays
            auto arrays = std::make_tuple(archetype->getComponentArray<Components>(static_cast<u32>(chunkIdx))...);
            
            // Iterate entities in chunk
            for (u32 row = 0; row < count; ++row) {
                Entity entity = chunk->getEntity(row);
                callback(entity, std::get<decltype(archetype->getComponentArray<Components>(0))>(arrays)[row]...);
            }
        }
    }
};

} // namespace nova::ecs
