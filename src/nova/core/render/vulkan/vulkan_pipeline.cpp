/**
 * @file vulkan_pipeline.cpp
 * @brief Nova GraphicsCore™ - Vulkan pipeline implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_pipeline.hpp"

namespace nova::render::vulkan {

// ============================================================================
// VulkanGraphicsPipeline
// ============================================================================

Result<std::unique_ptr<VulkanGraphicsPipeline>> VulkanGraphicsPipeline::create(
    VulkanDevice& device,
    const GraphicsPipelineDesc& desc,
    VkRenderPass renderPass
) {
    auto pipeline = std::unique_ptr<VulkanGraphicsPipeline>(
        new VulkanGraphicsPipeline(device));
    
    pipeline->m_name = desc.name;
    
    // Create pipeline layout first
    auto layoutResult = pipeline->createLayout();
    if (!layoutResult) {
        return std::unexpected(layoutResult.error());
    }
    
    // Create the pipeline
    auto pipelineResult = pipeline->createPipeline(desc, renderPass);
    if (!pipelineResult) {
        return std::unexpected(pipelineResult.error());
    }
    
    return pipeline;
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice& device)
    : m_device(device)
{
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    if (m_pipeline) {
        funcs.vkDestroyPipeline(m_device.getDevice(), m_pipeline, nullptr);
    }
    
    if (m_layout) {
        funcs.vkDestroyPipelineLayout(m_device.getDevice(), m_layout, nullptr);
    }
}

Result<void> VulkanGraphicsPipeline::createLayout() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Create a simple pipeline layout (no descriptor sets or push constants for now)
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pSetLayouts = nullptr;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;
    
    VkResult result = funcs.vkCreatePipelineLayout(
        m_device.getDevice(), &layoutInfo, nullptr, &m_layout);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create pipeline layout: " + std::string(vkResultToString(result))));
    }
    
    return {};
}

Result<void> VulkanGraphicsPipeline::createPipeline(
    const GraphicsPipelineDesc& desc,
    VkRenderPass renderPass
) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Vertex input bindings
    std::vector<VkVertexInputBindingDescription> bindingDescs;
    bindingDescs.reserve(desc.vertexInput.bindings.size());
    
    for (const auto& binding : desc.vertexInput.bindings) {
        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = binding.binding;
        bindingDesc.stride = binding.stride;
        bindingDesc.inputRate = (binding.inputRate == VertexInputRate::Instance)
            ? VK_VERTEX_INPUT_RATE_INSTANCE
            : VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescs.push_back(bindingDesc);
    }
    
    // Vertex attributes
    std::vector<VkVertexInputAttributeDescription> attributeDescs;
    attributeDescs.reserve(desc.vertexInput.attributes.size());
    
    for (const auto& attr : desc.vertexInput.attributes) {
        VkVertexInputAttributeDescription attrDesc{};
        attrDesc.location = attr.location;
        attrDesc.binding = attr.binding;
        attrDesc.format = toVkVertexFormat(attr.format);
        attrDesc.offset = attr.offset;
        attributeDescs.push_back(attrDesc);
    }
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<u32>(bindingDescs.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescs.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescs.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = toVkPrimitiveTopology(desc.topology);
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // Viewport state (dynamic)
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = desc.viewportCount;
    viewportState.pViewports = nullptr; // Dynamic
    viewportState.scissorCount = desc.scissorCount;
    viewportState.pScissors = nullptr; // Dynamic
    
    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = desc.rasterizer.depthClampEnable ? VK_TRUE : VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = toVkPolygonMode(desc.rasterizer.polygonMode);
    rasterizer.cullMode = toVkCullMode(desc.rasterizer.cullMode);
    rasterizer.frontFace = toVkFrontFace(desc.rasterizer.frontFace);
    rasterizer.depthBiasEnable = desc.rasterizer.depthBiasEnable ? VK_TRUE : VK_FALSE;
    rasterizer.depthBiasConstantFactor = desc.rasterizer.depthBiasConstantFactor;
    rasterizer.depthBiasSlopeFactor = desc.rasterizer.depthBiasSlopeFactor;
    rasterizer.depthBiasClamp = desc.rasterizer.depthBiasClamp;
    rasterizer.lineWidth = desc.rasterizer.lineWidth;
    
    // Multisampling (disabled for now)
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    // Depth/stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = desc.depthStencil.depthTestEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = desc.depthStencil.depthWriteEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = toVkCompareOp(desc.depthStencil.depthCompareOp);
    depthStencil.depthBoundsTestEnable = desc.depthStencil.depthBoundsTestEnable ? VK_TRUE : VK_FALSE;
    depthStencil.stencilTestEnable = desc.depthStencil.stencilTestEnable ? VK_TRUE : VK_FALSE;
    depthStencil.minDepthBounds = desc.depthStencil.minDepthBounds;
    depthStencil.maxDepthBounds = desc.depthStencil.maxDepthBounds;
    
    // Color blend attachments
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    colorBlendAttachments.reserve(desc.blendStates.size());
    
    for (const auto& blend : desc.blendStates) {
        VkPipelineColorBlendAttachmentState attachment{};
        attachment.blendEnable = blend.blendEnable ? VK_TRUE : VK_FALSE;
        attachment.srcColorBlendFactor = toVkBlendFactor(blend.srcColorBlendFactor);
        attachment.dstColorBlendFactor = toVkBlendFactor(blend.dstColorBlendFactor);
        attachment.colorBlendOp = toVkBlendOp(blend.colorBlendOp);
        attachment.srcAlphaBlendFactor = toVkBlendFactor(blend.srcAlphaBlendFactor);
        attachment.dstAlphaBlendFactor = toVkBlendFactor(blend.dstAlphaBlendFactor);
        attachment.alphaBlendOp = toVkBlendOp(blend.alphaBlendOp);
        attachment.colorWriteMask = static_cast<VkColorComponentFlags>(blend.colorWriteMask);
        colorBlendAttachments.push_back(attachment);
    }
    
    // Ensure at least one color blend attachment
    if (colorBlendAttachments.empty()) {
        VkPipelineColorBlendAttachmentState defaultAttachment{};
        defaultAttachment.blendEnable = VK_FALSE;
        defaultAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachments.push_back(defaultAttachment);
    }
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = static_cast<u32>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();
    
    // Dynamic state
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Shader stages - load from desc.shaders
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(desc.shaders.size());
    
    for (const auto& shaderInfo : desc.shaders) {
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        
        // Convert shader stage to Vulkan stage flag
        switch (shaderInfo.stage) {
            case ShaderStage::Vertex:
                stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case ShaderStage::Fragment:
                stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            case ShaderStage::Geometry:
                stageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
                break;
            case ShaderStage::TessellationControl:
                stageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                break;
            case ShaderStage::TessellationEvaluation:
                stageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                break;
            default:
                continue;  // Skip invalid stages
        }
        
        // Note: In a full implementation, shaderInfo.shader would be a handle to
        // a VkShaderModule that was created via createShader(). The module would
        // be retrieved from the device's shader resource map.
        stageInfo.module = VK_NULL_HANDLE;  // Would be looked up from shader handle
        stageInfo.pName = shaderInfo.entryPoint.empty() ? "main" : shaderInfo.entryPoint.c_str();
        
        shaderStages.push_back(stageInfo);
    }
    
    // Validate we have at least vertex and fragment shaders for graphics pipeline
    if (shaderStages.empty()) {
        return std::unexpected(errors::graphics(
            "Graphics pipeline requires at least vertex and fragment shaders"));
    }
    
    // Create pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<u32>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.empty() ? nullptr : shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = desc.subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    VkResult result = funcs.vkCreateGraphicsPipelines(
        m_device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create graphics pipeline: " + std::string(vkResultToString(result))));
    }
    
    return {};
}

VkFormat VulkanGraphicsPipeline::toVkVertexFormat(VertexFormat format) noexcept {
    switch (format) {
        case VertexFormat::Float:        return VK_FORMAT_R32_SFLOAT;
        case VertexFormat::Float2:       return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::Float3:       return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat::Float4:       return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexFormat::Int:          return VK_FORMAT_R32_SINT;
        case VertexFormat::Int2:         return VK_FORMAT_R32G32_SINT;
        case VertexFormat::Int3:         return VK_FORMAT_R32G32B32_SINT;
        case VertexFormat::Int4:         return VK_FORMAT_R32G32B32A32_SINT;
        case VertexFormat::UInt:         return VK_FORMAT_R32_UINT;
        case VertexFormat::UInt2:        return VK_FORMAT_R32G32_UINT;
        case VertexFormat::UInt3:        return VK_FORMAT_R32G32B32_UINT;
        case VertexFormat::UInt4:        return VK_FORMAT_R32G32B32A32_UINT;
        case VertexFormat::Half2:        return VK_FORMAT_R16G16_SFLOAT;
        case VertexFormat::Half4:        return VK_FORMAT_R16G16B16A16_SFLOAT;
        case VertexFormat::UByte4:       return VK_FORMAT_R8G8B8A8_UINT;
        case VertexFormat::UByte4Norm:   return VK_FORMAT_R8G8B8A8_UNORM;
        case VertexFormat::SByte4:       return VK_FORMAT_R8G8B8A8_SINT;
        case VertexFormat::SByte4Norm:   return VK_FORMAT_R8G8B8A8_SNORM;
        case VertexFormat::UShort2:      return VK_FORMAT_R16G16_UINT;
        case VertexFormat::UShort2Norm:  return VK_FORMAT_R16G16_UNORM;
        case VertexFormat::UShort4:      return VK_FORMAT_R16G16B16A16_UINT;
        case VertexFormat::UShort4Norm:  return VK_FORMAT_R16G16B16A16_UNORM;
        case VertexFormat::Short2:       return VK_FORMAT_R16G16_SINT;
        case VertexFormat::Short2Norm:   return VK_FORMAT_R16G16_SNORM;
        case VertexFormat::Short4:       return VK_FORMAT_R16G16B16A16_SINT;
        case VertexFormat::Short4Norm:   return VK_FORMAT_R16G16B16A16_SNORM;
        case VertexFormat::UInt1010102Norm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case VertexFormat::Int1010102Norm:  return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
        default:                         return VK_FORMAT_R32G32B32_SFLOAT;
    }
}

VkPrimitiveTopology VulkanGraphicsPipeline::toVkPrimitiveTopology(PrimitiveTopology topology) noexcept {
    switch (topology) {
        case PrimitiveTopology::PointList:                  return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList:                   return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip:                  return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::TriangleList:               return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip:              return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case PrimitiveTopology::LineListWithAdjacency:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::LineStripWithAdjacency:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleListWithAdjacency:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleStripWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
        case PrimitiveTopology::PatchList:                  return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        default:                                            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

// ============================================================================
// VulkanComputePipeline
// ============================================================================

Result<std::unique_ptr<VulkanComputePipeline>> VulkanComputePipeline::create(
    VulkanDevice& device,
    const ComputePipelineDesc& desc
) {
    auto pipeline = std::unique_ptr<VulkanComputePipeline>(
        new VulkanComputePipeline(device));
    
    pipeline->m_name = desc.name;
    
    // Create pipeline layout
    auto layoutResult = pipeline->createLayout();
    if (!layoutResult) {
        return std::unexpected(layoutResult.error());
    }
    
    // Create the pipeline
    auto pipelineResult = pipeline->createPipeline(desc);
    if (!pipelineResult) {
        return std::unexpected(pipelineResult.error());
    }
    
    return pipeline;
}

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice& device)
    : m_device(device)
{
}

VulkanComputePipeline::~VulkanComputePipeline() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    if (m_pipeline) {
        funcs.vkDestroyPipeline(m_device.getDevice(), m_pipeline, nullptr);
    }
    
    if (m_layout) {
        funcs.vkDestroyPipelineLayout(m_device.getDevice(), m_layout, nullptr);
    }
}

Result<void> VulkanComputePipeline::createLayout() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pSetLayouts = nullptr;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;
    
    VkResult result = funcs.vkCreatePipelineLayout(
        m_device.getDevice(), &layoutInfo, nullptr, &m_layout);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create compute pipeline layout: " + std::string(vkResultToString(result))));
    }
    
    return {};
}

Result<void> VulkanComputePipeline::createPipeline(const ComputePipelineDesc& desc) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Validate compute shader is provided
    if (desc.shader.entryPoint.empty()) {
        return std::unexpected(errors::validation(
            "Compute pipeline requires a valid shader with entry point"));
    }
    
    // Shader stage - compute pipelines have exactly one shader stage
    VkPipelineShaderStageCreateInfo shaderStage{};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    
    // Note: In a full implementation, desc.shader would contain a handle to
    // a VkShaderModule that was created via createShader(). The module would
    // be retrieved from the device's shader resource map using the handle.
    shaderStage.module = VK_NULL_HANDLE;  // Would be looked up from shader handle
    shaderStage.pName = desc.shader.entryPoint.c_str();
    
    // Compute pipeline create info
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStage;
    pipelineInfo.layout = m_layout;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    VkResult result = funcs.vkCreateComputePipelines(
        m_device.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create compute pipeline: " + std::string(vkResultToString(result))));
    }
    
    return {};
}

// ============================================================================
// VulkanPipelineCache
// ============================================================================

Result<std::unique_ptr<VulkanPipelineCache>> VulkanPipelineCache::create(
    VulkanDevice& device,
    const void* initialData,
    u64 initialDataSize
) {
    auto cache = std::unique_ptr<VulkanPipelineCache>(
        new VulkanPipelineCache(device));
    
    const auto& funcs = device.getDeviceFuncs();
    
    VkPipelineCacheCreateInfo cacheInfo{};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    cacheInfo.initialDataSize = initialDataSize;
    cacheInfo.pInitialData = initialData;
    
    VkResult result = funcs.vkCreatePipelineCache(
        device.getDevice(), &cacheInfo, nullptr, &cache->m_cache);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create pipeline cache: " + std::string(vkResultToString(result))));
    }
    
    return cache;
}

VulkanPipelineCache::VulkanPipelineCache(VulkanDevice& device)
    : m_device(device)
{
}

VulkanPipelineCache::~VulkanPipelineCache() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    if (m_cache) {
        funcs.vkDestroyPipelineCache(m_device.getDevice(), m_cache, nullptr);
    }
}

Result<std::vector<u8>> VulkanPipelineCache::getData() const {
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Get the cache data size
    size_t dataSize = 0;
    VkResult result = funcs.vkGetPipelineCacheData(
        m_device.getDevice(), m_cache, &dataSize, nullptr);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to get pipeline cache data size: " + std::string(vkResultToString(result))));
    }
    
    // Get the cache data
    std::vector<u8> data(dataSize);
    result = funcs.vkGetPipelineCacheData(
        m_device.getDevice(), m_cache, &dataSize, data.data());
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to get pipeline cache data: " + std::string(vkResultToString(result))));
    }
    
    return data;
}

} // namespace nova::render::vulkan
