/**
 * @file lighting.hpp
 * @brief NovaCore Engine™ - Advanced Lighting System
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Production-grade lighting system supporting:
 * - Directional lights (sun, moon)
 * - Point lights with attenuation
 * - Spot lights with cone control
 * - Area lights (sphere, rectangle)
 * - Environment probes for IBL
 * - Clustered forward+ rendering
 * - Shadow mapping for all light types
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>

#include <string>
#include <vector>
#include <array>

namespace nova::render {

// ============================================================================
// Light Configuration Constants
// ============================================================================

/**
 * @brief Lighting system configuration constants
 */
struct LightingConfig {
    /// Maximum number of directional lights
    static constexpr u32 MAX_DIRECTIONAL_LIGHTS = 4;
    
    /// Maximum number of point lights (total in scene)
    static constexpr u32 MAX_POINT_LIGHTS = 4096;
    
    /// Maximum number of spot lights (total in scene)
    static constexpr u32 MAX_SPOT_LIGHTS = 2048;
    
    /// Maximum number of area lights (total in scene)
    static constexpr u32 MAX_AREA_LIGHTS = 512;
    
    /// Maximum number of environment probes
    static constexpr u32 MAX_ENVIRONMENT_PROBES = 128;
    
    /// Maximum lights per cluster in clustered rendering
    static constexpr u32 MAX_LIGHTS_PER_CLUSTER = 256;
    
    /// Cluster grid dimensions (X)
    static constexpr u32 CLUSTER_GRID_X = 16;
    
    /// Cluster grid dimensions (Y)
    static constexpr u32 CLUSTER_GRID_Y = 9;
    
    /// Cluster grid dimensions (Z - depth slices)
    static constexpr u32 CLUSTER_GRID_Z = 24;
    
    /// Total number of clusters
    static constexpr u32 TOTAL_CLUSTERS = CLUSTER_GRID_X * CLUSTER_GRID_Y * CLUSTER_GRID_Z;
    
    /// Shadow map resolution for directional lights
    static constexpr u32 DIRECTIONAL_SHADOW_RESOLUTION = 4096;
    
    /// Shadow map resolution for point lights (cubemap face)
    static constexpr u32 POINT_SHADOW_RESOLUTION = 1024;
    
    /// Shadow map resolution for spot lights
    static constexpr u32 SPOT_SHADOW_RESOLUTION = 2048;
    
    /// Number of cascades for directional shadow mapping
    static constexpr u32 SHADOW_CASCADE_COUNT = 4;
    
    /// PCF (Percentage Closer Filtering) kernel size
    static constexpr u32 PCF_KERNEL_SIZE = 5;
    
    /// Minimum light intensity to be considered active
    static constexpr f32 MIN_LIGHT_INTENSITY = 0.001f;
    
    /// Default ambient light intensity
    static constexpr f32 DEFAULT_AMBIENT_INTENSITY = 0.03f;
};

// ============================================================================
// Light Types
// ============================================================================

/**
 * @brief Light type enumeration
 */
enum class LightType : u8 {
    Directional = 0,    ///< Infinite distance light (sun, moon)
    Point = 1,          ///< Omnidirectional point light
    Spot = 2,           ///< Cone-shaped spotlight
    AreaSphere = 3,     ///< Spherical area light
    AreaRect = 4,       ///< Rectangular area light
    AreaDisc = 5,       ///< Disc-shaped area light
    AreaTube = 6        ///< Tube/line area light
};

/**
 * @brief Get light type name as string
 */
[[nodiscard]] constexpr const char* getLightTypeName(LightType type) noexcept {
    switch (type) {
        case LightType::Directional: return "Directional";
        case LightType::Point: return "Point";
        case LightType::Spot: return "Spot";
        case LightType::AreaSphere: return "AreaSphere";
        case LightType::AreaRect: return "AreaRect";
        case LightType::AreaDisc: return "AreaDisc";
        case LightType::AreaTube: return "AreaTube";
        default: return "Unknown";
    }
}

/**
 * @brief Shadow quality preset
 */
enum class ShadowQuality : u8 {
    Off = 0,            ///< No shadows
    Low = 1,            ///< Basic hard shadows
    Medium = 2,         ///< PCF soft shadows
    High = 3,           ///< PCSS (Percentage Closer Soft Shadows)
    Ultra = 4           ///< Ray-traced shadows (if available)
};

