/**
 * @file particle_gpu.hpp
 * @brief Advanced GPU Particle Simulation System for NovaCore Engine
 *
 * Provides high-performance GPU-accelerated particle systems including:
 * - Compute shader-based particle simulation
 * - GPU-driven emission and spawning
 * - Advanced forces (turbulence, curl noise, vector fields)
 * - Particle collision with depth buffer
 * - Sort-free blended rendering
 * - Ribbon and trail particles
 * - Mesh particles with instancing
 * - Event-driven sub-emitters
 *
 * Copyright (c) NovaForge Platform - All rights reserved.
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <memory>
#include <unordered_map>

namespace nova {

// ============================================================================
// Configuration Constants
// ============================================================================

/**
 * @brief Configuration constants for GPU particle system
 */
struct GPUParticleConfig {
    // Particle limits
    static constexpr u32 MAX_PARTICLES_PER_EMITTER = 1048576;  // 1M particles
    static constexpr u32 MAX_EMITTERS = 256;
    static constexpr u32 MAX_EMITTER_EVENTS = 32;
    static constexpr u32 MAX_FORCE_FIELDS = 64;
    static constexpr u32 MAX_COLLISION_PLANES = 16;
    static constexpr u32 MAX_VECTOR_FIELD_RESOLUTION = 256;
    
    // Rendering limits
    static constexpr u32 MAX_PARTICLE_TEXTURES = 8;
    static constexpr u32 MAX_RIBBONS_PER_EMITTER = 1024;
    static constexpr u32 MAX_RIBBON_SEGMENTS = 64;
    static constexpr u32 MAX_MESH_PARTICLES = 65536;
    
    // Simulation
    static constexpr u32 PARTICLE_WORKGROUP_SIZE = 256;
    static constexpr u32 EMISSION_WORKGROUP_SIZE = 64;
    static constexpr u32 SORT_WORKGROUP_SIZE = 512;
    static constexpr f32 DEFAULT_FIXED_TIMESTEP = 1.0f / 60.0f;
    
    // Memory
    static constexpr u32 PARTICLE_STRIDE = 64;  // Bytes per particle
    static constexpr u32 DEAD_LIST_BLOCK_SIZE = 1024;
};

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief GPU particle simulation mode
 */
enum class GPUSimulationMode : u8 {
    Standard,       ///< Standard compute simulation
    Persistent,     ///< Persistent particles (no respawn)
    EventDriven,    ///< Spawns based on events
    Scripted,       ///< Custom simulation via expression graphs
    
    Count
};

inline constexpr const char* getGPUSimulationModeName(GPUSimulationMode mode) {
    switch (mode) {
        case GPUSimulationMode::Standard: return "Standard";
        case GPUSimulationMode::Persistent: return "Persistent";
        case GPUSimulationMode::EventDriven: return "EventDriven";
        case GPUSimulationMode::Scripted: return "Scripted";
        default: return "Unknown";
    }
}

/**
 * @brief GPU particle rendering mode
 */
enum class GPURenderMode : u8 {
    Billboard,      ///< Camera-facing quads
    StretchedBillboard, ///< Velocity-stretched billboards
    Mesh,           ///< Instanced meshes
    Ribbon,         ///< Ribbon/trail particles
    Beam,           ///< Laser/beam particles
    DecalProjection, ///< Projected decals
    VolumeLight,    ///< Volumetric light particles
    
    Count
};

inline constexpr const char* getGPURenderModeName(GPURenderMode mode) {
    switch (mode) {
        case GPURenderMode::Billboard: return "Billboard";
        case GPURenderMode::StretchedBillboard: return "StretchedBillboard";
        case GPURenderMode::Mesh: return "Mesh";
        case GPURenderMode::Ribbon: return "Ribbon";
        case GPURenderMode::Beam: return "Beam";
        case GPURenderMode::DecalProjection: return "DecalProjection";
        case GPURenderMode::VolumeLight: return "VolumeLight";
        default: return "Unknown";
    }
}

/**
 * @brief Force field type
 */
enum class ForceFieldType : u8 {
    Directional,    ///< Constant direction force (gravity, wind)
    Point,          ///< Point attractor/repulsor
    Vortex,         ///< Spinning vortex
    VectorField,    ///< 3D vector field texture
    CurlNoise,      ///< Procedural curl noise
    Turbulence,     ///< Multi-octave turbulence
    Drag,           ///< Linear/quadratic drag
    Orbital,        ///< Orbital motion around axis
    
    Count
};

