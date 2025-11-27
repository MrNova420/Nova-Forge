/**
 * @file physics.hpp
 * @brief NovaCore Physics System - Main Include Header
 * 
 * Provides the complete physics simulation system including:
 * - Collision detection (broad phase + narrow phase)
 * - Rigid body dynamics
 * - Physics world management
 * - Integration with NovaCore ECS
 * 
 * The physics system is designed for mobile-first performance:
 * - SIMD-optimized collision detection (NEON/AVX2)
 * - Cache-friendly data structures
 * - Multi-threaded simulation support
 * - Thermal-aware throttling
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "physics_types.hpp"
#include "collision_shape.hpp"
#include "rigid_body.hpp"
#include "physics_world.hpp"

namespace nova::physics {

/**
 * @brief Physics system version information
 */
constexpr u32 PHYSICS_VERSION_MAJOR = 0;
constexpr u32 PHYSICS_VERSION_MINOR = 1;
constexpr u32 PHYSICS_VERSION_PATCH = 0;

/**
 * @brief Get the physics system version string
 * @return Version string in format "major.minor.patch"
 */
inline constexpr const char* getPhysicsVersion() {
    return "0.1.0";
}

} // namespace nova::physics
