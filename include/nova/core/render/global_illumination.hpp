/**
 * @file global_illumination.hpp
 * @brief Global Illumination System for NovaCore Engine
 * 
 * Provides comprehensive global illumination solutions:
 * - Screen-Space Global Illumination (SSGI)
 * - Ray-traced Global Illumination (RTGI)
 * - Voxel-based Global Illumination (VXGI)
 * - Light Propagation Volumes (LPV)
 * - Irradiance probes and probe grids
 * - Reflection probes
 * - Spherical Harmonics lighting
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
 * @brief Global illumination system configuration constants
 */
struct GIConfig {
    /// Maximum probe grid dimensions
    static constexpr u32 MAX_PROBE_GRID_DIM = 64;
    
    /// Maximum probes per grid
    static constexpr u32 MAX_PROBES = MAX_PROBE_GRID_DIM * MAX_PROBE_GRID_DIM * MAX_PROBE_GRID_DIM;
    
    /// Spherical harmonics order (L2 = 9 coefficients)
    static constexpr u32 SH_COEFFICIENT_COUNT = 9;
    
    /// Maximum reflection probe resolution
    static constexpr u32 MAX_REFLECTION_PROBE_RES = 1024;
    
    /// Default reflection probe resolution
    static constexpr u32 DEFAULT_REFLECTION_PROBE_RES = 256;
    
    /// Maximum ray bounces for RTGI
    static constexpr u32 MAX_RAY_BOUNCES = 8;
    
    /// Default ray bounces
    static constexpr u32 DEFAULT_RAY_BOUNCES = 2;
    
    /// SSGI maximum steps
    static constexpr u32 SSGI_MAX_STEPS = 32;
    
    /// Voxel grid default resolution
    static constexpr u32 VOXEL_GRID_DEFAULT_RES = 128;
    
    /// LPV cascade count
    static constexpr u32 LPV_CASCADE_COUNT = 3;
    
    /// Irradiance probe octahedral map size
    static constexpr u32 IRRADIANCE_PROBE_SIZE = 8;
    
    /// Distance probe octahedral map size
    static constexpr u32 DISTANCE_PROBE_SIZE = 16;
};

// =============================================================================
// Enumerations
// =============================================================================

/**
 * @brief Global illumination technique
 */
enum class GITechnique : u32 {
    /// GI disabled
    None = 0,
    
    /// Screen-Space GI (fast, limited)
    SSGI = 1,
    
    /// Light Propagation Volumes
    LPV = 2,
    
    /// Voxel-based GI (VXGI)
    VXGI = 3,
    
    /// Ray-traced GI (hardware RT required)
    RTGI = 4,
    
    /// Irradiance probe grid (DDGI-like)
    ProbeGrid = 5,
    
    /// Combined SSGI + Probes
    Hybrid = 6,
    
    COUNT
};

/**
 * @brief Get GI technique name
 */
[[nodiscard]] constexpr const char* getGITechniqueName(GITechnique technique) noexcept {
    switch (technique) {
        case GITechnique::None: return "None";
        case GITechnique::SSGI: return "SSGI";
        case GITechnique::LPV: return "LPV";
        case GITechnique::VXGI: return "VXGI";
        case GITechnique::RTGI: return "RTGI";
        case GITechnique::ProbeGrid: return "Probe Grid";
        case GITechnique::Hybrid: return "Hybrid";
        default: return "Unknown";
    }
}

/**
 * @brief GI quality level
 */
enum class GIQuality : u32 {
    /// Lowest quality (mobile)
    Low = 0,
    
    /// Medium quality
    Medium = 1,
    
    /// High quality
    High = 2,
    
    /// Ultra quality (desktop)
    Ultra = 3,
    
    COUNT
};

/**
 * @brief Get GI quality name
 */
[[nodiscard]] constexpr const char* getGIQualityName(GIQuality quality) noexcept {
    switch (quality) {
        case GIQuality::Low: return "Low";
        case GIQuality::Medium: return "Medium";
        case GIQuality::High: return "High";
        case GIQuality::Ultra: return "Ultra";
        default: return "Unknown";
    }
}

/**
 * @brief Probe state
 */
enum class ProbeState : u32 {
    /// Probe needs full update
    Invalid = 0,
    
    /// Probe is being updated
    Updating = 1,
    
    /// Probe data is valid
    Valid = 2,
    
