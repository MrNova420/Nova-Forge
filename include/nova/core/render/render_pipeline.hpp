/**
 * @file render_pipeline.hpp
 * @brief Graphics and compute pipeline descriptors
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include "shader.hpp"
#include <vector>
#include <string>

namespace nova::render {

/**
 * @brief Vertex input rate
 */
enum class VertexInputRate : u8 {
    Vertex,     ///< Per-vertex data
    Instance    ///< Per-instance data
};

/**
 * @brief Vertex attribute format
 */
enum class VertexFormat : u8 {
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    UInt,
    UInt2,
    UInt3,
    UInt4,
    Half2,
    Half4,
    UByte4,
    UByte4Norm,
    SByte4,
    SByte4Norm,
    UShort2,
    UShort2Norm,
    UShort4,
    UShort4Norm,
    Short2,
    Short2Norm,
    Short4,
    Short4Norm,
    UInt1010102Norm,
    Int1010102Norm
};

/**
 * @brief Get vertex format size in bytes
 */
[[nodiscard]] constexpr u32 getVertexFormatSize(VertexFormat format) {
    switch (format) {
        case VertexFormat::Float: return 4;
        case VertexFormat::Float2: return 8;
        case VertexFormat::Float3: return 12;
        case VertexFormat::Float4: return 16;
        case VertexFormat::Int:
        case VertexFormat::UInt: return 4;
        case VertexFormat::Int2:
        case VertexFormat::UInt2: return 8;
        case VertexFormat::Int3:
        case VertexFormat::UInt3: return 12;
        case VertexFormat::Int4:
        case VertexFormat::UInt4: return 16;
        case VertexFormat::Half2: return 4;
        case VertexFormat::Half4: return 8;
        case VertexFormat::UByte4:
        case VertexFormat::UByte4Norm:
        case VertexFormat::SByte4:
        case VertexFormat::SByte4Norm:
        case VertexFormat::UInt1010102Norm:
        case VertexFormat::Int1010102Norm: return 4;
        case VertexFormat::UShort2:
        case VertexFormat::UShort2Norm:
        case VertexFormat::Short2:
        case VertexFormat::Short2Norm: return 4;
        case VertexFormat::UShort4:
        case VertexFormat::UShort4Norm:
        case VertexFormat::Short4:
        case VertexFormat::Short4Norm: return 8;
    }
    return 0;
}

/**
 * @brief Vertex input binding description
 */
struct VertexInputBinding {
    u32 binding = 0;
    u32 stride = 0;
    VertexInputRate inputRate = VertexInputRate::Vertex;
};

/**
 * @brief Vertex attribute description
 */
struct VertexAttribute {
    u32 location = 0;
    u32 binding = 0;
    VertexFormat format = VertexFormat::Float3;
    u32 offset = 0;
};

/**
 * @brief Vertex input layout description
 */
struct VertexInputLayout {
    std::vector<VertexInputBinding> bindings;
    std::vector<VertexAttribute> attributes;
    
    /// Create a simple layout with position only
    static VertexInputLayout positionOnly() {
        VertexInputLayout layout;
        layout.bindings.push_back({0, 12, VertexInputRate::Vertex});
        layout.attributes.push_back({0, 0, VertexFormat::Float3, 0});
        return layout;
    }
    
    /// Create a layout with position and normal
    static VertexInputLayout positionNormal() {
        VertexInputLayout layout;
        layout.bindings.push_back({0, 24, VertexInputRate::Vertex});
        layout.attributes.push_back({0, 0, VertexFormat::Float3, 0});
        layout.attributes.push_back({1, 0, VertexFormat::Float3, 12});
        return layout;
    }
    
    /// Create a layout with position, normal, and UV
    static VertexInputLayout positionNormalUV() {
        VertexInputLayout layout;
        layout.bindings.push_back({0, 32, VertexInputRate::Vertex});
        layout.attributes.push_back({0, 0, VertexFormat::Float3, 0});
        layout.attributes.push_back({1, 0, VertexFormat::Float3, 12});
        layout.attributes.push_back({2, 0, VertexFormat::Float2, 24});
        return layout;
    }
    
    /// Create a layout with position, normal, UV, and tangent
    static VertexInputLayout positionNormalUVTangent() {
        VertexInputLayout layout;
        layout.bindings.push_back({0, 48, VertexInputRate::Vertex});
        layout.attributes.push_back({0, 0, VertexFormat::Float3, 0});
        layout.attributes.push_back({1, 0, VertexFormat::Float3, 12});
        layout.attributes.push_back({2, 0, VertexFormat::Float2, 24});
        layout.attributes.push_back({3, 0, VertexFormat::Float4, 32});
        return layout;
    }
};

/**
 * @brief Graphics pipeline creation descriptor
 */
struct GraphicsPipelineDesc {
    /// Shader stages
    std::vector<ShaderStageInfo> shaders;
    
    /// Vertex input layout
    VertexInputLayout vertexInput;
    
    /// Primitive topology
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    
    /// Rasterizer state
    RasterizerState rasterizer;
    
    /// Depth/stencil state
    DepthStencilState depthStencil;
    
    /// Blend states for each color attachment
    std::vector<BlendState> blendStates;
    
    /// Render pass this pipeline is compatible with
    RenderPassHandle renderPass;
    
    /// Subpass index within the render pass
    u32 subpass = 0;
    
    /// Number of viewports (for dynamic viewports)
    u32 viewportCount = 1;
    
    /// Number of scissors (for dynamic scissors)
    u32 scissorCount = 1;
    
    /// Debug name
    std::string name;
    
    // Factory methods
    static GraphicsPipelineDesc simple(ShaderHandle vertexShader, 
                                       ShaderHandle fragmentShader,
                                       RenderPassHandle renderPass) {
        GraphicsPipelineDesc desc;
        desc.shaders.push_back({vertexShader, ShaderStage::Vertex, "main"});
        desc.shaders.push_back({fragmentShader, ShaderStage::Fragment, "main"});
        desc.vertexInput = VertexInputLayout::positionOnly();
        desc.renderPass = renderPass;
        desc.blendStates.push_back(BlendState::opaque());
        return desc;
    }
};

/**
 * @brief Compute pipeline creation descriptor
 */
struct ComputePipelineDesc {
    /// Compute shader
    ShaderStageInfo shader;
    
    /// Debug name
    std::string name;
    
    static ComputePipelineDesc create(ShaderHandle computeShader) {
        ComputePipelineDesc desc;
        desc.shader = {computeShader, ShaderStage::Compute, "main"};
        return desc;
    }
};

} // namespace nova::render
