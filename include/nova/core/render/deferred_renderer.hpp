/**
 * @file deferred_renderer.hpp
 * @brief Deferred Rendering Pipeline for NovaCore Engine
 * 
 * A comprehensive deferred rendering system implementing:
 * - G-Buffer management with configurable render targets
 * - Physically-based shading with metallic-roughness workflow
 * - Screen-space ambient occlusion (SSAO, HBAO+, GTAO)
 * - Clustered lighting integration
 * - Stencil-based light volumes
 * - Decal rendering
 * - Screen-space reflections
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/math/mat4.hpp"
#include "nova/core/render/render_types.hpp"

#include <array>
#include <vector>
#include <optional>
#include <functional>
#include <string>
#include <unordered_map>

namespace nova {

// =============================================================================
// Configuration Constants
// =============================================================================

/**
 * @brief Deferred rendering system configuration constants
 */
struct DeferredConfig {
    /// Maximum G-Buffer render targets
    static constexpr usize MAX_GBUFFER_TARGETS = 8;
    
    /// Maximum decals per frame
    static constexpr usize MAX_DECALS = 4096;
    
    /// Maximum light volumes per frame  
    static constexpr usize MAX_LIGHT_VOLUMES = 2048;
    
    /// Default G-Buffer resolution scale (1.0 = native)
    static constexpr f32 DEFAULT_RESOLUTION_SCALE = 1.0f;
    
    /// Maximum mip levels for hi-z buffer
    static constexpr u32 MAX_HIZ_MIPS = 12;
    
    /// SSAO kernel sample count (high quality)
    static constexpr u32 SSAO_KERNEL_SIZE_HIGH = 64;
    
    /// SSAO kernel sample count (medium quality)
    static constexpr u32 SSAO_KERNEL_SIZE_MEDIUM = 32;
    
    /// SSAO kernel sample count (low quality)
    static constexpr u32 SSAO_KERNEL_SIZE_LOW = 16;
    
    /// SSAO noise texture size
    static constexpr u32 SSAO_NOISE_SIZE = 4;
    
    /// SSR max ray march steps
    static constexpr u32 SSR_MAX_STEPS = 64;
    
    /// SSR binary search refinement iterations
    static constexpr u32 SSR_REFINEMENT_STEPS = 8;
};

// =============================================================================
// Enumerations
// =============================================================================

/**
 * @brief G-Buffer target types for deferred rendering
 */
enum class GBufferTarget : u32 {
    /// Albedo RGB + Metallic A (RGBA8 or RGBA16F)
    AlbedoMetallic = 0,
    
    /// World-space normal XY + Roughness Z + AO W (RGBA16F)
    NormalRoughnessAO = 1,
    
    /// Motion vectors XY + Emissive luminance ZW (RGBA16F)
    MotionEmissive = 2,
    
    /// Velocity XY for TAA + material ID (RGBA16F)
    VelocityMaterialID = 3,
    
    /// Depth buffer (D32F or D24S8)
    Depth = 4,
    
    /// Stencil buffer for light masking
    Stencil = 5,
    
    /// Shading model ID + flags (R8)
    ShadingFlags = 6,
    
    /// Custom data slot (application-defined)
    Custom = 7,
    
    COUNT
};

/**
 * @brief Get the name of a G-Buffer target
 */
[[nodiscard]] constexpr const char* getGBufferTargetName(GBufferTarget target) noexcept {
    switch (target) {
        case GBufferTarget::AlbedoMetallic: return "AlbedoMetallic";
        case GBufferTarget::NormalRoughnessAO: return "NormalRoughnessAO";
        case GBufferTarget::MotionEmissive: return "MotionEmissive";
        case GBufferTarget::VelocityMaterialID: return "VelocityMaterialID";
        case GBufferTarget::Depth: return "Depth";
        case GBufferTarget::Stencil: return "Stencil";
        case GBufferTarget::ShadingFlags: return "ShadingFlags";
        case GBufferTarget::Custom: return "Custom";
        default: return "Unknown";
    }
}

/**
 * @brief Ambient occlusion technique
 */
enum class AOTechnique : u32 {
    /// No ambient occlusion
    None = 0,
    
    /// Screen-Space Ambient Occlusion (classic)
    SSAO = 1,
    
    /// Horizon-Based Ambient Occlusion+
    HBAOPlus = 2,
    