inline constexpr const char* getForceFieldTypeName(ForceFieldType type) {
    switch (type) {
        case ForceFieldType::Directional: return "Directional";
        case ForceFieldType::Point: return "Point";
        case ForceFieldType::Vortex: return "Vortex";
        case ForceFieldType::VectorField: return "VectorField";
        case ForceFieldType::CurlNoise: return "CurlNoise";
        case ForceFieldType::Turbulence: return "Turbulence";
        case ForceFieldType::Drag: return "Drag";
        case ForceFieldType::Orbital: return "Orbital";
        default: return "Unknown";
    }
}

/**
 * @brief Collision mode
 */
enum class ParticleCollisionMode : u8 {
    None,           ///< No collision
    DepthBuffer,    ///< Collide with depth buffer
    Planes,         ///< Collide with planes
    SDF,            ///< Signed distance field collision
    
    Count
};

inline constexpr const char* getParticleCollisionModeName(ParticleCollisionMode mode) {
    switch (mode) {
        case ParticleCollisionMode::None: return "None";
        case ParticleCollisionMode::DepthBuffer: return "DepthBuffer";
        case ParticleCollisionMode::Planes: return "Planes";
        case ParticleCollisionMode::SDF: return "SDF";
        default: return "Unknown";
    }
}

/**
 * @brief Particle sort mode for transparency
 */
enum class ParticleSortMode : u8 {
    None,           ///< No sorting (additive blending)
    OldestFirst,    ///< Sort by spawn time
    NewestFirst,    ///< Reverse sort by spawn time
    ByDistance,     ///< Sort by camera distance
    ByDistanceReverse, ///< Reverse distance sort
    
    Count
};

/**
 * @brief Emission event type
 */
enum class EmissionEventType : u8 {
    Burst,          ///< One-time burst emission
    OnCollision,    ///< Emit on particle collision
    OnDeath,        ///< Emit when particle dies
    OnLifetime,     ///< Emit at specific lifetime
    OnDistance,     ///< Emit after traveling distance
    OnTrigger,      ///< Emit on external trigger
    
    Count
};

/**
 * @brief Ribbon texture mode
 */
enum class RibbonTextureMode : u8 {
    Stretch,            ///< Stretch texture over entire ribbon
    Tile,               ///< Tile texture based on length
    DistributePerSegment, ///< One tile per segment
    
    Count
};

// ============================================================================
// GPU Particle Data Structures
// ============================================================================

/**
 * @brief GPU particle data (aligned for compute shaders)
 */
struct alignas(16) GPUParticle {
    Vec3 position;          // 12 bytes
    f32 age;                // 4 bytes  = 16 total
    
    Vec3 velocity;          // 12 bytes
    f32 lifetime;           // 4 bytes  = 16 total
    
    Vec3 size;              // 12 bytes (can be non-uniform)
    f32 rotation;           // 4 bytes  = 16 total
    
    Vec4 color;             // 16 bytes = 16 total
    
    // Total: 64 bytes (GPUParticleConfig::PARTICLE_STRIDE)
    
    [[nodiscard]] f32 normalizedAge() const {
        return lifetime > 0.0f ? age / lifetime : 1.0f;
    }
    
    [[nodiscard]] bool isDead() const {
        return age >= lifetime;
    }
    
    [[nodiscard]] f32 remainingLifetime() const {
        return std::max(0.0f, lifetime - age);
    }
};

static_assert(sizeof(GPUParticle) >= 64, "GPUParticle must be at least 64 bytes");

/**
 * @brief Extended particle attributes (optional secondary buffer)
 */
struct alignas(16) GPUParticleExtended {
    Vec3 angularVelocity;   // 12 bytes
    f32 mass;               // 4 bytes  = 16 total
    
    Vec3 initialVelocity;   // 12 bytes
    f32 initialSize;        // 4 bytes  = 16 total
    
    u32 randomSeed;         // 4 bytes
    u32 emitterIndex;       // 4 bytes
    u32 parentIndex;        // 4 bytes (for ribbons/trails)
    u32 flags;              // 4 bytes  = 16 total
    
    Vec4 customData;        // 16 bytes = 16 total
    
    // Total: 64 bytes
    
    // Flag bits
    static constexpr u32 FLAG_COLLIDED = 1 << 0;
    static constexpr u32 FLAG_TRIGGERED = 1 << 1;
    static constexpr u32 FLAG_RIBBON_HEAD = 1 << 2;
    static constexpr u32 FLAG_FROZEN = 1 << 3;
    
    [[nodiscard]] bool hasCollided() const { return (flags & FLAG_COLLIDED) != 0; }
    [[nodiscard]] bool hasTriggered() const { return (flags & FLAG_TRIGGERED) != 0; }
    [[nodiscard]] bool isRibbonHead() const { return (flags & FLAG_RIBBON_HEAD) != 0; }
    [[nodiscard]] bool isFrozen() const { return (flags & FLAG_FROZEN) != 0; }
};

