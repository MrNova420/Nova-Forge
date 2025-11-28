/**
 * @file material.hpp
 * @brief Production-grade PBR material system for NovaCore engine
 * 
 * Implements a complete Physically Based Rendering material system with:
 * - Standard PBR workflow (metallic-roughness and specular-glossiness)
 * - Bindless texturing support for efficient rendering
 * - Material instancing for memory efficiency
 * - Shader permutation generation
 * - Material serialization and caching
 * - Clear coat, subsurface scattering, anisotropy, sheen support
 * - Real-time material parameter updates
 * 
 * Following NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md Phase 1: Core Rendering
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/render/render_types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <array>
#include <bitset>

namespace nova::render {

// ============================================================================
// Material System Configuration
// ============================================================================

/**
 * @brief Material system configuration constants
 */
struct MaterialConfig {
    /// Maximum number of textures per material
    static constexpr u32 MAX_TEXTURES_PER_MATERIAL = 16;
    
    /// Maximum number of material instances
    static constexpr u32 MAX_MATERIAL_INSTANCES = 65536;
    
    /// Maximum number of shader permutations
    static constexpr u32 MAX_SHADER_PERMUTATIONS = 4096;
    
    /// Default albedo color (white)
    static constexpr math::Vec4 DEFAULT_ALBEDO = {1.0f, 1.0f, 1.0f, 1.0f};
    
    /// Default metallic value
    static constexpr f32 DEFAULT_METALLIC = 0.0f;
    
    /// Default roughness value
    static constexpr f32 DEFAULT_ROUGHNESS = 0.5f;
    
    /// Default ambient occlusion
    static constexpr f32 DEFAULT_AO = 1.0f;
    
    /// Default emissive intensity
    static constexpr f32 DEFAULT_EMISSIVE_INTENSITY = 1.0f;
    
    /// Default normal strength
    static constexpr f32 DEFAULT_NORMAL_STRENGTH = 1.0f;
    
    /// Default IOR (index of refraction) for dielectrics
    static constexpr f32 DEFAULT_IOR = 1.5f;
    
    /// Maximum material parameter buffer size (bytes)
    static constexpr u32 MAX_PARAMETER_BUFFER_SIZE = 256;
    
    /// Material cache version for serialization
    static constexpr u32 CACHE_VERSION = 1;
};

// ============================================================================
// Material Enumerations
// ============================================================================

/**
 * @brief Material blend mode
 */
enum class BlendMode : u8 {
    Opaque = 0,         ///< No transparency
    Masked,             ///< Alpha cutout (binary transparency)
    Translucent,        ///< Standard alpha blending
    Additive,           ///< Additive blending (particles, effects)
    Modulate,           ///< Multiplicative blending
    PreMultiplied       ///< Pre-multiplied alpha blending
};

/**
 * @brief Get blend mode name
 */
constexpr const char* getBlendModeName(BlendMode mode) {
    switch (mode) {
        case BlendMode::Opaque: return "Opaque";
        case BlendMode::Masked: return "Masked";
        case BlendMode::Translucent: return "Translucent";
        case BlendMode::Additive: return "Additive";
        case BlendMode::Modulate: return "Modulate";
        case BlendMode::PreMultiplied: return "PreMultiplied";
    }
    return "Unknown";
}

/**
 * @brief Material shading model
 */
enum class ShadingModel : u8 {
    Unlit = 0,          ///< No lighting calculations
    DefaultLit,         ///< Standard PBR lighting
    Subsurface,         ///< Subsurface scattering (skin, wax, leaves)
    ClearCoat,          ///< Clear coat layer (car paint, lacquer)
    Hair,               ///< Specialized hair/fur shading
    Cloth,              ///< Fabric shading with sheen
    Eye,                ///< Realistic eye rendering
    Foliage,            ///< Vegetation with transmission
    ThinTranslucent,    ///< Thin surfaces like paper, leaves
    TwoSidedFoliage     ///< Two-sided vegetation rendering
};

/**
 * @brief Get shading model name
 */
constexpr const char* getShadingModelName(ShadingModel model) {
    switch (model) {
        case ShadingModel::Unlit: return "Unlit";
        case ShadingModel::DefaultLit: return "DefaultLit";
        case ShadingModel::Subsurface: return "Subsurface";
        case ShadingModel::ClearCoat: return "ClearCoat";
        case ShadingModel::Hair: return "Hair";
        case ShadingModel::Cloth: return "Cloth";
        case ShadingModel::Eye: return "Eye";
        case ShadingModel::Foliage: return "Foliage";
        case ShadingModel::ThinTranslucent: return "ThinTranslucent";
        case ShadingModel::TwoSidedFoliage: return "TwoSidedFoliage";
    }
    return "Unknown";
}