    /// Probe is disabled
    Disabled = 3,
    
    COUNT
};

/**
 * @brief Get probe state name
 */
[[nodiscard]] constexpr const char* getProbeStateName(ProbeState state) noexcept {
    switch (state) {
        case ProbeState::Invalid: return "Invalid";
        case ProbeState::Updating: return "Updating";
        case ProbeState::Valid: return "Valid";
        case ProbeState::Disabled: return "Disabled";
        default: return "Unknown";
    }
}

/**
 * @brief Probe update mode
 */
enum class ProbeUpdateMode : u32 {
    /// Update all probes every frame
    EveryFrame = 0,
    
    /// Update one probe per frame (round-robin)
    Incremental = 1,
    
    /// Update based on visibility/importance
    Adaptive = 2,
    
    /// Manual update only
    Manual = 3,
    
    COUNT
};

/**
 * @brief Reflection probe type
 */
enum class ReflectionProbeType : u32 {
    /// Box projection
    Box = 0,
    
    /// Sphere projection
    Sphere = 1,
    
    /// Planar reflection
    Planar = 2,
    
    COUNT
};

// =============================================================================
// Spherical Harmonics
// =============================================================================

/**
 * @brief L2 Spherical Harmonics coefficients (9 coefficients)
 */
struct SphericalHarmonics {
    /// SH coefficients (RGB, 9 each = 27 values)
    std::array<Vec3, GIConfig::SH_COEFFICIENT_COUNT> coefficients = {};
    
    /**
     * @brief Default constructor (black/zero)
     */
    SphericalHarmonics() noexcept {
        for (auto& c : coefficients) {
            c = Vec3{0.0f, 0.0f, 0.0f};
        }
    }
    
    /**
     * @brief Evaluate SH for a direction
     * @param dir Normalized direction
     * @return Irradiance in that direction
     */
    [[nodiscard]] Vec3 evaluate(const Vec3& dir) const noexcept {
        // L0 (constant)
        Vec3 result = coefficients[0] * 0.282095f;
        
        // L1 (linear)
        result = result + coefficients[1] * (0.488603f * dir.y);
        result = result + coefficients[2] * (0.488603f * dir.z);
        result = result + coefficients[3] * (0.488603f * dir.x);
        
        // L2 (quadratic)
        result = result + coefficients[4] * (1.092548f * dir.x * dir.y);
        result = result + coefficients[5] * (1.092548f * dir.y * dir.z);
        result = result + coefficients[6] * (0.315392f * (3.0f * dir.z * dir.z - 1.0f));
        result = result + coefficients[7] * (1.092548f * dir.x * dir.z);
        result = result + coefficients[8] * (0.546274f * (dir.x * dir.x - dir.y * dir.y));
        
        return result;
    }
    
    /**
     * @brief Add a sample to the SH
     * @param dir Normalized direction
     * @param color Sample color/radiance
     * @param weight Sample weight
     */
    void addSample(const Vec3& dir, const Vec3& color, f32 weight = 1.0f) noexcept {
        // L0
        coefficients[0] = coefficients[0] + color * (weight * 0.282095f);
        
        // L1
        coefficients[1] = coefficients[1] + color * (weight * 0.488603f * dir.y);
        coefficients[2] = coefficients[2] + color * (weight * 0.488603f * dir.z);
        coefficients[3] = coefficients[3] + color * (weight * 0.488603f * dir.x);
        
        // L2
        coefficients[4] = coefficients[4] + color * (weight * 1.092548f * dir.x * dir.y);
        coefficients[5] = coefficients[5] + color * (weight * 1.092548f * dir.y * dir.z);
        coefficients[6] = coefficients[6] + color * (weight * 0.315392f * (3.0f * dir.z * dir.z - 1.0f));
        coefficients[7] = coefficients[7] + color * (weight * 1.092548f * dir.x * dir.z);
        coefficients[8] = coefficients[8] + color * (weight * 0.546274f * (dir.x * dir.x - dir.y * dir.y));
    }
    
    /**
     * @brief Scale all coefficients
     */
    void scale(f32 s) noexcept {
        for (auto& c : coefficients) {
            c = c * s;
        }
    }
    
    /**
     * @brief Add another SH
     */
    void add(const SphericalHarmonics& other) noexcept {
        for (usize i = 0; i < coefficients.size(); ++i) {
            coefficients[i] = coefficients[i] + other.coefficients[i];
        }
    }
    