    /// Ground Truth Ambient Occlusion
    GTAO = 3,
    
    /// Ray-traced ambient occlusion (RTX)
    RTAO = 4,
    
    COUNT
};

/**
 * @brief Get the name of an AO technique
 */
[[nodiscard]] constexpr const char* getAOTechniqueName(AOTechnique technique) noexcept {
    switch (technique) {
        case AOTechnique::None: return "None";
        case AOTechnique::SSAO: return "SSAO";
        case AOTechnique::HBAOPlus: return "HBAO+";
        case AOTechnique::GTAO: return "GTAO";
        case AOTechnique::RTAO: return "RTAO";
        default: return "Unknown";
    }
}

/**
 * @brief Screen-space reflection quality
 */
enum class SSRQuality : u32 {
    /// SSR disabled
    Off = 0,
    
    /// Low quality - fewer samples, no roughness blur
    Low = 1,
    
    /// Medium quality - moderate samples
    Medium = 2,
    
    /// High quality - full samples, roughness blur
    High = 3,
    
    /// Ultra quality - hierarchical tracing, temporal filtering
    Ultra = 4,
    
    COUNT
};

/**
 * @brief Get SSR quality name
 */
[[nodiscard]] constexpr const char* getSSRQualityName(SSRQuality quality) noexcept {
    switch (quality) {
        case SSRQuality::Off: return "Off";
        case SSRQuality::Low: return "Low";
        case SSRQuality::Medium: return "Medium";
        case SSRQuality::High: return "High";
        case SSRQuality::Ultra: return "Ultra";
        default: return "Unknown";
    }
}

/**
 * @brief Light volume shape for deferred light rendering
 */
enum class LightVolumeShape : u32 {
    /// Full-screen quad (directional lights, ambient)
    FullScreen = 0,
    
    /// Sphere volume (point lights)
    Sphere = 1,
    
    /// Cone volume (spot lights)  
    Cone = 2,
    
    /// Box volume (area lights)
    Box = 3,
    
    /// Custom mesh volume
    CustomMesh = 4,
    
    COUNT
};

/**
 * @brief Decal blending mode
 */
enum class DecalBlendMode : u32 {
    /// Replace albedo completely
    Replace = 0,
    
    /// Alpha blend with underlying surface
    AlphaBlend = 1,
    
    /// Multiply with underlying surface
    Multiply = 2,
    
    /// Additive blend
    Additive = 3,
    
    /// Normal map blending (reorient normals)
    NormalBlend = 4,
    
    /// PBR parameter overlay (roughness, metallic)
    PBROverlay = 5,
    
    COUNT
};

/**
 * @brief Get decal blend mode name
 */
[[nodiscard]] constexpr const char* getDecalBlendModeName(DecalBlendMode mode) noexcept {
    switch (mode) {
        case DecalBlendMode::Replace: return "Replace";
        case DecalBlendMode::AlphaBlend: return "AlphaBlend";
        case DecalBlendMode::Multiply: return "Multiply";
        case DecalBlendMode::Additive: return "Additive";
        case DecalBlendMode::NormalBlend: return "NormalBlend";
        case DecalBlendMode::PBROverlay: return "PBROverlay";
        default: return "Unknown";
    }
}

/**
 * @brief Deferred rendering pass phase
 */
enum class DeferredPhase : u32 {
    /// Depth pre-pass for early-z
    DepthPrePass = 0,
    
    /// G-Buffer fill pass
    GBufferFill = 1,
    
    /// Decal projection pass
    DecalPass = 2,
    
    /// Ambient occlusion computation
    AOPass = 3,
    
    /// Screen-space shadows
    ScreenSpaceShadows = 4,
    
    /// Lighting accumulation
    LightingPass = 5,
    
    /// Screen-space reflections
    SSRPass = 6,
    
    /// Volumetric lighting
    VolumetricPass = 7,
    
    /// Final composition
    CompositePass = 8,
    
    COUNT
};

/**
 * @brief G-Buffer texture format (internal representation)
 */
enum class GBufferFormat : u32 {
    /// 8-bit RGBA SRGB
    RGBA8_SRGB = 0,
    
    /// 16-bit RGBA float
    RGBA16F = 1,
    
    /// 32-bit depth float
    D32F = 2,
    