/**
 * @brief Material texture slot types
 */
enum class TextureSlot : u8 {
    Albedo = 0,         ///< Base color / diffuse
    Normal,             ///< Normal map
    MetallicRoughness,  ///< Combined metallic (B) + roughness (G) map
    AmbientOcclusion,   ///< Ambient occlusion map
    Emissive,           ///< Emissive map
    Height,             ///< Height/displacement map
    DetailNormal,       ///< Detail normal map
    DetailAlbedo,       ///< Detail albedo map
    SubsurfaceColor,    ///< Subsurface scattering color
    ClearCoatNormal,    ///< Clear coat normal map
    Anisotropy,         ///< Anisotropy direction map
    Sheen,              ///< Sheen/fuzz map for cloth
    Transmission,       ///< Transmission/opacity map
    Thickness,          ///< Thickness map for thin surfaces
    SpecularColor,      ///< Specular color (for specular workflow)
    Custom0             ///< Custom texture slot
};

/**
 * @brief Get texture slot name
 */
constexpr const char* getTextureSlotName(TextureSlot slot) {
    switch (slot) {
        case TextureSlot::Albedo: return "Albedo";
        case TextureSlot::Normal: return "Normal";
        case TextureSlot::MetallicRoughness: return "MetallicRoughness";
        case TextureSlot::AmbientOcclusion: return "AmbientOcclusion";
        case TextureSlot::Emissive: return "Emissive";
        case TextureSlot::Height: return "Height";
        case TextureSlot::DetailNormal: return "DetailNormal";
        case TextureSlot::DetailAlbedo: return "DetailAlbedo";
        case TextureSlot::SubsurfaceColor: return "SubsurfaceColor";
        case TextureSlot::ClearCoatNormal: return "ClearCoatNormal";
        case TextureSlot::Anisotropy: return "Anisotropy";
        case TextureSlot::Sheen: return "Sheen";
        case TextureSlot::Transmission: return "Transmission";
        case TextureSlot::Thickness: return "Thickness";
        case TextureSlot::SpecularColor: return "SpecularColor";
        case TextureSlot::Custom0: return "Custom0";
    }
    return "Unknown";
}

/**
 * @brief Material workflow type
 */
enum class MaterialWorkflow : u8 {
    MetallicRoughness = 0,  ///< PBR metallic-roughness workflow (default)
    SpecularGlossiness      ///< PBR specular-glossiness workflow
};

/**
 * @brief Material feature flags
 */
enum class MaterialFeature : u32 {
    None                = 0,
    HasAlbedoMap        = 1 << 0,
    HasNormalMap        = 1 << 1,
    HasMetallicRoughness= 1 << 2,
    HasAOMap            = 1 << 3,
    HasEmissiveMap      = 1 << 4,
    HasHeightMap        = 1 << 5,
    HasDetailMaps       = 1 << 6,
    UseVertexColors     = 1 << 7,
    DoubleSided         = 1 << 8,
    ReceiveShadows      = 1 << 9,
    CastShadows         = 1 << 10,
    UseParallaxMapping  = 1 << 11,
    UseTriplanarMapping = 1 << 12,
    UseRefraction       = 1 << 13,
    UseSubsurface       = 1 << 14,
    UseClearCoat        = 1 << 15,
    UseAnisotropy       = 1 << 16,
    UseSheen            = 1 << 17,
    UseTransmission     = 1 << 18,
    UseIridescence      = 1 << 19,
    DepthWriteDisabled  = 1 << 20,
    DepthTestDisabled   = 1 << 21,
    UseInstancing       = 1 << 22,
    UseSkinning         = 1 << 23,
    UseMorphTargets     = 1 << 24
};

/**
 * @brief Bitwise operators for MaterialFeature
 */
