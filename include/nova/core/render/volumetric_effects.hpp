/**
 * @file volumetric_effects.hpp
 * @brief Volumetric Effects System for NovaCore Engine
 * 
 * Provides comprehensive volumetric rendering:
 * - Volumetric fog with distance/height attenuation
 * - Volumetric lighting (god rays)
 * - Cloud rendering (procedural and texture-based)
 * - Participating media (smoke, dust, steam)
 * - Froxel-based volumetric system
 * - Temporal reprojection for smooth results
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/math/mat4.hpp"

#include <array>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace nova {

// =============================================================================
// Configuration Constants
// =============================================================================

/**
 * @brief Volumetric effects configuration constants
 */
struct VolumetricConfig {
    /// Default froxel grid width
    static constexpr u32 DEFAULT_FROXEL_WIDTH = 160;
    
    /// Default froxel grid height
    static constexpr u32 DEFAULT_FROXEL_HEIGHT = 90;
    
    /// Default froxel depth slices
    static constexpr u32 DEFAULT_FROXEL_DEPTH = 64;
    
    /// Maximum froxel depth
    static constexpr u32 MAX_FROXEL_DEPTH = 256;
    
    /// Maximum fog volumes
    static constexpr u32 MAX_FOG_VOLUMES = 64;
    
    /// Maximum light shafts
    static constexpr u32 MAX_LIGHT_SHAFTS = 16;
    
    /// Default scattering coefficient
    static constexpr f32 DEFAULT_SCATTERING = 0.01f;
    
    /// Default absorption coefficient
    static constexpr f32 DEFAULT_ABSORPTION = 0.001f;
    
    /// Maximum ray march steps
    static constexpr u32 MAX_RAY_MARCH_STEPS = 128;
    
    /// Temporal reprojection blend factor
    static constexpr f32 TEMPORAL_BLEND = 0.95f;
    
    /// Noise texture size
    static constexpr u32 NOISE_TEXTURE_SIZE = 128;
};

// =============================================================================
// Enumerations
// =============================================================================

/**
 * @brief Volumetric fog mode
 */
enum class FogMode : u32 {
    /// No fog
    None = 0,
    
    /// Linear distance fog
    Linear = 1,
    
    /// Exponential distance fog
    Exponential = 2,
    
    /// Exponential squared distance fog
    ExponentialSquared = 3,
    
    /// Height-based fog
    Height = 4,
    
    /// Volumetric fog (full raymarching)
    Volumetric = 5,
    
    COUNT
};

/**
 * @brief Get fog mode name
 */
[[nodiscard]] constexpr const char* getFogModeName(FogMode mode) noexcept {
    switch (mode) {
        case FogMode::None: return "None";
        case FogMode::Linear: return "Linear";
        case FogMode::Exponential: return "Exponential";
        case FogMode::ExponentialSquared: return "Exponential Squared";
        case FogMode::Height: return "Height";
        case FogMode::Volumetric: return "Volumetric";
        default: return "Unknown";
    }
}

/**
 * @brief Fog volume shape
 */
enum class FogVolumeShape : u32 {
    /// Global (infinite) fog
    Global = 0,
    
    /// Box-shaped volume
    Box = 1,
    
    /// Sphere-shaped volume
    Sphere = 2,
    
    /// Cylinder-shaped volume
    Cylinder = 3,
    
    /// Cone-shaped volume
    Cone = 4,
    
    COUNT
};

/**
 * @brief Get fog volume shape name
 */
[[nodiscard]] constexpr const char* getFogVolumeShapeName(FogVolumeShape shape) noexcept {
    switch (shape) {
        case FogVolumeShape::Global: return "Global";
        case FogVolumeShape::Box: return "Box";
        case FogVolumeShape::Sphere: return "Sphere";
        case FogVolumeShape::Cylinder: return "Cylinder";
        case FogVolumeShape::Cone: return "Cone";
        default: return "Unknown";
    }
}

/**
 * @brief Cloud rendering mode
 */