    /// 24-bit depth + 8-bit stencil
    D24S8 = 3,
    
    /// 8-bit red
    R8 = 4,
    
    /// 16-bit red float
    R16F = 5,
    
    COUNT
};

/**
 * @brief Get bytes per pixel for G-Buffer format
 */
[[nodiscard]] constexpr u32 getGBufferFormatBytesPerPixel(GBufferFormat format) noexcept {
    switch (format) {
        case GBufferFormat::RGBA8_SRGB: return 4;
        case GBufferFormat::RGBA16F: return 8;
        case GBufferFormat::D32F: return 4;
        case GBufferFormat::D24S8: return 4;
        case GBufferFormat::R8: return 1;
        case GBufferFormat::R16F: return 2;
        default: return 4;
    }
}

// =============================================================================
// Data Structures
// =============================================================================

/**
 * @brief G-Buffer texture descriptor
 */
struct GBufferTextureDesc {
    /// Target slot
    GBufferTarget target = GBufferTarget::AlbedoMetallic;
    
    /// Texture format
    GBufferFormat format = GBufferFormat::RGBA8_SRGB;
    
    /// Clear value (RGBA)
    std::array<f32, 4> clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /// Enable for writing
    bool enabled = true;
    
    /// Debug name
    std::string name = "GBuffer";
    
    /**
     * @brief Create albedo+metallic target descriptor
     */
    [[nodiscard]] static GBufferTextureDesc createAlbedoMetallic() noexcept {
        GBufferTextureDesc desc;
        desc.target = GBufferTarget::AlbedoMetallic;
        desc.format = GBufferFormat::RGBA8_SRGB;
        desc.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        desc.name = "GBuffer_AlbedoMetallic";
        return desc;
    }
    
    /**
     * @brief Create normal+roughness+AO target descriptor
     */
    [[nodiscard]] static GBufferTextureDesc createNormalRoughnessAO() noexcept {
        GBufferTextureDesc desc;
        desc.target = GBufferTarget::NormalRoughnessAO;
        desc.format = GBufferFormat::RGBA16F;
        desc.clearValue = {0.5f, 0.5f, 0.5f, 1.0f}; // Default normal up, full AO
        desc.name = "GBuffer_NormalRoughnessAO";
        return desc;
    }
    
    /**
     * @brief Create motion+emissive target descriptor
     */
    [[nodiscard]] static GBufferTextureDesc createMotionEmissive() noexcept {
        GBufferTextureDesc desc;
        desc.target = GBufferTarget::MotionEmissive;
        desc.format = GBufferFormat::RGBA16F;
        desc.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        desc.name = "GBuffer_MotionEmissive";
        return desc;
    }
    
    /**
     * @brief Create depth target descriptor
     */
    [[nodiscard]] static GBufferTextureDesc createDepth() noexcept {
        GBufferTextureDesc desc;
        desc.target = GBufferTarget::Depth;
        desc.format = GBufferFormat::D32F;
        desc.clearValue = {1.0f, 0.0f, 0.0f, 0.0f}; // Far plane depth
        desc.name = "GBuffer_Depth";
        return desc;
    }
};

/**
 * @brief G-Buffer configuration
 */
struct GBufferConfig {
    /// Resolution width
    u32 width = 1920;
    
    /// Resolution height
    u32 height = 1080;
    
    /// Resolution scale factor (0.5 = half res, 2.0 = supersampling)
    f32 resolutionScale = 1.0f;
    
    /// Target descriptors
    std::vector<GBufferTextureDesc> targets;
    
    /// Enable velocity buffer for TAA
    bool enableVelocity = true;
    
    /// Enable emissive buffer
    bool enableEmissive = true;
    
    /// Enable stencil for light masking
    bool enableStencil = true;
    
    /**
     * @brief Get scaled width
     */
    [[nodiscard]] u32 getScaledWidth() const noexcept {
        return static_cast<u32>(static_cast<f32>(width) * resolutionScale);
    }
    
    /**
     * @brief Get scaled height
     */
    [[nodiscard]] u32 getScaledHeight() const noexcept {
        return static_cast<u32>(static_cast<f32>(height) * resolutionScale);
    }
    