/**
 * @brief Get shadow quality name as string
 */
[[nodiscard]] constexpr const char* getShadowQualityName(ShadowQuality quality) noexcept {
    switch (quality) {
        case ShadowQuality::Off: return "Off";
        case ShadowQuality::Low: return "Low";
        case ShadowQuality::Medium: return "Medium";
        case ShadowQuality::High: return "High";
        case ShadowQuality::Ultra: return "Ultra";
        default: return "Unknown";
    }
}

/**
 * @brief Light attenuation model
 */
enum class AttenuationModel : u8 {
    None = 0,           ///< No attenuation (directional lights)
    Linear = 1,         ///< Linear falloff: 1 / distance
    Quadratic = 2,      ///< Physically correct: 1 / distance²
    InverseSquare = 3,  ///< Same as quadratic with constant term
    Exponential = 4,    ///< Exponential falloff for artistic control
    Custom = 5          ///< User-defined attenuation curve
};

// ============================================================================
// Light Data Structures
// ============================================================================

/**
 * @brief Common light properties shared by all light types
 */
struct LightBase {
    /// Light color (linear RGB, not sRGB)
    Vec3 color{1.0f, 1.0f, 1.0f};
    
    /// Light intensity in lumens (for point/spot) or lux (for directional)
    f32 intensity = 1000.0f;
    
    /// Whether the light is enabled
    bool enabled = true;
    
    /// Whether the light casts shadows
    bool castShadows = true;
    
    /// Shadow bias to prevent shadow acne
    f32 shadowBias = 0.0005f;
    
    /// Normal offset bias for shadow sampling
    f32 shadowNormalBias = 0.02f;
    
    /// Shadow softness (for PCSS)
    f32 shadowSoftness = 1.0f;
    
    /// Shadow fade distance (0 = no fade)
    f32 shadowFadeDistance = 0.0f;
    
    /// Light affects specular highlights
    bool affectsSpecular = true;
    
    /// Light contributes to diffuse illumination
    bool affectsDiffuse = true;
    
    /// Volumetric light contribution (for fog/atmosphere)
    f32 volumetricIntensity = 1.0f;
    
    /// Temperature in Kelvin (0 = use color directly)
    f32 temperature = 0.0f;
    
    /// Layer mask for selective lighting
    u32 layerMask = 0xFFFFFFFF;
};

/**
 * @brief Directional light (sun, moon, infinite distance lights)
 */
struct DirectionalLight : LightBase {
    /// Light direction (normalized, points toward light source)
    Vec3 direction{0.0f, -1.0f, 0.0f};
    
    /// Angular diameter for soft shadows (in radians, sun ≈ 0.0093)
    f32 angularDiameter = 0.0093f;
    
    /// Cascade split distances (normalized, 0-1)
    std::array<f32, LightingConfig::SHADOW_CASCADE_COUNT> cascadeSplits{0.05f, 0.15f, 0.4f, 1.0f};
    
    /// Cascade blend region size (0-1)
    f32 cascadeBlendRegion = 0.1f;
    
    /// Shadow distance (how far shadows are rendered)
    f32 shadowDistance = 200.0f;
    
    /// Shadow depth bias per cascade (larger for farther cascades)
    std::array<f32, LightingConfig::SHADOW_CASCADE_COUNT> cascadeBias{0.0005f, 0.001f, 0.002f, 0.004f};
    
    /**
     * @brief Convert color temperature to RGB
     * @param kelvin Temperature in Kelvin (1000-40000)
     * @return RGB color (linear)
     */
    [[nodiscard]] static Vec3 temperatureToRGB(f32 kelvin) {
        // Approximate blackbody radiation using Planckian locus
        kelvin = nova::math::clamp(kelvin, 1000.0f, 40000.0f);
        
        f32 temp = kelvin / 100.0f;
        f32 r, g, b;
        
        // Red
        if (temp <= 66.0f) {
            r = 255.0f;
        } else {
            r = 329.698727446f * std::pow(temp - 60.0f, -0.1332047592f);
        }
        
        // Green
        if (temp <= 66.0f) {
            g = 99.4708025861f * std::log(temp) - 161.1195681661f;
        } else {
            g = 288.1221695283f * std::pow(temp - 60.0f, -0.0755148492f);
        }
        
        // Blue
        if (temp >= 66.0f) {
            b = 255.0f;
        } else if (temp <= 19.0f) {
            b = 0.0f;
        } else {
            b = 138.5177312231f * std::log(temp - 10.0f) - 305.0447927307f;
        }
        
        return Vec3{
            nova::math::clamp(r / 255.0f, 0.0f, 1.0f),
            nova::math::clamp(g / 255.0f, 0.0f, 1.0f),
            nova::math::clamp(b / 255.0f, 0.0f, 1.0f)
        };
    }
    
