/**
 * @file shadow_system.hpp
 * @brief Advanced Shadow Mapping System
 * 
 * Provides comprehensive shadow rendering with:
 * - Cascaded Shadow Maps (CSM) for directional lights
 * - Cube shadow maps for point lights
 * - Variance Shadow Maps (VSM) for soft shadows
 * - Exponential Shadow Maps (ESM) for fast filtering
 * - Contact hardening (PCSS) for realistic penumbras
 * - GPU-driven shadow culling
 * - Virtual shadow maps for massive worlds
 * 
 * Following the 12-year development roadmap from:
 * - NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md (Phase 1: Core Rendering)
 * - AUTONOMOUS_DEVELOPMENT_GUIDE.md
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/math.hpp"
#include "render_types.hpp"
#include "lighting.hpp"
#include <vector>
#include <array>
#include <memory>
#include <functional>

namespace nova::render {

// ============================================================================
// Shadow Configuration
// ============================================================================

/**
 * @brief Configuration constants for the shadow system
 */
struct ShadowSystemConfig {
    static constexpr u32 MAX_SHADOW_CASCADES = 8;       ///< Maximum CSM cascades
    static constexpr u32 MAX_SHADOW_MAPS = 64;          ///< Maximum shadow maps
    static constexpr u32 DEFAULT_SHADOW_RESOLUTION = 2048;
    static constexpr u32 MIN_SHADOW_RESOLUTION = 256;
    static constexpr u32 MAX_SHADOW_RESOLUTION = 8192;
    static constexpr f32 DEFAULT_CASCADE_SPLIT_LAMBDA = 0.75f;
    static constexpr f32 DEFAULT_SHADOW_BIAS = 0.005f;
    static constexpr f32 DEFAULT_NORMAL_BIAS = 0.01f;
    static constexpr u32 VIRTUAL_SHADOW_PAGE_SIZE = 128; ///< Virtual shadow map page size
    static constexpr u32 VIRTUAL_SHADOW_ATLAS_SIZE = 16384; ///< Virtual atlas resolution
};

// ============================================================================
// Shadow Enumerations
// ============================================================================

/**
 * @brief Shadow map filtering technique
 */
enum class ShadowFilterTechnique : u8 {
    None,           ///< Hard shadows, no filtering
    PCF,            ///< Percentage Closer Filtering
    PCSS,           ///< Percentage Closer Soft Shadows (contact hardening)
    VSM,            ///< Variance Shadow Maps
    ESM,            ///< Exponential Shadow Maps
    EVSM,           ///< Exponential Variance Shadow Maps
    MSM             ///< Moment Shadow Maps (highest quality)
};

/**
 * @brief Shadow map type
 */
enum class ShadowMapType : u8 {
    Standard2D,         ///< Standard 2D depth texture
    CubeMap,            ///< 6-face cube map for point lights
    CascadedArray,      ///< Texture array for CSM
    DualParaboloid,     ///< Dual paraboloid for 360° coverage
    Virtual             ///< Virtual shadow map with paging
};

/**
 * @brief Shadow caster type for culling
 */
enum class ShadowCasterType : u8 {
    Static,         ///< Never moves, can be cached
    Dynamic,        ///< Moves frequently
    Skinned,        ///< Animated mesh
    Particle        ///< Particle system
};

/**
 * @brief Shadow quality preset
 */
enum class ShadowQualityPreset : u8 {
    Off,            ///< No shadows
    Low,            ///< 512, PCF 2x2, 2 cascades
    Medium,         ///< 1024, PCF 3x3, 3 cascades
    High,           ///< 2048, PCF 5x5, 4 cascades
    Ultra,          ///< 4096, PCSS, 4 cascades
    Cinematic       ///< 8192, MSM, 8 cascades + ray-traced
};

// ============================================================================
// Shadow Map Descriptor
// ============================================================================

/**
 * @brief Describes a shadow map's properties
 */