    /**
     * @brief Create default G-Buffer configuration
     */
    [[nodiscard]] static GBufferConfig createDefault(u32 w, u32 h) noexcept {
        GBufferConfig config;
        config.width = w;
        config.height = h;
        config.targets.push_back(GBufferTextureDesc::createAlbedoMetallic());
        config.targets.push_back(GBufferTextureDesc::createNormalRoughnessAO());
        config.targets.push_back(GBufferTextureDesc::createMotionEmissive());
        config.targets.push_back(GBufferTextureDesc::createDepth());
        return config;
    }
    
    /**
     * @brief Create minimal G-Buffer configuration (mobile)
     */
    [[nodiscard]] static GBufferConfig createMinimal(u32 w, u32 h) noexcept {
        GBufferConfig config;
        config.width = w;
        config.height = h;
        config.enableVelocity = false;
        config.enableEmissive = false;
        config.targets.push_back(GBufferTextureDesc::createAlbedoMetallic());
        config.targets.push_back(GBufferTextureDesc::createNormalRoughnessAO());
        config.targets.push_back(GBufferTextureDesc::createDepth());
        return config;
    }
};

/**
 * @brief SSAO configuration parameters
 */
struct SSAOConfig {
    /// AO technique to use
    AOTechnique technique = AOTechnique::GTAO;
    
    /// Sample kernel size
    u32 kernelSize = DeferredConfig::SSAO_KERNEL_SIZE_MEDIUM;
    
    /// Sample radius in world units
    f32 radius = 0.5f;
    
    /// AO strength/intensity
    f32 intensity = 1.0f;
    
    /// Bias to prevent self-occlusion
    f32 bias = 0.025f;
    
    /// Power curve for contrast
    f32 power = 2.0f;
    
    /// Blur passes for denoising
    u32 blurPasses = 2;
    
    /// Enable temporal filtering
    bool temporalFilter = true;
    
    /// Half-resolution rendering
    bool halfResolution = false;
    
    /**
     * @brief Validate and clamp configuration
     */
    void validate() noexcept {
        kernelSize = std::clamp(kernelSize, 8u, 128u);
        radius = std::clamp(radius, 0.01f, 5.0f);
        intensity = std::clamp(intensity, 0.0f, 4.0f);
        bias = std::clamp(bias, 0.0f, 0.1f);
        power = std::clamp(power, 0.5f, 4.0f);
        blurPasses = std::clamp(blurPasses, 0u, 4u);
    }
    
    /**
     * @brief Create low quality preset
     */
    [[nodiscard]] static SSAOConfig createLow() noexcept {
        SSAOConfig config;
        config.technique = AOTechnique::SSAO;
        config.kernelSize = DeferredConfig::SSAO_KERNEL_SIZE_LOW;
        config.halfResolution = true;
        config.blurPasses = 1;
        config.temporalFilter = false;
        return config;
    }
    
    /**
     * @brief Create high quality preset
     */
    [[nodiscard]] static SSAOConfig createHigh() noexcept {
        SSAOConfig config;
        config.technique = AOTechnique::GTAO;
        config.kernelSize = DeferredConfig::SSAO_KERNEL_SIZE_HIGH;
        config.halfResolution = false;
        config.blurPasses = 2;
        config.temporalFilter = true;
        return config;
    }
};

/**
 * @brief Screen-space reflection configuration
 */
struct SSRConfig {
    /// SSR quality level
    SSRQuality quality = SSRQuality::Medium;
    
    /// Maximum ray march steps
    u32 maxSteps = DeferredConfig::SSR_MAX_STEPS;
    
    /// Binary search refinement iterations
    u32 refinementSteps = DeferredConfig::SSR_REFINEMENT_STEPS;
    
    /// Maximum roughness for reflections (higher = more blur)
    f32 maxRoughness = 0.5f;
    
    /// Ray thickness for hit detection
    f32 thickness = 0.1f;
    
    /// Stride for hierarchical tracing
    u32 stride = 4;
    
    /// Fade at screen edges
    f32 edgeFade = 0.1f;
    
    /// Enable temporal filtering
    bool temporalFilter = true;
    
    /// Enable roughness-based blur
    bool roughnessBlur = true;
    
    /**
     * @brief Get steps based on quality level
     */
    [[nodiscard]] u32 getEffectiveSteps() const noexcept {
        switch (quality) {
            case SSRQuality::Off: return 0;
            case SSRQuality::Low: return 16;
            case SSRQuality::Medium: return 32;
            case SSRQuality::High: return 64;
            case SSRQuality::Ultra: return 128;
            default: return maxSteps;
        }
    }
    