    /**
     * @brief Lerp between two SH
     */
    [[nodiscard]] static SphericalHarmonics lerp(
        const SphericalHarmonics& a, const SphericalHarmonics& b, f32 t) noexcept {
        
        SphericalHarmonics result;
        for (usize i = 0; i < result.coefficients.size(); ++i) {
            result.coefficients[i] = Vec3{
                a.coefficients[i].x * (1.0f - t) + b.coefficients[i].x * t,
                a.coefficients[i].y * (1.0f - t) + b.coefficients[i].y * t,
                a.coefficients[i].z * (1.0f - t) + b.coefficients[i].z * t
            };
        }
        return result;
    }
    
    /**
     * @brief Get dominant light direction
     */
    [[nodiscard]] Vec3 getDominantDirection() const noexcept {
        // L1 band approximates the dominant direction
        Vec3 dir{coefficients[3].x, coefficients[1].x, coefficients[2].x};
        f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (len > 0.0001f) {
            return Vec3{dir.x / len, dir.y / len, dir.z / len};
        }
        return Vec3{0.0f, 1.0f, 0.0f};
    }
};

// =============================================================================
// Irradiance Probe
// =============================================================================

/**
 * @brief Irradiance probe data
 */
struct IrradianceProbe {
    /// World-space position
    Vec3 position = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Probe state
    ProbeState state = ProbeState::Invalid;
    
    /// SH irradiance data
    SphericalHarmonics irradiance;
    
    /// Mean distance (for visibility)
    f32 meanDistance = 0.0f;
    
    /// Variance of distance (for soft shadows)
    f32 distanceVariance = 0.0f;
    
    /// Last frame updated
    u32 lastUpdateFrame = 0;
    
    /// Probe influence radius
    f32 influenceRadius = 10.0f;
    
    /**
     * @brief Check if probe needs update
     */
    [[nodiscard]] bool needsUpdate(u32 currentFrame, u32 maxAge = 60) const noexcept {
        if (state == ProbeState::Invalid || state == ProbeState::Disabled) {
            return state == ProbeState::Invalid;
        }
        return currentFrame - lastUpdateFrame > maxAge;
    }
    
    /**
     * @brief Calculate influence weight at a point
     */
    [[nodiscard]] f32 calculateInfluence(const Vec3& point) const noexcept {
        Vec3 diff = Vec3{point.x - position.x, point.y - position.y, point.z - position.z};
        f32 dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        
        if (dist >= influenceRadius) {
            return 0.0f;
        }
        
        // Smooth falloff
        f32 t = dist / influenceRadius;
        return 1.0f - t * t * (3.0f - 2.0f * t); // smoothstep
    }
};

/**
 * @brief GPU-aligned irradiance probe data
 */
struct alignas(16) GPUIrradianceProbe {
    /// Position + influence radius
    Vec4 positionRadius;
    
    /// SH coefficients (first 4)
    std::array<Vec4, 4> shCoeffs0;
    
    /// SH coefficients (last 5, with padding)
    std::array<Vec4, 5> shCoeffs1;
    
    /// State + padding
    u32 state;
    u32 _pad0;
    u32 _pad1;
    u32 _pad2;
    
    /**
     * @brief Create from IrradianceProbe
     */
    [[nodiscard]] static GPUIrradianceProbe fromProbe(const IrradianceProbe& probe) noexcept {
        GPUIrradianceProbe gpu;
        
        gpu.positionRadius = Vec4{probe.position.x, probe.position.y, 
                                  probe.position.z, probe.influenceRadius};
        
        // Pack SH coefficients
        const auto& sh = probe.irradiance.coefficients;
        gpu.shCoeffs0[0] = Vec4{sh[0].x, sh[0].y, sh[0].z, sh[1].x};
        gpu.shCoeffs0[1] = Vec4{sh[1].y, sh[1].z, sh[2].x, sh[2].y};
        gpu.shCoeffs0[2] = Vec4{sh[2].z, sh[3].x, sh[3].y, sh[3].z};
        gpu.shCoeffs0[3] = Vec4{sh[4].x, sh[4].y, sh[4].z, sh[5].x};
        
        gpu.shCoeffs1[0] = Vec4{sh[5].y, sh[5].z, sh[6].x, sh[6].y};
        gpu.shCoeffs1[1] = Vec4{sh[6].z, sh[7].x, sh[7].y, sh[7].z};
        gpu.shCoeffs1[2] = Vec4{sh[8].x, sh[8].y, sh[8].z, 0.0f};
        gpu.shCoeffs1[3] = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
        gpu.shCoeffs1[4] = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
        
        gpu.state = static_cast<u32>(probe.state);
        gpu._pad0 = 0;
        gpu._pad1 = 0;
        gpu._pad2 = 0;
        
        return gpu;
    }
};