enum class CloudMode : u32 {
    /// No clouds
    None = 0,
    
    /// 2D cloud layer (skybox texture)
    Layer2D = 1,
    
    /// Procedural 3D clouds
    Procedural3D = 2,
    
    /// Volumetric ray-marched clouds
    Volumetric = 3,
    
    COUNT
};

/**
 * @brief Get cloud mode name
 */
[[nodiscard]] constexpr const char* getCloudModeName(CloudMode mode) noexcept {
    switch (mode) {
        case CloudMode::None: return "None";
        case CloudMode::Layer2D: return "2D Layer";
        case CloudMode::Procedural3D: return "Procedural 3D";
        case CloudMode::Volumetric: return "Volumetric";
        default: return "Unknown";
    }
}

/**
 * @brief Light shaft quality
 */
enum class LightShaftQuality : u32 {
    /// Disabled
    Off = 0,
    
    /// Low quality (fewer samples)
    Low = 1,
    
    /// Medium quality
    Medium = 2,
    
    /// High quality (more samples)
    High = 3,
    
    COUNT
};

/**
 * @brief Get light shaft quality name
 */
[[nodiscard]] constexpr const char* getLightShaftQualityName(LightShaftQuality quality) noexcept {
    switch (quality) {
        case LightShaftQuality::Off: return "Off";
        case LightShaftQuality::Low: return "Low";
        case LightShaftQuality::Medium: return "Medium";
        case LightShaftQuality::High: return "High";
        default: return "Unknown";
    }
}

/**
 * @brief Volumetric quality preset
 */
enum class VolumetricQuality : u32 {
    /// Lowest quality (mobile)
    Low = 0,
    
    /// Medium quality
    Medium = 1,
    
    /// High quality
    High = 2,
    
    /// Ultra quality
    Ultra = 3,
    
    COUNT
};

// =============================================================================
// Data Structures
// =============================================================================

/**
 * @brief Global fog settings
 */
struct FogSettings {
    /// Fog mode
    FogMode mode = FogMode::Exponential;
    
    /// Fog color
    Vec3 color = Vec3{0.7f, 0.75f, 0.8f};
    
    /// Fog density
    f32 density = 0.02f;
    
    /// Fog start distance (for linear)
    f32 startDistance = 10.0f;
    
    /// Fog end distance (for linear)
    f32 endDistance = 100.0f;
    
    /// Height fog floor
    f32 heightFloor = 0.0f;
    
    /// Height fog falloff
    f32 heightFalloff = 0.1f;
    
    /// Maximum fog opacity
    f32 maxOpacity = 1.0f;
    
    /// Enable sun scattering in fog
    bool sunScattering = true;
    
    /// Sun scattering intensity
    f32 sunScatteringIntensity = 1.0f;
    
    /// Mie scattering phase (0 = isotropic, 1 = forward)
    f32 miePhase = 0.8f;
    
    /**
     * @brief Calculate fog factor for linear mode
     */
    [[nodiscard]] f32 calculateLinearFog(f32 distance) const noexcept {
        if (mode != FogMode::Linear) return 0.0f;
        return std::clamp((distance - startDistance) / (endDistance - startDistance), 0.0f, maxOpacity);
    }
    
    /**
     * @brief Calculate fog factor for exponential mode
     */
    [[nodiscard]] f32 calculateExponentialFog(f32 distance) const noexcept {
        if (mode == FogMode::Exponential) {
            return std::clamp(1.0f - std::exp(-density * distance), 0.0f, maxOpacity);
        }
        if (mode == FogMode::ExponentialSquared) {
            f32 d = density * distance;
            return std::clamp(1.0f - std::exp(-d * d), 0.0f, maxOpacity);
        }
        return 0.0f;
    }
    
    /**
     * @brief Calculate height fog factor
     */
    [[nodiscard]] f32 calculateHeightFog(f32 height) const noexcept {
        if (height <= heightFloor) {
            return maxOpacity;
        }
        f32 heightAboveFloor = height - heightFloor;
        return std::clamp(std::exp(-heightFalloff * heightAboveFloor), 0.0f, maxOpacity);
    }
    
