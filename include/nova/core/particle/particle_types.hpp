/**
 * @file particle_types.hpp
 * @brief NovaCore Particle System™ - Core Particle Types
 * 
 * Production-grade particle system types for the NovaCore engine.
 * Supports GPU instanced particles, modules, curves, and forces.
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace nova::particle {

// Use types from nova namespace
using nova::u8;
using nova::u16;
using nova::u32;
using nova::u64;
using nova::i32;
using nova::f32;
using nova::f64;
using nova::usize;
using nova::Handle;

// Use types from nova::math namespace
using nova::math::Vec2;
using nova::math::Vec3;
using nova::math::Vec4;
using nova::math::Mat4;
using nova::math::Quat;

/// Color type (RGBA, 0-1 range)
using Color = Vec4;

// ============================================================================
// Handle Types
// ============================================================================

/// Handle to a particle emitter
using ParticleEmitterHandle = Handle<struct ParticleEmitterTag>;

/// Handle to a particle system asset
using ParticleSystemHandle = Handle<struct ParticleSystemTag>;

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Particle simulation space
 */
enum class SimulationSpace : u8 {
    Local,          ///< Particles move with emitter
    World           ///< Particles independent of emitter
};

/**
 * @brief Particle emission shape
 */
enum class EmissionShape : u8 {
    Point,          ///< Emit from single point
    Sphere,         ///< Emit from sphere surface/volume
    Hemisphere,     ///< Emit from hemisphere
    Cone,           ///< Emit in cone shape
    Box,            ///< Emit from box volume
    Circle,         ///< Emit from circle edge/area
    Rectangle,      ///< Emit from rectangle
    Edge,           ///< Emit along edge
    Mesh,           ///< Emit from mesh surface
    Skinned         ///< Emit from skinned mesh
};

/**
 * @brief Particle render mode
 */
enum class RenderMode : u8 {
    Billboard,      ///< Face camera (2D quad)
    StretchedBillboard, ///< Stretch based on velocity
    HorizontalBillboard, ///< Face up
    VerticalBillboard, ///< Face horizontal only
    Mesh,           ///< 3D mesh particles
    Trail           ///< Trail rendering
};

/**
 * @brief Particle sort mode
 */
enum class SortMode : u8 {
    None,           ///< No sorting
    ByDistance,     ///< Sort by camera distance
    OldestFirst,    ///< Sort by spawn time (oldest first)
    YoungestFirst,  ///< Sort by spawn time (youngest first)
    ByDepth         ///< Sort by depth for proper blending
};

/**
 * @brief Particle blend mode
 */
enum class ParticleBlendMode : u8 {
    Additive,       ///< Add to framebuffer
    AlphaBlend,     ///< Standard alpha blending
    Multiply,       ///< Multiply with framebuffer
    Premultiplied,  ///< Premultiplied alpha
    SoftAdditive    ///< Soft additive blend
};

/**
 * @brief Curve interpolation type
 */
enum class CurveType : u8 {
    Constant,       ///< Single constant value
    Linear,         ///< Linear interpolation
    Curve,          ///< Bezier curve
    RandomBetweenTwo, ///< Random between two values
    RandomBetweenCurves ///< Random between two curves
};

/**
 * @brief Force type
 */
enum class ForceType : u8 {
    Gravity,        ///< Constant directional force
    Wind,           ///< Wind force with noise
    Turbulence,     ///< Turbulent/chaotic force
    Vortex,         ///< Spinning vortex force
    Attractor,      ///< Point attractor
    Repulsor,       ///< Point repulsor
    Drag,           ///< Air resistance
    Custom          ///< Custom force function
};

/**
 * @brief Collision mode
 */
enum class CollisionMode : u8 {
    None,           ///< No collision
    World,          ///< Collide with world geometry
    Planes,         ///< Collide with planes only
    DepthBuffer     ///< Collide with depth buffer
};

/**
 * @brief Collision response
 */
enum class CollisionResponse : u8 {
    Kill,           ///< Kill particle on collision
    Bounce,         ///< Bounce off surface
    Stick,          ///< Stick to surface
    Callback        ///< Custom callback
};

// ============================================================================
// Curve Structures
// ============================================================================

/**
 * @brief Key point in an animation curve
 */
struct CurveKey {
    f32 time = 0.0f;        ///< Time (0-1 for normalized lifetime)
    f32 value = 0.0f;       ///< Value at this time
    f32 inTangent = 0.0f;   ///< Incoming tangent
    f32 outTangent = 0.0f;  ///< Outgoing tangent
};

