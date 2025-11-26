/**
 * @file render_types.hpp
 * @brief Core rendering types and enumerations
 * 
 * Defines platform-agnostic types for the NovaCore rendering system.
 * These types abstract away backend-specific details while providing
 * a consistent API.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include <array>
#include <string>

namespace nova::render {

// ============================================================================
// Graphics Backend
// ============================================================================

/**
 * @brief Supported graphics backends
 */
enum class GraphicsBackend : u8 {
    None = 0,       ///< No backend (for testing)
    Vulkan,         ///< Vulkan 1.3+ (Android, Linux, Windows)
    Metal,          ///< Metal 3 (iOS, macOS)
    WebGPU,         ///< WebGPU (Web browsers)
    D3D12,          ///< DirectX 12 (Windows, Xbox)
    OpenGLES,       ///< OpenGL ES 3.0+ (Legacy mobile fallback)
    OpenGL,         ///< OpenGL 4.6 (Legacy desktop fallback)
    Software        ///< CPU software renderer (Universal fallback)
};

/**
 * @brief Get backend name string
 */
constexpr const char* getBackendName(GraphicsBackend backend) {
    switch (backend) {
        case GraphicsBackend::None: return "None";
        case GraphicsBackend::Vulkan: return "Vulkan";
        case GraphicsBackend::Metal: return "Metal";
        case GraphicsBackend::WebGPU: return "WebGPU";
        case GraphicsBackend::D3D12: return "D3D12";
        case GraphicsBackend::OpenGLES: return "OpenGL ES";
        case GraphicsBackend::OpenGL: return "OpenGL";
        case GraphicsBackend::Software: return "Software";
    }
    return "Unknown";
}

// ============================================================================
// Quality Tiers
// ============================================================================

/**
 * @brief Rendering quality tiers
 * 
 * NovaCore adapts to device capabilities automatically.
 * Every tier provides the BEST possible quality for that hardware.
 */
enum class QualityTier : u8 {
    Minimal = 0,    ///< Ultra-low devices (2014-2017, $50): 540p 30FPS
    Basic,          ///< Low-end (2017-2020, $150): 720p 30-40FPS
    Standard,       ///< Mid-range (2020-2023, $300): 1080p 60FPS
    High,           ///< High-end (2023-2024, $600): 1440p 60-120FPS
    Ultra           ///< Flagship (2024+, $1000+): 4K 120FPS RT
};

/**
 * @brief Get quality tier name
 */
constexpr const char* getQualityTierName(QualityTier tier) {
    switch (tier) {
        case QualityTier::Minimal: return "Minimal";
        case QualityTier::Basic: return "Basic";
        case QualityTier::Standard: return "Standard";
        case QualityTier::High: return "High";
        case QualityTier::Ultra: return "Ultra";
    }
    return "Unknown";
}

// ============================================================================
// Texture Formats
// ============================================================================

/**
 * @brief Texture/image formats
 */
enum class TextureFormat : u16 {
    Unknown = 0,
    
    // 8-bit formats
    R8_UNORM,
    R8_SNORM,
    R8_UINT,
    R8_SINT,
    
    // 16-bit formats
    R16_FLOAT,
    R16_UINT,
    R16_SINT,
    RG8_UNORM,
    RG8_SNORM,
    
    // 32-bit formats
    R32_FLOAT,
    R32_UINT,
    R32_SINT,
    RG16_FLOAT,
    RG16_UINT,
    RGBA8_UNORM,
    RGBA8_SRGB,
    RGBA8_SNORM,
    BGRA8_UNORM,
    BGRA8_SRGB,
    RGB10A2_UNORM,
    RG11B10_FLOAT,
    
    // 64-bit formats
    RG32_FLOAT,
    RGBA16_FLOAT,
    RGBA16_UNORM,
    
    // 128-bit formats
    RGBA32_FLOAT,
    