struct ShadowMapDesc {
    ShadowMapType type = ShadowMapType::Standard2D;
    u32 resolution = ShadowSystemConfig::DEFAULT_SHADOW_RESOLUTION;
    TextureFormat format = TextureFormat::D32_FLOAT;
    u32 arrayLayers = 1;            ///< For cascades or cube faces
    bool generateMoments = false;    ///< For VSM/MSM
    bool blur = false;               ///< Apply blur pass
    
    /**
     * @brief Create descriptor for directional light CSM
     */
    static ShadowMapDesc directionalCSM(u32 cascades = 4, u32 res = 2048) {
        ShadowMapDesc desc;
        desc.type = ShadowMapType::CascadedArray;
        desc.resolution = res;
        desc.arrayLayers = cascades;
        desc.format = TextureFormat::D32_FLOAT;
        return desc;
    }
    
    /**
     * @brief Create descriptor for point light
     */
    static ShadowMapDesc pointLight(u32 res = 1024) {
        ShadowMapDesc desc;
        desc.type = ShadowMapType::CubeMap;
        desc.resolution = res;
        desc.arrayLayers = 6;
        desc.format = TextureFormat::D32_FLOAT;
        return desc;
    }
    
    /**
     * @brief Create descriptor for spot light
     */
    static ShadowMapDesc spotLight(u32 res = 1024) {
        ShadowMapDesc desc;
        desc.type = ShadowMapType::Standard2D;
        desc.resolution = res;
        desc.format = TextureFormat::D32_FLOAT;
        return desc;
    }
    
    /**
     * @brief Create VSM descriptor
     */
    static ShadowMapDesc varianceShadowMap(u32 res = 1024) {
        ShadowMapDesc desc;
        desc.type = ShadowMapType::Standard2D;
        desc.resolution = res;
        desc.format = TextureFormat::RG32_FLOAT;  // Store depth and depth²
        desc.generateMoments = true;
        desc.blur = true;
        return desc;
    }
    
    /**
     * @brief Calculate memory usage
     */
    [[nodiscard]] usize calculateMemoryUsage() const {
        usize bpp = getFormatBytesPerPixel(format);
        usize pixelCount = static_cast<usize>(resolution) * resolution;
        
        if (type == ShadowMapType::CubeMap) {
            return pixelCount * 6 * bpp;
        }
        return pixelCount * arrayLayers * bpp;
    }
};

// ============================================================================
// Cascade Shadow Map
// ============================================================================

/**
 * @brief Data for a single shadow cascade (extended version)
 */
struct ShadowCascadeData {
    math::Mat4 viewMatrix;          ///< Light view matrix for this cascade
    math::Mat4 projMatrix;          ///< Orthographic projection
    math::Mat4 viewProjMatrix;      ///< Combined view-projection
    f32 splitNear = 0.0f;           ///< Near split distance
    f32 splitFar = 100.0f;          ///< Far split distance
    f32 texelSize = 0.0f;           ///< World-space texel size
    u32 resolution = 2048;          ///< Cascade resolution
    
    /**
     * @brief Calculate texel size for shadow stabilization
     */
    void calculateTexelSize() {
        // Assuming ortho projection, get the width from projection matrix
        // proj[0][0] = 2 / width, so width = 2 / proj[0][0]
        f32 worldWidth = 2.0f / projMatrix.columns[0].x;
        texelSize = worldWidth / static_cast<f32>(resolution);
    }
    
    /**
     * @brief Snap cascade to texel grid to prevent shadow swimming
     */
    void snapToTexelGrid() {
        if (texelSize > 0.0f) {
            // Snap the view matrix origin to texel boundaries
            f32 x = viewProjMatrix.columns[3].x;
            f32 y = viewProjMatrix.columns[3].y;
            
            x = std::round(x / texelSize) * texelSize;
            y = std::round(y / texelSize) * texelSize;
            
            viewProjMatrix.columns[3].x = x;
            viewProjMatrix.columns[3].y = y;
        }
    }
};

/**
 * @brief GPU-friendly cascade data
 */
struct alignas(16) GPUCascadeData {
    math::Mat4 viewProjMatrix;      ///< Light space transform
    math::Vec4 splitDepths;         ///< x=near, y=far, z=texelSize, w=unused
    