    /**
     * @brief Get the effective color (temperature or direct)
     */
    [[nodiscard]] Vec3 getEffectiveColor() const {
        if (temperature > 0.0f) {
            return temperatureToRGB(temperature) * color;
        }
        return color;
    }
};

/**
 * @brief Point light (omnidirectional local light)
 */
struct PointLight : LightBase {
    /// Light position in world space
    Vec3 position{0.0f, 0.0f, 0.0f};
    
    /// Light radius (affects attenuation and shadow near plane)
    f32 radius = 0.1f;
    
    /// Maximum range (lights beyond this distance are culled)
    f32 range = 10.0f;
    
    /// Attenuation model
    AttenuationModel attenuation = AttenuationModel::Quadratic;
    
    /// IES profile index (-1 = no IES profile)
    i32 iesProfileIndex = -1;
    
    /**
     * @brief Calculate attenuation at a given distance
     * @param distance Distance from light center
     * @return Attenuation factor (0-1)
     */
    [[nodiscard]] f32 calculateAttenuation(f32 distance) const {
        if (distance >= range) return 0.0f;
        
        // Windowed attenuation to ensure smooth falloff at range
        f32 windowNumerator = square(saturate(1.0f - square(square(distance / range))));
        
        switch (attenuation) {
            case AttenuationModel::None:
                return 1.0f;
            case AttenuationModel::Linear:
                return windowNumerator / std::max(distance, 0.001f);
            case AttenuationModel::Quadratic:
            case AttenuationModel::InverseSquare:
                return windowNumerator / std::max(distance * distance, 0.0001f);
            case AttenuationModel::Exponential:
                return windowNumerator * std::exp(-distance / (range * 0.25f));
            default:
                return windowNumerator / std::max(distance * distance, 0.0001f);
        }
    }

private:
    [[nodiscard]] static f32 square(f32 x) noexcept { return x * x; }
    [[nodiscard]] static f32 saturate(f32 x) noexcept { return nova::math::clamp(x, 0.0f, 1.0f); }
};

/**
 * @brief Spot light (cone-shaped directional local light)
 */
struct SpotLight : LightBase {
    /// Light position in world space
    Vec3 position{0.0f, 0.0f, 0.0f};
    
    /// Light direction (normalized)
    Vec3 direction{0.0f, -1.0f, 0.0f};
    
    /// Light radius (affects shadows)
    f32 radius = 0.1f;
    
    /// Maximum range
    f32 range = 10.0f;
    
    /// Inner cone angle in radians (full intensity)
    f32 innerConeAngle = math::radians(25.0f);
    
    /// Outer cone angle in radians (falloff to zero)
    f32 outerConeAngle = math::radians(35.0f);
    
    /// Attenuation model
    AttenuationModel attenuation = AttenuationModel::Quadratic;
    
    /// IES profile index (-1 = no IES profile)
    i32 iesProfileIndex = -1;
    
    /**
     * @brief Calculate spotlight cone attenuation
     * @param lightToFragment Direction from light to fragment (normalized)
     * @return Cone attenuation factor (0-1)
     */
    [[nodiscard]] f32 calculateConeAttenuation(const Vec3& lightToFragment) const {
        // lightToFragment points from light toward fragment
        // direction is where light is pointing
        // When fragment is in light direction, dot product is positive (1 when aligned)
        f32 cosAngle = lightToFragment.dot(direction);
        f32 cosInner = std::cos(innerConeAngle);
        f32 cosOuter = std::cos(outerConeAngle);
        
        return saturate((cosAngle - cosOuter) / (cosInner - cosOuter));
    }
    