    // Depth/stencil formats
    D16_UNORM,
    D24_UNORM_S8_UINT,
    D32_FLOAT,
    D32_FLOAT_S8_UINT,
    
    // Compressed formats
    BC1_UNORM,          // DXT1
    BC1_SRGB,
    BC2_UNORM,          // DXT3
    BC2_SRGB,
    BC3_UNORM,          // DXT5
    BC3_SRGB,
    BC4_UNORM,
    BC4_SNORM,
    BC5_UNORM,
    BC5_SNORM,
    BC6H_UFLOAT,
    BC6H_SFLOAT,
    BC7_UNORM,
    BC7_SRGB,
    
    // Mobile compressed formats
    ETC2_RGB8_UNORM,
    ETC2_RGB8_SRGB,
    ETC2_RGBA8_UNORM,
    ETC2_RGBA8_SRGB,
    ASTC_4x4_UNORM,
    ASTC_4x4_SRGB,
    ASTC_5x5_UNORM,
    ASTC_5x5_SRGB,
    ASTC_6x6_UNORM,
    ASTC_6x6_SRGB,
    ASTC_8x8_UNORM,
    ASTC_8x8_SRGB,
};

/**
 * @brief Get bytes per pixel for a format
 */
[[nodiscard]] constexpr u32 getFormatBytesPerPixel(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8_UNORM:
        case TextureFormat::R8_SNORM:
        case TextureFormat::R8_UINT:
        case TextureFormat::R8_SINT:
            return 1;
            
        case TextureFormat::R16_FLOAT:
        case TextureFormat::R16_UINT:
        case TextureFormat::R16_SINT:
        case TextureFormat::RG8_UNORM:
        case TextureFormat::RG8_SNORM:
        case TextureFormat::D16_UNORM:
            return 2;
            
        case TextureFormat::D24_UNORM_S8_UINT:
        case TextureFormat::R32_FLOAT:
        case TextureFormat::R32_UINT:
        case TextureFormat::R32_SINT:
        case TextureFormat::RG16_FLOAT:
        case TextureFormat::RG16_UINT:
        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::RGBA8_SRGB:
        case TextureFormat::RGBA8_SNORM:
        case TextureFormat::BGRA8_UNORM:
        case TextureFormat::BGRA8_SRGB:
        case TextureFormat::RGB10A2_UNORM:
        case TextureFormat::RG11B10_FLOAT:
        case TextureFormat::D32_FLOAT:
            return 4;
            
        case TextureFormat::D32_FLOAT_S8_UINT:
            return 5;
            
        case TextureFormat::RG32_FLOAT:
        case TextureFormat::RGBA16_FLOAT:
        case TextureFormat::RGBA16_UNORM:
            return 8;
            
        case TextureFormat::RGBA32_FLOAT:
            return 16;
            
        default:
            return 0; // Compressed or unknown
    }
}

/**
 * @brief Check if format is a depth format
 */
[[nodiscard]] constexpr bool isDepthFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::D16_UNORM:
        case TextureFormat::D24_UNORM_S8_UINT:
        case TextureFormat::D32_FLOAT:
        case TextureFormat::D32_FLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Check if format has stencil
 */