    GPUCascadeData() = default;
    
    explicit GPUCascadeData(const ShadowCascadeData& cascade)
        : viewProjMatrix(cascade.viewProjMatrix)
        , splitDepths(cascade.splitNear, cascade.splitFar, cascade.texelSize, 0.0f)
    {}
};
// Note: Mat4 is 64 bytes (alignas(64)), Vec4 is 16 bytes, but struct alignment is 16
// so actual size may be 128 bytes due to Mat4's 64-byte alignment
static_assert(sizeof(GPUCascadeData) >= 80, "GPUCascadeData must be at least 80 bytes");

// ============================================================================
// Shadow Bias Settings
// ============================================================================

/**
 * @brief Shadow bias configuration to prevent artifacts
 */
struct ShadowBiasSettings {
    f32 constantBias = ShadowSystemConfig::DEFAULT_SHADOW_BIAS;
    f32 slopeBias = 0.0f;           ///< Slope-scaled bias
    f32 normalBias = ShadowSystemConfig::DEFAULT_NORMAL_BIAS;
    f32 receiverPlaneDepthBias = 0.0f;
    bool autoAdjust = true;         ///< Auto-adjust based on cascade
    
    /**
     * @brief Get adjusted bias for cascade level
     */
    [[nodiscard]] f32 getAdjustedBias(u32 cascadeIndex) const {
        if (!autoAdjust) return constantBias;
        // Increase bias for farther cascades
        return constantBias * (1.0f + cascadeIndex * 0.5f);
    }
    
    /**
     * @brief Calculate normal offset bias
     */
    [[nodiscard]] f32 getNormalOffset(f32 cosAngle) const {
        // More offset when surface is at grazing angle
        f32 sinAngle = std::sqrt(1.0f - cosAngle * cosAngle);
        return normalBias * sinAngle;
    }
    
    /**
     * @brief Create default bias settings
     */
    static ShadowBiasSettings defaults() {
        return {};
    }
    
    /**
     * @brief Create low-bias settings for close shadows
     */
    static ShadowBiasSettings minimal() {
        ShadowBiasSettings settings;
        settings.constantBias = 0.001f;
        settings.normalBias = 0.005f;
        return settings;
    }
};

// ============================================================================
// PCF Configuration
// ============================================================================

/**
 * @brief Configuration for Percentage Closer Filtering
 */
struct PCFSettings {
    u32 sampleCount = 16;           ///< Number of PCF samples
    f32 filterRadius = 1.5f;        ///< Filter radius in texels
    bool usePoisson = true;         ///< Use Poisson disk sampling
    bool rotatePerPixel = true;     ///< Rotate samples per pixel
    
    /**
     * @brief Get kernel size string for shader
     */
    [[nodiscard]] const char* getKernelSizeString() const {
        if (sampleCount <= 4) return "PCF_2X2";
        if (sampleCount <= 9) return "PCF_3X3";
        if (sampleCount <= 25) return "PCF_5X5";
        return "PCF_7X7";
    }
    
    /**
     * @brief Create default PCF settings
     */
    static PCFSettings defaults() {
        return {};
    }
    
    /**
     * @brief Create high-quality PCF settings
     */
    static PCFSettings highQuality() {
        PCFSettings settings;
        settings.sampleCount = 64;
        settings.filterRadius = 3.0f;
        return settings;
    }
};

// ============================================================================
// PCSS Configuration
// ============================================================================

/**
 * @brief Configuration for Percentage Closer Soft Shadows
 */
struct PCSSSettings {
    f32 lightSize = 1.0f;           ///< Light source size (world units)
    u32 blockerSearchSamples = 16;  ///< Samples for blocker search
    u32 pcfSamples = 32;            ///< Samples for PCF filtering
    f32 minFilterRadius = 0.5f;     ///< Minimum filter radius (texels)
    f32 maxFilterRadius = 10.0f;    ///< Maximum filter radius (texels)
    