static_assert(sizeof(GPUParticleExtended) >= 64, "GPUParticleExtended must be at least 64 bytes");

/**
 * @brief GPU emitter state (updated each frame)
 */
struct alignas(16) GPUEmitterState {
    Mat4 localToWorld;      // 64 bytes
    
    Vec3 position;          // 12 bytes
    f32 time;               // 4 bytes  = 16 total
    
    Vec3 prevPosition;      // 12 bytes (for velocity inheritance)
    f32 deltaTime;          // 4 bytes  = 16 total
    
    Vec4 emissionColor;     // 16 bytes = 16 total
    
    u32 aliveCount;         // 4 bytes
    u32 spawnCount;         // 4 bytes
    u32 maxParticles;       // 4 bytes
    u32 randomSeed;         // 4 bytes  = 16 total
    
    // Total: 128 bytes
    
    [[nodiscard]] f32 occupancy() const {
        return maxParticles > 0 ? static_cast<f32>(aliveCount) / maxParticles : 0.0f;
    }
    
    [[nodiscard]] bool isFull() const {
        return aliveCount >= maxParticles;
    }
    
    [[nodiscard]] Vec3 velocity() const {
        return deltaTime > 0.0f ? (position - prevPosition) / deltaTime : Vec3{0, 0, 0};
    }
};

static_assert(sizeof(GPUEmitterState) >= 128, "GPUEmitterState must be at least 128 bytes");

/**
 * @brief Force field GPU data
 */
struct alignas(16) GPUForceField {
    Vec3 position;          // 12 bytes
    f32 strength;           // 4 bytes  = 16 total
    
    Vec3 direction;         // 12 bytes (for directional, axis for vortex)
    f32 radius;             // 4 bytes  = 16 total
    
    Vec3 extents;           // 12 bytes (for box regions)
    f32 falloff;            // 4 bytes  = 16 total
    
    u32 type;               // ForceFieldType
    u32 enabled;
    f32 frequency;          // For noise/turbulence
    f32 amplitude;          // 4 bytes  = 16 total
    
    // Total: 64 bytes
    
    [[nodiscard]] f32 calculateInfluence(const Vec3& particlePos) const {
        f32 dist = (particlePos - position).length();
        if (dist >= radius) return 0.0f;
        if (falloff <= 0.0f) return 1.0f;
        f32 t = dist / radius;
        return std::pow(1.0f - t, falloff);
    }
};

static_assert(sizeof(GPUForceField) >= 64, "GPUForceField must be at least 64 bytes");

/**
 * @brief Collision plane GPU data
 */
struct alignas(16) GPUCollisionPlane {
    Vec3 normal;            // 12 bytes
    f32 distance;           // 4 bytes  = 16 total
    
    f32 bounciness;         // 4 bytes
    f32 friction;           // 4 bytes
    f32 lifetimeLoss;       // 4 bytes (0-1, how much lifetime to remove)
    u32 enabled;            // 4 bytes  = 16 total
    
    // Total: 32 bytes
    
    [[nodiscard]] f32 signedDistance(const Vec3& point) const {
        return normal.dot(point) - distance;
    }
    
    [[nodiscard]] Vec3 project(const Vec3& point) const {
        f32 dist = signedDistance(point);
        return point - normal * dist;
    }
    
    [[nodiscard]] Vec3 reflect(const Vec3& velocity) const {
        return velocity - normal * (2.0f * velocity.dot(normal));
    }
};

static_assert(sizeof(GPUCollisionPlane) >= 32, "GPUCollisionPlane must be at least 32 bytes");

// ============================================================================
// Emitter Configuration
// ============================================================================

/**
 * @brief Emission shape parameters
 */
struct EmissionShape {
    enum class Type : u8 {
        Point,
        Sphere,
        Hemisphere,
        Cone,
        Box,
        Circle,
        Edge,
        Mesh,           ///< Emit from mesh surface
        SkinnedMesh,    ///< Emit from animated mesh
        
        Count
    } type = Type::Point;
    
    Vec3 position{0, 0, 0};
    Vec3 scale{1, 1, 1};
    Quat rotation = Quat::identity();
    
    // Shape-specific parameters
    f32 radius = 1.0f;
    f32 radiusThickness = 0.0f;  // 0 = surface only, 1 = solid
    f32 arc = 360.0f;            // For cone/sphere (degrees)
    f32 angle = 25.0f;           // Cone angle (degrees)
    
    // Mesh emission
    u64 meshHandle = 0;
    bool emitFromVertices = false;
    bool emitFromEdges = false;
    bool emitFromTriangles = true;
    