    /**
     * @brief Check if SSR is enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return quality != SSRQuality::Off;
    }
};

/**
 * @brief Decal instance data
 */
struct DecalData {
    /// World-space transform matrix
    Mat4 worldMatrix = Mat4::identity();
    
    /// Inverse world matrix for projection
    Mat4 invWorldMatrix = Mat4::identity();
    
    /// Albedo texture handle
    u32 albedoTexture = 0;
    
    /// Normal texture handle
    u32 normalTexture = 0;
    
    /// PBR (roughness/metallic) texture handle
    u32 pbrTexture = 0;
    
    /// Albedo color tint
    Vec4 albedoTint = Vec4{1.0f, 1.0f, 1.0f, 1.0f};
    
    /// Blend mode
    DecalBlendMode blendMode = DecalBlendMode::AlphaBlend;
    
    /// Normal blend strength
    f32 normalStrength = 1.0f;
    
    /// Angle fade (cos of max angle from surface normal)
    f32 angleFade = 0.5f;
    
    /// Sort priority
    i32 priority = 0;
    
    /// Enable depth-based fade
    bool depthFade = true;
    
    /**
     * @brief Update inverse matrix from world matrix
     */
    void updateInverseMatrix() noexcept {
        invWorldMatrix = worldMatrix.inverse();
    }
};

/**
 * @brief GPU-aligned decal data for shader consumption
 */
struct alignas(16) GPUDecalData {
    /// World matrix (columns 0-3)
    Vec4 worldMatrixCol0;
    Vec4 worldMatrixCol1;
    Vec4 worldMatrixCol2;
    Vec4 worldMatrixCol3;
    
    /// Inverse world matrix (columns 0-3)
    Vec4 invWorldMatrixCol0;
    Vec4 invWorldMatrixCol1;
    Vec4 invWorldMatrixCol2;
    Vec4 invWorldMatrixCol3;
    
    /// Albedo tint
    Vec4 albedoTint;
    
    /// Parameters: normalStrength, angleFade, blendMode, padding
    Vec4 parameters;
    
    /// Texture indices: albedo, normal, pbr, padding
    std::array<u32, 4> textureIndices;
    
    /**
     * @brief Create from DecalData
     */
    [[nodiscard]] static GPUDecalData fromDecal(const DecalData& decal) noexcept {
        GPUDecalData gpu;
        
        // Extract world matrix columns
        gpu.worldMatrixCol0 = Vec4{decal.worldMatrix[0][0], decal.worldMatrix[1][0], 
                                   decal.worldMatrix[2][0], decal.worldMatrix[3][0]};
        gpu.worldMatrixCol1 = Vec4{decal.worldMatrix[0][1], decal.worldMatrix[1][1],
                                   decal.worldMatrix[2][1], decal.worldMatrix[3][1]};
        gpu.worldMatrixCol2 = Vec4{decal.worldMatrix[0][2], decal.worldMatrix[1][2],
                                   decal.worldMatrix[2][2], decal.worldMatrix[3][2]};
        gpu.worldMatrixCol3 = Vec4{decal.worldMatrix[0][3], decal.worldMatrix[1][3],
                                   decal.worldMatrix[2][3], decal.worldMatrix[3][3]};
        
        // Extract inverse world matrix columns
        gpu.invWorldMatrixCol0 = Vec4{decal.invWorldMatrix[0][0], decal.invWorldMatrix[1][0],
                                      decal.invWorldMatrix[2][0], decal.invWorldMatrix[3][0]};
        gpu.invWorldMatrixCol1 = Vec4{decal.invWorldMatrix[0][1], decal.invWorldMatrix[1][1],
                                      decal.invWorldMatrix[2][1], decal.invWorldMatrix[3][1]};
        gpu.invWorldMatrixCol2 = Vec4{decal.invWorldMatrix[0][2], decal.invWorldMatrix[1][2],
                                      decal.invWorldMatrix[2][2], decal.invWorldMatrix[3][2]};
        gpu.invWorldMatrixCol3 = Vec4{decal.invWorldMatrix[0][3], decal.invWorldMatrix[1][3],
                                      decal.invWorldMatrix[2][3], decal.invWorldMatrix[3][3]};
        
        gpu.albedoTint = decal.albedoTint;
        gpu.parameters = Vec4{decal.normalStrength, decal.angleFade, 
                             static_cast<f32>(decal.blendMode), 0.0f};
        gpu.textureIndices = {decal.albedoTexture, decal.normalTexture, decal.pbrTexture, 0};
        
        return gpu;
    }
};

/**
 * @brief Light volume for deferred lighting
 */
struct LightVolume {
    /// Volume shape type
    LightVolumeShape shape = LightVolumeShape::Sphere;
    