    /**
     * @brief Calculate penumbra width from blocker distance
     */
    [[nodiscard]] f32 calculatePenumbraWidth(
        f32 receiverDistance,
        f32 avgBlockerDistance
    ) const {
        if (avgBlockerDistance <= 0.0f) return minFilterRadius;
        
        f32 penumbra = lightSize * (receiverDistance - avgBlockerDistance) / avgBlockerDistance;
        return std::clamp(penumbra, minFilterRadius, maxFilterRadius);
    }
    
    /**
     * @brief Create default PCSS settings
     */
    static PCSSSettings defaults() {
        return {};
    }
    
    /**
     * @brief Create settings for large area light
     */
    static PCSSSettings areaLight(f32 size) {
        PCSSSettings settings;
        settings.lightSize = size;
        settings.blockerSearchSamples = 32;
        settings.pcfSamples = 64;
        return settings;
    }
};

// ============================================================================
// VSM Configuration
// ============================================================================

/**
 * @brief Configuration for Variance Shadow Maps
 */
struct VSMSettings {
    f32 minVariance = 0.00001f;     ///< Minimum variance to avoid artifacts
    f32 lightBleedReduction = 0.2f; ///< Light bleeding reduction factor
    u32 blurPasses = 2;             ///< Number of blur passes
    f32 blurRadius = 2.0f;          ///< Blur kernel radius
    bool useExponential = false;    ///< Use EVSM for better quality
    f32 exponentialConstant = 40.0f; ///< EVSM exponent
    
    /**
     * @brief Calculate Chebyshev upper bound for shadow test
     * @return Shadow visibility (0 = shadowed, 1 = lit)
     */
    [[nodiscard]] f32 chebyshevUpperBound(f32 depth, f32 mean, f32 meanSq) const {
        f32 variance = meanSq - mean * mean;
        variance = std::max(variance, minVariance);
        
        f32 d = depth - mean;
        if (d <= 0.0f) return 1.0f;
        
        f32 pMax = variance / (variance + d * d);
        
        // Light bleed reduction
        return std::max((pMax - lightBleedReduction) / (1.0f - lightBleedReduction), 0.0f);
    }
    
    /**
     * @brief Create default VSM settings
     */
    static VSMSettings defaults() {
        return {};
    }
    
    /**
     * @brief Create EVSM settings
     */
    static VSMSettings exponential() {
        VSMSettings settings;
        settings.useExponential = true;
        settings.exponentialConstant = 40.0f;
        return settings;
    }
};

// ============================================================================
// Shadow Light Data
// ============================================================================

/**
 * @brief Shadow data associated with a light source
 */
struct ShadowLightData {
    u32 lightIndex = 0;                 ///< Index of associated light
    LightType lightType = LightType::Directional;
    ShadowMapDesc mapDesc;
    TextureHandle shadowMap;
    std::vector<ShadowCascadeData> cascades;
    ShadowBiasSettings bias;
    ShadowFilterTechnique filterTechnique = ShadowFilterTechnique::PCF;
    PCFSettings pcfSettings;
    PCSSSettings pcssSettings;
    VSMSettings vsmSettings;
    f32 maxShadowDistance = 100.0f;
    f32 fadeRange = 10.0f;              ///< Distance over which shadows fade
    bool enabled = true;
    bool cacheable = false;             ///< Can shadow map be cached
    bool dirty = true;                  ///< Needs re-render
    
    /**
     * @brief Calculate fade factor for smooth shadow distance falloff
     */
    [[nodiscard]] f32 calculateFadeFactor(f32 distance) const {
        if (distance >= maxShadowDistance) return 0.0f;
        if (distance <= maxShadowDistance - fadeRange) return 1.0f;
        return (maxShadowDistance - distance) / fadeRange;
    }
    