    /**
     * @brief Calculate total attenuation at a point
     * @param fragmentPosition Position to evaluate
     * @return Total attenuation factor (0-1)
     */
    [[nodiscard]] f32 calculateTotalAttenuation(const Vec3& fragmentPosition) const {
        Vec3 toFragment = fragmentPosition - position;
        f32 distance = toFragment.length();
        
        if (distance >= range) return 0.0f;
        
        Vec3 lightToFragment = toFragment / distance;
        f32 distanceAtten = calculateDistanceAttenuation(distance);
        f32 coneAtten = calculateConeAttenuation(lightToFragment);
        
        return distanceAtten * coneAtten;
    }

private:
    [[nodiscard]] f32 calculateDistanceAttenuation(f32 distance) const {
        f32 windowNumerator = square(saturate(1.0f - square(square(distance / range))));
        
        switch (attenuation) {
            case AttenuationModel::None:
                return 1.0f;
            case AttenuationModel::Linear:
                return windowNumerator / std::max(distance, 0.001f);
            case AttenuationModel::Quadratic:
            case AttenuationModel::InverseSquare:
                return windowNumerator / std::max(distance * distance, 0.0001f);
            default:
                return windowNumerator / std::max(distance * distance, 0.0001f);
        }
    }
    
    [[nodiscard]] static f32 square(f32 x) noexcept { return x * x; }
    [[nodiscard]] static f32 saturate(f32 x) noexcept { return nova::math::clamp(x, 0.0f, 1.0f); }
};

/**
 * @brief Area light (rectangle, sphere, disc, or tube)
 */
struct AreaLight : LightBase {
    /// Light type (AreaSphere, AreaRect, AreaDisc, AreaTube)
    LightType type = LightType::AreaRect;
    
    /// Light position in world space
    Vec3 position{0.0f, 0.0f, 0.0f};
    
    /// Light direction/normal (for rect/disc)
    Vec3 direction{0.0f, -1.0f, 0.0f};
    
    /// Right vector (for rect/tube)
    Vec3 right{1.0f, 0.0f, 0.0f};
    
    /// Light dimensions
    /// For rect: width, height (Z unused)
    /// For sphere/disc: radius (Y, Z unused)
    /// For tube: length, radius (Z unused)
    Vec3 dimensions{1.0f, 1.0f, 0.0f};
    
    /// Maximum range
    f32 range = 20.0f;
    
    /// Two-sided emission (for rect/disc)
    bool twoSided = false;
    
    /**
     * @brief Get the area of the light source
     * @return Area in square units
     */
    [[nodiscard]] f32 getArea() const {
        switch (type) {
            case LightType::AreaSphere:
                return 4.0f * math::PI_F32 * dimensions.x * dimensions.x;
            case LightType::AreaRect:
                return dimensions.x * dimensions.y;
            case LightType::AreaDisc:
                return math::PI_F32 * dimensions.x * dimensions.x;
            case LightType::AreaTube:
                return 2.0f * math::PI_F32 * dimensions.y * dimensions.x; // length * 2πr
            default:
                return 1.0f;
        }
    }
    
    /**
     * @brief Get luminous flux (lumens) from luminance
     * @return Total luminous flux
     */
    [[nodiscard]] f32 getLuminousFlux() const {
        // intensity is luminance (cd/m²), flux = luminance * area * π (for Lambertian)
        return intensity * getArea() * math::PI_F32;
    }
};

// ============================================================================
// Environment Lighting
// ============================================================================

/**
 * @brief Environment probe for image-based lighting (IBL)
 */
struct EnvironmentProbe {
    /// Probe position in world space
    Vec3 position{0.0f, 0.0f, 0.0f};
    
    /// Probe influence radius
    f32 radius = 10.0f;
    
    /// Blend distance for smooth transitions
    f32 blendDistance = 2.0f;
    
    /// Probe box extents (for box projection)
    Vec3 boxExtents{10.0f, 10.0f, 10.0f};
    
    /// Whether to use box projection
    bool useBoxProjection = false;
    
    /// Whether the probe is enabled
    bool enabled = true;
    
    /// Probe priority (higher = more important)
    i32 priority = 0;
    
    /// Cubemap texture handle
    Handle<void> cubemapHandle;
    
    /// Irradiance map handle (for diffuse IBL)
    Handle<void> irradianceHandle;
    
    /// Pre-filtered map handle (for specular IBL)
    Handle<void> prefilteredHandle;
    
    /// BRDF integration LUT handle
    Handle<void> brdfLutHandle;
    
