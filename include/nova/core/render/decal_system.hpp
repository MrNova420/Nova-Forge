/**
 * @file decal_system.hpp
 * @brief NovaCore Decal System™ - Deferred and forward decal rendering
 * 
 * Production-grade decal system for the NovaCore engine.
 * Supports deferred decals, projected textures, dynamic decals,
 * and efficient batching for large numbers of decals.
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <functional>

namespace nova::render {

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
// Configuration Constants
// ============================================================================

/**
 * @brief Decal system configuration constants
 */
namespace DecalConfig {
    /// Maximum decals per batch
    constexpr u32 MAX_DECALS_PER_BATCH = 256;
    
    /// Maximum total decals
    constexpr u32 MAX_TOTAL_DECALS = 4096;
    
    /// Default decal atlas size
    constexpr u32 DEFAULT_ATLAS_SIZE = 4096;
    
    /// Default decal lifetime (seconds, 0 = permanent)
    constexpr f32 DEFAULT_LIFETIME = 0.0f;
    
    /// Minimum decal size in world units
    constexpr f32 MIN_DECAL_SIZE = 0.01f;
    
    /// Maximum decal size in world units
    constexpr f32 MAX_DECAL_SIZE = 100.0f;
    
    /// Decal depth bias to prevent z-fighting
    constexpr f32 DEFAULT_DEPTH_BIAS = 0.001f;
    
    /// Maximum decals per surface (for clustering)
    constexpr u32 MAX_DECALS_PER_CLUSTER = 64;
}

// ============================================================================
// Handle Types
// ============================================================================

/// Handle to a decal instance
using DecalHandle = Handle<struct DecalTag>;

/// Handle to a decal material
using DecalMaterialHandle = Handle<struct DecalMaterialTag>;

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Decal projection type
 */
enum class DecalProjection : u8 {
    Box,                ///< Box projection (default)
    Sphere,             ///< Spherical projection
    Cylinder,           ///< Cylindrical projection
    Planar,             ///< Planar projection (for flat surfaces)
    Triplanar           ///< Triplanar projection
};

/**
 * @brief Get projection type name
 */
inline const char* getProjectionName(DecalProjection proj) {
    switch (proj) {
        case DecalProjection::Box: return "Box";
        case DecalProjection::Sphere: return "Sphere";
        case DecalProjection::Cylinder: return "Cylinder";
        case DecalProjection::Planar: return "Planar";
        case DecalProjection::Triplanar: return "Triplanar";
        default: return "Unknown";
    }
}

/**
 * @brief Decal blend mode
 */
enum class DecalBlendMode : u8 {
    Normal,             ///< Standard alpha blending
    Additive,           ///< Additive blending
    Multiply,           ///< Multiply blending
    DBuffer,            ///< Deferred buffer blending (UE4-style)
    Stain               ///< Color stain (tints surface color)
};

/**
 * @brief Get blend mode name
 */
inline const char* getBlendModeName(DecalBlendMode mode) {
    switch (mode) {
        case DecalBlendMode::Normal: return "Normal";
        case DecalBlendMode::Additive: return "Additive";
        case DecalBlendMode::Multiply: return "Multiply";
        case DecalBlendMode::DBuffer: return "DBuffer";
        case DecalBlendMode::Stain: return "Stain";
        default: return "Unknown";
    }
}

/**
 * @brief Decal render queue
 */
enum class DecalQueue : u8 {
    BeforeLighting,     ///< Render before deferred lighting
    AfterLighting,      ///< Render after deferred lighting
    AfterOpaques,       ///< Render after opaque objects
    BeforeTransparents, ///< Render before transparent objects
    AfterAll            ///< Render last
};

/**
 * @brief Decal sorting mode
 */
enum class DecalSortMode : u8 {
    None,               ///< No sorting
    Priority,           ///< Sort by priority
    Distance,           ///< Sort by distance to camera
    Age,                ///< Sort by age (oldest first)
    Material            ///< Sort by material for batching
};

