/**
 * @file particle_system.hpp
 * @brief NovaCore Particle System™ - Main Particle Manager
 * 
 * Production-grade particle system manager for the NovaCore engine.
 * Handles particle simulation, emission, forces, and rendering.
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include "particle_types.hpp"
#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>

namespace nova::particle {

/**
 * @brief Particle system configuration
 */
struct ParticleSystemConfig {
    u32 maxEmitters = 256;          ///< Maximum active emitters
    u32 maxParticlesTotal = 100000; ///< Maximum total particles
    u32 maxForceFields = 64;        ///< Maximum global force fields
    bool enableCollision = true;     ///< Enable collision detection
    bool enableGPUSimulation = false; ///< Use GPU for simulation
};

/**
 * @brief Particle emitter instance
 */
class ParticleEmitter {
public:
    ParticleEmitter() = default;
    ~ParticleEmitter() = default;
    
    /// Initialize emitter with system data
    void initialize(const ParticleSystemData& data);
    
    /// Reset emitter
    void reset();
    
    /// Update emitter and particles
    void update(f32 deltaTime, const std::vector<ForceField>& globalForces);
    
    // Playback control
    void play();
    void pause();
    void stop();
    void restart();
    
    bool isPlaying() const { return m_playing; }
    bool isPaused() const { return m_paused; }
    bool isAlive() const { return m_playing || !m_particles.empty(); }
    
    // Transform
    void setPosition(const Vec3& pos) { m_position = pos; }
    void setRotation(const Quat& rot) { m_rotation = rot; }
    void setScale(const Vec3& scale) { m_scale = scale; }
    
    Vec3 getPosition() const { return m_position; }
    Quat getRotation() const { return m_rotation; }
    Vec3 getScale() const { return m_scale; }
    
    // Properties
    void setPlaybackSpeed(f32 speed) { m_playbackSpeed = speed; }
    f32 getPlaybackSpeed() const { return m_playbackSpeed; }
    
    // Emission control
    void emit(u32 count);
    void triggerBurst(u32 burstIndex);
    
    // Callbacks
    void setSpawnCallback(ParticleSpawnCallback callback);
    void setDeathCallback(ParticleDeathCallback callback);
    void setCollisionCallback(ParticleCollisionCallback callback);
    
    // Access particles for rendering
    const std::vector<Particle>& getParticles() const { return m_particles; }
    u32 getParticleCount() const { return static_cast<u32>(m_particles.size()); }
    
    // Get system data
    const ParticleSystemData& getData() const { return m_data; }
    
private:
    ParticleSystemData m_data;
    std::vector<Particle> m_particles;
    
    // Transform
    Vec3 m_position{};
    Quat m_rotation{};
    Vec3 m_scale{1.0f, 1.0f, 1.0f};
    
    // Playback state
    bool m_playing = false;
    bool m_paused = false;
    f32 m_playbackSpeed = 1.0f;
    f32 m_time = 0.0f;
    f32 m_emissionAccumulator = 0.0f;
    
    // Burst tracking
    std::vector<i32> m_burstCyclesRemaining;
    std::vector<f32> m_burstNextTime;
    
    // Random state
    u32 m_randomSeed = 12345;
    
    // Callbacks
    ParticleSpawnCallback m_spawnCallback;
    ParticleDeathCallback m_deathCallback;
    ParticleCollisionCallback m_collisionCallback;
    
    // Internal methods
    void emitParticle();
    void updateParticle(Particle& p, f32 dt, const std::vector<ForceField>& forces);
    void applyModules(Particle& p, f32 dt);
    void applyForces(Particle& p, f32 dt, const std::vector<ForceField>& forces);
    void checkCollisions(Particle& p, f32 dt);
    void removeDeadParticles();
    
    Vec3 getEmissionPosition();
    Vec3 getEmissionDirection();
    u32 nextRandom();
    f32 randomFloat();
    f32 randomFloat(f32 minVal, f32 maxVal);
};

/**
 * @brief Main particle system singleton
 */
class ParticleManager {
public:
    /// Get singleton instance
    static ParticleManager& get();
    
    /// Initialize the system
    bool initialize(const ParticleSystemConfig& config = {});
    
    /// Shutdown the system
    void shutdown();
    
    /// Update all emitters
    void update(f32 deltaTime);
    
    // Emitter management
    ParticleEmitterHandle createEmitter(const ParticleSystemData& data);
    void destroyEmitter(ParticleEmitterHandle handle);
    ParticleEmitter* getEmitter(ParticleEmitterHandle handle);
    
    // System assets
    ParticleSystemHandle loadSystem(const std::string& path);
    ParticleSystemHandle createSystem(const ParticleSystemData& data);
    void unloadSystem(ParticleSystemHandle handle);
    const ParticleSystemData* getSystem(ParticleSystemHandle handle) const;
    
    // Global force fields
    void addGlobalForceField(const ForceField& field);
    void removeGlobalForceField(const std::string& name);
    void clearGlobalForceFields();
    
    // Statistics
    const ParticleStats& getStats() const { return m_stats; }
    void resetStats();
    
    // Configuration
    const ParticleSystemConfig& getConfig() const { return m_config; }
    
private:
    ParticleManager() = default;
    ~ParticleManager() = default;
    
    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;
    
    bool m_initialized = false;
    ParticleSystemConfig m_config;
    ParticleStats m_stats;
    
    // Storage
    std::unordered_map<u64, std::unique_ptr<ParticleEmitter>> m_emitters;
    std::unordered_map<u64, ParticleSystemData> m_systems;
    std::vector<ForceField> m_globalForces;
    
    std::atomic<u64> m_nextEmitterId{1};
    std::atomic<u64> m_nextSystemId{1};
    
    mutable std::mutex m_mutex;
    
    // Loading helpers
    bool loadSystemFromFile(const std::string& path, ParticleSystemData& outData);
};

// ============================================================================
// Convenience Functions
// ============================================================================

/**
 * @brief Initialize particle system
 */
inline bool initializeParticles(const ParticleSystemConfig& config = {}) {
    return ParticleManager::get().initialize(config);
}

/**
 * @brief Shutdown particle system
 */
inline void shutdownParticles() {
    ParticleManager::get().shutdown();
}

/**
 * @brief Update particle system
 */
inline void updateParticles(f32 deltaTime) {
    ParticleManager::get().update(deltaTime);
}

} // namespace nova::particle