    /**
     * @brief Create default outdoor fog
     */
    [[nodiscard]] static FogSettings createOutdoor() noexcept {
        FogSettings settings;
        settings.mode = FogMode::Height;
        settings.color = Vec3{0.7f, 0.8f, 0.9f};
        settings.density = 0.005f;
        settings.heightFloor = 0.0f;
        settings.heightFalloff = 0.02f;
        return settings;
    }
    
    /**
     * @brief Create dense fog
     */
    [[nodiscard]] static FogSettings createDense() noexcept {
        FogSettings settings;
        settings.mode = FogMode::Exponential;
        settings.color = Vec3{0.6f, 0.65f, 0.7f};
        settings.density = 0.1f;
        settings.maxOpacity = 0.95f;
        return settings;
    }
};

/**
 * @brief Fog volume (local fog area)
 */
struct FogVolume {
    /// Volume name
    std::string name = "FogVolume";
    
    /// Volume shape
    FogVolumeShape shape = FogVolumeShape::Box;
    
    /// World-space position
    Vec3 position = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Rotation (Euler angles in radians)
    Vec3 rotation = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Box extents (for box shape)
    Vec3 boxExtents = Vec3{10.0f, 10.0f, 10.0f};
    
    /// Sphere radius (for sphere shape)
    f32 sphereRadius = 10.0f;
    
    /// Cylinder radius
    f32 cylinderRadius = 5.0f;
    
    /// Cylinder height
    f32 cylinderHeight = 10.0f;
    
    /// Fog color
    Vec3 color = Vec3{0.8f, 0.85f, 0.9f};
    
    /// Density multiplier
    f32 densityMultiplier = 1.0f;
    
    /// Blend distance (soft edge)
    f32 blendDistance = 1.0f;
    
    /// Priority for blending
    i32 priority = 0;
    
    /// Is volume enabled
    bool enabled = true;
    
    /**
     * @brief Check if point is inside volume
     */
    [[nodiscard]] bool containsPoint(const Vec3& point) const noexcept {
        if (!enabled) return false;
        
        Vec3 local = Vec3{point.x - position.x, point.y - position.y, point.z - position.z};
        
        switch (shape) {
            case FogVolumeShape::Global:
                return true;
                
            case FogVolumeShape::Box:
                return std::abs(local.x) <= boxExtents.x &&
                       std::abs(local.y) <= boxExtents.y &&
                       std::abs(local.z) <= boxExtents.z;
                
            case FogVolumeShape::Sphere: {
                f32 distSq = local.x * local.x + local.y * local.y + local.z * local.z;
                return distSq <= sphereRadius * sphereRadius;
            }
            
            case FogVolumeShape::Cylinder: {
                f32 horizDistSq = local.x * local.x + local.z * local.z;
                return horizDistSq <= cylinderRadius * cylinderRadius &&
                       std::abs(local.y) <= cylinderHeight * 0.5f;
            }
            
            default:
                return false;
        }
    }
    
    /**
     * @brief Calculate density at point (with falloff)
     */
    [[nodiscard]] f32 getDensityAtPoint(const Vec3& point) const noexcept {
        if (!enabled) return 0.0f;
        
        Vec3 local = Vec3{point.x - position.x, point.y - position.y, point.z - position.z};
        f32 distToEdge = 0.0f;
        
        switch (shape) {
            case FogVolumeShape::Global:
                return densityMultiplier;
                
            case FogVolumeShape::Box: {
                Vec3 absLocal = Vec3{std::abs(local.x), std::abs(local.y), std::abs(local.z)};
                distToEdge = std::min({
                    boxExtents.x - absLocal.x,
                    boxExtents.y - absLocal.y,
                    boxExtents.z - absLocal.z
                });
                break;
            }
            
            case FogVolumeShape::Sphere: {
                f32 dist = std::sqrt(local.x * local.x + local.y * local.y + local.z * local.z);
                distToEdge = sphereRadius - dist;
                break;
            }
            
            default:
                return 0.0f;
        }
        
        if (distToEdge < 0.0f) return 0.0f;
        if (distToEdge >= blendDistance) return densityMultiplier;
        
        return densityMultiplier * (distToEdge / blendDistance);
    }
};