[[nodiscard]] constexpr bool hasStencil(TextureFormat format) {
    switch (format) {
        case TextureFormat::D24_UNORM_S8_UINT:
        case TextureFormat::D32_FLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Check if format is sRGB
 */
[[nodiscard]] constexpr bool isSRGB(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_SRGB:
        case TextureFormat::BGRA8_SRGB:
        case TextureFormat::BC1_SRGB:
        case TextureFormat::BC2_SRGB:
        case TextureFormat::BC3_SRGB:
        case TextureFormat::BC7_SRGB:
        case TextureFormat::ETC2_RGB8_SRGB:
        case TextureFormat::ETC2_RGBA8_SRGB:
        case TextureFormat::ASTC_4x4_SRGB:
        case TextureFormat::ASTC_5x5_SRGB:
        case TextureFormat::ASTC_6x6_SRGB:
        case TextureFormat::ASTC_8x8_SRGB:
            return true;
        default:
            return false;
    }
}

// ============================================================================
// Primitive Topology
// ============================================================================

/**
 * @brief Primitive topology for rendering
 */
enum class PrimitiveTopology : u8 {
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan,
    LineListWithAdjacency,
    LineStripWithAdjacency,
    TriangleListWithAdjacency,
    TriangleStripWithAdjacency,
    PatchList
};

// ============================================================================
// Blend State
// ============================================================================

/**
 * @brief Blend factor
 */
enum class BlendFactor : u8 {
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
    Src1Color,
    OneMinusSrc1Color,
    Src1Alpha,
    OneMinusSrc1Alpha
};

/**
 * @brief Blend operation
 */
enum class BlendOp : u8 {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

/**
 * @brief Color component flags
 */
enum class ColorComponent : u8 {
    None = 0,
    R = 1 << 0,
    G = 1 << 1,
    B = 1 << 2,
    A = 1 << 3,
    RGB = R | G | B,
    RGBA = R | G | B | A
};

/**
 * @brief Blend state for a single render target
 */
struct BlendState {
    bool blendEnable = false;
    BlendFactor srcColorBlendFactor = BlendFactor::One;
    BlendFactor dstColorBlendFactor = BlendFactor::Zero;
    BlendOp colorBlendOp = BlendOp::Add;
    BlendFactor srcAlphaBlendFactor = BlendFactor::One;
    BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;
    BlendOp alphaBlendOp = BlendOp::Add;
    ColorComponent colorWriteMask = ColorComponent::RGBA;
    
    /// Create opaque blend state (no blending)
    static BlendState opaque() { return {}; }
    
    /// Create alpha blend state
    static BlendState alphaBlend() {
        BlendState state;
        state.blendEnable = true;
        state.srcColorBlendFactor = BlendFactor::SrcAlpha;
        state.dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
        state.srcAlphaBlendFactor = BlendFactor::One;
        state.dstAlphaBlendFactor = BlendFactor::OneMinusSrcAlpha;
        return state;
    }
    
    /// Create additive blend state
    static BlendState additive() {
        BlendState state;
        state.blendEnable = true;
        state.srcColorBlendFactor = BlendFactor::One;
        state.dstColorBlendFactor = BlendFactor::One;
        return state;
    }
};

// ============================================================================
// Depth/Stencil State
// ============================================================================

/**
 * @brief Comparison function
 */
enum class CompareOp : u8 {
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
};

/**
 * @brief Stencil operation
 */
enum class StencilOp : u8 {
    Keep,
    Zero,
    Replace,
    IncrementClamp,
    DecrementClamp,
    Invert,
    IncrementWrap,
    DecrementWrap
};

/**
 * @brief Stencil operation state
 */
struct StencilOpState {
    StencilOp failOp = StencilOp::Keep;
    StencilOp passOp = StencilOp::Keep;
    StencilOp depthFailOp = StencilOp::Keep;
    CompareOp compareOp = CompareOp::Always;
    u32 compareMask = 0xFF;
    u32 writeMask = 0xFF;
    u32 reference = 0;
};

/**
 * @brief Depth/stencil state
 */
struct DepthStencilState {
    bool depthTestEnable = true;
    bool depthWriteEnable = true;
    CompareOp depthCompareOp = CompareOp::Less; // Reversed-Z uses Greater
    bool depthBoundsTestEnable = false;
    bool stencilTestEnable = false;
    StencilOpState front;
    StencilOpState back;
    f32 minDepthBounds = 0.0f;
    f32 maxDepthBounds = 1.0f;
    
    /// Create default depth state (write + test)
    static DepthStencilState enabled() { return {}; }
    
    /// Create depth state with reversed-Z (better precision)
    static DepthStencilState reversedZ() {
        DepthStencilState state;
        state.depthCompareOp = CompareOp::Greater;
        return state;
    }
    
    /// Create read-only depth state
    static DepthStencilState readOnly() {
        DepthStencilState state;
        state.depthWriteEnable = false;
        return state;
    }
    
    /// Create disabled depth state
    static DepthStencilState disabled() {
        DepthStencilState state;
        state.depthTestEnable = false;
        state.depthWriteEnable = false;
        return state;
    }
};

// ============================================================================
// Rasterizer State
// ============================================================================

/**
 * @brief Polygon fill mode
 */
enum class PolygonMode : u8 {
    Fill,
    Line,
    Point
};

/**
 * @brief Cull mode
 */
enum class CullMode : u8 {
    None,
    Front,
    Back,
    FrontAndBack
};

/**
 * @brief Front face winding
 */
enum class FrontFace : u8 {
    CounterClockwise,
    Clockwise
};

/**
 * @brief Rasterizer state
 */
struct RasterizerState {
    PolygonMode polygonMode = PolygonMode::Fill;
    CullMode cullMode = CullMode::Back;
    FrontFace frontFace = FrontFace::CounterClockwise;
    bool depthClampEnable = false;
    bool rasterizerDiscardEnable = false;
    bool depthBiasEnable = false;
    f32 depthBiasConstantFactor = 0.0f;
    f32 depthBiasClamp = 0.0f;
    f32 depthBiasSlopeFactor = 0.0f;
    f32 lineWidth = 1.0f;
    
    /// Create default rasterizer state
    static RasterizerState defaults() { return {}; }
    
    /// Create wireframe rasterizer state
    static RasterizerState wireframe() {
        RasterizerState state;
        state.polygonMode = PolygonMode::Line;
        state.cullMode = CullMode::None;
        return state;
    }
    
    /// Create double-sided rasterizer state
    static RasterizerState doubleSided() {
        RasterizerState state;
        state.cullMode = CullMode::None;
        return state;
    }
};

// ============================================================================
// Sampler State
// ============================================================================

/**
 * @brief Texture filter mode
 */
enum class FilterMode : u8 {
    Nearest,
    Linear
};

/**
 * @brief Sampler mipmap mode
 */
enum class MipmapMode : u8 {
    Nearest,
    Linear
};

/**
 * @brief Texture address/wrap mode
 */
enum class AddressMode : u8 {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge
};

/**
 * @brief Border color
 */
enum class BorderColor : u8 {
    TransparentBlack,
    OpaqueBlack,
    OpaqueWhite
};

/**
 * @brief Sampler state descriptor
 */
struct SamplerDesc {
    FilterMode minFilter = FilterMode::Linear;
    FilterMode magFilter = FilterMode::Linear;
    MipmapMode mipmapMode = MipmapMode::Linear;
    AddressMode addressModeU = AddressMode::Repeat;
    AddressMode addressModeV = AddressMode::Repeat;
    AddressMode addressModeW = AddressMode::Repeat;
    f32 mipLodBias = 0.0f;
    bool anisotropyEnable = true;
    f32 maxAnisotropy = 16.0f;
    bool compareEnable = false;
    CompareOp compareOp = CompareOp::Always;
    f32 minLod = 0.0f;
    f32 maxLod = 1000.0f;
    BorderColor borderColor = BorderColor::TransparentBlack;
    
    /// Create default sampler (trilinear filtering)
    static SamplerDesc defaults() { return {}; }
    
    /// Create point sampler (no filtering)
    static SamplerDesc point() {
        SamplerDesc desc;
        desc.minFilter = FilterMode::Nearest;
        desc.magFilter = FilterMode::Nearest;
        desc.mipmapMode = MipmapMode::Nearest;
        desc.anisotropyEnable = false;
        return desc;
    }
    
    /// Create shadow sampler
    static SamplerDesc shadow() {
        SamplerDesc desc;
        desc.compareEnable = true;
        desc.compareOp = CompareOp::Less;
        desc.addressModeU = AddressMode::ClampToBorder;
        desc.addressModeV = AddressMode::ClampToBorder;
        desc.borderColor = BorderColor::OpaqueWhite;
        return desc;
    }
};

// ============================================================================
// Viewport and Scissor
// ============================================================================

/**
 * @brief Viewport definition
 */
struct Viewport {
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 width = 0.0f;
    f32 height = 0.0f;
    f32 minDepth = 0.0f;
    f32 maxDepth = 1.0f;
    
    Viewport() = default;
    Viewport(f32 w, f32 h) : width(w), height(h) {}
    Viewport(f32 xPos, f32 yPos, f32 w, f32 h) : x(xPos), y(yPos), width(w), height(h) {}
};

/**
 * @brief Scissor rectangle
 */
struct Scissor {
    i32 x = 0;
    i32 y = 0;
    u32 width = 0;
    u32 height = 0;
    
    Scissor() = default;
    Scissor(u32 w, u32 h) : width(w), height(h) {}
    Scissor(i32 xPos, i32 yPos, u32 w, u32 h) : x(xPos), y(yPos), width(w), height(h) {}
};

// ============================================================================
// Clear Values
// ============================================================================

/**
 * @brief Clear color value
 */
struct ClearColor {
    f32 r = 0.0f;
    f32 g = 0.0f;
    f32 b = 0.0f;
    f32 a = 1.0f;
    
    ClearColor() = default;
    ClearColor(f32 red, f32 green, f32 blue, f32 alpha = 1.0f) 
        : r(red), g(green), b(blue), a(alpha) {}
    
    static ClearColor black() { return {0.0f, 0.0f, 0.0f, 1.0f}; }
    static ClearColor white() { return {1.0f, 1.0f, 1.0f, 1.0f}; }
    static ClearColor cornflowerBlue() { return {0.392f, 0.584f, 0.929f, 1.0f}; }
};

/**
 * @brief Clear depth/stencil value
 */
struct ClearDepthStencil {
    f32 depth = 1.0f;
    u32 stencil = 0;
    
    ClearDepthStencil() = default;
    ClearDepthStencil(f32 depthValue, u32 stencilValue = 0) 
        : depth(depthValue), stencil(stencilValue) {}
    
    /// Reversed-Z clear value (0 = far)
    static ClearDepthStencil reversedZ() { return {0.0f, 0}; }
};

// ============================================================================
// GPU Resource Handles
// ============================================================================

/**
 * @brief Type-safe GPU resource handle
 */
template<typename Tag>
class ResourceHandle {
private:
    u64 m_id = 0;
    
public:
    constexpr ResourceHandle() = default;
    constexpr explicit ResourceHandle(u64 id) : m_id(id) {}
    
    [[nodiscard]] constexpr bool isValid() const { return m_id != 0; }
    [[nodiscard]] constexpr u64 id() const { return m_id; }
    
    constexpr bool operator==(const ResourceHandle& other) const { return m_id == other.m_id; }
    constexpr bool operator!=(const ResourceHandle& other) const { return m_id != other.m_id; }
    constexpr explicit operator bool() const { return isValid(); }
    
    static constexpr ResourceHandle invalid() { return {}; }
};

// Resource handle types
struct BufferTag {};
struct TextureTag {};
struct SamplerTag {};
struct ShaderTag {};
struct PipelineTag {};
struct RenderPassTag {};
struct FramebufferTag {};
struct DescriptorSetTag {};

using BufferHandle = ResourceHandle<BufferTag>;
using TextureHandle = ResourceHandle<TextureTag>;
using SamplerHandle = ResourceHandle<SamplerTag>;
using ShaderHandle = ResourceHandle<ShaderTag>;
using PipelineHandle = ResourceHandle<PipelineTag>;
using RenderPassHandle = ResourceHandle<RenderPassTag>;
using FramebufferHandle = ResourceHandle<FramebufferTag>;
using DescriptorSetHandle = ResourceHandle<DescriptorSetTag>;

} // namespace nova::render