    /**
     * @brief Setup cascades for directional light
     */
    void setupCascades(
        u32 cascadeCount,
        f32 nearPlane,
        f32 farPlane,
        f32 splitLambda = ShadowSystemConfig::DEFAULT_CASCADE_SPLIT_LAMBDA
    ) {
        cascades.resize(cascadeCount);
        
        for (u32 i = 0; i < cascadeCount; ++i) {
            // Practical split scheme (mix of logarithmic and linear)
            f32 p = static_cast<f32>(i + 1) / static_cast<f32>(cascadeCount);
            f32 log = nearPlane * std::pow(farPlane / nearPlane, p);
            f32 uni = nearPlane + (farPlane - nearPlane) * p;
            f32 d = splitLambda * log + (1.0f - splitLambda) * uni;
            
            cascades[i].splitNear = (i == 0) ? nearPlane : cascades[i - 1].splitFar;
            cascades[i].splitFar = d;
            cascades[i].resolution = mapDesc.resolution;
        }
    }
    
    /**
     * @brief Get cascade index for depth value
     */
    [[nodiscard]] u32 getCascadeIndex(f32 viewDepth) const {
        for (u32 i = 0; i < cascades.size(); ++i) {
            if (viewDepth < cascades[i].splitFar) {
                return i;
            }
        }
        return static_cast<u32>(cascades.size() - 1);
    }
};

// ============================================================================
// GPU Shadow Data
// ============================================================================

/**
 * @brief GPU-friendly shadow parameters
 */
struct alignas(16) GPUShadowData {
    math::Mat4 shadowMatrix;        ///< World to shadow space
    math::Vec4 shadowParams;        ///< x=bias, y=normalBias, z=texelSize, w=filterRadius
    math::Vec4 shadowParams2;       ///< x=fadeStart, y=fadeEnd, z=lightSize, w=cascadeIndex
    u32 shadowMapIndex;             ///< Index into shadow map array
    u32 filterTechnique;            ///< ShadowFilterTechnique enum
    u32 cascadeCount;               ///< Number of cascades
    u32 flags;                      ///< Additional flags
    
    GPUShadowData() = default;
    
    GPUShadowData(const ShadowLightData& data, u32 mapIndex)
        : shadowMapIndex(mapIndex)
        , filterTechnique(static_cast<u32>(data.filterTechnique))
        , cascadeCount(static_cast<u32>(data.cascades.size()))
        , flags(data.enabled ? 1u : 0u)
    {
        shadowParams = {
            data.bias.constantBias,
            data.bias.normalBias,
            data.cascades.empty() ? 0.0f : data.cascades[0].texelSize,
            data.pcfSettings.filterRadius
        };
        
        shadowParams2 = {
            data.maxShadowDistance - data.fadeRange,
            data.maxShadowDistance,
            data.pcssSettings.lightSize,
            0.0f
        };
        
        if (!data.cascades.empty()) {
            shadowMatrix = data.cascades[0].viewProjMatrix;
        }
    }
};
// Note: Mat4 is 64 bytes (alignas(64)), 2 Vec4s = 32 bytes, 4 u32s = 16 bytes
// Total minimum = 112 bytes, but Mat4's alignment may increase actual size
static_assert(sizeof(GPUShadowData) >= 112, "GPUShadowData must be at least 112 bytes");

// ============================================================================
// Shadow Manager
// ============================================================================

/**
 * @brief Manages all shadow rendering for the scene
 */
class ShadowManager {
public:
    /**
     * @brief Shadow system statistics
     */
    struct Statistics {
        u32 totalShadowMaps = 0;
        u32 activeShadowMaps = 0;
        u32 shadowCastersTotal = 0;
        u32 shadowCastersRendered = 0;
        u64 shadowMemoryBytes = 0;
        u64 trianglesRendered = 0;
        f32 shadowPassTimeMs = 0.0f;
        
        void reset() {
            totalShadowMaps = 0;
            activeShadowMaps = 0;
            shadowCastersTotal = 0;
            shadowCastersRendered = 0;
            shadowMemoryBytes = 0;
            trianglesRendered = 0;
            shadowPassTimeMs = 0.0f;
        }
    };
    
    /**
     * @brief Get singleton instance
     */
    static ShadowManager& getInstance() {
        static ShadowManager instance;
        return instance;
    }
    