    [[nodiscard]] Vec3 getRandomPoint(u32 seed) const {
        // Simple pseudo-random implementation
        auto random01 = [seed](u32 offset) {
            u32 h = seed + offset;
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;
            return static_cast<f32>(h) / static_cast<f32>(0xFFFFFFFF);
        };
        
        switch (type) {
            case Type::Point:
                return position;
            case Type::Sphere: {
                f32 u = random01(0) * 2.0f - 1.0f;
                f32 theta = random01(1) * 2.0f * 3.14159265f;
                f32 r = std::sqrt(1.0f - u * u);
                f32 radialScale = radiusThickness + (1.0f - radiusThickness) * std::cbrt(random01(2));
                return position + Vec3{r * std::cos(theta), u, r * std::sin(theta)} * radius * radialScale;
            }
            case Type::Box:
                return position + Vec3{
                    (random01(0) - 0.5f) * scale.x,
                    (random01(1) - 0.5f) * scale.y,
                    (random01(2) - 0.5f) * scale.z
                };
            default:
                return position;
        }
    }
};

/**
 * @brief Color over lifetime gradient
 */
struct ColorGradient {
    struct Key {
        f32 time = 0.0f;      // 0-1 normalized
        Vec4 color{1, 1, 1, 1};
    };
    
    std::vector<Key> keys;
    
    ColorGradient() {
        keys.push_back({0.0f, {1, 1, 1, 1}});
        keys.push_back({1.0f, {1, 1, 1, 0}});
    }
    
    [[nodiscard]] Vec4 evaluate(f32 t) const {
        if (keys.empty()) return {1, 1, 1, 1};
        if (keys.size() == 1 || t <= keys.front().time) return keys.front().color;
        if (t >= keys.back().time) return keys.back().color;
        
        // Find surrounding keys
        for (usize i = 0; i < keys.size() - 1; ++i) {
            if (t >= keys[i].time && t <= keys[i + 1].time) {
                f32 localT = (t - keys[i].time) / (keys[i + 1].time - keys[i].time);
                return keys[i].color + (keys[i + 1].color - keys[i].color) * localT;
            }
        }
        
        return keys.back().color;
    }
    
    // Presets
    static ColorGradient fireColors() {
        ColorGradient g;
        g.keys.clear();
        g.keys.push_back({0.0f, {1.0f, 0.9f, 0.3f, 1.0f}});   // Yellow
        g.keys.push_back({0.3f, {1.0f, 0.5f, 0.0f, 0.9f}});   // Orange
        g.keys.push_back({0.7f, {0.8f, 0.1f, 0.0f, 0.5f}});   // Red
        g.keys.push_back({1.0f, {0.2f, 0.0f, 0.0f, 0.0f}});   // Dark red, transparent
        return g;
    }
    
    static ColorGradient smokeColors() {
        ColorGradient g;
        g.keys.clear();
        g.keys.push_back({0.0f, {0.3f, 0.3f, 0.3f, 0.0f}});
        g.keys.push_back({0.1f, {0.4f, 0.4f, 0.4f, 0.8f}});
        g.keys.push_back({0.8f, {0.5f, 0.5f, 0.5f, 0.3f}});
        g.keys.push_back({1.0f, {0.6f, 0.6f, 0.6f, 0.0f}});
        return g;
    }
    
    static ColorGradient sparkColors() {
        ColorGradient g;
        g.keys.clear();
        g.keys.push_back({0.0f, {1.0f, 1.0f, 0.8f, 1.0f}});
        g.keys.push_back({0.5f, {1.0f, 0.7f, 0.3f, 1.0f}});
        g.keys.push_back({1.0f, {0.8f, 0.3f, 0.1f, 0.0f}});
        return g;
    }
};

/**
 * @brief Size over lifetime curve
 */
struct SizeCurve {
    struct Key {
        f32 time = 0.0f;
        f32 value = 1.0f;
    };
    
    std::vector<Key> keys;
    f32 multiplier = 1.0f;
    
    SizeCurve() {
        keys.push_back({0.0f, 1.0f});
        keys.push_back({1.0f, 1.0f});
    }
    
    [[nodiscard]] f32 evaluate(f32 t) const {
        if (keys.empty()) return multiplier;
        if (keys.size() == 1 || t <= keys.front().time) return keys.front().value * multiplier;
        if (t >= keys.back().time) return keys.back().value * multiplier;
        
        for (usize i = 0; i < keys.size() - 1; ++i) {
            if (t >= keys[i].time && t <= keys[i + 1].time) {
                f32 localT = (t - keys[i].time) / (keys[i + 1].time - keys[i].time);
                return (keys[i].value + (keys[i + 1].value - keys[i].value) * localT) * multiplier;
            }
        }
        
        return keys.back().value * multiplier;
    }
    