/**
 * @brief Froxel grid configuration
 */
struct FroxelGridConfig {
    /// Grid width (froxels)
    u32 width = VolumetricConfig::DEFAULT_FROXEL_WIDTH;
    
    /// Grid height (froxels)
    u32 height = VolumetricConfig::DEFAULT_FROXEL_HEIGHT;
    
    /// Grid depth slices
    u32 depth = VolumetricConfig::DEFAULT_FROXEL_DEPTH;
    
    /// Near plane distance
    f32 nearPlane = 0.1f;
    
    /// Far plane distance
    f32 farPlane = 100.0f;
    
    /// Use logarithmic depth distribution
    bool logarithmicDepth = true;
    
    /**
     * @brief Get total froxel count
     */
    [[nodiscard]] u32 getTotalFroxels() const noexcept {
        return width * height * depth;
    }
    
    /**
     * @brief Get depth slice from linear depth
     */
    [[nodiscard]] u32 getDepthSlice(f32 linearDepth) const noexcept {
        if (linearDepth <= nearPlane) return 0;
        if (linearDepth >= farPlane) return depth - 1;
        
        if (logarithmicDepth) {
            f32 logNear = std::log(nearPlane);
            f32 logFar = std::log(farPlane);
            f32 logDepth = std::log(linearDepth);
            f32 t = (logDepth - logNear) / (logFar - logNear);
            return static_cast<u32>(t * (depth - 1));
        }
        
        f32 t = (linearDepth - nearPlane) / (farPlane - nearPlane);
        return static_cast<u32>(t * (depth - 1));
    }
    
    /**
     * @brief Get linear depth from slice
     */
    [[nodiscard]] f32 getSliceDepth(u32 slice) const noexcept {
        if (slice >= depth) slice = depth - 1;
        f32 t = static_cast<f32>(slice) / static_cast<f32>(depth - 1);
        
        if (logarithmicDepth) {
            f32 logNear = std::log(nearPlane);
            f32 logFar = std::log(farPlane);
            return std::exp(logNear + t * (logFar - logNear));
        }
        
        return nearPlane + t * (farPlane - nearPlane);
    }
};

/**
 * @brief Cloud layer settings
 */
struct CloudSettings {
    /// Cloud rendering mode
    CloudMode mode = CloudMode::None;
    
    /// Cloud layer altitude (meters)
    f32 altitude = 2000.0f;
    
    /// Cloud layer thickness
    f32 thickness = 500.0f;
    
    /// Cloud coverage (0-1)
    f32 coverage = 0.5f;
    
    /// Cloud density
    f32 density = 0.5f;
    
    /// Cloud color (top)
    Vec3 colorTop = Vec3{1.0f, 1.0f, 1.0f};
    
    /// Cloud color (bottom/shadow)
    Vec3 colorBottom = Vec3{0.6f, 0.65f, 0.7f};
    
    /// Wind direction (XZ plane)
    Vec2 windDirection = Vec2{1.0f, 0.0f};
    
    /// Wind speed (m/s)
    f32 windSpeed = 10.0f;
    
    /// Detail noise scale
    f32 detailScale = 0.1f;
    
    /// Detail noise strength
    f32 detailStrength = 0.3f;
    
    /// Ray march steps (for volumetric)
    u32 rayMarchSteps = 64;
    
    /// Light steps (for volumetric lighting)
    u32 lightSteps = 8;
    
    /// Enable shadow casting
    bool castShadows = true;
    