    /// World-space position
    Vec3 position = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Volume scale/radius
    Vec3 scale = Vec3{1.0f, 1.0f, 1.0f};
    
    /// Rotation (for cone/box volumes)
    Vec3 rotation = Vec3{0.0f, 0.0f, 0.0f};
    
    /// Associated light index
    u32 lightIndex = 0;
    
    /// Use stencil masking
    bool useStencil = true;
    
    /**
     * @brief Get bounding sphere radius
     */
    [[nodiscard]] f32 getBoundingRadius() const noexcept {
        return std::max({scale.x, scale.y, scale.z});
    }
    
    /**
     * @brief Check if volume is potentially visible
     */
    [[nodiscard]] bool isVisible(const Vec3& cameraPos, f32 farPlane) const noexcept {
        f32 distance = (position - cameraPos).length();
        return distance - getBoundingRadius() < farPlane;
    }
};

/**
 * @brief Deferred renderer statistics
 */
struct DeferredStats {
    /// Number of objects rendered to G-Buffer
    u32 gbufferObjects = 0;
    
    /// Number of decals rendered
    u32 decalsRendered = 0;
    
    /// Number of light volumes processed
    u32 lightVolumes = 0;
    
    /// Number of pixels lit
    u64 pixelsLit = 0;
    
    /// G-Buffer fill time (ms)
    f32 gbufferTimeMs = 0.0f;
    
    /// AO pass time (ms)
    f32 aoTimeMs = 0.0f;
    
    /// Lighting pass time (ms)
    f32 lightingTimeMs = 0.0f;
    
    /// SSR pass time (ms)
    f32 ssrTimeMs = 0.0f;
    
    /// Total frame time (ms)
    f32 totalTimeMs = 0.0f;
    
    /// G-Buffer memory usage (bytes)
    u64 gbufferMemory = 0;
    
    /**
     * @brief Reset statistics
     */
    void reset() noexcept {
        gbufferObjects = 0;
        decalsRendered = 0;
        lightVolumes = 0;
        pixelsLit = 0;
        gbufferTimeMs = 0.0f;
        aoTimeMs = 0.0f;
        lightingTimeMs = 0.0f;
        ssrTimeMs = 0.0f;
        totalTimeMs = 0.0f;
    }
};

// =============================================================================
// Deferred Renderer Class
// =============================================================================

/**
 * @brief Complete deferred rendering pipeline
 * 
 * Manages G-Buffer creation, lighting passes, and post-effects integration.
 */
class DeferredRenderer {
public:
    /**
     * @brief Default constructor
     */
    DeferredRenderer() = default;
    
    /**
     * @brief Initialize with G-Buffer configuration
     */
    explicit DeferredRenderer(const GBufferConfig& config) 
        : m_gbufferConfig(config) {
        initializeGBuffer();
    }
    
    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set G-Buffer configuration
     */
    void setGBufferConfig(const GBufferConfig& config) noexcept {
        m_gbufferConfig = config;
        m_needsRebuild = true;
    }
    
    /**
     * @brief Get current G-Buffer configuration
     */
    [[nodiscard]] const GBufferConfig& getGBufferConfig() const noexcept {
        return m_gbufferConfig;
    }
    
    /**
     * @brief Set SSAO configuration
     */
    void setSSAOConfig(const SSAOConfig& config) noexcept {
        m_ssaoConfig = config;
        m_ssaoConfig.validate();
    }
    
    /**
     * @brief Get SSAO configuration
     */
    [[nodiscard]] const SSAOConfig& getSSAOConfig() const noexcept {
        return m_ssaoConfig;
    }
    
    /**
     * @brief Set SSR configuration
     */
    void setSSRConfig(const SSRConfig& config) noexcept {
        m_ssrConfig = config;
    }
    