/**
 * @brief Animation curve for particle properties over lifetime
 */
struct ParticleCurve {
    CurveType type = CurveType::Constant;
    f32 constantValue = 1.0f;
    f32 minValue = 0.0f;
    f32 maxValue = 1.0f;
    std::vector<CurveKey> keys;
    std::vector<CurveKey> keysMax; // For RandomBetweenCurves
    
    /// Evaluate curve at time t (0-1)
    f32 evaluate(f32 t) const;
    
    /// Evaluate with random seed for random modes
    f32 evaluateRandom(f32 t, u32 seed) const;
};

/**
 * @brief Gradient for color over lifetime
 */
struct ColorGradient {
    struct ColorKey {
        f32 time = 0.0f;
        Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    };
    
    struct AlphaKey {
        f32 time = 0.0f;
        f32 alpha = 1.0f;
    };
    
    std::vector<ColorKey> colorKeys;
    std::vector<AlphaKey> alphaKeys;
    
    /// Evaluate gradient at time t (0-1)
    Color evaluate(f32 t) const;
    
    /// Default white gradient
    static ColorGradient white() {
        ColorGradient g;
        g.colorKeys.push_back({0.0f, Color(1.0f, 1.0f, 1.0f, 1.0f)});
        g.alphaKeys.push_back({0.0f, 1.0f});
        return g;
    }
};

/**
 * @brief Min-max value (constant or range)
 */
struct MinMaxValue {
    CurveType type = CurveType::Constant;
    f32 constantValue = 1.0f;
    f32 minValue = 0.0f;
    f32 maxValue = 1.0f;
    
    /// Get value (random if range)
    f32 getValue(u32 seed = 0) const;
    
    /// Create constant
    static MinMaxValue constant(f32 val) {
        MinMaxValue v;
        v.type = CurveType::Constant;
        v.constantValue = val;
        return v;
    }
    
    /// Create range
    static MinMaxValue range(f32 minVal, f32 maxVal) {
        MinMaxValue v;
        v.type = CurveType::RandomBetweenTwo;
        v.minValue = minVal;
        v.maxValue = maxVal;
        return v;
    }
};

/**
 * @brief 3D min-max value
 */
struct MinMaxVec3 {
    Vec3 min{};
    Vec3 max{};
    bool separateAxes = false;
    
    Vec3 getValue(u32 seed = 0) const;
    
    static MinMaxVec3 constant(const Vec3& val) {
        MinMaxVec3 v;
        v.min = val;
        v.max = val;
        return v;
    }
    
    static MinMaxVec3 range(const Vec3& minVal, const Vec3& maxVal) {
        MinMaxVec3 v;
        v.min = minVal;
        v.max = maxVal;
        return v;
    }
};

// ============================================================================
// Particle Data
// ============================================================================

/**
 * @brief Single particle instance data
 */
struct Particle {
    Vec3 position{};            ///< World/local position
    Vec3 velocity{};            ///< Current velocity
    Vec3 rotation{};            ///< Euler rotation
    Vec3 angularVelocity{};     ///< Angular velocity
    Vec3 size{1.0f, 1.0f, 1.0f}; ///< Particle size
    Color color = Color(1.0f, 1.0f, 1.0f, 1.0f); ///< Particle color
    
    f32 lifetime = 0.0f;        ///< Current age
    f32 maxLifetime = 1.0f;     ///< Total lifetime
    
    u32 randomSeed = 0;         ///< Per-particle random seed
    u16 sortKey = 0;            ///< Sort key for rendering
    u8 textureIndex = 0;        ///< Sprite sheet frame
    bool alive = true;          ///< Is particle alive
    
    /// Get normalized lifetime (0-1)
    f32 getNormalizedLifetime() const {
        return (maxLifetime > 0.0f) ? (lifetime / maxLifetime) : 0.0f;
    }
    
    /// Get remaining lifetime
    f32 getRemainingLifetime() const {
        return maxLifetime - lifetime;
    }
};

// ============================================================================
// Emitter Modules
// ============================================================================

/**
 * @brief Main emitter configuration
 */
struct MainModule {
    f32 duration = 5.0f;        ///< Emitter duration
    bool loop = true;           ///< Loop emitter
    f32 startDelay = 0.0f;      ///< Delay before starting
    
    MinMaxValue startLifetime = MinMaxValue::constant(5.0f);
    MinMaxValue startSpeed = MinMaxValue::constant(5.0f);
    MinMaxValue startSize = MinMaxValue::constant(1.0f);
    MinMaxVec3 startRotation{};
    ColorGradient startColor = ColorGradient::white();
    