    /**
     * @brief Check if clouds are enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return mode != CloudMode::None;
    }
    
    /**
     * @brief Create cirrus clouds preset
     */
    [[nodiscard]] static CloudSettings createCirrus() noexcept {
        CloudSettings settings;
        settings.mode = CloudMode::Layer2D;
        settings.altitude = 8000.0f;
        settings.coverage = 0.3f;
        settings.density = 0.2f;
        return settings;
    }
    
    /**
     * @brief Create cumulus clouds preset
     */
    [[nodiscard]] static CloudSettings createCumulus() noexcept {
        CloudSettings settings;
        settings.mode = CloudMode::Volumetric;
        settings.altitude = 1500.0f;
        settings.thickness = 1000.0f;
        settings.coverage = 0.6f;
        settings.density = 0.7f;
        settings.rayMarchSteps = 128;
        return settings;
    }
    
    /**
     * @brief Create overcast preset
     */
    [[nodiscard]] static CloudSettings createOvercast() noexcept {
        CloudSettings settings;
        settings.mode = CloudMode::Volumetric;
        settings.altitude = 500.0f;
        settings.thickness = 800.0f;
        settings.coverage = 0.9f;
        settings.density = 0.8f;
        settings.colorBottom = Vec3{0.4f, 0.42f, 0.45f};
        return settings;
    }
};

/**
 * @brief Light shaft (god ray) settings
 */
struct LightShaftSettings {
    /// Light shaft quality
    LightShaftQuality quality = LightShaftQuality::Medium;
    
    /// Enable light shafts
    bool enabled = true;
    
    /// Number of samples
    u32 sampleCount = 64;
    
    /// Intensity multiplier
    f32 intensity = 1.0f;
    
    /// Decay (falloff per sample)
    f32 decay = 0.96f;
    
    /// Density (scattering)
    f32 density = 1.0f;
    
    /// Weight (blend factor)
    f32 weight = 0.5f;
    
    /// Exposure (brightness)
    f32 exposure = 1.0f;
    
    /// Color tint
    Vec3 color = Vec3{1.0f, 0.95f, 0.9f};
    
    /// Threshold (minimum brightness to create rays)
    f32 threshold = 0.9f;
    
    /**
     * @brief Get effective sample count based on quality
     */
    [[nodiscard]] u32 getEffectiveSampleCount() const noexcept {
        switch (quality) {
            case LightShaftQuality::Off: return 0;
            case LightShaftQuality::Low: return 32;
            case LightShaftQuality::Medium: return 64;
            case LightShaftQuality::High: return 128;
            default: return sampleCount;
        }
    }
    
    /**
     * @brief Check if enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return enabled && quality != LightShaftQuality::Off;
    }
};

/**
 * @brief Participating media (smoke, dust, etc.)
 */
struct ParticipatingMedia {
    /// Media name
    std::string name = "Media";
    
    /// World-space position
    Vec3 position = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Bounding box extents
    Vec3 extents = Vec3{5.0f, 5.0f, 5.0f};
    
    /// Scattering coefficient (RGB)
    Vec3 scattering = Vec3{0.1f, 0.1f, 0.1f};
    
    /// Absorption coefficient (RGB)
    Vec3 absorption = Vec3{0.01f, 0.01f, 0.01f};
    
    /// Phase function anisotropy (-1 = back, 0 = iso, 1 = forward)
    f32 phaseG = 0.0f;
    
    /// Emission (for glowing effects)
    Vec3 emission = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Density multiplier
    f32 density = 1.0f;
    
    /// Noise scale
    f32 noiseScale = 1.0f;
    
    /// Noise intensity
    f32 noiseIntensity = 0.5f;
    
    /// Animation speed
    f32 animationSpeed = 1.0f;
    
    /// Is enabled
    bool enabled = true;
    
    /**
     * @brief Get extinction coefficient (scattering + absorption)
     */
    [[nodiscard]] Vec3 getExtinction() const noexcept {
        return Vec3{
            scattering.x + absorption.x,
            scattering.y + absorption.y,
            scattering.z + absorption.z
        };
    }
    