    /**
     * @brief Calculate influence weight at a position
     * @param worldPosition Position to evaluate
     * @return Influence weight (0-1)
     */
    [[nodiscard]] f32 calculateInfluence(const Vec3& worldPosition) const {
        f32 distance = (worldPosition - position).length();
        
        if (distance >= radius) return 0.0f;
        if (distance <= radius - blendDistance) return 1.0f;
        
        return (radius - distance) / blendDistance;
    }
};

/**
 * @brief Global ambient lighting settings
 */
struct AmbientLight {
    /// Sky color (from above)
    Vec3 skyColor{0.5f, 0.7f, 1.0f};
    
    /// Ground color (from below)
    Vec3 groundColor{0.1f, 0.08f, 0.05f};
    
    /// Equator color (from sides)
    Vec3 equatorColor{0.3f, 0.3f, 0.35f};
    
    /// Overall ambient intensity
    f32 intensity = LightingConfig::DEFAULT_AMBIENT_INTENSITY;
    
    /// Ambient occlusion influence (0-1)
    f32 aoInfluence = 1.0f;
    
    /// Use environment map for ambient
    bool useEnvironmentMap = false;
    
    /// Environment map rotation (radians)
    f32 environmentRotation = 0.0f;
    
    /// Environment map handle
    Handle<void> environmentMap;
    
    /**
     * @brief Sample ambient color for a normal direction
     * @param normal Surface normal (world space)
     * @return Ambient color contribution
     */
    [[nodiscard]] Vec3 sampleAmbient(const Vec3& normal) const {
        // Trilinear blend between sky, equator, and ground
        f32 skyWeight = saturate(normal.y);
        f32 groundWeight = saturate(-normal.y);
        f32 equatorWeight = 1.0f - skyWeight - groundWeight;
        
        return (skyColor * skyWeight + equatorColor * equatorWeight + groundColor * groundWeight) * intensity;
    }

private:
    [[nodiscard]] static f32 saturate(f32 x) noexcept { return nova::math::clamp(x, 0.0f, 1.0f); }
};

// ============================================================================
// Light Cluster Data (for clustered forward+)
// ============================================================================

/**
 * @brief Light cluster for clustered forward+ rendering
 */
struct LightCluster {
    /// Number of point lights in this cluster
    u16 pointLightCount = 0;
    
    /// Number of spot lights in this cluster
    u16 spotLightCount = 0;
    
    /// Number of area lights in this cluster
    u16 areaLightCount = 0;
    
    /// Number of environment probes in this cluster
    u16 probeCount = 0;
    
    /// Offset into the global light index buffer
    u32 lightIndexOffset = 0;
    
    /// Cluster AABB minimum (view space)
    Vec3 aabbMin;
    
    /// Cluster AABB maximum (view space)
    Vec3 aabbMax;
};

/**
 * @brief GPU-friendly light data for shader consumption
 */
struct GPULightData {
    // First 4 floats: position + type
    Vec3 position;
    f32 type;  // Encoded as float for alignment
    
    // Second 4 floats: direction + range
    Vec3 direction;
    f32 range;
    
    // Third 4 floats: color + intensity
    Vec3 color;
    f32 intensity;
    
    // Fourth 4 floats: attenuation params
    f32 innerConeAngle;
    f32 outerConeAngle;
    f32 radius;
    f32 shadowIndex;  // -1 if no shadow, otherwise shadow map index
    
    /**
     * @brief Create GPU data from a point light
     */
    [[nodiscard]] static GPULightData fromPointLight(const PointLight& light, i32 shadowIdx = -1) {
        GPULightData data{};
        data.position = light.position;
        data.type = static_cast<f32>(LightType::Point);
        data.direction = Vec3{0.0f, -1.0f, 0.0f};
        data.range = light.range;
        data.color = light.color;
        data.intensity = light.intensity;
        data.innerConeAngle = 0.0f;
        data.outerConeAngle = 0.0f;
        data.radius = light.radius;
        data.shadowIndex = static_cast<f32>(shadowIdx);
        return data;
    }
    
    /**
     * @brief Create GPU data from a spot light
     */
    [[nodiscard]] static GPULightData fromSpotLight(const SpotLight& light, i32 shadowIdx = -1) {
        GPULightData data{};
        data.position = light.position;
        data.type = static_cast<f32>(LightType::Spot);
        data.direction = light.direction;
        data.range = light.range;
        data.color = light.color;
        data.intensity = light.intensity;
        data.innerConeAngle = light.innerConeAngle;
        data.outerConeAngle = light.outerConeAngle;
        data.radius = light.radius;
        data.shadowIndex = static_cast<f32>(shadowIdx);
        return data;
    }
};