// =============================================================================
// Probe Grid
// =============================================================================

/**
 * @brief Irradiance probe grid configuration
 */
struct ProbeGridConfig {
    /// Grid dimensions (probes per axis)
    u32 gridSizeX = 8;
    u32 gridSizeY = 4;
    u32 gridSizeZ = 8;
    
    /// World-space bounds min
    Vec3 boundsMin = Vec3{-50.0f, 0.0f, -50.0f};
    
    /// World-space bounds max
    Vec3 boundsMax = Vec3{50.0f, 20.0f, 50.0f};
    
    /// Probe update mode
    ProbeUpdateMode updateMode = ProbeUpdateMode::Incremental;
    
    /// Number of rays per probe update
    u32 raysPerProbe = 256;
    
    /// Maximum ray distance
    f32 maxRayDistance = 100.0f;
    
    /// Normal bias for ray origin
    f32 normalBias = 0.2f;
    
    /// View bias for ray origin
    f32 viewBias = 0.2f;
    
    /**
     * @brief Get total probe count
     */
    [[nodiscard]] u32 getTotalProbes() const noexcept {
        return gridSizeX * gridSizeY * gridSizeZ;
    }
    
    /**
     * @brief Get probe spacing
     */
    [[nodiscard]] Vec3 getProbeSpacing() const noexcept {
        return Vec3{
            (boundsMax.x - boundsMin.x) / static_cast<f32>(gridSizeX - 1),
            (boundsMax.y - boundsMin.y) / static_cast<f32>(gridSizeY - 1),
            (boundsMax.z - boundsMin.z) / static_cast<f32>(gridSizeZ - 1)
        };
    }
    
    /**
     * @brief Get probe position by index
     */
    [[nodiscard]] Vec3 getProbePosition(u32 x, u32 y, u32 z) const noexcept {
        Vec3 spacing = getProbeSpacing();
        return Vec3{
            boundsMin.x + static_cast<f32>(x) * spacing.x,
            boundsMin.y + static_cast<f32>(y) * spacing.y,
            boundsMin.z + static_cast<f32>(z) * spacing.z
        };
    }
    
    /**
     * @brief Get probe linear index
     */
    [[nodiscard]] u32 getProbeIndex(u32 x, u32 y, u32 z) const noexcept {
        return x + y * gridSizeX + z * gridSizeX * gridSizeY;
    }
    
    /**
     * @brief Get grid coordinates from position
     */
    [[nodiscard]] bool getGridCoords(const Vec3& pos, u32& outX, u32& outY, u32& outZ) const noexcept {
        Vec3 spacing = getProbeSpacing();
        
        f32 fx = (pos.x - boundsMin.x) / spacing.x;
        f32 fy = (pos.y - boundsMin.y) / spacing.y;
        f32 fz = (pos.z - boundsMin.z) / spacing.z;
        
        if (fx < 0 || fy < 0 || fz < 0 ||
            fx >= gridSizeX || fy >= gridSizeY || fz >= gridSizeZ) {
            return false;
        }
        
        outX = static_cast<u32>(fx);
        outY = static_cast<u32>(fy);
        outZ = static_cast<u32>(fz);
        
        return true;
    }
};

// =============================================================================
// Reflection Probe
// =============================================================================

/**
 * @brief Reflection probe data
 */
struct ReflectionProbe {
    /// Probe name
    std::string name = "ReflectionProbe";
    
    /// World-space position
    Vec3 position = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Box extents (for box projection)
    Vec3 boxExtents = Vec3{10.0f, 10.0f, 10.0f};
    
    /// Sphere radius (for sphere projection)
    f32 sphereRadius = 10.0f;
    
    /// Probe type
    ReflectionProbeType type = ReflectionProbeType::Box;
    
    /// Cubemap resolution
    u32 resolution = GIConfig::DEFAULT_REFLECTION_PROBE_RES;
    