    /**
     * @brief Get albedo (scattering / extinction)
     */
    [[nodiscard]] Vec3 getAlbedo() const noexcept {
        Vec3 ext = getExtinction();
        return Vec3{
            ext.x > 0.0f ? scattering.x / ext.x : 0.0f,
            ext.y > 0.0f ? scattering.y / ext.y : 0.0f,
            ext.z > 0.0f ? scattering.z / ext.z : 0.0f
        };
    }
    
    /**
     * @brief Create smoke preset
     */
    [[nodiscard]] static ParticipatingMedia createSmoke() noexcept {
        ParticipatingMedia media;
        media.name = "Smoke";
        media.scattering = Vec3{0.3f, 0.3f, 0.3f};
        media.absorption = Vec3{0.05f, 0.05f, 0.05f};
        media.phaseG = 0.3f;
        media.noiseIntensity = 0.7f;
        return media;
    }
    
    /**
     * @brief Create dust preset
     */
    [[nodiscard]] static ParticipatingMedia createDust() noexcept {
        ParticipatingMedia media;
        media.name = "Dust";
        media.scattering = Vec3{0.2f, 0.18f, 0.15f};
        media.absorption = Vec3{0.01f, 0.01f, 0.01f};
        media.phaseG = 0.6f;
        media.noiseScale = 0.5f;
        return media;
    }
    
    /**
     * @brief Create steam preset
     */
    [[nodiscard]] static ParticipatingMedia createSteam() noexcept {
        ParticipatingMedia media;
        media.name = "Steam";
        media.scattering = Vec3{0.9f, 0.9f, 0.9f};
        media.absorption = Vec3{0.001f, 0.001f, 0.001f};
        media.phaseG = 0.1f;
        media.density = 0.5f;
        return media;
    }
};

/**
 * @brief Volumetric rendering statistics
 */
struct VolumetricStats {
    /// Fog computation time (ms)
    f32 fogTimeMs = 0.0f;
    
    /// Cloud computation time (ms)
    f32 cloudTimeMs = 0.0f;
    
    /// Light shaft time (ms)
    f32 lightShaftTimeMs = 0.0f;
    
    /// Total volumetric time (ms)
    f32 totalTimeMs = 0.0f;
    
    /// Active fog volumes
    u32 activeFogVolumes = 0;
    
    /// Active media volumes
    u32 activeMediaVolumes = 0;
    
    /// Froxels computed
    u32 froxelsComputed = 0;
    
    /// Rays marched (for clouds)
    u64 raysMarched = 0;
    
    /**
     * @brief Reset per-frame stats
     */
    void resetFrameStats() noexcept {
        fogTimeMs = 0.0f;
        cloudTimeMs = 0.0f;
        lightShaftTimeMs = 0.0f;
        totalTimeMs = 0.0f;
        froxelsComputed = 0;
        raysMarched = 0;
    }
    
    /**
     * @brief Calculate total time
     */
    void calculateTotal() noexcept {
        totalTimeMs = fogTimeMs + cloudTimeMs + lightShaftTimeMs;
    }
};

// =============================================================================
// Volumetric Effects Manager
// =============================================================================

/**
 * @brief Volumetric effects manager
 * 
 * Manages fog, clouds, light shafts, and participating media.
 */
class VolumetricManager {
public:
    /**
     * @brief Default constructor
     */
    VolumetricManager() = default;
    
    // -------------------------------------------------------------------------
    // Global Fog Configuration
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set global fog settings
     */
    void setFogSettings(const FogSettings& settings) noexcept {
        m_fogSettings = settings;
    }
    
    /**
     * @brief Get global fog settings
     */
    [[nodiscard]] const FogSettings& getFogSettings() const noexcept {
        return m_fogSettings;
    }
    
    /**
     * @brief Set froxel grid config
     */
    void setFroxelConfig(const FroxelGridConfig& config) noexcept {
        m_froxelConfig = config;
    }
    
    /**
     * @brief Get froxel grid config
     */
    [[nodiscard]] const FroxelGridConfig& getFroxelConfig() const noexcept {
        return m_froxelConfig;
    }
    