    /**
     * @brief Get SSR configuration
     */
    [[nodiscard]] const SSRConfig& getSSRConfig() const noexcept {
        return m_ssrConfig;
    }
    
    // -------------------------------------------------------------------------
    // Rendering Operations
    // -------------------------------------------------------------------------
    
    /**
     * @brief Begin deferred rendering frame
     */
    void beginFrame() noexcept {
        m_stats.reset();
        m_decals.clear();
        m_lightVolumes.clear();
        m_currentPhase = DeferredPhase::DepthPrePass;
        
        if (m_needsRebuild) {
            rebuildGBuffer();
            m_needsRebuild = false;
        }
    }
    
    /**
     * @brief End deferred rendering frame
     */
    void endFrame() noexcept {
        m_currentPhase = DeferredPhase::CompositePass;
        calculateStats();
    }
    
    /**
     * @brief Execute depth pre-pass
     */
    void executeDepthPrePass() noexcept {
        m_currentPhase = DeferredPhase::DepthPrePass;
        // Actual rendering would happen here via command buffer
    }
    
    /**
     * @brief Execute G-Buffer fill pass
     */
    void executeGBufferPass() noexcept {
        m_currentPhase = DeferredPhase::GBufferFill;
        // Record G-Buffer objects rendered
    }
    
    /**
     * @brief Execute decal pass
     */
    void executeDecalPass() noexcept {
        m_currentPhase = DeferredPhase::DecalPass;
        m_stats.decalsRendered = static_cast<u32>(m_decals.size());
    }
    
    /**
     * @brief Execute ambient occlusion pass
     */
    void executeAOPass() noexcept {
        m_currentPhase = DeferredPhase::AOPass;
        if (m_ssaoConfig.technique == AOTechnique::None) {
            return;
        }
        // AO computation would happen here
    }
    
    /**
     * @brief Execute lighting pass
     */
    void executeLightingPass() noexcept {
        m_currentPhase = DeferredPhase::LightingPass;
        m_stats.lightVolumes = static_cast<u32>(m_lightVolumes.size());
    }
    
    /**
     * @brief Execute SSR pass
     */
    void executeSSRPass() noexcept {
        m_currentPhase = DeferredPhase::SSRPass;
        if (!m_ssrConfig.isEnabled()) {
            return;
        }
        // SSR computation would happen here
    }
    
    // -------------------------------------------------------------------------
    // Decal Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Add a decal for rendering
     */
    void addDecal(const DecalData& decal) noexcept {
        if (m_decals.size() < DeferredConfig::MAX_DECALS) {
            m_decals.push_back(decal);
        }
    }
    
    /**
     * @brief Get all decals
     */
    [[nodiscard]] const std::vector<DecalData>& getDecals() const noexcept {
        return m_decals;
    }
    
    /**
     * @brief Clear all decals
     */
    void clearDecals() noexcept {
        m_decals.clear();
    }
    
    /**
     * @brief Sort decals by priority
     */
    void sortDecals() noexcept {
        std::sort(m_decals.begin(), m_decals.end(),
            [](const DecalData& a, const DecalData& b) {
                return a.priority < b.priority;
            });
    }
    
    // -------------------------------------------------------------------------
    // Light Volume Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Add a light volume
     */
    void addLightVolume(const LightVolume& volume) noexcept {
        if (m_lightVolumes.size() < DeferredConfig::MAX_LIGHT_VOLUMES) {
            m_lightVolumes.push_back(volume);
        }
    }
    
    /**
     * @brief Get all light volumes
     */
    [[nodiscard]] const std::vector<LightVolume>& getLightVolumes() const noexcept {
        return m_lightVolumes;
    }
    
    /**
     * @brief Clear all light volumes
     */
    void clearLightVolumes() noexcept {
        m_lightVolumes.clear();
    }
    
    // -------------------------------------------------------------------------
    // Statistics
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get rendering statistics
     */
    [[nodiscard]] const DeferredStats& getStats() const noexcept {
        return m_stats;
    }
    
    /**
     * @brief Get current rendering phase
     */
    [[nodiscard]] DeferredPhase getCurrentPhase() const noexcept {
        return m_currentPhase;
    }
    
    /**
     * @brief Check if G-Buffer needs rebuild
     */
    [[nodiscard]] bool needsRebuild() const noexcept {
        return m_needsRebuild;
    }
    