    // Presets
    static SizeCurve growThenShrink() {
        SizeCurve c;
        c.keys.clear();
        c.keys.push_back({0.0f, 0.0f});
        c.keys.push_back({0.2f, 1.0f});
        c.keys.push_back({0.8f, 1.0f});
        c.keys.push_back({1.0f, 0.0f});
        return c;
    }
    
    static SizeCurve shrinkToNothing() {
        SizeCurve c;
        c.keys.clear();
        c.keys.push_back({0.0f, 1.0f});
        c.keys.push_back({1.0f, 0.0f});
        return c;
    }
    
    static SizeCurve growContinuously() {
        SizeCurve c;
        c.keys.clear();
        c.keys.push_back({0.0f, 0.0f});
        c.keys.push_back({1.0f, 1.0f});
        return c;
    }
};

/**
 * @brief GPU emitter descriptor
 */
struct GPUEmitterDesc {
    std::string name;
    u32 maxParticles = 10000;
    GPUSimulationMode simulationMode = GPUSimulationMode::Standard;
    GPURenderMode renderMode = GPURenderMode::Billboard;
    
    // Emission
    EmissionShape emissionShape;
    f32 emissionRate = 100.0f;          // Particles per second
    f32 emissionRateMultiplier = 1.0f;
    Vec2 lifetime{1.0f, 2.0f};          // Min/max lifetime
    
    // Initial values
    Vec2 startSize{0.1f, 0.2f};         // Min/max size
    Vec2 startSpeed{1.0f, 5.0f};        // Min/max speed
    Vec2 startRotation{0.0f, 360.0f};   // Min/max rotation (degrees)
    Vec4 startColor{1, 1, 1, 1};
    
    // Over lifetime
    ColorGradient colorOverLifetime;
    SizeCurve sizeOverLifetime;
    f32 rotationSpeedOverLifetime = 0.0f;
    f32 gravityModifier = 1.0f;
    
    // Velocity
    f32 velocityDamping = 0.0f;         // 0 = no damping, 1 = full stop
    f32 inheritVelocity = 0.0f;         // 0-1, inherit emitter velocity
    
    // Collision
    ParticleCollisionMode collisionMode = ParticleCollisionMode::None;
    f32 collisionBounciness = 0.5f;
    f32 collisionFriction = 0.5f;
    f32 collisionLifetimeLoss = 0.0f;
    
    // Rendering
    ParticleSortMode sortMode = ParticleSortMode::None;
    u64 materialHandle = 0;
    u64 textureHandle = 0;
    Vec2 textureSheetTiles{1, 1};       // For animated sprites
    f32 textureSheetSpeed = 1.0f;
    
    // Ribbon/Trail specific
    RibbonTextureMode ribbonTextureMode = RibbonTextureMode::Stretch;
    f32 ribbonWidth = 0.1f;
    u32 ribbonSegments = 16;
    f32 ribbonMinVertexDistance = 0.1f;
    
    // Mesh particle specific
    u64 meshHandle = 0;
    bool meshAlignToVelocity = true;
    
    // Sub-emitters
    std::vector<u32> subEmitterIndices;
    
    [[nodiscard]] u32 estimateMemoryBytes() const {
        u32 baseMemory = maxParticles * GPUParticleConfig::PARTICLE_STRIDE;
        u32 extendedMemory = maxParticles * sizeof(GPUParticleExtended);
        u32 deadListMemory = maxParticles * sizeof(u32);
        return baseMemory + extendedMemory + deadListMemory;
    }
};

// ============================================================================
// GPU Particle System Manager
// ============================================================================

/**
 * @brief GPU particle system statistics
 */
struct GPUParticleStats {
    u64 totalParticles = 0;
    u64 aliveParticles = 0;
    u64 spawnedThisFrame = 0;
    u64 diedThisFrame = 0;
    u32 activeEmitters = 0;
    f32 simulationTimeMs = 0.0f;
    f32 sortTimeMs = 0.0f;
    f32 renderTimeMs = 0.0f;
    u64 gpuMemoryBytes = 0;
    
    [[nodiscard]] f32 occupancy() const {
        return totalParticles > 0 ? static_cast<f32>(aliveParticles) / totalParticles : 0.0f;
    }
    
    [[nodiscard]] f32 totalTimeMs() const {
        return simulationTimeMs + sortTimeMs + renderTimeMs;
    }
    