    f32 gravityModifier = 0.0f;
    SimulationSpace simulationSpace = SimulationSpace::Local;
    f32 simulationSpeed = 1.0f;
    
    u32 maxParticles = 1000;
    bool playOnAwake = true;
    bool prewarm = false;
};

/**
 * @brief Emission rate and burst configuration
 */
struct EmissionModule {
    bool enabled = true;
    MinMaxValue rateOverTime = MinMaxValue::constant(10.0f);
    MinMaxValue rateOverDistance = MinMaxValue::constant(0.0f);
    
    struct Burst {
        f32 time = 0.0f;
        MinMaxValue count = MinMaxValue::constant(30.0f);
        i32 cycles = 1;
        f32 interval = 0.01f;
        f32 probability = 1.0f;
    };
    std::vector<Burst> bursts;
};

/**
 * @brief Shape module for emission area
 */
struct ShapeModule {
    bool enabled = true;
    EmissionShape shape = EmissionShape::Cone;
    
    // Shape-specific parameters
    f32 radius = 1.0f;
    f32 radiusThickness = 1.0f;     ///< 0 = surface, 1 = full volume
    f32 angle = 25.0f;              ///< Cone angle
    f32 arc = 360.0f;               ///< Partial arc for circle/cone
    f32 length = 5.0f;              ///< Box/edge length
    Vec3 scale{1.0f, 1.0f, 1.0f};   ///< Shape scale
    Vec3 position{};                 ///< Shape offset
    Vec3 rotation{};                 ///< Shape rotation
    
    bool alignToDirection = false;  ///< Align particles to emission direction
    f32 randomDirectionAmount = 0.0f; ///< Randomize direction
    f32 sphericalDirectionAmount = 0.0f; ///< Add spherical direction
};

/**
 * @brief Velocity over lifetime module
 */
struct VelocityOverLifetimeModule {
    bool enabled = false;
    MinMaxVec3 linear{};
    ParticleCurve orbital = {CurveType::Constant, 0.0f};
    ParticleCurve radial = {CurveType::Constant, 0.0f};
    ParticleCurve speedModifier = {CurveType::Constant, 1.0f};
    SimulationSpace space = SimulationSpace::Local;
};

/**
 * @brief Size over lifetime module
 */
struct SizeOverLifetimeModule {
    bool enabled = false;
    ParticleCurve size = {CurveType::Constant, 1.0f};
    bool separateAxes = false;
    ParticleCurve sizeX = {CurveType::Constant, 1.0f};
    ParticleCurve sizeY = {CurveType::Constant, 1.0f};
    ParticleCurve sizeZ = {CurveType::Constant, 1.0f};
};

/**
 * @brief Color over lifetime module
 */
struct ColorOverLifetimeModule {
    bool enabled = false;
    ColorGradient color = ColorGradient::white();
};

/**
 * @brief Rotation over lifetime module
 */
struct RotationOverLifetimeModule {
    bool enabled = false;
    ParticleCurve angularVelocity = {CurveType::Constant, 0.0f};
    bool separateAxes = false;
    ParticleCurve angularVelocityX = {CurveType::Constant, 0.0f};
    ParticleCurve angularVelocityY = {CurveType::Constant, 0.0f};
    ParticleCurve angularVelocityZ = {CurveType::Constant, 0.0f};
};

/**
 * @brief Force over lifetime module
 */
struct ForceOverLifetimeModule {
    bool enabled = false;
    MinMaxVec3 force{};
    SimulationSpace space = SimulationSpace::Local;
};

/**
 * @brief Noise module for turbulence
 */
struct NoiseModule {
    bool enabled = false;
    f32 strength = 1.0f;
    f32 frequency = 0.5f;
    i32 octaves = 1;
    f32 scrollSpeed = 0.0f;
    bool damping = true;
    bool separateAxes = false;
    ParticleCurve strengthX = {CurveType::Constant, 1.0f};
    ParticleCurve strengthY = {CurveType::Constant, 1.0f};
    ParticleCurve strengthZ = {CurveType::Constant, 1.0f};
};

/**
 * @brief Collision module
 */
struct CollisionModule {
    bool enabled = false;
    CollisionMode mode = CollisionMode::None;
    CollisionResponse response = CollisionResponse::Kill;
    
    f32 bounce = 0.0f;
    f32 lifetimeLoss = 0.0f;
    f32 radiusScale = 1.0f;
    
    std::vector<Vec4> planes; // Collision planes (xyz = normal, w = distance)
};