    /**
     * @brief Calculate G-Buffer memory usage
     */
    [[nodiscard]] u64 calculateGBufferMemory() const noexcept {
        u64 memory = 0;
        u32 w = m_gbufferConfig.getScaledWidth();
        u32 h = m_gbufferConfig.getScaledHeight();
        
        for (const auto& target : m_gbufferConfig.targets) {
            u32 bpp = getGBufferFormatBytesPerPixel(target.format);
            memory += static_cast<u64>(w) * h * bpp;
        }
        
        return memory;
    }

private:
    /**
     * @brief Initialize G-Buffer resources
     */
    void initializeGBuffer() noexcept {
        m_stats.gbufferMemory = calculateGBufferMemory();
    }
    
    /**
     * @brief Rebuild G-Buffer on configuration change
     */
    void rebuildGBuffer() noexcept {
        initializeGBuffer();
    }
    
    /**
     * @brief Calculate frame statistics
     */
    void calculateStats() noexcept {
        m_stats.totalTimeMs = m_stats.gbufferTimeMs + m_stats.aoTimeMs + 
                              m_stats.lightingTimeMs + m_stats.ssrTimeMs;
    }
    
    // Configuration
    GBufferConfig m_gbufferConfig;
    SSAOConfig m_ssaoConfig;
    SSRConfig m_ssrConfig;
    
    // Runtime state
    std::vector<DecalData> m_decals;
    std::vector<LightVolume> m_lightVolumes;
    DeferredPhase m_currentPhase = DeferredPhase::DepthPrePass;
    bool m_needsRebuild = false;
    
    // Statistics
    DeferredStats m_stats;
};

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Generate SSAO kernel samples
 * @param samples Output array for samples
 * @param count Number of samples to generate
 */
inline void generateSSAOKernel(std::vector<Vec3>& samples, u32 count) noexcept {
    samples.clear();
    samples.reserve(count);
    
    for (u32 i = 0; i < count; ++i) {
        // Generate random point in hemisphere
        f32 u = static_cast<f32>(i) / static_cast<f32>(count);
        f32 theta = u * 2.0f * 3.14159265359f;
        f32 phi = std::acos(1.0f - u);
        
        Vec3 sample{
            std::sin(phi) * std::cos(theta),
            std::sin(phi) * std::sin(theta),
            std::cos(phi)
        };
        
        // Scale sample to be more distributed near origin
        f32 scale = static_cast<f32>(i) / static_cast<f32>(count);
        scale = 0.1f + scale * scale * 0.9f; // lerp(0.1, 1.0, scale^2)
        sample = sample * scale;
        
        samples.push_back(sample);
    }
}

/**
 * @brief Generate SSAO noise texture data
 * @param noise Output array for noise vectors
 * @param size Noise texture dimensions (size x size)
 */
inline void generateSSAONoise(std::vector<Vec3>& noise, u32 size) noexcept {
    noise.clear();
    u32 count = size * size;
    noise.reserve(count);
    
    for (u32 i = 0; i < count; ++i) {
        // Random rotation around Z axis
        f32 angle = static_cast<f32>(i) * 2.0f * 3.14159265359f / static_cast<f32>(count);
        noise.push_back(Vec3{std::cos(angle), std::sin(angle), 0.0f});
    }
}

/**
 * @brief Calculate screen-space position from depth
 * @param uv Screen UV coordinates [0,1]
 * @param depth Depth value [0,1]
 * @param invProjection Inverse projection matrix
 * @return View-space position
 */
[[nodiscard]] inline Vec3 reconstructViewPosition(
    const Vec2& uv, f32 depth, const Mat4& invProjection) noexcept {
    
    // Convert to NDC
    Vec4 ndc{
        uv.x * 2.0f - 1.0f,
        uv.y * 2.0f - 1.0f,
        depth * 2.0f - 1.0f,
        1.0f
    };
    
    // Transform to view space
    Vec4 viewPos = invProjection * ndc;
    
    // Perspective divide
    if (std::abs(viewPos.w) > 0.0001f) {
        return Vec3{viewPos.x / viewPos.w, viewPos.y / viewPos.w, viewPos.z / viewPos.w};
    }
    
    return Vec3{0.0f, 0.0f, 0.0f};
}

} // namespace nova