/**
 * @brief Decal fade mode
 */
enum class DecalFadeMode : u8 {
    None,               ///< No fading
    Time,               ///< Fade over lifetime
    Distance,           ///< Fade with distance
    Angle,              ///< Fade based on viewing angle
    Combined            ///< Combine all fade modes
};

/**
 * @brief Decal channel flags
 */
enum class DecalChannels : u8 {
    None = 0,
    Albedo = 1 << 0,    ///< Write to albedo/diffuse
    Normal = 1 << 1,    ///< Write to normal map
    Roughness = 1 << 2, ///< Write to roughness
    Metallic = 1 << 3,  ///< Write to metallic
    Emissive = 1 << 4,  ///< Write to emissive
    AO = 1 << 5,        ///< Write to ambient occlusion
    All = 0x3F          ///< Write to all channels
};

inline DecalChannels operator|(DecalChannels a, DecalChannels b) {
    return static_cast<DecalChannels>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline DecalChannels operator&(DecalChannels a, DecalChannels b) {
    return static_cast<DecalChannels>(static_cast<u8>(a) & static_cast<u8>(b));
}

inline bool hasChannel(DecalChannels flags, DecalChannels channel) {
    return (static_cast<u8>(flags) & static_cast<u8>(channel)) != 0;
}

// ============================================================================
// Decal Material
// ============================================================================

/**
 * @brief Decal material properties
 */
struct DecalMaterial {
    std::string name;
    
    // Texture handles (0 = no texture)
    u32 albedoTexture = 0;
    u32 normalTexture = 0;
    u32 roughnessTexture = 0;
    u32 metallicTexture = 0;
    u32 emissiveTexture = 0;
    u32 opacityTexture = 0;
    
    // Default values when no texture
    Color albedoColor{1.0f, 1.0f, 1.0f, 1.0f};
    f32 roughness = 0.5f;
    f32 metallic = 0.0f;
    Color emissiveColor{0.0f, 0.0f, 0.0f, 1.0f};
    f32 emissiveIntensity = 0.0f;
    
    // Channel mask
    DecalChannels channels = DecalChannels::Albedo;
    
    // Blend settings
    DecalBlendMode blendMode = DecalBlendMode::Normal;
    f32 opacity = 1.0f;
    
    // Normal blending
    f32 normalStrength = 1.0f;
    bool normalBlendWithBase = true;
    
    // UV settings
    Vec2 uvScale{1.0f, 1.0f};
    Vec2 uvOffset{0.0f, 0.0f};
    f32 uvRotation = 0.0f;
    
    /// Check if material affects specific channel
    bool affectsAlbedo() const { return hasChannel(channels, DecalChannels::Albedo); }
    bool affectsNormal() const { return hasChannel(channels, DecalChannels::Normal); }
    bool affectsRoughness() const { return hasChannel(channels, DecalChannels::Roughness); }
    bool affectsMetallic() const { return hasChannel(channels, DecalChannels::Metallic); }
    bool affectsEmissive() const { return hasChannel(channels, DecalChannels::Emissive); }
    
    /// Create basic decal material
    static DecalMaterial basic(const Color& color = Color{1.0f, 1.0f, 1.0f, 1.0f}) {
        DecalMaterial mat;
        mat.albedoColor = color;
        mat.channels = DecalChannels::Albedo;
        return mat;
    }
    
    /// Create blood/damage decal
    static DecalMaterial blood() {
        DecalMaterial mat;
        mat.name = "Blood";
        mat.albedoColor = Color{0.5f, 0.05f, 0.02f, 0.9f};
        mat.roughness = 0.3f;
        mat.channels = DecalChannels::Albedo | DecalChannels::Roughness;
        return mat;
    }
    
    /// Create bullet hole decal
    static DecalMaterial bulletHole() {
        DecalMaterial mat;
        mat.name = "Bullet Hole";
        mat.albedoColor = Color{0.1f, 0.1f, 0.1f, 1.0f};
        mat.roughness = 0.8f;
        mat.channels = DecalChannels::Albedo | DecalChannels::Normal | DecalChannels::Roughness;
        return mat;
    }
    
    /// Create dirt/mud decal
    static DecalMaterial dirt() {
        DecalMaterial mat;
        mat.name = "Dirt";
        mat.albedoColor = Color{0.4f, 0.3f, 0.2f, 0.7f};
        mat.roughness = 0.9f;
        mat.blendMode = DecalBlendMode::Stain;
        mat.channels = DecalChannels::Albedo | DecalChannels::Roughness;
        return mat;
    }
    
    /// Create graffiti/paint decal
    static DecalMaterial paint(const Color& color) {
        DecalMaterial mat;
        mat.name = "Paint";
        mat.albedoColor = color;
        mat.roughness = 0.4f;
        mat.metallic = 0.1f;
        mat.channels = DecalChannels::Albedo | DecalChannels::Roughness | DecalChannels::Metallic;
        return mat;
    }
    
    /// Create emissive decal (hologram, etc.)
    static DecalMaterial emissive(const Color& color, f32 intensity = 5.0f) {
        DecalMaterial mat;
        mat.name = "Emissive";
        mat.albedoColor = Color{0.0f, 0.0f, 0.0f, 0.0f};
        mat.emissiveColor = color;
        mat.emissiveIntensity = intensity;
        mat.blendMode = DecalBlendMode::Additive;
        mat.channels = DecalChannels::Emissive;
        return mat;
    }
};

// ============================================================================
// Decal Instance
// ============================================================================

/**
 * @brief Transform for decal placement
 */
struct DecalTransform {
    Vec3 position{0.0f, 0.0f, 0.0f};
    Quat rotation = Quat::identity();
    Vec3 size{1.0f, 1.0f, 1.0f};      ///< Half-extents for box projection
    
    /// Get world matrix
    Mat4 getWorldMatrix() const {
        Mat4 result = rotation.toMat4();
        // Scale (column-major: columns[col].row)
        result.columns[0].x *= size.x; result.columns[0].y *= size.x; result.columns[0].z *= size.x;
        result.columns[1].x *= size.y; result.columns[1].y *= size.y; result.columns[1].z *= size.y;
        result.columns[2].x *= size.z; result.columns[2].y *= size.z; result.columns[2].z *= size.z;
        // Translation (column 3)
        result.columns[3].x = position.x;
        result.columns[3].y = position.y;
        result.columns[3].z = position.z;
        return result;
    }
    
    /// Get inverse world matrix (for projection)
    Mat4 getInverseWorldMatrix() const {
        Quat invRot = rotation.conjugate();
        Vec3 invScale{1.0f / size.x, 1.0f / size.y, 1.0f / size.z};
        Vec3 rotatedPos = invRot * position;  // Quat * Vec3 works
        Vec3 invPos = Vec3{-rotatedPos.x, -rotatedPos.y, -rotatedPos.z};
        
        Mat4 result = invRot.toMat4();
        // Apply inverse scale (columns are col-major)
        result.columns[0].x *= invScale.x; result.columns[1].x *= invScale.x; result.columns[2].x *= invScale.x;
        result.columns[0].y *= invScale.y; result.columns[1].y *= invScale.y; result.columns[2].y *= invScale.y;
        result.columns[0].z *= invScale.z; result.columns[1].z *= invScale.z; result.columns[2].z *= invScale.z;
        // Translation (apply after scale) - in column 3
        result.columns[3].x = invPos.x * invScale.x;
        result.columns[3].y = invPos.y * invScale.y;
        result.columns[3].z = invPos.z * invScale.z;
        return result;
    }
    
    /// Get forward direction (projection direction)
    Vec3 getForward() const {
        return rotation * Vec3{0.0f, 0.0f, 1.0f};
    }
    
    /// Get up direction
    Vec3 getUp() const {
        return rotation * Vec3{0.0f, 1.0f, 0.0f};
    }
    
    /// Get right direction
    Vec3 getRight() const {
        return rotation * Vec3{1.0f, 0.0f, 0.0f};
    }
    
    /// Create transform looking at target
    static DecalTransform lookAt(const Vec3& pos, const Vec3& target, const Vec3& up = Vec3{0.0f, 1.0f, 0.0f}) {
        DecalTransform t;
        t.position = pos;
        Vec3 forward = (target - pos).normalized();
        t.rotation = Quat::lookRotation(forward, up);
        return t;
    }
    
    /// Create transform from position and normal
    static DecalTransform fromNormal(const Vec3& pos, const Vec3& normal, f32 size = 1.0f) {
        DecalTransform t;
        t.position = pos;
        t.size = Vec3{size, size, size * 0.1f}; // Thin along projection axis
        
        // Calculate rotation from normal
        Vec3 up = std::abs(normal.y) > 0.99f ? Vec3{1.0f, 0.0f, 0.0f} : Vec3{0.0f, 1.0f, 0.0f};
        t.rotation = Quat::lookRotation(-normal, up);
        
        return t;
    }
};

/**
 * @brief Fade settings for decals
 */
struct DecalFade {
    DecalFadeMode mode = DecalFadeMode::None;
    
    // Time fade
    f32 fadeInTime = 0.0f;              ///< Time to fade in
    f32 fadeOutTime = 0.5f;             ///< Time to fade out
    f32 lifetime = 0.0f;                ///< Total lifetime (0 = permanent)
    
    // Distance fade
    f32 fadeStartDistance = 50.0f;      ///< Start fading at this distance
    f32 fadeEndDistance = 100.0f;       ///< Fully faded at this distance
    
    // Angle fade
    f32 fadeAngleStart = 70.0f;         ///< Start fading at this angle from normal
    f32 fadeAngleEnd = 85.0f;           ///< Fully faded at this angle
    
    /// Calculate fade factor based on current state
    f32 calculateFade(f32 age, f32 distance, f32 angle) const {
        f32 fade = 1.0f;
        
        if (mode == DecalFadeMode::None) return 1.0f;
        
        // Time fade
        if (mode == DecalFadeMode::Time || mode == DecalFadeMode::Combined) {
            if (lifetime > 0.0f) {
                // Fade in
                if (fadeInTime > 0.0f && age < fadeInTime) {
                    fade *= age / fadeInTime;
                }
                // Fade out
                f32 timeUntilDeath = lifetime - age;
                if (fadeOutTime > 0.0f && timeUntilDeath < fadeOutTime) {
                    fade *= timeUntilDeath / fadeOutTime;
                }
            }
        }
        
        // Distance fade
        if (mode == DecalFadeMode::Distance || mode == DecalFadeMode::Combined) {
            if (distance > fadeStartDistance) {
                f32 t = (distance - fadeStartDistance) / (fadeEndDistance - fadeStartDistance);
                fade *= 1.0f - std::clamp(t, 0.0f, 1.0f);
            }
        }
        
        // Angle fade
        if (mode == DecalFadeMode::Angle || mode == DecalFadeMode::Combined) {
            if (angle > fadeAngleStart) {
                f32 t = (angle - fadeAngleStart) / (fadeAngleEnd - fadeAngleStart);
                fade *= 1.0f - std::clamp(t, 0.0f, 1.0f);
            }
        }
        
        return std::clamp(fade, 0.0f, 1.0f);
    }
    
    /// Is decal expired?
    bool isExpired(f32 age) const {
        return lifetime > 0.0f && age >= lifetime;
    }
    
    /// Create permanent decal settings
    static DecalFade permanent() {
        return DecalFade{}; // Defaults are permanent
    }
    
    /// Create temporary decal with time fade
    static DecalFade temporary(f32 duration, f32 fadeOut = 0.5f) {
        DecalFade f;
        f.mode = DecalFadeMode::Time;
        f.lifetime = duration;
        f.fadeOutTime = fadeOut;
        return f;
    }
    
    /// Create distance-faded decal
    static DecalFade distanceFade(f32 start, f32 end) {
        DecalFade f;
        f.mode = DecalFadeMode::Distance;
        f.fadeStartDistance = start;
        f.fadeEndDistance = end;
        return f;
    }
};

/**
 * @brief Decal instance data
 */
struct Decal {
    DecalHandle handle;
    DecalMaterialHandle material;
    
    DecalTransform transform;
    DecalProjection projection = DecalProjection::Box;
    DecalQueue queue = DecalQueue::BeforeLighting;
    
    // Instance properties
    Color colorTint{1.0f, 1.0f, 1.0f, 1.0f};
    f32 opacity = 1.0f;
    i32 priority = 0;               ///< Higher = rendered on top
    
    // Fade settings
    DecalFade fade;
    f32 age = 0.0f;                 ///< Current age in seconds
    
    // Culling
    u32 layerMask = 0xFFFFFFFF;     ///< Which layers this decal affects
    f32 depthBias = DecalConfig::DEFAULT_DEPTH_BIAS;
    
    // State
    bool enabled = true;
    bool visible = true;            ///< Set by culling
    f32 currentFade = 1.0f;         ///< Current computed fade value
    
    /// Update decal state
    void update(f32 deltaTime, const Vec3& cameraPos) {
        age += deltaTime;
        
        // Calculate distance and angle for fade
        f32 distance = (transform.position - cameraPos).length();
        Vec3 toCamera = (cameraPos - transform.position).normalized();
        f32 angle = std::acos(std::clamp(toCamera.dot(transform.getForward()), -1.0f, 1.0f));
        angle = angle * (180.0f / 3.14159265f);
        
        currentFade = fade.calculateFade(age, distance, angle);
    }
    
    /// Is decal expired?
    bool isExpired() const {
        return fade.isExpired(age);
    }
    
    /// Get effective opacity
    f32 getEffectiveOpacity() const {
        return opacity * colorTint.w * currentFade;
    }
    
    /// Get bounding sphere radius (for culling)
    f32 getBoundingRadius() const {
        return transform.size.length();
    }
};

// ============================================================================
// GPU Data Structures
// ============================================================================

/**
 * @brief GPU-ready decal data (must match shader)
 */
struct alignas(16) GPUDecalData {
    Mat4 worldToDecal;              ///< Transform from world to decal space
    Vec4 colorTint;                 ///< RGBA tint
    Vec4 uvScaleOffset;             ///< xy = scale, zw = offset
    Vec4 params;                    ///< x = opacity, y = normalStrength, z = depthBias, w = projection type
    Vec4 channelMask;               ///< Which channels to write (as floats for shader)
};

/**
 * @brief GPU-ready decal batch data
 */
struct GPUDecalBatch {
    std::vector<GPUDecalData> decals;
    u32 materialIndex = 0;
    DecalBlendMode blendMode = DecalBlendMode::Normal;
    DecalQueue queue = DecalQueue::BeforeLighting;
    
    usize count() const { return decals.size(); }
    bool canAdd() const { return decals.size() < DecalConfig::MAX_DECALS_PER_BATCH; }
};

// ============================================================================
// Decal Spawner
// ============================================================================

/**
 * @brief Settings for spawning decals procedurally
 */
struct DecalSpawnSettings {
    DecalMaterialHandle material;
    
    // Size variation
    Vec2 sizeRange{0.5f, 1.5f};     ///< Min/max size multiplier
    
    // Rotation variation
    bool randomRotation = true;
    f32 rotationRange = 360.0f;     ///< Rotation range in degrees
    
    // Color variation
    bool randomColor = false;
    std::vector<Color> colorPalette;
    
    // Fade settings
    DecalFade fade;
    
    // Spawn limits
    u32 maxDecalsInArea = 10;       ///< Max decals within radius
    f32 minSpacing = 0.5f;          ///< Minimum distance between decals
    
    /// Generate random decal properties
    Decal spawn(const Vec3& position, const Vec3& normal, u32 seed = 0) const {
        Decal decal;
        decal.material = material;
        
        // Random size within range
        f32 sizeT = static_cast<f32>(seed & 0xFF) / 255.0f;
        f32 size = sizeRange.x + (sizeRange.y - sizeRange.x) * sizeT;
        
        // Transform from position/normal
        decal.transform = DecalTransform::fromNormal(position, normal, size);
        
        // Random rotation
        if (randomRotation) {
            f32 rotT = static_cast<f32>((seed >> 8) & 0xFF) / 255.0f;
            f32 rot = rotT * rotationRange * (3.14159265f / 180.0f);
            decal.transform.rotation = decal.transform.rotation * Quat::fromAxisAngle(Vec3{0.0f, 0.0f, 1.0f}, rot);
        }
        
        // Random color
        if (randomColor && !colorPalette.empty()) {
            u32 colorIdx = (seed >> 16) % static_cast<u32>(colorPalette.size());
            decal.colorTint = colorPalette[colorIdx];
        }
        
        decal.fade = fade;
        
        return decal;
    }
};

// ============================================================================
// Decal Manager
// ============================================================================

/**
 * @brief Decal system statistics
 */
struct DecalStats {
    u32 totalDecals = 0;
    u32 visibleDecals = 0;
    u32 culledDecals = 0;
    u32 batchCount = 0;
    u32 drawCalls = 0;
    f64 updateTimeMs = 0.0;
    f64 renderTimeMs = 0.0;
    u32 decalsSpawnedThisFrame = 0;
    u32 decalsRemovedThisFrame = 0;
};

/**
 * @brief Decal system manager singleton
 */
class DecalManager {
public:
    /// Get singleton instance
    static DecalManager& getInstance() {
        static DecalManager instance;
        return instance;
    }
    
    /// Initialize decal system
    void initialize() {
        m_initialized = true;
        m_decals.reserve(DecalConfig::MAX_TOTAL_DECALS);
    }
    
    /// Shutdown decal system
    void shutdown() {
        m_decals.clear();
        m_materials.clear();
        m_initialized = false;
    }
    
    /// Update all decals
    void update(f32 deltaTime, const Vec3& cameraPos) {
        m_stats.decalsRemovedThisFrame = 0;
        
        // Update all decals
        for (auto& decal : m_decals) {
            decal.update(deltaTime, cameraPos);
        }
        
        // Remove expired decals
        auto it = std::remove_if(m_decals.begin(), m_decals.end(),
            [this](const Decal& d) {
                if (d.isExpired()) {
                    m_stats.decalsRemovedThisFrame++;
                    return true;
                }
                return false;
            });
        m_decals.erase(it, m_decals.end());
        
        m_stats.totalDecals = static_cast<u32>(m_decals.size());
    }
    
    /// Add a decal
    DecalHandle addDecal(const Decal& decal) {
        if (m_decals.size() >= DecalConfig::MAX_TOTAL_DECALS) {
            // Remove oldest decal if at limit
            m_decals.erase(m_decals.begin());
        }
        
        Decal newDecal = decal;
        newDecal.handle = DecalHandle{m_nextId++};
        m_decals.push_back(newDecal);
        m_stats.decalsSpawnedThisFrame++;
        
        return newDecal.handle;
    }
    
    /// Spawn decal at hit point
    DecalHandle spawnDecal(DecalMaterialHandle material, const Vec3& position, 
                           const Vec3& normal, f32 size = 1.0f, 
                           const DecalFade& fade = DecalFade::permanent()) {
        Decal decal;
        decal.material = material;
        decal.transform = DecalTransform::fromNormal(position, normal, size);
        decal.fade = fade;
        return addDecal(decal);
    }
    
    /// Spawn decal with full settings
    DecalHandle spawnDecal(const DecalSpawnSettings& settings, const Vec3& position, 
                           const Vec3& normal, u32 seed = 0) {
        Decal decal = settings.spawn(position, normal, seed);
        return addDecal(decal);
    }
    
    /// Remove a decal
    bool removeDecal(DecalHandle handle) {
        auto it = std::find_if(m_decals.begin(), m_decals.end(),
            [handle](const Decal& d) { return d.handle == handle; });
        if (it != m_decals.end()) {
            m_decals.erase(it);
            return true;
        }
        return false;
    }
    
    /// Get decal by handle
    Decal* getDecal(DecalHandle handle) {
        auto it = std::find_if(m_decals.begin(), m_decals.end(),
            [handle](const Decal& d) { return d.handle == handle; });
        return it != m_decals.end() ? &(*it) : nullptr;
    }
    
    /// Register material
    DecalMaterialHandle registerMaterial(const DecalMaterial& material) {
        DecalMaterialHandle handle{m_nextMaterialId++};
        m_materials[handle.value] = material;
        return handle;
    }
    
    /// Get material
    const DecalMaterial* getMaterial(DecalMaterialHandle handle) const {
        auto it = m_materials.find(handle.value);
        return it != m_materials.end() ? &it->second : nullptr;
    }
    
    /// Clear all decals
    void clearAll() {
        m_decals.clear();
    }
    
    /// Clear decals matching predicate
    template<typename Pred>
    void clearIf(Pred predicate) {
        auto it = std::remove_if(m_decals.begin(), m_decals.end(), predicate);
        m_decals.erase(it, m_decals.end());
    }
    
    /// Get all decals
    const std::vector<Decal>& getDecals() const { return m_decals; }
    
    /// Get visible decals for rendering
    std::vector<const Decal*> getVisibleDecals(DecalQueue queue) const {
        std::vector<const Decal*> result;
        for (const auto& decal : m_decals) {
            if (decal.enabled && decal.visible && decal.queue == queue) {
                result.push_back(&decal);
            }
        }
        return result;
    }
    
    /// Build GPU batches for rendering
    std::vector<GPUDecalBatch> buildBatches(DecalQueue queue, DecalSortMode sortMode = DecalSortMode::Material) const {
        std::vector<GPUDecalBatch> batches;
        
        // Get visible decals for this queue
        auto visibleDecals = getVisibleDecals(queue);
        if (visibleDecals.empty()) return batches;
        
        // Sort decals
        switch (sortMode) {
            case DecalSortMode::Priority:
                std::sort(visibleDecals.begin(), visibleDecals.end(),
                    [](const Decal* a, const Decal* b) { return a->priority < b->priority; });
                break;
            case DecalSortMode::Age:
                std::sort(visibleDecals.begin(), visibleDecals.end(),
                    [](const Decal* a, const Decal* b) { return a->age > b->age; });
                break;
            case DecalSortMode::Material:
                std::sort(visibleDecals.begin(), visibleDecals.end(),
                    [](const Decal* a, const Decal* b) { return a->material.value < b->material.value; });
                break;
            default:
                break;
        }
        
        // Build batches
        GPUDecalBatch currentBatch;
        DecalMaterialHandle currentMaterial{0};
        
        for (const Decal* decal : visibleDecals) {
            // Start new batch if material changed or batch full
            if (decal->material.value != currentMaterial.value || !currentBatch.canAdd()) {
                if (!currentBatch.decals.empty()) {
                    batches.push_back(std::move(currentBatch));
                    currentBatch = GPUDecalBatch{};
                }
                currentMaterial = decal->material;
                currentBatch.materialIndex = currentMaterial.value;
                currentBatch.queue = queue;
                
                // Get blend mode from material
                const DecalMaterial* mat = getMaterial(currentMaterial);
                if (mat) currentBatch.blendMode = mat->blendMode;
            }
            
            // Add decal to batch
            GPUDecalData gpuData;
            gpuData.worldToDecal = decal->transform.getInverseWorldMatrix();
            gpuData.colorTint = decal->colorTint * Color{1.0f, 1.0f, 1.0f, decal->getEffectiveOpacity()};
            
            const DecalMaterial* mat = getMaterial(decal->material);
            if (mat) {
                gpuData.uvScaleOffset = Vec4{mat->uvScale.x, mat->uvScale.y, mat->uvOffset.x, mat->uvOffset.y};
                gpuData.params = Vec4{mat->opacity, mat->normalStrength, decal->depthBias, static_cast<f32>(decal->projection)};
                gpuData.channelMask = Vec4{
                    mat->affectsAlbedo() ? 1.0f : 0.0f,
                    mat->affectsNormal() ? 1.0f : 0.0f,
                    mat->affectsRoughness() ? 1.0f : 0.0f,
                    mat->affectsMetallic() ? 1.0f : 0.0f
                };
            }
            
            currentBatch.decals.push_back(gpuData);
        }
        
        // Add final batch
        if (!currentBatch.decals.empty()) {
            batches.push_back(std::move(currentBatch));
        }
        
        return batches;
    }
    
    /// Get statistics
    const DecalStats& getStats() const { return m_stats; }
    
    /// Is system initialized?
    bool isInitialized() const { return m_initialized; }
    
private:
    DecalManager() = default;
    
    bool m_initialized = false;
    std::vector<Decal> m_decals;
    std::unordered_map<u32, DecalMaterial> m_materials;
    u32 m_nextId = 1;
    u32 m_nextMaterialId = 1;
    DecalStats m_stats;
};

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Project world position to decal UV coordinates
 */
inline Vec2 projectToDecalUV(const Vec3& worldPos, const DecalTransform& decalTransform) {
    Mat4 invWorld = decalTransform.getInverseWorldMatrix();
    Vec4 localPos = invWorld * Vec4{worldPos.x, worldPos.y, worldPos.z, 1.0f};
    
    // UV from XY position in decal space (-1 to 1 -> 0 to 1)
    return Vec2{
        localPos.x * 0.5f + 0.5f,
        localPos.y * 0.5f + 0.5f
    };
}

/**
 * @brief Check if world position is inside decal volume
 */
inline bool isInsideDecal(const Vec3& worldPos, const DecalTransform& decalTransform, 
                          DecalProjection projection = DecalProjection::Box) {
    Mat4 invWorld = decalTransform.getInverseWorldMatrix();
    Vec4 localPos = invWorld * Vec4{worldPos.x, worldPos.y, worldPos.z, 1.0f};
    
    switch (projection) {
        case DecalProjection::Box:
            return std::abs(localPos.x) <= 1.0f && 
                   std::abs(localPos.y) <= 1.0f && 
                   std::abs(localPos.z) <= 1.0f;
        
        case DecalProjection::Sphere: {
            f32 dist = std::sqrt(localPos.x * localPos.x + 
                                 localPos.y * localPos.y + 
                                 localPos.z * localPos.z);
            return dist <= 1.0f;
        }
        
        case DecalProjection::Cylinder: {
            f32 dist2D = std::sqrt(localPos.x * localPos.x + localPos.y * localPos.y);
            return dist2D <= 1.0f && std::abs(localPos.z) <= 1.0f;
        }
        
        default:
            return std::abs(localPos.x) <= 1.0f && std::abs(localPos.y) <= 1.0f;
    }
}

/**
 * @brief Calculate decal depth at position (for sorting)
 */
inline f32 getDecalDepth(const Vec3& worldPos, const DecalTransform& decalTransform) {
    Mat4 invWorld = decalTransform.getInverseWorldMatrix();
    Vec4 localPos = invWorld * Vec4{worldPos.x, worldPos.y, worldPos.z, 1.0f};
    return localPos.z; // Depth along projection axis
}

} // namespace nova::render