    void reset() {
        totalParticles = 0;
        aliveParticles = 0;
        spawnedThisFrame = 0;
        diedThisFrame = 0;
        activeEmitters = 0;
        simulationTimeMs = 0.0f;
        sortTimeMs = 0.0f;
        renderTimeMs = 0.0f;
        gpuMemoryBytes = 0;
    }
};

/**
 * @brief GPU particle emitter instance
 */
class GPUParticleEmitter {
public:
    GPUParticleEmitter(const GPUEmitterDesc& desc) : m_desc(desc) {
        m_state.maxParticles = desc.maxParticles;
    }
    
    /**
     * @brief Get emitter description
     */
    [[nodiscard]] const GPUEmitterDesc& desc() const { return m_desc; }
    
    /**
     * @brief Get emitter state
     */
    [[nodiscard]] const GPUEmitterState& state() const { return m_state; }
    
    /**
     * @brief Set emitter position
     */
    void setPosition(const Vec3& position) {
        m_state.prevPosition = m_state.position;
        m_state.position = position;
    }
    
    /**
     * @brief Set emitter transform
     */
    void setTransform(const Mat4& transform) {
        m_state.localToWorld = transform;
        // Extract position from matrix
        m_state.prevPosition = m_state.position;
        m_state.position = Vec3{transform[3][0], transform[3][1], transform[3][2]};
    }
    
    /**
     * @brief Set emission color multiplier
     */
    void setEmissionColor(const Vec4& color) {
        m_state.emissionColor = color;
    }
    
    /**
     * @brief Trigger a burst emission
     */
    void burst(u32 count) {
        m_pendingBurst += count;
    }
    
    /**
     * @brief Check if emitter is playing
     */
    [[nodiscard]] bool isPlaying() const { return m_playing; }
    
    /**
     * @brief Start emitter
     */
    void play() { m_playing = true; }
    
    /**
     * @brief Stop emitter (no new particles, existing continue)
     */
    void stop() { m_playing = false; }
    
    /**
     * @brief Clear all particles
     */
    void clear() {
        m_state.aliveCount = 0;
        m_pendingBurst = 0;
    }
    
    /**
     * @brief Update emitter (called before GPU simulation)
     */
    void update(f32 deltaTime) {
        m_state.deltaTime = deltaTime;
        m_state.time += deltaTime;
        m_state.randomSeed++;
        
        // Calculate spawn count
        if (m_playing) {
            m_emissionAccumulator += m_desc.emissionRate * m_desc.emissionRateMultiplier * deltaTime;
            u32 toSpawn = static_cast<u32>(m_emissionAccumulator);
            m_emissionAccumulator -= toSpawn;
            m_state.spawnCount = toSpawn + m_pendingBurst;
            m_pendingBurst = 0;
        } else {
            m_state.spawnCount = m_pendingBurst;
            m_pendingBurst = 0;
        }
        
        // Clamp to available space
        u32 available = m_state.maxParticles - m_state.aliveCount;
        m_state.spawnCount = std::min(m_state.spawnCount, available);
    }
    
    /**
     * @brief Get alive particle count
     */
    [[nodiscard]] u32 aliveCount() const { return m_state.aliveCount; }
    
    /**
     * @brief Set alive count (called after GPU simulation)
     */
    void setAliveCount(u32 count) { m_state.aliveCount = count; }

private:
    GPUEmitterDesc m_desc;
    GPUEmitterState m_state{};
    bool m_playing = true;
    f32 m_emissionAccumulator = 0.0f;
    u32 m_pendingBurst = 0;
};

/**
 * @brief GPU Particle Manager singleton
 */
class GPUParticleManager {
public:
    /**
     * @brief Get singleton instance
     */
    static GPUParticleManager& get() {
        static GPUParticleManager instance;
        return instance;
    }
    