    /// Probe state
    ProbeState state = ProbeState::Invalid;
    
    /// Priority for update scheduling
    i32 priority = 0;
    
    /// HDR intensity multiplier
    f32 intensity = 1.0f;
    
    /// Blend distance for transitions
    f32 blendDistance = 1.0f;
    
    /// Last frame updated
    u32 lastUpdateFrame = 0;
    
    /// Cubemap texture handle (0 = not allocated)
    u32 cubemapHandle = 0;
    
    /// Is real-time updated
    bool realtime = false;
    
    /**
     * @brief Calculate influence weight at a point
     */
    [[nodiscard]] f32 calculateInfluence(const Vec3& point) const noexcept {
        if (type == ReflectionProbeType::Sphere) {
            Vec3 diff = Vec3{point.x - position.x, point.y - position.y, point.z - position.z};
            f32 dist = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            
            if (dist >= sphereRadius + blendDistance) {
                return 0.0f;
            }
            if (dist <= sphereRadius - blendDistance) {
                return 1.0f;
            }
            
            return 1.0f - (dist - sphereRadius + blendDistance) / (2.0f * blendDistance);
        }
        
        // Box influence
        Vec3 localPos = Vec3{point.x - position.x, point.y - position.y, point.z - position.z};
        Vec3 absLocal = Vec3{std::abs(localPos.x), std::abs(localPos.y), std::abs(localPos.z)};
        
        Vec3 distToEdge = Vec3{
            boxExtents.x - absLocal.x,
            boxExtents.y - absLocal.y,
            boxExtents.z - absLocal.z
        };
        
        f32 minDist = std::min({distToEdge.x, distToEdge.y, distToEdge.z});
        
        if (minDist < 0.0f) {
            return 0.0f;
        }
        if (minDist >= blendDistance) {
            return 1.0f;
        }
        
        return minDist / blendDistance;
    }
    
    /**
     * @brief Check if point is inside probe volume
     */
    [[nodiscard]] bool containsPoint(const Vec3& point) const noexcept {
        if (type == ReflectionProbeType::Sphere) {
            Vec3 diff = Vec3{point.x - position.x, point.y - position.y, point.z - position.z};
            f32 distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            return distSq <= sphereRadius * sphereRadius;
        }
        
        // Box containment
        Vec3 localPos = Vec3{point.x - position.x, point.y - position.y, point.z - position.z};
        return std::abs(localPos.x) <= boxExtents.x &&
               std::abs(localPos.y) <= boxExtents.y &&
               std::abs(localPos.z) <= boxExtents.z;
    }
};

// =============================================================================
// SSGI Configuration
// =============================================================================

/**
 * @brief Screen-space GI configuration
 */
struct SSGIConfig {
    /// Enable SSGI
    bool enabled = true;
    
    /// Quality level
    GIQuality quality = GIQuality::Medium;
    
    /// Number of samples per pixel
    u32 sampleCount = 8;
    
    /// Maximum ray distance (screen space)
    f32 maxDistance = 0.5f;
    
    /// Thickness for hit detection
    f32 thickness = 0.1f;
    
    /// Intensity multiplier
    f32 intensity = 1.0f;
    
    /// Enable temporal filtering
    bool temporalFilter = true;
    
    /// Temporal feedback (0.9 = smooth, 0.7 = responsive)
    f32 temporalFeedback = 0.9f;
    
    /// Half-resolution rendering
    bool halfResolution = true;
    
    /// Blur passes for denoising
    u32 blurPasses = 2;
    
    /**
     * @brief Get effective sample count based on quality
     */
    [[nodiscard]] u32 getEffectiveSampleCount() const noexcept {
        switch (quality) {
            case GIQuality::Low: return 4;
            case GIQuality::Medium: return 8;
            case GIQuality::High: return 16;
            case GIQuality::Ultra: return 32;
            default: return sampleCount;
        }
    }
};

// =============================================================================
// RTGI Configuration
// =============================================================================

/**
 * @brief Ray-traced GI configuration
 */
struct RTGIConfig {
    /// Enable RTGI
    bool enabled = true;
    
    /// Quality level
    GIQuality quality = GIQuality::High;
    
    /// Number of rays per pixel
    u32 raysPerPixel = 1;
    
    /// Maximum bounce count
    u32 maxBounces = GIConfig::DEFAULT_RAY_BOUNCES;
    
