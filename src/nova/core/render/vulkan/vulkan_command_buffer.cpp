/**
 * @file vulkan_command_buffer.cpp
 * @brief Nova GraphicsCore™ - Vulkan command buffer implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_command_buffer.hpp"
#include <cstring>

namespace nova::render::vulkan {

// ============================================================================
// Static Factory
// ============================================================================

Result<std::unique_ptr<VulkanCommandBuffer>> VulkanCommandBuffer::create(
    VulkanDevice& device,
    CommandBufferType type,
    VkCommandPool pool
) {
    auto cmdBuffer = std::unique_ptr<VulkanCommandBuffer>(
        new VulkanCommandBuffer(device, type, pool));
    
    // Allocate the command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    const auto& funcs = device.getDeviceFuncs();
    VkResult result = funcs.vkAllocateCommandBuffers(
        device.getDevice(), &allocInfo, &cmdBuffer->m_commandBuffer);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to allocate command buffer: " + std::string(vkResultToString(result))));
    }
    
    return cmdBuffer;
}

VulkanCommandBuffer::VulkanCommandBuffer(
    VulkanDevice& device,
    CommandBufferType type,
    VkCommandPool pool
)
    : m_device(device)
    , m_pool(pool)
    , m_type(type)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    if (m_commandBuffer) {
        const auto& funcs = m_device.getDeviceFuncs();
        funcs.vkFreeCommandBuffers(m_device.getDevice(), m_pool, 1, &m_commandBuffer);
    }
}

// ============================================================================
// Recording Control
// ============================================================================

void VulkanCommandBuffer::begin() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;
    
    VkResult result = funcs.vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    if (result == VK_SUCCESS) {
        m_state = CommandBufferState::Recording;
    }
}

void VulkanCommandBuffer::end() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    VkResult result = funcs.vkEndCommandBuffer(m_commandBuffer);
    if (result == VK_SUCCESS) {
        m_state = CommandBufferState::Executable;
    } else {
        m_state = CommandBufferState::Invalid;
    }
}

void VulkanCommandBuffer::reset() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    VkResult result = funcs.vkResetCommandBuffer(m_commandBuffer, 0);
    if (result == VK_SUCCESS) {
        m_state = CommandBufferState::Initial;
        m_boundPipeline = VK_NULL_HANDLE;
        m_boundPipelineLayout = VK_NULL_HANDLE;
        m_inRenderPass = false;
    }
}

// ============================================================================
// Render Pass Commands
// ============================================================================

void VulkanCommandBuffer::beginRenderPass(const RenderPassBeginInfo& beginInfo) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Convert clear values
    std::vector<VkClearValue> clearValues;
    for (const auto& clear : beginInfo.clearColors) {
        VkClearValue vkClear{};
        vkClear.color.float32[0] = clear.r;
        vkClear.color.float32[1] = clear.g;
        vkClear.color.float32[2] = clear.b;
        vkClear.color.float32[3] = clear.a;
        clearValues.push_back(vkClear);
    }
    
    // Add depth/stencil clear value if needed
    VkClearValue depthClear{};
    depthClear.depthStencil.depth = beginInfo.clearDepthStencil.depth;
    depthClear.depthStencil.stencil = beginInfo.clearDepthStencil.stencil;
    clearValues.push_back(depthClear);
    
    VkRenderPassBeginInfo vkBeginInfo{};
    vkBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // Note: In full implementation, we would look up VkRenderPass and VkFramebuffer from handles
    // For now, these are stubs that will be populated when resource tracking is complete
    vkBeginInfo.renderPass = VK_NULL_HANDLE;  // TODO: Look up from beginInfo.renderPass
    vkBeginInfo.framebuffer = VK_NULL_HANDLE; // TODO: Look up from beginInfo.framebuffer
    vkBeginInfo.renderArea.offset.x = beginInfo.renderAreaX;
    vkBeginInfo.renderArea.offset.y = beginInfo.renderAreaY;
    vkBeginInfo.renderArea.extent.width = beginInfo.renderAreaWidth;
    vkBeginInfo.renderArea.extent.height = beginInfo.renderAreaHeight;
    vkBeginInfo.clearValueCount = static_cast<u32>(clearValues.size());
    vkBeginInfo.pClearValues = clearValues.data();
    
    funcs.vkCmdBeginRenderPass(m_commandBuffer, &vkBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    m_inRenderPass = true;
}

void VulkanCommandBuffer::endRenderPass() {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdEndRenderPass(m_commandBuffer);
    m_inRenderPass = false;
}

void VulkanCommandBuffer::nextSubpass() {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdNextSubpass(m_commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

// ============================================================================
// Pipeline State
// ============================================================================

void VulkanCommandBuffer::bindPipeline(PipelineHandle pipeline) {
    // TODO: Look up actual VkPipeline and VkPipelineLayout from handle
    // For now, this is a placeholder that will be fully implemented
    // when the pipeline cache system is complete
    
    // Determine bind point based on command buffer type
    VkPipelineBindPoint bindPoint = (m_type == CommandBufferType::Compute)
        ? VK_PIPELINE_BIND_POINT_COMPUTE
        : VK_PIPELINE_BIND_POINT_GRAPHICS;
    
    m_bindPoint = bindPoint;
    (void)pipeline; // Suppress unused warning until full implementation
    // funcs.vkCmdBindPipeline(m_commandBuffer, bindPoint, vkPipeline);
}

void VulkanCommandBuffer::setViewport(const Viewport& viewport) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    VkViewport vkViewport{};
    vkViewport.x = viewport.x;
    vkViewport.y = viewport.y;
    vkViewport.width = viewport.width;
    vkViewport.height = viewport.height;
    vkViewport.minDepth = viewport.minDepth;
    vkViewport.maxDepth = viewport.maxDepth;
    
    funcs.vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
}

void VulkanCommandBuffer::setViewports(std::span<const Viewport> viewports) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    std::vector<VkViewport> vkViewports;
    vkViewports.reserve(viewports.size());
    
    for (const auto& viewport : viewports) {
        VkViewport vkViewport{};
        vkViewport.x = viewport.x;
        vkViewport.y = viewport.y;
        vkViewport.width = viewport.width;
        vkViewport.height = viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;
        vkViewports.push_back(vkViewport);
    }
    
    funcs.vkCmdSetViewport(m_commandBuffer, 0, 
                           static_cast<u32>(vkViewports.size()), vkViewports.data());
}

void VulkanCommandBuffer::setScissor(const Scissor& scissor) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    VkRect2D vkScissor{};
    vkScissor.offset.x = scissor.x;
    vkScissor.offset.y = scissor.y;
    vkScissor.extent.width = scissor.width;
    vkScissor.extent.height = scissor.height;
    
    funcs.vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
}

void VulkanCommandBuffer::setScissors(std::span<const Scissor> scissors) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    std::vector<VkRect2D> vkScissors;
    vkScissors.reserve(scissors.size());
    
    for (const auto& scissor : scissors) {
        VkRect2D vkScissor{};
        vkScissor.offset.x = scissor.x;
        vkScissor.offset.y = scissor.y;
        vkScissor.extent.width = scissor.width;
        vkScissor.extent.height = scissor.height;
        vkScissors.push_back(vkScissor);
    }
    
    funcs.vkCmdSetScissor(m_commandBuffer, 0, 
                          static_cast<u32>(vkScissors.size()), vkScissors.data());
}

void VulkanCommandBuffer::setBlendConstants(f32 r, f32 g, f32 b, f32 a) {
    const auto& funcs = m_device.getDeviceFuncs();
    f32 blendConstants[4] = {r, g, b, a};
    funcs.vkCmdSetBlendConstants(m_commandBuffer, blendConstants);
}

void VulkanCommandBuffer::setDepthBounds(f32 minDepth, f32 maxDepth) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdSetDepthBounds(m_commandBuffer, minDepth, maxDepth);
}

void VulkanCommandBuffer::setStencilReference(u32 reference) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdSetStencilReference(m_commandBuffer, 
                                    VK_STENCIL_FACE_FRONT_AND_BACK, reference);
}

void VulkanCommandBuffer::setLineWidth(f32 width) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdSetLineWidth(m_commandBuffer, width);
}

// ============================================================================
// Resource Binding
// ============================================================================

void VulkanCommandBuffer::bindVertexBuffers(
    u32 firstBinding, 
    std::span<const VertexBufferBinding> bindings
) {
    // TODO: Implement when buffer resource tracking is complete
    // Will need to look up VkBuffer handles from BufferHandle
}

void VulkanCommandBuffer::bindVertexBuffer(u32 binding, BufferHandle buffer, u64 offset) {
    // TODO: Implement when buffer resource tracking is complete
}

void VulkanCommandBuffer::bindIndexBuffer(BufferHandle buffer, u64 offset, IndexType indexType) {
    // TODO: Implement when buffer resource tracking is complete
}

void VulkanCommandBuffer::bindDescriptorSets(
    u32 firstSet, 
    std::span<const DescriptorSetHandle> sets,
    std::span<const u32> dynamicOffsets
) {
    // TODO: Implement when descriptor set system is complete
}

void VulkanCommandBuffer::pushConstants(const PushConstantUpdate& update) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    if (m_boundPipelineLayout && update.data && update.size > 0) {
        funcs.vkCmdPushConstants(
            m_commandBuffer,
            m_boundPipelineLayout,
            VK_SHADER_STAGE_ALL,
            update.offset,
            update.size,
            update.data
        );
    }
}

// ============================================================================
// Draw Commands
// ============================================================================

void VulkanCommandBuffer::draw(const DrawParams& params) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdDraw(m_commandBuffer, 
                    params.vertexCount, params.instanceCount,
                    params.firstVertex, params.firstInstance);
}

void VulkanCommandBuffer::draw(u32 vertexCount, u32 instanceCount, 
                                u32 firstVertex, u32 firstInstance) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, 
                    firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndexed(const DrawIndexedParams& params) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdDrawIndexed(m_commandBuffer,
                           params.indexCount, params.instanceCount,
                           params.firstIndex, params.vertexOffset,
                           params.firstInstance);
}

void VulkanCommandBuffer::drawIndexed(u32 indexCount, u32 instanceCount,
                                       u32 firstIndex, i32 vertexOffset,
                                       u32 firstInstance) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount,
                           firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::drawIndirect(BufferHandle buffer, u64 offset, 
                                        u32 drawCount, u32 stride) {
    // TODO: Implement when buffer resource tracking is complete
}

void VulkanCommandBuffer::drawIndexedIndirect(BufferHandle buffer, u64 offset,
                                               u32 drawCount, u32 stride) {
    // TODO: Implement when buffer resource tracking is complete
}

void VulkanCommandBuffer::drawIndirectCount(BufferHandle buffer, u64 offset,
                                             BufferHandle countBuffer, u64 countOffset,
                                             u32 maxDrawCount, u32 stride) {
    // TODO: Implement when buffer resource tracking is complete
}

void VulkanCommandBuffer::drawIndexedIndirectCount(BufferHandle buffer, u64 offset,
                                                    BufferHandle countBuffer, u64 countOffset,
                                                    u32 maxDrawCount, u32 stride) {
    // TODO: Implement when buffer resource tracking is complete
}

// ============================================================================
// Compute Commands
// ============================================================================

void VulkanCommandBuffer::dispatch(const DispatchParams& params) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdDispatch(m_commandBuffer, 
                        params.groupCountX, params.groupCountY, params.groupCountZ);
}

void VulkanCommandBuffer::dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) {
    const auto& funcs = m_device.getDeviceFuncs();
    funcs.vkCmdDispatch(m_commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void VulkanCommandBuffer::dispatchIndirect(BufferHandle buffer, u64 offset) {
    // TODO: Implement when buffer resource tracking is complete
}

// ============================================================================
// Transfer Commands
// ============================================================================

void VulkanCommandBuffer::copyBuffer(BufferHandle src, BufferHandle dst,
                                      std::span<const BufferCopyRegion> regions) {
    // TODO: Implement when buffer resource tracking is complete
}

void VulkanCommandBuffer::copyBuffer(BufferHandle src, BufferHandle dst,
                                      u64 srcOffset, u64 dstOffset, u64 size) {
    // TODO: Implement when buffer resource tracking is complete
}

void VulkanCommandBuffer::copyTexture(TextureHandle src, TextureHandle dst,
                                       std::span<const ImageCopyRegion> regions) {
    // TODO: Implement when texture resource tracking is complete
}

void VulkanCommandBuffer::copyBufferToTexture(BufferHandle src, TextureHandle dst,
                                               std::span<const BufferImageCopyRegion> regions) {
    // TODO: Implement when resource tracking is complete
}

void VulkanCommandBuffer::copyTextureToBuffer(TextureHandle src, BufferHandle dst,
                                               std::span<const BufferImageCopyRegion> regions) {
    // TODO: Implement when resource tracking is complete
}

void VulkanCommandBuffer::fillBuffer(BufferHandle buffer, u64 offset, u64 size, u32 data) {
    // TODO: Implement when buffer resource tracking is complete
}

void VulkanCommandBuffer::updateBuffer(BufferHandle buffer, u64 offset, 
                                        const void* data, u64 size) {
    // TODO: Implement when buffer resource tracking is complete
}

// ============================================================================
// Synchronization
// ============================================================================

void VulkanCommandBuffer::pipelineBarrier(
    PipelineStage srcStage, PipelineStage dstStage,
    std::span<const MemoryBarrier> memoryBarriers,
    std::span<const BufferMemoryBarrier> bufferBarriers,
    std::span<const ImageMemoryBarrier> imageBarriers
) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Convert memory barriers
    std::vector<VkMemoryBarrier> vkMemoryBarriers;
    vkMemoryBarriers.reserve(memoryBarriers.size());
    for (const auto& barrier : memoryBarriers) {
        VkMemoryBarrier vkBarrier{};
        vkBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        vkBarrier.srcAccessMask = toVkAccessFlags(barrier.srcAccessMask);
        vkBarrier.dstAccessMask = toVkAccessFlags(barrier.dstAccessMask);
        vkMemoryBarriers.push_back(vkBarrier);
    }
    
    // Convert buffer barriers
    std::vector<VkBufferMemoryBarrier> vkBufferBarriers;
    vkBufferBarriers.reserve(bufferBarriers.size());
    for (const auto& barrier : bufferBarriers) {
        VkBufferMemoryBarrier vkBarrier{};
        vkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        vkBarrier.srcAccessMask = toVkAccessFlags(barrier.srcAccessMask);
        vkBarrier.dstAccessMask = toVkAccessFlags(barrier.dstAccessMask);
        vkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        vkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // vkBarrier.buffer = ... // TODO: Look up from handle
        vkBarrier.offset = barrier.offset;
        vkBarrier.size = barrier.size == 0 ? VK_WHOLE_SIZE : barrier.size;
        vkBufferBarriers.push_back(vkBarrier);
    }
    
    // Convert image barriers
    std::vector<VkImageMemoryBarrier> vkImageBarriers;
    vkImageBarriers.reserve(imageBarriers.size());
    for (const auto& barrier : imageBarriers) {
        VkImageMemoryBarrier vkBarrier{};
        vkBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vkBarrier.srcAccessMask = toVkAccessFlags(barrier.srcAccessMask);
        vkBarrier.dstAccessMask = toVkAccessFlags(barrier.dstAccessMask);
        vkBarrier.oldLayout = toVkImageLayout(barrier.oldLayout);
        vkBarrier.newLayout = toVkImageLayout(barrier.newLayout);
        vkBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        vkBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // vkBarrier.image = ... // TODO: Look up from handle
        vkBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vkBarrier.subresourceRange.baseMipLevel = barrier.baseMipLevel;
        vkBarrier.subresourceRange.levelCount = barrier.mipLevelCount;
        vkBarrier.subresourceRange.baseArrayLayer = barrier.baseArrayLayer;
        vkBarrier.subresourceRange.layerCount = barrier.arrayLayerCount;
        vkImageBarriers.push_back(vkBarrier);
    }
    
    funcs.vkCmdPipelineBarrier(
        m_commandBuffer,
        toVkStageFlags(srcStage),
        toVkStageFlags(dstStage),
        0,
        static_cast<u32>(vkMemoryBarriers.size()),
        vkMemoryBarriers.empty() ? nullptr : vkMemoryBarriers.data(),
        static_cast<u32>(vkBufferBarriers.size()),
        vkBufferBarriers.empty() ? nullptr : vkBufferBarriers.data(),
        static_cast<u32>(vkImageBarriers.size()),
        vkImageBarriers.empty() ? nullptr : vkImageBarriers.data()
    );
}

// ============================================================================
// Debug
// ============================================================================

void VulkanCommandBuffer::beginDebugLabel(const char* name, f32 r, f32 g, f32 b, f32 a) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    if (funcs.vkCmdBeginDebugUtilsLabelEXT) {
        VkDebugUtilsLabelEXT label{};
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name;
        label.color[0] = r;
        label.color[1] = g;
        label.color[2] = b;
        label.color[3] = a;
        funcs.vkCmdBeginDebugUtilsLabelEXT(m_commandBuffer, &label);
    }
}

void VulkanCommandBuffer::endDebugLabel() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    if (funcs.vkCmdEndDebugUtilsLabelEXT) {
        funcs.vkCmdEndDebugUtilsLabelEXT(m_commandBuffer);
    }
}

void VulkanCommandBuffer::insertDebugLabel(const char* name, f32 r, f32 g, f32 b, f32 a) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    if (funcs.vkCmdInsertDebugUtilsLabelEXT) {
        VkDebugUtilsLabelEXT label{};
        label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name;
        label.color[0] = r;
        label.color[1] = g;
        label.color[2] = b;
        label.color[3] = a;
        funcs.vkCmdInsertDebugUtilsLabelEXT(m_commandBuffer, &label);
    }
}

// ============================================================================
// Conversion Utilities
// ============================================================================

VkPipelineStageFlags VulkanCommandBuffer::toVkStageFlags(PipelineStage stage) noexcept {
    VkPipelineStageFlags flags = 0;
    
    if ((stage & PipelineStage::TopOfPipe) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    if ((stage & PipelineStage::DrawIndirect) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    if ((stage & PipelineStage::VertexInput) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    if ((stage & PipelineStage::VertexShader) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    if ((stage & PipelineStage::FragmentShader) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if ((stage & PipelineStage::EarlyFragmentTests) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    if ((stage & PipelineStage::LateFragmentTests) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    if ((stage & PipelineStage::ColorAttachmentOutput) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if ((stage & PipelineStage::ComputeShader) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    if ((stage & PipelineStage::Transfer) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    if ((stage & PipelineStage::BottomOfPipe) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    if ((stage & PipelineStage::Host) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_HOST_BIT;
    if ((stage & PipelineStage::AllGraphics) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    if ((stage & PipelineStage::AllCommands) != PipelineStage::None)
        flags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    
    if (flags != 0) {
        return flags;
    }
    return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}

VkAccessFlags VulkanCommandBuffer::toVkAccessFlags(Access access) noexcept {
    VkAccessFlags flags = 0;
    
    if ((access & Access::IndirectCommandRead) != Access::None)
        flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    if ((access & Access::IndexRead) != Access::None)
        flags |= VK_ACCESS_INDEX_READ_BIT;
    if ((access & Access::VertexAttributeRead) != Access::None)
        flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    if ((access & Access::UniformRead) != Access::None)
        flags |= VK_ACCESS_UNIFORM_READ_BIT;
    if ((access & Access::InputAttachmentRead) != Access::None)
        flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    if ((access & Access::ShaderRead) != Access::None)
        flags |= VK_ACCESS_SHADER_READ_BIT;
    if ((access & Access::ShaderWrite) != Access::None)
        flags |= VK_ACCESS_SHADER_WRITE_BIT;
    if ((access & Access::ColorAttachmentRead) != Access::None)
        flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    if ((access & Access::ColorAttachmentWrite) != Access::None)
        flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if ((access & Access::DepthStencilAttachmentRead) != Access::None)
        flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if ((access & Access::DepthStencilAttachmentWrite) != Access::None)
        flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if ((access & Access::TransferRead) != Access::None)
        flags |= VK_ACCESS_TRANSFER_READ_BIT;
    if ((access & Access::TransferWrite) != Access::None)
        flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    if ((access & Access::HostRead) != Access::None)
        flags |= VK_ACCESS_HOST_READ_BIT;
    if ((access & Access::HostWrite) != Access::None)
        flags |= VK_ACCESS_HOST_WRITE_BIT;
    if ((access & Access::MemoryRead) != Access::None)
        flags |= VK_ACCESS_MEMORY_READ_BIT;
    if ((access & Access::MemoryWrite) != Access::None)
        flags |= VK_ACCESS_MEMORY_WRITE_BIT;
    
    return flags;
}

VkImageLayout VulkanCommandBuffer::toVkImageLayout(ImageLayout layout) noexcept {
    switch (layout) {
        case ImageLayout::Undefined:                     return VK_IMAGE_LAYOUT_UNDEFINED;
        case ImageLayout::General:                       return VK_IMAGE_LAYOUT_GENERAL;
        case ImageLayout::ColorAttachment:               return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageLayout::DepthStencilAttachment:        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageLayout::DepthStencilReadOnly:          return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case ImageLayout::ShaderReadOnly:                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case ImageLayout::TransferSrc:                   return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageLayout::TransferDst:                   return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ImageLayout::Present:                       return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default:                                         return VK_IMAGE_LAYOUT_GENERAL;
    }
}

VkIndexType VulkanCommandBuffer::toVkIndexType(IndexType type) noexcept {
    switch (type) {
        case IndexType::UInt16: return VK_INDEX_TYPE_UINT16;
        case IndexType::UInt32: return VK_INDEX_TYPE_UINT32;
        default:                return VK_INDEX_TYPE_UINT16;
    }
}

} // namespace nova::render::vulkan