/**
 * @brief Sub-emitter trigger type
 */
enum class SubEmitterTrigger : u8 {
    Birth,
    Death,
    Collision,
    Manual
};

/**
 * @brief Sub-emitter configuration
 */
struct SubEmitterModule {
    bool enabled = false;
    
    struct SubEmitter {
        ParticleSystemHandle system;
        SubEmitterTrigger trigger = SubEmitterTrigger::Death;
        f32 probability = 1.0f;
        bool inheritVelocity = false;
        bool inheritColor = false;
    };
    std::vector<SubEmitter> subEmitters;
};

/**
 * @brief Texture sheet animation module
 */
struct TextureSheetModule {
    bool enabled = false;
    i32 tilesX = 1;
    i32 tilesY = 1;
    ParticleCurve frameOverTime = {CurveType::Constant, 0.0f};
    f32 cyclesPerSecond = 1.0f;
    bool randomStart = false;
};

/**
 * @brief Renderer configuration
 */
struct RendererModule {
    bool enabled = true;
    RenderMode mode = RenderMode::Billboard;
    SortMode sortMode = SortMode::None;
    ParticleBlendMode blendMode = ParticleBlendMode::AlphaBlend;
    
    f32 sortingFudge = 0.0f;
    f32 minParticleSize = 0.0f;
    f32 maxParticleSize = 10000.0f;
    f32 normalDirection = 1.0f;
    
    // Stretched billboard
    f32 lengthScale = 2.0f;
    f32 velocityScale = 0.0f;
    
    // Mesh rendering
    // MeshHandle mesh;
    
    // Material
    // MaterialHandle material;
    u32 materialSlot = 0;
    
    // Trails
    bool trails = false;
    f32 trailRatio = 0.5f;
    f32 trailLifetime = 1.0f;
    f32 trailMinimumVertexDistance = 0.1f;
    bool trailDieWithParticle = true;
};

// ============================================================================
// Force Field
// ============================================================================

/**
 * @brief Force field affecting particles
 */
struct ForceField {
    std::string name;
    ForceType type = ForceType::Gravity;
    
    Vec3 position{};            ///< World position
    Vec3 direction{0.0f, -1.0f, 0.0f}; ///< Direction for directional forces
    f32 strength = 9.81f;       ///< Force strength
    f32 range = 0.0f;           ///< 0 = infinite range
    f32 falloff = 1.0f;         ///< Distance falloff exponent
    
    // For vortex
    Vec3 axis{0.0f, 1.0f, 0.0f};
    
    // For wind/turbulence
    f32 frequency = 1.0f;
    f32 noiseStrength = 0.0f;
    
    /// Calculate force at position
    Vec3 calculateForce(const Vec3& particlePos, const Vec3& particleVel) const;
};

// ============================================================================
// Particle System Data
// ============================================================================

/**
 * @brief Complete particle system configuration
 */
struct ParticleSystemData {
    std::string name;
    
    // Modules
    MainModule main;
    EmissionModule emission;
    ShapeModule shape;
    VelocityOverLifetimeModule velocityOverLifetime;
    SizeOverLifetimeModule sizeOverLifetime;
    ColorOverLifetimeModule colorOverLifetime;
    RotationOverLifetimeModule rotationOverLifetime;
    ForceOverLifetimeModule forceOverLifetime;
    NoiseModule noise;
    CollisionModule collision;
    SubEmitterModule subEmitters;
    TextureSheetModule textureSheet;
    RendererModule renderer;
    
    // External forces
    std::vector<ForceField> forces;
};

// ============================================================================
// Statistics
// ============================================================================

/**
 * @brief Particle system statistics
 */
struct ParticleStats {
    u32 activeEmitters = 0;         ///< Number of active emitters
    u32 totalParticles = 0;         ///< Total live particles
    u32 particlesEmitted = 0;       ///< Particles emitted this frame
    u32 particlesDied = 0;          ///< Particles that died this frame
    f64 simulationTimeMs = 0.0;     ///< Simulation time
    f64 renderTimeMs = 0.0;         ///< Render time
};

// ============================================================================
// Callbacks
// ============================================================================

/// Particle collision callback
using ParticleCollisionCallback = std::function<void(Particle&, const Vec3& hitPoint, 
                                                      const Vec3& hitNormal)>;

/// Particle spawn callback
using ParticleSpawnCallback = std::function<void(Particle&)>;

/// Particle death callback
using ParticleDeathCallback = std::function<void(const Particle&)>;

} // namespace nova::particle