    // -------------------------------------------------------------------------
    // Fog Volume Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Add fog volume
     */
    u32 addFogVolume(const FogVolume& volume) noexcept {
        if (m_fogVolumes.size() >= VolumetricConfig::MAX_FOG_VOLUMES) {
            return 0;
        }
        
        u32 id = m_nextFogVolumeId++;
        m_fogVolumes[id] = volume;
        return id;
    }
    
    /**
     * @brief Remove fog volume
     */
    void removeFogVolume(u32 id) noexcept {
        m_fogVolumes.erase(id);
    }
    
    /**
     * @brief Get fog volume
     */
    [[nodiscard]] FogVolume* getFogVolume(u32 id) noexcept {
        auto it = m_fogVolumes.find(id);
        return it != m_fogVolumes.end() ? &it->second : nullptr;
    }
    
    /**
     * @brief Get all fog volumes
     */
    [[nodiscard]] const std::unordered_map<u32, FogVolume>& getFogVolumes() const noexcept {
        return m_fogVolumes;
    }
    
    /**
     * @brief Sample fog at point (combines global and local)
     */
    [[nodiscard]] Vec4 sampleFogAtPoint(const Vec3& point, f32 distance) const noexcept {
        Vec4 result{0.0f, 0.0f, 0.0f, 0.0f};
        
        // Global fog
        f32 globalFogFactor = 0.0f;
        if (m_fogSettings.mode == FogMode::Linear) {
            globalFogFactor = m_fogSettings.calculateLinearFog(distance);
        } else if (m_fogSettings.mode == FogMode::Exponential || 
                   m_fogSettings.mode == FogMode::ExponentialSquared) {
            globalFogFactor = m_fogSettings.calculateExponentialFog(distance);
        } else if (m_fogSettings.mode == FogMode::Height) {
            globalFogFactor = m_fogSettings.calculateHeightFog(point.y);
        }
        
        if (globalFogFactor > 0.0f) {
            result = Vec4{
                m_fogSettings.color.x * globalFogFactor,
                m_fogSettings.color.y * globalFogFactor,
                m_fogSettings.color.z * globalFogFactor,
                globalFogFactor
            };
        }
        
        // Local fog volumes
        for (const auto& [id, volume] : m_fogVolumes) {
            f32 localDensity = volume.getDensityAtPoint(point);
            if (localDensity > 0.0f) {
                // Blend with priority (simple max for now)
                f32 localFactor = localDensity * globalFogFactor;
                result.x = std::max(result.x, volume.color.x * localFactor);
                result.y = std::max(result.y, volume.color.y * localFactor);
                result.z = std::max(result.z, volume.color.z * localFactor);
                result.w = std::max(result.w, localFactor);
            }
        }
        
        return result;
    }
    
    // -------------------------------------------------------------------------
    // Cloud Configuration
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set cloud settings
     */
    void setCloudSettings(const CloudSettings& settings) noexcept {
        m_cloudSettings = settings;
    }
    
    /**
     * @brief Get cloud settings
     */
    [[nodiscard]] const CloudSettings& getCloudSettings() const noexcept {
        return m_cloudSettings;
    }
    
    // -------------------------------------------------------------------------
    // Light Shaft Configuration
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set light shaft settings
     */
    void setLightShaftSettings(const LightShaftSettings& settings) noexcept {
        m_lightShaftSettings = settings;
    }
    
    /**
     * @brief Get light shaft settings
     */
    [[nodiscard]] const LightShaftSettings& getLightShaftSettings() const noexcept {
        return m_lightShaftSettings;
    }
    
    // -------------------------------------------------------------------------
    // Participating Media Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Add participating media
     */
    u32 addMedia(const ParticipatingMedia& media) noexcept {
        u32 id = m_nextMediaId++;
        m_mediaVolumes[id] = media;
        return id;
    }
    
    /**
     * @brief Remove participating media
     */
    void removeMedia(u32 id) noexcept {
        m_mediaVolumes.erase(id);
    }
    