inline MaterialFeature operator|(MaterialFeature a, MaterialFeature b) {
    return static_cast<MaterialFeature>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline MaterialFeature operator&(MaterialFeature a, MaterialFeature b) {
    return static_cast<MaterialFeature>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline MaterialFeature& operator|=(MaterialFeature& a, MaterialFeature b) {
    a = a | b;
    return a;
}

inline bool hasFeature(MaterialFeature features, MaterialFeature flag) {
    return (static_cast<u32>(features) & static_cast<u32>(flag)) != 0;
}

// ============================================================================
// Texture Reference
// ============================================================================

/**
 * @brief Reference to a texture with UV transform
 */
struct MaterialTexture {
    TextureHandle handle;           ///< GPU texture handle
    u32 bindlessIndex = 0;          ///< Bindless texture index
    math::Vec2 uvOffset{0.0f, 0.0f};    ///< UV offset
    math::Vec2 uvScale{1.0f, 1.0f};     ///< UV scale
    f32 uvRotation = 0.0f;          ///< UV rotation (radians)
    u8 uvChannel = 0;               ///< Which UV channel to use
    
    /**
     * @brief Check if texture is valid
     */
    [[nodiscard]] bool isValid() const { return handle.isValid(); }
    
    /**
     * @brief Compute UV transform matrix (3x3 for 2D transform)
     */
    [[nodiscard]] std::array<f32, 9> computeUVMatrix() const {
        const f32 cosR = std::cos(uvRotation);
        const f32 sinR = std::sin(uvRotation);
        
        // Scale -> Rotate -> Translate
        return {
            uvScale.x * cosR,  -uvScale.y * sinR, uvOffset.x,
            uvScale.x * sinR,   uvScale.y * cosR, uvOffset.y,
            0.0f,               0.0f,             1.0f
        };
    }
};

// ============================================================================
// PBR Material Parameters
// ============================================================================

/**
 * @brief Standard PBR material parameters
 * 
 * Memory layout is optimized for GPU uniform buffer alignment.
 * Total size: 128 bytes (aligned to 16 bytes)
 */
struct alignas(16) PBRParameters {
    // Base color (16 bytes)
    math::Vec4 albedo{1.0f, 1.0f, 1.0f, 1.0f};
    
    // Emissive color with intensity packed in alpha (16 bytes)
    math::Vec4 emissive{0.0f, 0.0f, 0.0f, 1.0f};
    
    // Subsurface color for SSS materials (16 bytes)
    math::Vec4 subsurfaceColor{1.0f, 0.2f, 0.1f, 1.0f};
    
    // Sheen color for cloth materials (16 bytes)
    math::Vec4 sheenColor{1.0f, 1.0f, 1.0f, 1.0f};
    
    // Material properties (16 bytes)
    f32 metallic = 0.0f;            ///< Metallic factor [0,1]
    f32 roughness = 0.5f;           ///< Roughness factor [0,1]
    f32 ambientOcclusion = 1.0f;    ///< Ambient occlusion factor [0,1]
    f32 normalStrength = 1.0f;      ///< Normal map intensity [0,2]
    
    // Advanced PBR parameters (16 bytes)
    f32 ior = 1.5f;                 ///< Index of refraction
    f32 transmission = 0.0f;        ///< Transmission/transparency [0,1]
    f32 thickness = 0.5f;           ///< Thickness for thin surfaces
    f32 attenuationDistance = 1.0f; ///< Attenuation distance for transmission
    
    // Clear coat parameters (16 bytes)
    f32 clearCoat = 0.0f;           ///< Clear coat intensity [0,1]
    f32 clearCoatRoughness = 0.03f; ///< Clear coat roughness [0,1]
    f32 clearCoatNormalStrength = 1.0f; ///< Clear coat normal strength
    f32 anisotropy = 0.0f;          ///< Anisotropy intensity [-1,1]
    
    // Subsurface and special effects (16 bytes)
    f32 subsurfaceIntensity = 0.0f; ///< Subsurface scattering intensity
    f32 subsurfaceRadius = 1.0f;    ///< Subsurface scattering radius
    f32 sheenIntensity = 0.0f;      ///< Sheen/fuzz intensity for cloth
    f32 sheenRoughness = 0.5f;      ///< Sheen roughness
    
    /**
     * @brief Create default PBR parameters
     */
    static PBRParameters defaults() {
        return {};
    }
    
    /**
     * @brief Create metal material parameters
     */
    static PBRParameters metal(const math::Vec3& color, f32 roughnessValue = 0.3f) {
        PBRParameters params;
        params.albedo = math::Vec4(color.x, color.y, color.z, 1.0f);
        params.metallic = 1.0f;
        params.roughness = roughnessValue;
        return params;
    }
    
    /**
     * @brief Create dielectric material parameters
     */
    static PBRParameters dielectric(const math::Vec3& color, f32 roughnessValue = 0.5f) {
        PBRParameters params;
        params.albedo = math::Vec4(color.x, color.y, color.z, 1.0f);
        params.metallic = 0.0f;
        params.roughness = roughnessValue;
        return params;
    }
    
    /**
     * @brief Create glass material parameters
     */
    static PBRParameters glass(f32 iorValue = 1.5f) {
        PBRParameters params;
        params.albedo = math::Vec4(1.0f, 1.0f, 1.0f, 0.0f);
        params.metallic = 0.0f;
        params.roughness = 0.0f;
        params.ior = iorValue;
        params.transmission = 1.0f;
        return params;
    }
    
    /**
     * @brief Create car paint material parameters
     */
    static PBRParameters carPaint(const math::Vec3& baseColor, const math::Vec3& flakeColor) {
        PBRParameters params;
        params.albedo = math::Vec4(baseColor.x, baseColor.y, baseColor.z, 1.0f);
        params.metallic = 0.9f;
        params.roughness = 0.2f;
        params.clearCoat = 1.0f;
        params.clearCoatRoughness = 0.03f;
        (void)flakeColor; // Flake color would be used in shader
        return params;
    }
    
    /**
     * @brief Create skin material parameters
     */
    static PBRParameters skin() {
        PBRParameters params;
        params.albedo = math::Vec4(0.8f, 0.6f, 0.5f, 1.0f);
        params.metallic = 0.0f;
        params.roughness = 0.5f;
        params.subsurfaceIntensity = 0.5f;
        params.subsurfaceRadius = 2.0f;
        params.subsurfaceColor = math::Vec4(1.0f, 0.2f, 0.1f, 1.0f);
        return params;
    }
    
    /**
     * @brief Create cloth/fabric material parameters
     */
    static PBRParameters cloth(const math::Vec3& color) {
        PBRParameters params;
        params.albedo = math::Vec4(color.x, color.y, color.z, 1.0f);
        params.metallic = 0.0f;
        params.roughness = 0.8f;
        params.sheenIntensity = 0.5f;
        params.sheenRoughness = 0.5f;
        params.sheenColor = math::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        return params;
    }
};

static_assert(sizeof(PBRParameters) == 128, "PBRParameters must be 128 bytes for GPU alignment");

// ============================================================================
// Material Definition
// ============================================================================

/**
 * @brief Material ID type
 */
using MaterialId = Handle<struct MaterialTag>;

/**
 * @brief Complete material definition
 * 
 * Contains all data needed to render a surface including:
 * - Shading model and blend mode
 * - PBR parameters
 * - Texture references
 * - Shader permutation info
 */
class Material {
public:
    /**
     * @brief Construct empty material
     */
    Material() = default;
    
    /**
     * @brief Construct material with name
     */
    explicit Material(std::string name) : m_name(std::move(name)) {}
    
    // ========================================================================
    // Basic Properties
    // ========================================================================
    
    /**
     * @brief Get material name
     */
    [[nodiscard]] const std::string& getName() const { return m_name; }
    
    /**
     * @brief Set material name
     */
    void setName(std::string name) { m_name = std::move(name); }
    
    /**
     * @brief Get material ID
     */
    [[nodiscard]] MaterialId getId() const { return m_id; }
    
    /**
     * @brief Set material ID (internal use)
     */
    void setId(MaterialId id) { m_id = id; }
    
    // ========================================================================
    // Shading Properties
    // ========================================================================
    
    /**
     * @brief Get shading model
     */
    [[nodiscard]] ShadingModel getShadingModel() const { return m_shadingModel; }
    
    /**
     * @brief Set shading model
     */
    void setShadingModel(ShadingModel model) { 
        m_shadingModel = model;
        m_dirty = true;
    }
    
    /**
     * @brief Get blend mode
     */
    [[nodiscard]] BlendMode getBlendMode() const { return m_blendMode; }
    
    /**
     * @brief Set blend mode
     */
    void setBlendMode(BlendMode mode) { 
        m_blendMode = mode;
        m_dirty = true;
    }
    
    /**
     * @brief Get material workflow
     */
    [[nodiscard]] MaterialWorkflow getWorkflow() const { return m_workflow; }
    
    /**
     * @brief Set material workflow
     */
    void setWorkflow(MaterialWorkflow workflow) { 
        m_workflow = workflow;
        m_dirty = true;
    }
    
    // ========================================================================
    // PBR Parameters
    // ========================================================================
    
    /**
     * @brief Get PBR parameters (read-only)
     */
    [[nodiscard]] const PBRParameters& getParameters() const { return m_params; }
    
    /**
     * @brief Get PBR parameters (mutable)
     */
    PBRParameters& getParameters() { 
        m_dirty = true;
        return m_params; 
    }
    
    /**
     * @brief Set PBR parameters
     */
    void setParameters(const PBRParameters& params) { 
        m_params = params;
        m_dirty = true;
    }
    
    /**
     * @brief Set albedo color
     */
    void setAlbedo(const math::Vec4& color) { 
        m_params.albedo = color;
        m_dirty = true;
    }
    
    /**
     * @brief Set metallic value
     */
    void setMetallic(f32 value) { 
        m_params.metallic = value;
        m_dirty = true;
    }
    
    /**
     * @brief Set roughness value
     */
    void setRoughness(f32 value) { 
        m_params.roughness = value;
        m_dirty = true;
    }
    
    /**
     * @brief Set emissive color and intensity
     */
    void setEmissive(const math::Vec3& color, f32 intensity = 1.0f) { 
        m_params.emissive = math::Vec4(color.x, color.y, color.z, intensity);
        m_dirty = true;
    }
    
    // ========================================================================
    // Texture Management
    // ========================================================================
    
    /**
     * @brief Set texture for slot
     */
    void setTexture(TextureSlot slot, const MaterialTexture& texture) {
        m_textures[static_cast<usize>(slot)] = texture;
        updateFeatureFlags();
        m_dirty = true;
    }
    
    /**
     * @brief Set texture handle for slot
     */
    void setTexture(TextureSlot slot, TextureHandle handle) {
        MaterialTexture tex;
        tex.handle = handle;
        setTexture(slot, tex);
    }
    
    /**
     * @brief Get texture for slot
     */
    [[nodiscard]] const MaterialTexture& getTexture(TextureSlot slot) const {
        return m_textures[static_cast<usize>(slot)];
    }
    
    /**
     * @brief Check if slot has texture
     */
    [[nodiscard]] bool hasTexture(TextureSlot slot) const {
        return m_textures[static_cast<usize>(slot)].isValid();
    }
    
    /**
     * @brief Clear texture from slot
     */
    void clearTexture(TextureSlot slot) {
        m_textures[static_cast<usize>(slot)] = MaterialTexture{};
        updateFeatureFlags();
        m_dirty = true;
    }
    
    // ========================================================================
    // Feature Flags
    // ========================================================================
    
    /**
     * @brief Get material feature flags
     */
    [[nodiscard]] MaterialFeature getFeatures() const { return m_features; }
    
    /**
     * @brief Set material feature flag
     */
    void setFeature(MaterialFeature flag, bool enabled) {
        if (enabled) {
            m_features |= flag;
        } else {
            m_features = static_cast<MaterialFeature>(
                static_cast<u32>(m_features) & ~static_cast<u32>(flag));
        }
        m_dirty = true;
    }
    
    /**
     * @brief Check if material has feature
     */
    [[nodiscard]] bool hasFeature(MaterialFeature flag) const {
        return nova::render::hasFeature(m_features, flag);
    }
    
    /**
     * @brief Check if material is double-sided
     */
    [[nodiscard]] bool isDoubleSided() const { 
        return hasFeature(MaterialFeature::DoubleSided); 
    }
    
    /**
     * @brief Set double-sided rendering
     */
    void setDoubleSided(bool doubleSided) { 
        setFeature(MaterialFeature::DoubleSided, doubleSided); 
    }
    
    /**
     * @brief Check if material casts shadows
     */
    [[nodiscard]] bool castsShadows() const { 
        return hasFeature(MaterialFeature::CastShadows); 
    }
    
    /**
     * @brief Set shadow casting
     */
    void setCastsShadows(bool casts) { 
        setFeature(MaterialFeature::CastShadows, casts); 
    }
    
    /**
     * @brief Check if material receives shadows
     */
    [[nodiscard]] bool receivesShadows() const { 
        return hasFeature(MaterialFeature::ReceiveShadows); 
    }
    
    /**
     * @brief Set shadow receiving
     */
    void setReceivesShadows(bool receives) { 
        setFeature(MaterialFeature::ReceiveShadows, receives); 
    }
    
    // ========================================================================
    // Alpha Settings
    // ========================================================================
    
    /**
     * @brief Get alpha cutoff threshold (for masked blend mode)
     */
    [[nodiscard]] f32 getAlphaCutoff() const { return m_alphaCutoff; }
    
    /**
     * @brief Set alpha cutoff threshold
     */
    void setAlphaCutoff(f32 cutoff) { 
        m_alphaCutoff = cutoff;
        m_dirty = true;
    }
    
    // ========================================================================
    // Render State
    // ========================================================================
    
    /**
     * @brief Get render queue priority (lower = rendered first)
     */
    [[nodiscard]] i32 getRenderQueue() const { return m_renderQueue; }
    
    /**
     * @brief Set render queue priority
     */
    void setRenderQueue(i32 queue) { m_renderQueue = queue; }
    
    /**
     * @brief Check if material needs update
     */
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    
    /**
     * @brief Mark material as clean (after GPU upload)
     */
    void markClean() { m_dirty = false; }
    
    /**
     * @brief Compute shader permutation hash
     */
    [[nodiscard]] u64 computeShaderHash() const {
        // Combine relevant properties into a unique hash for shader selection
        u64 hash = constHash("material");
        hash = hash * 31 + static_cast<u64>(m_shadingModel);
        hash = hash * 31 + static_cast<u64>(m_blendMode);
        hash = hash * 31 + static_cast<u64>(m_workflow);
        hash = hash * 31 + static_cast<u64>(m_features);
        return hash;
    }
    
    // ========================================================================
    // GPU Data
    // ========================================================================
    
    /**
     * @brief Get GPU parameter buffer data
     */
    [[nodiscard]] const void* getGPUData() const { return &m_params; }
    
    /**
     * @brief Get GPU parameter buffer size
     */
    [[nodiscard]] usize getGPUDataSize() const { return sizeof(PBRParameters); }
    
    /**
     * @brief Get bindless texture indices for shader
     */
    [[nodiscard]] std::array<u32, MaterialConfig::MAX_TEXTURES_PER_MATERIAL> 
    getBindlessIndices() const {
        std::array<u32, MaterialConfig::MAX_TEXTURES_PER_MATERIAL> indices{};
        for (usize i = 0; i < MaterialConfig::MAX_TEXTURES_PER_MATERIAL; ++i) {
            indices[i] = m_textures[i].bindlessIndex;
        }
        return indices;
    }
    
    // ========================================================================
    // Factory Methods
    // ========================================================================
    
    /**
     * @brief Create default unlit material
     */
    static Material createUnlit(const std::string& name, const math::Vec4& color) {
        Material mat(name);
        mat.setShadingModel(ShadingModel::Unlit);
        mat.setAlbedo(color);
        return mat;
    }
    
    /**
     * @brief Create default PBR material
     */
    static Material createPBR(const std::string& name) {
        Material mat(name);
        mat.setShadingModel(ShadingModel::DefaultLit);
        mat.setBlendMode(BlendMode::Opaque);
        mat.setFeature(MaterialFeature::CastShadows, true);
        mat.setFeature(MaterialFeature::ReceiveShadows, true);
        return mat;
    }
    
    /**
     * @brief Create metal material
     */
    static Material createMetal(const std::string& name, const math::Vec3& color, f32 roughness = 0.3f) {
        Material mat(name);
        mat.setShadingModel(ShadingModel::DefaultLit);
        mat.setParameters(PBRParameters::metal(color, roughness));
        mat.setFeature(MaterialFeature::CastShadows, true);
        mat.setFeature(MaterialFeature::ReceiveShadows, true);
        return mat;
    }
    
    /**
     * @brief Create glass material
     */
    static Material createGlass(const std::string& name, f32 ior = 1.5f) {
        Material mat(name);
        mat.setShadingModel(ShadingModel::DefaultLit);
        mat.setBlendMode(BlendMode::Translucent);
        mat.setParameters(PBRParameters::glass(ior));
        mat.setFeature(MaterialFeature::UseRefraction, true);
        mat.setFeature(MaterialFeature::UseTransmission, true);
        mat.setFeature(MaterialFeature::ReceiveShadows, true);
        return mat;
    }
    
    /**
     * @brief Create skin material
     */
    static Material createSkin(const std::string& name) {
        Material mat(name);
        mat.setShadingModel(ShadingModel::Subsurface);
        mat.setParameters(PBRParameters::skin());
        mat.setFeature(MaterialFeature::UseSubsurface, true);
        mat.setFeature(MaterialFeature::CastShadows, true);
        mat.setFeature(MaterialFeature::ReceiveShadows, true);
        return mat;
    }
    
    /**
     * @brief Create cloth material
     */
    static Material createCloth(const std::string& name, const math::Vec3& color) {
        Material mat(name);
        mat.setShadingModel(ShadingModel::Cloth);
        mat.setParameters(PBRParameters::cloth(color));
        mat.setFeature(MaterialFeature::UseSheen, true);
        mat.setFeature(MaterialFeature::DoubleSided, true);
        mat.setFeature(MaterialFeature::CastShadows, true);
        mat.setFeature(MaterialFeature::ReceiveShadows, true);
        return mat;
    }
    
    /**
     * @brief Create car paint material
     */
    static Material createCarPaint(const std::string& name, const math::Vec3& baseColor) {
        Material mat(name);
        mat.setShadingModel(ShadingModel::ClearCoat);
        mat.setParameters(PBRParameters::carPaint(baseColor, math::Vec3(1.0f)));
        mat.setFeature(MaterialFeature::UseClearCoat, true);
        mat.setFeature(MaterialFeature::CastShadows, true);
        mat.setFeature(MaterialFeature::ReceiveShadows, true);
        return mat;
    }

private:
    /**
     * @brief Update feature flags based on textures
     */
    void updateFeatureFlags() {
        if (hasTexture(TextureSlot::Albedo)) {
            m_features |= MaterialFeature::HasAlbedoMap;
        }
        if (hasTexture(TextureSlot::Normal)) {
            m_features |= MaterialFeature::HasNormalMap;
        }
        if (hasTexture(TextureSlot::MetallicRoughness)) {
            m_features |= MaterialFeature::HasMetallicRoughness;
        }
        if (hasTexture(TextureSlot::AmbientOcclusion)) {
            m_features |= MaterialFeature::HasAOMap;
        }
        if (hasTexture(TextureSlot::Emissive)) {
            m_features |= MaterialFeature::HasEmissiveMap;
        }
        if (hasTexture(TextureSlot::Height)) {
            m_features |= MaterialFeature::HasHeightMap;
        }
        if (hasTexture(TextureSlot::DetailNormal) || hasTexture(TextureSlot::DetailAlbedo)) {
            m_features |= MaterialFeature::HasDetailMaps;
        }
    }
    
    // Material identification
    std::string m_name;
    MaterialId m_id;
    
    // Shading properties
    ShadingModel m_shadingModel = ShadingModel::DefaultLit;
    BlendMode m_blendMode = BlendMode::Opaque;
    MaterialWorkflow m_workflow = MaterialWorkflow::MetallicRoughness;
    
    // PBR parameters
    PBRParameters m_params;
    
    // Textures
    std::array<MaterialTexture, MaterialConfig::MAX_TEXTURES_PER_MATERIAL> m_textures{};
    
    // Feature flags
    MaterialFeature m_features = MaterialFeature::None;
    
    // Alpha settings
    f32 m_alphaCutoff = 0.5f;
    
    // Render settings
    i32 m_renderQueue = 0;
    
    // State tracking
    bool m_dirty = true;
};

// ============================================================================
// Material Instance
// ============================================================================

/**
 * @brief Material instance for efficient parameter variations
 * 
 * Shares shader and textures with parent material but allows
 * per-instance parameter overrides for efficient rendering of
 * many similar objects.
 */
class MaterialInstance {
public:
    /**
     * @brief Construct material instance
     */
    explicit MaterialInstance(std::shared_ptr<Material> parent)
        : m_parent(std::move(parent)) {
        if (m_parent) {
            m_params = m_parent->getParameters();
        }
    }
    
    /**
     * @brief Get parent material
     */
    [[nodiscard]] const Material* getParent() const { return m_parent.get(); }
    
    /**
     * @brief Get instance parameters
     */
    [[nodiscard]] const PBRParameters& getParameters() const { return m_params; }
    
    /**
     * @brief Get mutable instance parameters
     */
    PBRParameters& getParameters() { 
        m_dirty = true;
        return m_params; 
    }
    
    /**
     * @brief Set albedo color override
     */
    void setAlbedo(const math::Vec4& color) { 
        m_params.albedo = color;
        m_dirty = true;
    }
    
    /**
     * @brief Set emissive override
     */
    void setEmissive(const math::Vec3& color, f32 intensity = 1.0f) { 
        m_params.emissive = math::Vec4(color.x, color.y, color.z, intensity);
        m_dirty = true;
    }
    
    /**
     * @brief Check if instance needs GPU update
     */
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    
    /**
     * @brief Mark instance as clean
     */
    void markClean() { m_dirty = false; }
    
    /**
     * @brief Reset to parent parameters
     */
    void resetToParent() {
        if (m_parent) {
            m_params = m_parent->getParameters();
            m_dirty = true;
        }
    }

private:
    std::shared_ptr<Material> m_parent;
    PBRParameters m_params;
    bool m_dirty = true;
};

// ============================================================================
// Material Library
// ============================================================================

/**
 * @brief Material library for managing and caching materials
 */
class MaterialLibrary {
public:
    /**
     * @brief Get singleton instance
     */
    static MaterialLibrary& instance() {
        static MaterialLibrary lib;
        return lib;
    }
    
    /**
     * @brief Create new material
     */
    std::shared_ptr<Material> createMaterial(const std::string& name) {
        auto mat = std::make_shared<Material>(name);
        mat->setId(MaterialId(m_nextId++));
        m_materials[name] = mat;
        return mat;
    }
    
    /**
     * @brief Get material by name
     */
    [[nodiscard]] std::shared_ptr<Material> getMaterial(const std::string& name) const {
        auto it = m_materials.find(name);
        return it != m_materials.end() ? it->second : nullptr;
    }
    
    /**
     * @brief Check if material exists
     */
    [[nodiscard]] bool hasMaterial(const std::string& name) const {
        return m_materials.find(name) != m_materials.end();
    }
    
    /**
     * @brief Remove material
     */
    void removeMaterial(const std::string& name) {
        m_materials.erase(name);
    }
    
    /**
     * @brief Get all material names
     */
    [[nodiscard]] std::vector<std::string> getMaterialNames() const {
        std::vector<std::string> names;
        names.reserve(m_materials.size());
        for (const auto& [name, _] : m_materials) {
            names.push_back(name);
        }
        return names;
    }
    
    /**
     * @brief Get material count
     */
    [[nodiscard]] usize getMaterialCount() const { return m_materials.size(); }
    
    /**
     * @brief Clear all materials
     */
    void clear() {
        m_materials.clear();
        m_nextId = 1;
    }
    
    /**
     * @brief Create standard material presets
     */
    void createDefaultMaterials() {
        // Default white material
        auto defaultMat = createMaterial("Default");
        defaultMat->setShadingModel(ShadingModel::DefaultLit);
        defaultMat->setFeature(MaterialFeature::CastShadows, true);
        defaultMat->setFeature(MaterialFeature::ReceiveShadows, true);
        
        // Unlit white material
        auto unlitMat = createMaterial("Unlit");
        unlitMat->setShadingModel(ShadingModel::Unlit);
        
        // Debug materials
        auto wireframeMat = createMaterial("Wireframe");
        wireframeMat->setShadingModel(ShadingModel::Unlit);
        wireframeMat->setAlbedo(math::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
        
        auto errorMat = createMaterial("Error");
        errorMat->setShadingModel(ShadingModel::Unlit);
        errorMat->setAlbedo(math::Vec4(1.0f, 0.0f, 1.0f, 1.0f));
        
        // Common PBR materials
        auto chrome = createMaterial("Chrome");
        chrome->setParameters(PBRParameters::metal(math::Vec3(0.9f, 0.9f, 0.9f), 0.1f));
        chrome->setFeature(MaterialFeature::CastShadows, true);
        chrome->setFeature(MaterialFeature::ReceiveShadows, true);
        
        auto gold = createMaterial("Gold");
        gold->setParameters(PBRParameters::metal(math::Vec3(1.0f, 0.766f, 0.336f), 0.3f));
        gold->setFeature(MaterialFeature::CastShadows, true);
        gold->setFeature(MaterialFeature::ReceiveShadows, true);
        
        auto copper = createMaterial("Copper");
        copper->setParameters(PBRParameters::metal(math::Vec3(0.955f, 0.637f, 0.538f), 0.3f));
        copper->setFeature(MaterialFeature::CastShadows, true);
        copper->setFeature(MaterialFeature::ReceiveShadows, true);
        
        auto rubber = createMaterial("Rubber");
        rubber->setParameters(PBRParameters::dielectric(math::Vec3(0.1f, 0.1f, 0.1f), 0.9f));
        rubber->setFeature(MaterialFeature::CastShadows, true);
        rubber->setFeature(MaterialFeature::ReceiveShadows, true);
        
        auto plastic = createMaterial("Plastic");
        plastic->setParameters(PBRParameters::dielectric(math::Vec3(0.8f, 0.1f, 0.1f), 0.4f));
        plastic->setFeature(MaterialFeature::CastShadows, true);
        plastic->setFeature(MaterialFeature::ReceiveShadows, true);
        
        auto glass = createMaterial("Glass");
        glass->setShadingModel(ShadingModel::DefaultLit);
        glass->setBlendMode(BlendMode::Translucent);
        glass->setParameters(PBRParameters::glass(1.5f));
        glass->setFeature(MaterialFeature::UseRefraction, true);
        glass->setFeature(MaterialFeature::UseTransmission, true);
    }

private:
    MaterialLibrary() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
    u64 m_nextId = 1;
};

} // namespace nova::render