    /// Maximum ray distance
    f32 maxRayDistance = 1000.0f;
    
    /// Intensity multiplier
    f32 intensity = 1.0f;
    
    /// Enable denoising
    bool denoise = true;
    
    /// Temporal accumulation
    bool temporalAccumulation = true;
    
    /// Spatial filter radius
    u32 filterRadius = 3;
    
    /**
     * @brief Get effective rays per pixel based on quality
     */
    [[nodiscard]] u32 getEffectiveRaysPerPixel() const noexcept {
        switch (quality) {
            case GIQuality::Low: return 1;
            case GIQuality::Medium: return 1;
            case GIQuality::High: return 2;
            case GIQuality::Ultra: return 4;
            default: return raysPerPixel;
        }
    }
};

// =============================================================================
// GI Statistics
// =============================================================================

/**
 * @brief Global illumination statistics
 */
struct GIStats {
    /// Current GI technique
    GITechnique technique = GITechnique::None;
    
    /// Total probes in grid
    u32 totalProbes = 0;
    
    /// Valid probes
    u32 validProbes = 0;
    
    /// Probes updated this frame
    u32 probesUpdatedThisFrame = 0;
    
    /// Reflection probes count
    u32 reflectionProbes = 0;
    
    /// Reflection probes updated this frame
    u32 reflectionProbesUpdatedThisFrame = 0;
    
    /// GI computation time (ms)
    f32 giTimeMs = 0.0f;
    
    /// Probe update time (ms)
    f32 probeUpdateTimeMs = 0.0f;
    
    /// Memory usage (bytes)
    u64 memoryUsage = 0;
    
    /// Rays traced this frame
    u64 raysThisFrame = 0;
    
    /**
     * @brief Reset per-frame stats
     */
    void resetFrameStats() noexcept {
        probesUpdatedThisFrame = 0;
        reflectionProbesUpdatedThisFrame = 0;
        giTimeMs = 0.0f;
        probeUpdateTimeMs = 0.0f;
        raysThisFrame = 0;
    }
};

// =============================================================================
// Global Illumination Manager
// =============================================================================

/**
 * @brief Global illumination manager
 * 
 * Manages GI techniques, probes, and rendering.
 */
class GIManager {
public:
    /**
     * @brief Default constructor
     */
    GIManager() = default;
    
    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set GI technique
     */
    void setTechnique(GITechnique technique) noexcept {
        m_technique = technique;
        m_stats.technique = technique;
    }
    
    /**
     * @brief Get current GI technique
     */
    [[nodiscard]] GITechnique getTechnique() const noexcept {
        return m_technique;
    }
    
    /**
     * @brief Set GI quality
     */
    void setQuality(GIQuality quality) noexcept {
        m_quality = quality;
    }
    
    /**
     * @brief Get GI quality
     */
    [[nodiscard]] GIQuality getQuality() const noexcept {
        return m_quality;
    }
    
    /**
     * @brief Set SSGI config
     */
    void setSSGIConfig(const SSGIConfig& config) noexcept {
        m_ssgiConfig = config;
    }
    
    /**
     * @brief Get SSGI config
     */
    [[nodiscard]] const SSGIConfig& getSSGIConfig() const noexcept {
        return m_ssgiConfig;
    }
    
    /**
     * @brief Set RTGI config
     */
    void setRTGIConfig(const RTGIConfig& config) noexcept {
        m_rtgiConfig = config;
    }
    
    /**
     * @brief Get RTGI config
     */
    [[nodiscard]] const RTGIConfig& getRTGIConfig() const noexcept {
        return m_rtgiConfig;
    }
    
    // -------------------------------------------------------------------------
    // Probe Grid Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Initialize probe grid
     */
    void initializeProbeGrid(const ProbeGridConfig& config) noexcept {
        m_probeGridConfig = config;
        m_irradianceProbes.clear();
        m_irradianceProbes.resize(config.getTotalProbes());
        
        // Initialize probe positions
        for (u32 z = 0; z < config.gridSizeZ; ++z) {
            for (u32 y = 0; y < config.gridSizeY; ++y) {
                for (u32 x = 0; x < config.gridSizeX; ++x) {
                    u32 idx = config.getProbeIndex(x, y, z);
                    m_irradianceProbes[idx].position = config.getProbePosition(x, y, z);
                    m_irradianceProbes[idx].state = ProbeState::Invalid;
                }
            }
        }
        
        m_stats.totalProbes = config.getTotalProbes();
    }
    