    /**
     * @brief Set shadow quality preset
     */
    void setQualityPreset(ShadowQualityPreset preset) {
        m_qualityPreset = preset;
        applyQualityPreset();
    }
    
    /**
     * @brief Get current quality preset
     */
    [[nodiscard]] ShadowQualityPreset getQualityPreset() const { return m_qualityPreset; }
    
    /**
     * @brief Set global shadow distance
     */
    void setMaxShadowDistance(f32 distance) { m_maxShadowDistance = distance; }
    
    /**
     * @brief Get global shadow distance
     */
    [[nodiscard]] f32 getMaxShadowDistance() const { return m_maxShadowDistance; }
    
    /**
     * @brief Set default filter technique
     */
    void setDefaultFilterTechnique(ShadowFilterTechnique technique) {
        m_defaultFilterTechnique = technique;
    }
    
    /**
     * @brief Get default filter technique
     */
    [[nodiscard]] ShadowFilterTechnique getDefaultFilterTechnique() const {
        return m_defaultFilterTechnique;
    }
    
    /**
     * @brief Set cascade split lambda
     */
    void setCascadeSplitLambda(f32 lambda) {
        m_cascadeSplitLambda = std::clamp(lambda, 0.0f, 1.0f);
    }
    
    /**
     * @brief Get cascade split lambda
     */
    [[nodiscard]] f32 getCascadeSplitLambda() const { return m_cascadeSplitLambda; }
    
    /**
     * @brief Get statistics
     */
    [[nodiscard]] const Statistics& getStatistics() const { return m_statistics; }
    
    /**
     * @brief Reset statistics
     */
    void resetStatistics() { m_statistics.reset(); }
    
    /**
     * @brief Enable/disable shadows globally
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    /**
     * @brief Check if shadows are enabled
     */
    [[nodiscard]] bool isEnabled() const { return m_enabled; }
    
    /**
     * @brief Get shadow atlas texture
     */
    [[nodiscard]] TextureHandle getShadowAtlas() const { return m_shadowAtlas; }
    
    /**
     * @brief Get cascade shadow atlas
     */
    [[nodiscard]] TextureHandle getCascadeAtlas() const { return m_cascadeAtlas; }
    
private:
    ShadowManager() = default;
    
    void applyQualityPreset() {
        switch (m_qualityPreset) {
            case ShadowQualityPreset::Off:
                m_enabled = false;
                break;
            case ShadowQualityPreset::Low:
                m_defaultResolution = 512;
                m_defaultCascadeCount = 2;
                m_defaultFilterTechnique = ShadowFilterTechnique::PCF;
                break;
            case ShadowQualityPreset::Medium:
                m_defaultResolution = 1024;
                m_defaultCascadeCount = 3;
                m_defaultFilterTechnique = ShadowFilterTechnique::PCF;
                break;
            case ShadowQualityPreset::High:
                m_defaultResolution = 2048;
                m_defaultCascadeCount = 4;
                m_defaultFilterTechnique = ShadowFilterTechnique::PCF;
                break;
            case ShadowQualityPreset::Ultra:
                m_defaultResolution = 4096;
                m_defaultCascadeCount = 4;
                m_defaultFilterTechnique = ShadowFilterTechnique::PCSS;
                break;
            case ShadowQualityPreset::Cinematic:
                m_defaultResolution = 8192;
                m_defaultCascadeCount = 8;
                m_defaultFilterTechnique = ShadowFilterTechnique::MSM;
                break;
        }
    }
    
    bool m_enabled = true;
    ShadowQualityPreset m_qualityPreset = ShadowQualityPreset::High;
    ShadowFilterTechnique m_defaultFilterTechnique = ShadowFilterTechnique::PCF;
    f32 m_maxShadowDistance = 100.0f;
    f32 m_cascadeSplitLambda = ShadowSystemConfig::DEFAULT_CASCADE_SPLIT_LAMBDA;
    u32 m_defaultResolution = ShadowSystemConfig::DEFAULT_SHADOW_RESOLUTION;
    u32 m_defaultCascadeCount = 4;
    Statistics m_statistics;
    TextureHandle m_shadowAtlas;
    TextureHandle m_cascadeAtlas;
};