    /**
     * @brief Initialize particle system
     */
    bool initialize() {
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Shutdown particle system
     */
    void shutdown() {
        m_emitters.clear();
        m_forceFields.clear();
        m_initialized = false;
    }
    
    /**
     * @brief Check if initialized
     */
    [[nodiscard]] bool isInitialized() const { return m_initialized; }
    
    /**
     * @brief Create a particle emitter
     */
    u64 createEmitter(const GPUEmitterDesc& desc) {
        u64 handle = m_nextHandle++;
        m_emitters[handle] = std::make_unique<GPUParticleEmitter>(desc);
        m_stats.totalParticles += desc.maxParticles;
        m_stats.gpuMemoryBytes += desc.estimateMemoryBytes();
        return handle;
    }
    
    /**
     * @brief Destroy a particle emitter
     */
    void destroyEmitter(u64 handle) {
        auto it = m_emitters.find(handle);
        if (it != m_emitters.end()) {
            m_stats.totalParticles -= it->second->desc().maxParticles;
            m_stats.gpuMemoryBytes -= it->second->desc().estimateMemoryBytes();
            m_emitters.erase(it);
        }
    }
    
    /**
     * @brief Get emitter by handle
     */
    [[nodiscard]] GPUParticleEmitter* getEmitter(u64 handle) {
        auto it = m_emitters.find(handle);
        return it != m_emitters.end() ? it->second.get() : nullptr;
    }
    
    /**
     * @brief Add a force field
     */
    u64 addForceField(const GPUForceField& field) {
        u64 handle = m_nextHandle++;
        m_forceFields[handle] = field;
        return handle;
    }
    
    /**
     * @brief Remove a force field
     */
    void removeForceField(u64 handle) {
        m_forceFields.erase(handle);
    }
    
    /**
     * @brief Get force field by handle
     */
    [[nodiscard]] GPUForceField* getForceField(u64 handle) {
        auto it = m_forceFields.find(handle);
        return it != m_forceFields.end() ? &it->second : nullptr;
    }
    
    /**
     * @brief Add a collision plane
     */
    u32 addCollisionPlane(const GPUCollisionPlane& plane) {
        if (m_collisionPlanes.size() >= GPUParticleConfig::MAX_COLLISION_PLANES) {
            return ~0u;
        }
        m_collisionPlanes.push_back(plane);
        return static_cast<u32>(m_collisionPlanes.size() - 1);
    }
    
    /**
     * @brief Clear all collision planes
     */
    void clearCollisionPlanes() {
        m_collisionPlanes.clear();
    }
    
    /**
     * @brief Update all emitters
     */
    void update(f32 deltaTime) {
        m_stats.spawnedThisFrame = 0;
        m_stats.diedThisFrame = 0;
        m_stats.activeEmitters = 0;
        m_stats.aliveParticles = 0;
        
        for (auto& [handle, emitter] : m_emitters) {
            emitter->update(deltaTime);
            m_stats.aliveParticles += emitter->aliveCount();
            m_stats.spawnedThisFrame += emitter->state().spawnCount;
            if (emitter->isPlaying() || emitter->aliveCount() > 0) {
                m_stats.activeEmitters++;
            }
        }
    }
    
    /**
     * @brief Simulate particles on GPU
     * @note In a real implementation, this dispatches compute shaders
     */
    void simulate() {
        // This would dispatch simulation compute shaders
        // For each emitter: spawn, simulate, collision, update dead list
    }
    
    /**
     * @brief Sort particles for rendering
     */
    void sort(const Vec3& cameraPosition) {
        m_sortCameraPosition = cameraPosition;
        // This would dispatch sort compute shaders
    }
    
    /**
     * @brief Render all particle systems
     */
    void render() {
        // This would dispatch rendering (indirect draw calls)
    }
    
    /**
     * @brief Get statistics
     */
    [[nodiscard]] const GPUParticleStats& stats() const { return m_stats; }
    
    /**
     * @brief Reset statistics
     */
    void resetStats() { m_stats.reset(); }
    
    /**
     * @brief Get emitter count
     */
    [[nodiscard]] usize emitterCount() const { return m_emitters.size(); }
    
    /**
     * @brief Get force field count
     */
    [[nodiscard]] usize forceFieldCount() const { return m_forceFields.size(); }

private:
    GPUParticleManager() = default;
    ~GPUParticleManager() = default;
    GPUParticleManager(const GPUParticleManager&) = delete;
    GPUParticleManager& operator=(const GPUParticleManager&) = delete;
    
    bool m_initialized = false;
    std::unordered_map<u64, std::unique_ptr<GPUParticleEmitter>> m_emitters;
    std::unordered_map<u64, GPUForceField> m_forceFields;
    std::vector<GPUCollisionPlane> m_collisionPlanes;
    u64 m_nextHandle = 1;
    GPUParticleStats m_stats;
    Vec3 m_sortCameraPosition{0, 0, 0};
};

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Create fire particle emitter preset
 */
inline GPUEmitterDesc createFireEmitterPreset(u32 maxParticles = 5000) {
    GPUEmitterDesc desc;
    desc.name = "Fire";
    desc.maxParticles = maxParticles;
    desc.renderMode = GPURenderMode::Billboard;
    
    desc.emissionShape.type = EmissionShape::Type::Cone;
    desc.emissionShape.angle = 15.0f;
    desc.emissionShape.radius = 0.2f;
    
    desc.emissionRate = 200.0f;
    desc.lifetime = {0.5f, 1.5f};
    desc.startSize = {0.1f, 0.3f};
    desc.startSpeed = {2.0f, 5.0f};
    
    desc.colorOverLifetime = ColorGradient::fireColors();
    desc.sizeOverLifetime = SizeCurve::shrinkToNothing();
    desc.gravityModifier = -0.5f;  // Rise upward
    
    return desc;
}

/**
 * @brief Create smoke particle emitter preset
 */
inline GPUEmitterDesc createSmokeEmitterPreset(u32 maxParticles = 3000) {
    GPUEmitterDesc desc;
    desc.name = "Smoke";
    desc.maxParticles = maxParticles;
    desc.renderMode = GPURenderMode::Billboard;
    
    desc.emissionShape.type = EmissionShape::Type::Point;
    
    desc.emissionRate = 50.0f;
    desc.lifetime = {2.0f, 4.0f};
    desc.startSize = {0.2f, 0.5f};
    desc.startSpeed = {0.5f, 1.5f};
    desc.startRotation = {0.0f, 360.0f};
    
    desc.colorOverLifetime = ColorGradient::smokeColors();
    desc.sizeOverLifetime = SizeCurve::growContinuously();
    desc.sizeOverLifetime.multiplier = 3.0f;
    desc.rotationSpeedOverLifetime = 30.0f;
    desc.gravityModifier = -0.1f;
    
    return desc;
}

/**
 * @brief Create spark particle emitter preset
 */
inline GPUEmitterDesc createSparkEmitterPreset(u32 maxParticles = 1000) {
    GPUEmitterDesc desc;
    desc.name = "Sparks";
    desc.maxParticles = maxParticles;
    desc.renderMode = GPURenderMode::StretchedBillboard;
    
    desc.emissionShape.type = EmissionShape::Type::Point;
    
    desc.emissionRate = 0.0f;  // Burst only
    desc.lifetime = {0.3f, 0.8f};
    desc.startSize = {0.02f, 0.05f};
    desc.startSpeed = {5.0f, 15.0f};
    
    desc.colorOverLifetime = ColorGradient::sparkColors();
    desc.sizeOverLifetime = SizeCurve::shrinkToNothing();
    desc.gravityModifier = 1.0f;
    desc.velocityDamping = 0.1f;
    
    desc.collisionMode = ParticleCollisionMode::Planes;
    desc.collisionBounciness = 0.3f;
    desc.collisionLifetimeLoss = 0.3f;
    
    return desc;
}

/**
 * @brief Create rain particle emitter preset
 */
inline GPUEmitterDesc createRainEmitterPreset(u32 maxParticles = 10000) {
    GPUEmitterDesc desc;
    desc.name = "Rain";
    desc.maxParticles = maxParticles;
    desc.renderMode = GPURenderMode::StretchedBillboard;
    
    desc.emissionShape.type = EmissionShape::Type::Box;
    desc.emissionShape.scale = {20.0f, 0.1f, 20.0f};
    desc.emissionShape.position = {0, 10, 0};
    
    desc.emissionRate = 2000.0f;
    desc.lifetime = {0.5f, 1.0f};
    desc.startSize = {0.01f, 0.02f};
    desc.startSpeed = {15.0f, 20.0f};
    desc.startColor = {0.7f, 0.8f, 1.0f, 0.6f};
    
    desc.gravityModifier = 2.0f;
    
    desc.collisionMode = ParticleCollisionMode::DepthBuffer;
    desc.collisionLifetimeLoss = 1.0f;  // Die on impact
    
    return desc;
}

/**
 * @brief Create ribbon/trail emitter preset
 */
inline GPUEmitterDesc createRibbonEmitterPreset(u32 maxParticles = 500) {
    GPUEmitterDesc desc;
    desc.name = "Ribbon";
    desc.maxParticles = maxParticles;
    desc.renderMode = GPURenderMode::Ribbon;
    
    desc.emissionShape.type = EmissionShape::Type::Point;
    
    desc.emissionRate = 60.0f;  // 60 points per second
    desc.lifetime = {1.0f, 1.0f};
    desc.startSize = {0.1f, 0.1f};
    desc.startSpeed = {0.0f, 0.0f};  // Follow emitter
    
    ColorGradient g;
    g.keys.clear();
    g.keys.push_back({0.0f, {1, 1, 1, 1}});
    g.keys.push_back({1.0f, {1, 1, 1, 0}});
    desc.colorOverLifetime = g;
    
    desc.ribbonTextureMode = RibbonTextureMode::Stretch;
    desc.ribbonWidth = 0.1f;
    desc.ribbonSegments = 32;
    desc.ribbonMinVertexDistance = 0.05f;
    
    desc.inheritVelocity = 1.0f;
    
    return desc;
}

} // namespace nova