    /**
     * @brief Get probe grid config
     */
    [[nodiscard]] const ProbeGridConfig& getProbeGridConfig() const noexcept {
        return m_probeGridConfig;
    }
    
    /**
     * @brief Get irradiance probes
     */
    [[nodiscard]] const std::vector<IrradianceProbe>& getIrradianceProbes() const noexcept {
        return m_irradianceProbes;
    }
    
    /**
     * @brief Get irradiance at a point
     */
    [[nodiscard]] Vec3 sampleIrradiance(const Vec3& position, const Vec3& normal) const noexcept {
        // Find surrounding probes and interpolate
        u32 x, y, z;
        if (!m_probeGridConfig.getGridCoords(position, x, y, z)) {
            return Vec3{0.0f, 0.0f, 0.0f};
        }
        
        // Simple nearest probe lookup (real implementation would do trilinear)
        u32 idx = m_probeGridConfig.getProbeIndex(x, y, z);
        if (idx < m_irradianceProbes.size() && 
            m_irradianceProbes[idx].state == ProbeState::Valid) {
            return m_irradianceProbes[idx].irradiance.evaluate(normal);
        }
        
        return Vec3{0.0f, 0.0f, 0.0f};
    }
    
    // -------------------------------------------------------------------------
    // Reflection Probe Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Add reflection probe
     */
    u32 addReflectionProbe(const ReflectionProbe& probe) noexcept {
        u32 id = m_nextReflectionProbeId++;
        m_reflectionProbes[id] = probe;
        m_stats.reflectionProbes++;
        return id;
    }
    
    /**
     * @brief Remove reflection probe
     */
    void removeReflectionProbe(u32 id) noexcept {
        auto it = m_reflectionProbes.find(id);
        if (it != m_reflectionProbes.end()) {
            m_reflectionProbes.erase(it);
            m_stats.reflectionProbes--;
        }
    }
    
    /**
     * @brief Get reflection probe
     */
    [[nodiscard]] ReflectionProbe* getReflectionProbe(u32 id) noexcept {
        auto it = m_reflectionProbes.find(id);
        return it != m_reflectionProbes.end() ? &it->second : nullptr;
    }
    
    /**
     * @brief Get all reflection probes
     */
    [[nodiscard]] const std::unordered_map<u32, ReflectionProbe>& getReflectionProbes() const noexcept {
        return m_reflectionProbes;
    }
    