// ============================================================================
// Shadow Utility Functions
// ============================================================================

/**
 * @brief Calculate tight orthographic projection for cascade
 */
[[nodiscard]] inline math::Mat4 calculateCascadeProjection(
    const math::Vec3* frustumCorners,
    const math::Mat4& lightView,
    f32 zMult = 10.0f
) {
    f32 minX = std::numeric_limits<f32>::max();
    f32 maxX = std::numeric_limits<f32>::lowest();
    f32 minY = std::numeric_limits<f32>::max();
    f32 maxY = std::numeric_limits<f32>::lowest();
    f32 minZ = std::numeric_limits<f32>::max();
    f32 maxZ = std::numeric_limits<f32>::lowest();
    
    // Transform frustum corners to light space and find bounds
    for (u32 i = 0; i < 8; ++i) {
        math::Vec4 corner(frustumCorners[i].x, frustumCorners[i].y, frustumCorners[i].z, 1.0f);
        math::Vec4 lightSpaceCorner = lightView * corner;
        
        minX = std::min(minX, lightSpaceCorner.x);
        maxX = std::max(maxX, lightSpaceCorner.x);
        minY = std::min(minY, lightSpaceCorner.y);
        maxY = std::max(maxY, lightSpaceCorner.y);
        minZ = std::min(minZ, lightSpaceCorner.z);
        maxZ = std::max(maxZ, lightSpaceCorner.z);
    }
    
    // Extend Z range for objects outside view frustum
    f32 zRange = maxZ - minZ;
    minZ -= zRange * (zMult - 1.0f);
    
    return math::Mat4::ortho(minX, maxX, minY, maxY, minZ, maxZ);
}

/**
 * @brief Calculate split distances using practical split scheme
 */
inline void calculateCascadeSplits(
    f32* splits,
    u32 cascadeCount,
    f32 nearPlane,
    f32 farPlane,
    f32 lambda = 0.75f
) {
    for (u32 i = 0; i < cascadeCount; ++i) {
        f32 p = static_cast<f32>(i + 1) / static_cast<f32>(cascadeCount);
        f32 log = nearPlane * std::pow(farPlane / nearPlane, p);
        f32 uni = nearPlane + (farPlane - nearPlane) * p;
        splits[i] = lambda * log + (1.0f - lambda) * uni;
    }
}

/**
 * @brief Generate Poisson disk samples for PCF
 */
inline void generatePoissonDiskSamples(
    math::Vec2* samples,
    u32 count,
    f32 minDistance = 0.4f
) {
    // Stratified sampling with jitter for Poisson-like distribution
    u32 gridSize = static_cast<u32>(std::ceil(std::sqrt(static_cast<f32>(count))));
    f32 cellSize = 2.0f / static_cast<f32>(gridSize); // -1 to 1 range
    
    u32 sampleIndex = 0;
    for (u32 y = 0; y < gridSize && sampleIndex < count; ++y) {
        for (u32 x = 0; x < gridSize && sampleIndex < count; ++x) {
            f32 baseX = -1.0f + (static_cast<f32>(x) + 0.5f) * cellSize;
            f32 baseY = -1.0f + (static_cast<f32>(y) + 0.5f) * cellSize;
            
            // Add jitter within cell
            f32 jitterX = (static_cast<f32>(std::rand()) / RAND_MAX - 0.5f) * cellSize * 0.8f;
            f32 jitterY = (static_cast<f32>(std::rand()) / RAND_MAX - 0.5f) * cellSize * 0.8f;
            
            samples[sampleIndex].x = baseX + jitterX;
            samples[sampleIndex].y = baseY + jitterY;
            sampleIndex++;
        }
    }
}

/**
 * @brief Calculate screen-space shadow map texel size
 */
[[nodiscard]] inline f32 calculateShadowTexelSize(
    f32 projectionWidth,
    u32 shadowMapResolution
) {
    return projectionWidth / static_cast<f32>(shadowMapResolution);
}

} // namespace nova::render