/**
 * @brief GPU-friendly directional light data
 */
struct GPUDirectionalLight {
    Vec3 direction;
    f32 intensity;
    Vec3 color;
    f32 shadowDistance;
    
    // Cascade view-projection matrices
    Mat4 cascadeViewProj[LightingConfig::SHADOW_CASCADE_COUNT];
    
    // Cascade split depths
    f32 cascadeSplits[LightingConfig::SHADOW_CASCADE_COUNT];
    
    // Per-cascade bias
    f32 cascadeBias[LightingConfig::SHADOW_CASCADE_COUNT];
    
    /**
     * @brief Create GPU data from directional light
     */
    [[nodiscard]] static GPUDirectionalLight fromDirectionalLight(const DirectionalLight& light) {
        GPUDirectionalLight data{};
        data.direction = light.direction;
        data.intensity = light.intensity;
        data.color = light.getEffectiveColor();
        data.shadowDistance = light.shadowDistance;
        for (u32 i = 0; i < LightingConfig::SHADOW_CASCADE_COUNT; ++i) {
            data.cascadeSplits[i] = light.cascadeSplits[i];
            data.cascadeBias[i] = light.cascadeBias[i];
        }
        return data;
    }
};

// ============================================================================
// Shadow Configuration
// ============================================================================

/**
 * @brief Shadow map configuration
 */
struct ShadowConfig {
    /// Shadow quality preset
    ShadowQuality quality = ShadowQuality::High;
    
    /// Directional light shadow resolution
    u32 directionalResolution = LightingConfig::DIRECTIONAL_SHADOW_RESOLUTION;
    
    /// Point light shadow resolution (per cubemap face)
    u32 pointResolution = LightingConfig::POINT_SHADOW_RESOLUTION;
    
    /// Spot light shadow resolution
    u32 spotResolution = LightingConfig::SPOT_SHADOW_RESOLUTION;
    
    /// Maximum shadow distance
    f32 maxDistance = 200.0f;
    
    /// Shadow fade start distance (as percentage of max)
    f32 fadeStartPercent = 0.8f;
    
    /// Enable contact shadows
    bool enableContactShadows = true;
    
    /// Contact shadow ray length
    f32 contactShadowLength = 0.5f;
    
    /// Enable screen-space shadows
    bool enableScreenSpaceShadows = false;
    
    /// PCF filter size
    u32 pcfFilterSize = LightingConfig::PCF_KERNEL_SIZE;
    
    /// PCSS blocker search samples
    u32 pcssBlockerSamples = 16;
    
    /// PCSS PCF samples
    u32 pcssPcfSamples = 32;
};

/**
 * @brief Shadow cascade data for a directional light
 */
struct ShadowCascade {
    /// View-projection matrix for this cascade
    Mat4 viewProjection;
    
    /// Cascade near plane
    f32 nearPlane = 0.0f;
    
    /// Cascade far plane
    f32 farPlane = 1.0f;
    
    /// Cascade world space radius
    f32 radius = 10.0f;
    
    /// Texel size for stable cascades
    f32 texelSize = 1.0f;
};

// ============================================================================
// Lighting System Interface
// ============================================================================

/**
 * @brief Lighting system statistics
 */
struct LightingStats {
    /// Number of active directional lights
    u32 activeDirectionalLights = 0;
    
    /// Number of active point lights
    u32 activePointLights = 0;
    
    /// Number of active spot lights
    u32 activeSpotLights = 0;
    
    /// Number of active area lights
    u32 activeAreaLights = 0;
    
    /// Number of active environment probes
    u32 activeProbes = 0;
    
    /// Number of shadow-casting lights
    u32 shadowCastingLights = 0;
    
    /// Total light culling time (ms)
    f32 cullingTimeMs = 0.0f;
    
    /// Cluster assignment time (ms)
    f32 clusterTimeMs = 0.0f;
    
    /// Shadow rendering time (ms)
    f32 shadowTimeMs = 0.0f;
    
    /// Number of visible lights after culling
    u32 visibleLights = 0;
    
    /// Average lights per cluster
    f32 avgLightsPerCluster = 0.0f;
    
    /// Maximum lights in any cluster
    u32 maxLightsInCluster = 0;
};

} // namespace nova::render