    /**
     * @brief Find relevant reflection probes for a point
     */
    [[nodiscard]] std::vector<u32> findRelevantReflectionProbes(
        const Vec3& position, u32 maxProbes = 4) const noexcept {
        
        std::vector<std::pair<u32, f32>> probesWithInfluence;
        
        for (const auto& [id, probe] : m_reflectionProbes) {
            f32 influence = probe.calculateInfluence(position);
            if (influence > 0.0f) {
                probesWithInfluence.push_back({id, influence});
            }
        }
        
        // Sort by influence (descending)
        std::sort(probesWithInfluence.begin(), probesWithInfluence.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Return top probes
        std::vector<u32> result;
        for (usize i = 0; i < std::min(probesWithInfluence.size(), static_cast<usize>(maxProbes)); ++i) {
            result.push_back(probesWithInfluence[i].first);
        }
        
        return result;
    }
    
    // -------------------------------------------------------------------------
    // Frame Operations
    // -------------------------------------------------------------------------
    
    /**
     * @brief Begin GI frame
     */
    void beginFrame(u32 frameNumber) noexcept {
        m_currentFrame = frameNumber;
        m_stats.resetFrameStats();
    }
    
    /**
     * @brief Update probes
     */
    void updateProbes(u32 maxUpdates = 4) noexcept {
        if (m_probeGridConfig.updateMode == ProbeUpdateMode::Incremental) {
            // Round-robin update
            for (u32 i = 0; i < maxUpdates && i < m_irradianceProbes.size(); ++i) {
                u32 idx = (m_nextProbeToUpdate + i) % m_irradianceProbes.size();
                auto& probe = m_irradianceProbes[idx];
                
                if (probe.state == ProbeState::Invalid || 
                    probe.needsUpdate(m_currentFrame)) {
                    // Simulate probe update (real implementation would trace rays)
                    probe.state = ProbeState::Valid;
                    probe.lastUpdateFrame = m_currentFrame;
                    m_stats.probesUpdatedThisFrame++;
                }
            }
            m_nextProbeToUpdate = (m_nextProbeToUpdate + maxUpdates) % 
                                   static_cast<u32>(m_irradianceProbes.size());
        }
        
        // Count valid probes
        m_stats.validProbes = 0;
        for (const auto& probe : m_irradianceProbes) {
            if (probe.state == ProbeState::Valid) {
                m_stats.validProbes++;
            }
        }
    }
    
    /**
     * @brief End GI frame
     */
    void endFrame() noexcept {
        // Update memory stats
        m_stats.memoryUsage = calculateMemoryUsage();
    }
    
    // -------------------------------------------------------------------------
    // Statistics
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get statistics
     */
    [[nodiscard]] const GIStats& getStats() const noexcept {
        return m_stats;
    }
    
    /**
     * @brief Calculate memory usage
     */
    [[nodiscard]] u64 calculateMemoryUsage() const noexcept {
        u64 memory = 0;
        
        // Irradiance probes
        memory += m_irradianceProbes.size() * sizeof(IrradianceProbe);
        
        // Reflection probes (estimate cubemap memory)
        for (const auto& [id, probe] : m_reflectionProbes) {
            // 6 faces * resolution^2 * 4 bytes (RGBA) * mips
            u64 cubemapSize = 6 * probe.resolution * probe.resolution * 4;
            // Add ~33% for mips
            memory += static_cast<u64>(cubemapSize * 1.33f);
        }
        
        return memory;
    }

private:
    // Configuration
    GITechnique m_technique = GITechnique::None;
    GIQuality m_quality = GIQuality::Medium;
    SSGIConfig m_ssgiConfig;
    RTGIConfig m_rtgiConfig;
    
    // Probe grid
    ProbeGridConfig m_probeGridConfig;
    std::vector<IrradianceProbe> m_irradianceProbes;
    u32 m_nextProbeToUpdate = 0;
    
    // Reflection probes
    std::unordered_map<u32, ReflectionProbe> m_reflectionProbes;
    u32 m_nextReflectionProbeId = 1;
    
    // Frame state
    u32 m_currentFrame = 0;
    
    // Statistics
    GIStats m_stats;
};

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Create ambient SH from a single color
 * @param color Ambient color
 * @return SH approximating uniform ambient lighting
 */
[[nodiscard]] inline SphericalHarmonics createAmbientSH(const Vec3& color) noexcept {
    SphericalHarmonics sh;
    // L0 band for uniform ambient
    sh.coefficients[0] = color * (2.0f * std::sqrt(3.14159265359f));
    return sh;
}

/**
 * @brief Create directional light SH
 * @param direction Light direction (normalized)
 * @param color Light color/intensity
 * @return SH approximating directional light
 */
[[nodiscard]] inline SphericalHarmonics createDirectionalSH(
    const Vec3& direction, const Vec3& color) noexcept {
    
    SphericalHarmonics sh;
    f32 scale = 2.0f * std::sqrt(3.14159265359f);
    
    sh.addSample(direction, color, scale);
    
    return sh;
}

/**
 * @brief Convolve SH for diffuse BRDF
 * @param sh Input SH
 * @return Convolved SH suitable for diffuse lighting
 */
[[nodiscard]] inline SphericalHarmonics convolveDiffuse(const SphericalHarmonics& sh) noexcept {
    SphericalHarmonics result;
    
    // Diffuse convolution coefficients (zonal harmonics)
    constexpr f32 A0 = 1.0f;
    constexpr f32 A1 = 2.0f / 3.0f;
    constexpr f32 A2 = 1.0f / 4.0f;
    
    // L0
    result.coefficients[0] = sh.coefficients[0] * A0;
    
    // L1
    result.coefficients[1] = sh.coefficients[1] * A1;
    result.coefficients[2] = sh.coefficients[2] * A1;
    result.coefficients[3] = sh.coefficients[3] * A1;
    
    // L2
    result.coefficients[4] = sh.coefficients[4] * A2;
    result.coefficients[5] = sh.coefficients[5] * A2;
    result.coefficients[6] = sh.coefficients[6] * A2;
    result.coefficients[7] = sh.coefficients[7] * A2;
    result.coefficients[8] = sh.coefficients[8] * A2;
    
    return result;
}

} // namespace nova
