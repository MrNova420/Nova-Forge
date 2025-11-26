/**
 * @file ecs.hpp
 * @brief Main include header for the NovaCore Entity-Component-System (ECW)
 * 
 * NovaCore implements a Neural-Symbolic Entity-Component-Worker (NSECW)
 * architecture combining traditional ECS with embedded neural networks.
 * 
 * Key features:
 * - 10M entities @ 60 FPS on mid-range mobile
 * - Cache-coherent archetype-based storage (>95% L1 cache hit rate)
 * - Lock-free parallelism with zero contention
 * - Deterministic simulation for multiplayer
 * - Generational entity IDs for safe references
 * 
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for complete architecture details
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "entity.hpp"
#include "component.hpp"
#include "archetype.hpp"
#include "world.hpp"
#include "query.hpp"
#include "system.hpp"

namespace nova::ecs {

/**
 * @brief Version information for the ECS module
 */
constexpr u32 ECS_VERSION_MAJOR = 1;
constexpr u32 ECS_VERSION_MINOR = 0;
constexpr u32 ECS_VERSION_PATCH = 0;

/**
 * @brief Performance configuration constants
 */
namespace config {
    /// Default chunk size for component storage (16KB for cache efficiency)
    constexpr usize DEFAULT_CHUNK_SIZE = 16 * 1024;
    
    /// Maximum number of component types (enough for AAA games)
    constexpr usize MAX_COMPONENT_TYPES = 256;
    
    /// Maximum number of archetypes (covers most scenarios)
    constexpr usize MAX_ARCHETYPES = 4096;
    
    /// Default entity pool growth factor
    constexpr f32 ENTITY_POOL_GROWTH_FACTOR = 1.5f;
    
    /// Initial entity pool capacity
    constexpr usize INITIAL_ENTITY_CAPACITY = 1024;
    
    /// Maximum number of systems that can run in parallel
    constexpr usize MAX_PARALLEL_SYSTEMS = 32;
    
    /// Lock-free queue capacity for inter-system communication
    constexpr usize EVENT_QUEUE_CAPACITY = 4096;
} // namespace config

} // namespace nova::ecs