    /**
     * @brief Get participating media
     */
    [[nodiscard]] ParticipatingMedia* getMedia(u32 id) noexcept {
        auto it = m_mediaVolumes.find(id);
        return it != m_mediaVolumes.end() ? &it->second : nullptr;
    }
    
    /**
     * @brief Get all media volumes
     */
    [[nodiscard]] const std::unordered_map<u32, ParticipatingMedia>& getMediaVolumes() const noexcept {
        return m_mediaVolumes;
    }
    
    // -------------------------------------------------------------------------
    // Frame Operations
    // -------------------------------------------------------------------------
    
    /**
     * @brief Begin volumetric frame
     */
    void beginFrame(u32 frameNumber) noexcept {
        m_currentFrame = frameNumber;
        m_stats.resetFrameStats();
        
        // Count active volumes
        m_stats.activeFogVolumes = 0;
        for (const auto& [id, vol] : m_fogVolumes) {
            if (vol.enabled) m_stats.activeFogVolumes++;
        }
        
        m_stats.activeMediaVolumes = 0;
        for (const auto& [id, media] : m_mediaVolumes) {
            if (media.enabled) m_stats.activeMediaVolumes++;
        }
    }
    
    /**
     * @brief End volumetric frame
     */
    void endFrame() noexcept {
        m_stats.calculateTotal();
    }
    
    // -------------------------------------------------------------------------
    // Statistics
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get statistics
     */
    [[nodiscard]] const VolumetricStats& getStats() const noexcept {
        return m_stats;
    }
    
    /**
     * @brief Get current frame
     */
    [[nodiscard]] u32 getCurrentFrame() const noexcept {
        return m_currentFrame;
    }

private:
    // Global settings
    FogSettings m_fogSettings;
    CloudSettings m_cloudSettings;
    LightShaftSettings m_lightShaftSettings;
    FroxelGridConfig m_froxelConfig;
    
    // Fog volumes
    std::unordered_map<u32, FogVolume> m_fogVolumes;
    u32 m_nextFogVolumeId = 1;
    
    // Participating media
    std::unordered_map<u32, ParticipatingMedia> m_mediaVolumes;
    u32 m_nextMediaId = 1;
    
    // Frame state
    u32 m_currentFrame = 0;
    
    // Statistics
    VolumetricStats m_stats;
};

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Calculate Henyey-Greenstein phase function
 * @param cosTheta Cosine of angle between view and light
 * @param g Anisotropy parameter (-1 to 1)
 * @return Phase function value
 */
[[nodiscard]] inline f32 henyeyGreenstein(f32 cosTheta, f32 g) noexcept {
    f32 g2 = g * g;
    f32 denom = 1.0f + g2 - 2.0f * g * cosTheta;
    return (1.0f - g2) / (4.0f * 3.14159265359f * std::pow(denom, 1.5f));
}

/**
 * @brief Calculate Schlick phase function (approximation of HG)
 * @param cosTheta Cosine of angle between view and light
 * @param k Anisotropy parameter
 * @return Phase function value
 */
[[nodiscard]] inline f32 schlickPhase(f32 cosTheta, f32 k) noexcept {
    f32 denom = 1.0f + k * cosTheta;
    return (1.0f - k * k) / (4.0f * 3.14159265359f * denom * denom);
}

/**
 * @brief Calculate Beer-Lambert transmittance
 * @param extinction Extinction coefficient
 * @param distance Distance through medium
 * @return Transmittance (0-1)
 */
[[nodiscard]] inline f32 beerLambert(f32 extinction, f32 distance) noexcept {
    return std::exp(-extinction * distance);
}

/**
 * @brief Calculate Beer-Lambert transmittance (RGB)
 */
[[nodiscard]] inline Vec3 beerLambertRGB(const Vec3& extinction, f32 distance) noexcept {
    return Vec3{
        std::exp(-extinction.x * distance),
        std::exp(-extinction.y * distance),
        std::exp(-extinction.z * distance)
    };
}

} // namespace nova
