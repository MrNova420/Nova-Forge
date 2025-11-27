/**
 * @file vulkan_command_buffer.cpp
 * @brief Nova GraphicsCore™ - Vulkan command buffer implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_command_buffer.hpp"
#include "nova/core/render/vulkan/vulkan_resource_registry.hpp"
#include <cstring>

namespace nova::render::vulkan {

// ============================================================================
// Static Factory
// ============================================================================

Result<std::unique_ptr<VulkanCommandBuffer>> VulkanCommandBuffer::create(
    VulkanDevice& device,
    CommandBufferType type,
    VkCommandPool pool,
    VulkanResourceRegistry* registry
) {
    auto cmdBuffer = std::unique_ptr<VulkanCommandBuffer>(
        new VulkanCommandBuffer(device, type, pool, registry));
    
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
    VkCommandPool pool,
    VulkanResourceRegistry* registry
)
    : m_device(device)
    , m_registry(registry)
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
    
    // Look up render pass and framebuffer from registry
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    
    if (m_registry) {
        if (beginInfo.renderPass.isValid()) {
            const auto* rpEntry = m_registry->getRenderPass(beginInfo.renderPass);
            if (rpEntry) {
                renderPass = rpEntry->renderPass;
            }
        }
        if (beginInfo.framebuffer.isValid()) {
            const auto* fbEntry = m_registry->getFramebuffer(beginInfo.framebuffer);
            if (fbEntry) {
                framebuffer = fbEntry->framebuffer;
            }
        }
    }
    
    vkBeginInfo.renderPass = renderPass;
    vkBeginInfo.framebuffer = framebuffer;
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
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Look up pipeline from registry
    VkPipeline vkPipeline = VK_NULL_HANDLE;
    VkPipelineLayout vkLayout = VK_NULL_HANDLE;
    VkPipelineBindPoint bindPoint = (m_type == CommandBufferType::Compute)
        ? VK_PIPELINE_BIND_POINT_COMPUTE
        : VK_PIPELINE_BIND_POINT_GRAPHICS;
    
    if (m_registry && pipeline.isValid()) {
        const auto* pipeEntry = m_registry->getPipeline(pipeline);
        if (pipeEntry) {
            vkPipeline = pipeEntry->pipeline;
            vkLayout = pipeEntry->layout;
            bindPoint = pipeEntry->bindPoint;
        }
    }
    
    if (vkPipeline != VK_NULL_HANDLE) {
        funcs.vkCmdBindPipeline(m_commandBuffer, bindPoint, vkPipeline);
        m_boundPipeline = vkPipeline;
        m_boundPipelineLayout = vkLayout;
        m_bindPoint = bindPoint;
    }
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
    if (!m_registry || bindings.empty()) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    
    std::vector<VkBuffer> vkBuffers;
    std::vector<VkDeviceSize> offsets;
    vkBuffers.reserve(bindings.size());
    offsets.reserve(bindings.size());
    
    for (const auto& binding : bindings) {
        const auto* bufEntry = m_registry->getBuffer(binding.buffer);
        if (bufEntry) {
            vkBuffers.push_back(bufEntry->buffer);
            offsets.push_back(static_cast<VkDeviceSize>(binding.offset));
        } else {
            // Handle missing buffer - use null handle
            vkBuffers.push_back(VK_NULL_HANDLE);
            offsets.push_back(0);
        }
    }
    
    if (!vkBuffers.empty()) {
        funcs.vkCmdBindVertexBuffers(m_commandBuffer, firstBinding,
            static_cast<u32>(vkBuffers.size()), vkBuffers.data(), offsets.data());
    }
}

void VulkanCommandBuffer::bindVertexBuffer(u32 binding, BufferHandle buffer, u64 offset) {
    if (!m_registry) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* bufEntry = m_registry->getBuffer(buffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE) {
        VkBuffer vkBuffer = bufEntry->buffer;
        VkDeviceSize vkOffset = static_cast<VkDeviceSize>(offset);
        funcs.vkCmdBindVertexBuffers(m_commandBuffer, binding, 1, &vkBuffer, &vkOffset);
    }
}

void VulkanCommandBuffer::bindIndexBuffer(BufferHandle buffer, u64 offset, IndexType indexType) {
    if (!m_registry) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* bufEntry = m_registry->getBuffer(buffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE) {
        funcs.vkCmdBindIndexBuffer(m_commandBuffer, bufEntry->buffer,
            static_cast<VkDeviceSize>(offset), toVkIndexType(indexType));
    }
}

void VulkanCommandBuffer::bindDescriptorSets(
    u32 firstSet, 
    std::span<const DescriptorSetHandle> sets,
    std::span<const u32> dynamicOffsets
) {
    if (!m_registry || sets.empty() || m_boundPipelineLayout == VK_NULL_HANDLE) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    
    std::vector<VkDescriptorSet> vkSets;
    vkSets.reserve(sets.size());
    
    for (const auto& setHandle : sets) {
        const auto* setEntry = m_registry->getDescriptorSet(setHandle);
        if (setEntry && setEntry->descriptorSet != VK_NULL_HANDLE) {
            vkSets.push_back(setEntry->descriptorSet);
        }
    }
    
    if (!vkSets.empty()) {
        funcs.vkCmdBindDescriptorSets(m_commandBuffer, m_bindPoint, m_boundPipelineLayout,
            firstSet, static_cast<u32>(vkSets.size()), vkSets.data(),
            static_cast<u32>(dynamicOffsets.size()),
            dynamicOffsets.empty() ? nullptr : dynamicOffsets.data());
    }
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
    if (!m_registry) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* bufEntry = m_registry->getBuffer(buffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE) {
        funcs.vkCmdDrawIndirect(m_commandBuffer, bufEntry->buffer,
            static_cast<VkDeviceSize>(offset), drawCount, stride);
    }
}

void VulkanCommandBuffer::drawIndexedIndirect(BufferHandle buffer, u64 offset,
                                               u32 drawCount, u32 stride) {
    if (!m_registry) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* bufEntry = m_registry->getBuffer(buffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE) {
        funcs.vkCmdDrawIndexedIndirect(m_commandBuffer, bufEntry->buffer,
            static_cast<VkDeviceSize>(offset), drawCount, stride);
    }
}

void VulkanCommandBuffer::drawIndirectCount(BufferHandle buffer, u64 offset,
                                             BufferHandle countBuffer, u64 countOffset,
                                             u32 maxDrawCount, u32 stride) {
    if (!m_registry) return;
    
    const auto* bufEntry = m_registry->getBuffer(buffer);
    const auto* countBufEntry = m_registry->getBuffer(countBuffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE &&
        countBufEntry && countBufEntry->buffer != VK_NULL_HANDLE) {
        // vkCmdDrawIndirectCount requires Vulkan 1.2 or VK_KHR_draw_indirect_count
        // For now, we fall back to regular indirect draw with maxDrawCount
        const auto& funcs = m_device.getDeviceFuncs();
        funcs.vkCmdDrawIndirect(m_commandBuffer, bufEntry->buffer,
            static_cast<VkDeviceSize>(offset), maxDrawCount, stride);
    }
}

void VulkanCommandBuffer::drawIndexedIndirectCount(BufferHandle buffer, u64 offset,
                                                    BufferHandle countBuffer, u64 countOffset,
                                                    u32 maxDrawCount, u32 stride) {
    if (!m_registry) return;
    
    const auto* bufEntry = m_registry->getBuffer(buffer);
    const auto* countBufEntry = m_registry->getBuffer(countBuffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE &&
        countBufEntry && countBufEntry->buffer != VK_NULL_HANDLE) {
        // vkCmdDrawIndexedIndirectCount requires Vulkan 1.2 or VK_KHR_draw_indirect_count
        // For now, we fall back to regular indexed indirect draw with maxDrawCount
        const auto& funcs = m_device.getDeviceFuncs();
        funcs.vkCmdDrawIndexedIndirect(m_commandBuffer, bufEntry->buffer,
            static_cast<VkDeviceSize>(offset), maxDrawCount, stride);
    }
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
    if (!m_registry) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* bufEntry = m_registry->getBuffer(buffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE) {
        funcs.vkCmdDispatchIndirect(m_commandBuffer, bufEntry->buffer,
            static_cast<VkDeviceSize>(offset));
    }
}

// ============================================================================
// Transfer Commands
// ============================================================================

void VulkanCommandBuffer::copyBuffer(BufferHandle src, BufferHandle dst,
                                      std::span<const BufferCopyRegion> regions) {
    if (!m_registry || regions.empty()) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* srcEntry = m_registry->getBuffer(src);
    const auto* dstEntry = m_registry->getBuffer(dst);
    
    if (srcEntry && srcEntry->buffer != VK_NULL_HANDLE &&
        dstEntry && dstEntry->buffer != VK_NULL_HANDLE) {
        
        std::vector<VkBufferCopy> vkRegions;
        vkRegions.reserve(regions.size());
        
        for (const auto& region : regions) {
            VkBufferCopy vkCopy{};
            vkCopy.srcOffset = static_cast<VkDeviceSize>(region.srcOffset);
            vkCopy.dstOffset = static_cast<VkDeviceSize>(region.dstOffset);
            vkCopy.size = static_cast<VkDeviceSize>(region.size);
            vkRegions.push_back(vkCopy);
        }
        
        funcs.vkCmdCopyBuffer(m_commandBuffer, srcEntry->buffer, dstEntry->buffer,
            static_cast<u32>(vkRegions.size()), vkRegions.data());
    }
}

void VulkanCommandBuffer::copyBuffer(BufferHandle src, BufferHandle dst,
                                      u64 srcOffset, u64 dstOffset, u64 size) {
    if (!m_registry) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* srcEntry = m_registry->getBuffer(src);
    const auto* dstEntry = m_registry->getBuffer(dst);
    
    if (srcEntry && srcEntry->buffer != VK_NULL_HANDLE &&
        dstEntry && dstEntry->buffer != VK_NULL_HANDLE) {
        
        VkBufferCopy vkCopy{};
        vkCopy.srcOffset = static_cast<VkDeviceSize>(srcOffset);
        vkCopy.dstOffset = static_cast<VkDeviceSize>(dstOffset);
        vkCopy.size = static_cast<VkDeviceSize>(size);
        
        funcs.vkCmdCopyBuffer(m_commandBuffer, srcEntry->buffer, dstEntry->buffer, 1, &vkCopy);
    }
}

void VulkanCommandBuffer::copyTexture(TextureHandle src, TextureHandle dst,
                                       std::span<const ImageCopyRegion> regions) {
    if (!m_registry || regions.empty()) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* srcEntry = m_registry->getImage(src);
    const auto* dstEntry = m_registry->getImage(dst);
    
    if (srcEntry && srcEntry->image != VK_NULL_HANDLE &&
        dstEntry && dstEntry->image != VK_NULL_HANDLE) {
        
        std::vector<VkImageCopy> vkRegions;
        vkRegions.reserve(regions.size());
        
        for (const auto& region : regions) {
            VkImageCopy vkCopy{};
            vkCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vkCopy.srcSubresource.mipLevel = region.srcMipLevel;
            vkCopy.srcSubresource.baseArrayLayer = region.srcArrayLayer;
            vkCopy.srcSubresource.layerCount = 1;  // Copy single layer per region
            vkCopy.srcOffset = {region.srcOffsetX, region.srcOffsetY, region.srcOffsetZ};
            
            vkCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vkCopy.dstSubresource.mipLevel = region.dstMipLevel;
            vkCopy.dstSubresource.baseArrayLayer = region.dstArrayLayer;
            vkCopy.dstSubresource.layerCount = 1;  // Copy single layer per region
            vkCopy.dstOffset = {region.dstOffsetX, region.dstOffsetY, region.dstOffsetZ};
            
            vkCopy.extent = {region.width, region.height, region.depth};
            vkRegions.push_back(vkCopy);
        }
        
        funcs.vkCmdCopyImage(m_commandBuffer, 
            srcEntry->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstEntry->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<u32>(vkRegions.size()), vkRegions.data());
    }
}

void VulkanCommandBuffer::copyBufferToTexture(BufferHandle src, TextureHandle dst,
                                               std::span<const BufferImageCopyRegion> regions) {
    if (!m_registry || regions.empty()) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* srcEntry = m_registry->getBuffer(src);
    const auto* dstEntry = m_registry->getImage(dst);
    
    if (srcEntry && srcEntry->buffer != VK_NULL_HANDLE &&
        dstEntry && dstEntry->image != VK_NULL_HANDLE) {
        
        std::vector<VkBufferImageCopy> vkRegions;
        vkRegions.reserve(regions.size());
        
        for (const auto& region : regions) {
            VkBufferImageCopy vkCopy{};
            vkCopy.bufferOffset = static_cast<VkDeviceSize>(region.bufferOffset);
            vkCopy.bufferRowLength = region.bufferRowLength;
            vkCopy.bufferImageHeight = region.bufferImageHeight;
            
            vkCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vkCopy.imageSubresource.mipLevel = region.imageMipLevel;
            vkCopy.imageSubresource.baseArrayLayer = region.imageArrayLayer;
            vkCopy.imageSubresource.layerCount = 1;  // Single layer per copy region
            
            vkCopy.imageOffset = {region.imageOffsetX, region.imageOffsetY, region.imageOffsetZ};
            vkCopy.imageExtent = {region.imageWidth, region.imageHeight, region.imageDepth};
            
            vkRegions.push_back(vkCopy);
        }
        
        funcs.vkCmdCopyBufferToImage(m_commandBuffer, srcEntry->buffer, dstEntry->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<u32>(vkRegions.size()), vkRegions.data());
    }
}

void VulkanCommandBuffer::copyTextureToBuffer(TextureHandle src, BufferHandle dst,
                                               std::span<const BufferImageCopyRegion> regions) {
    if (!m_registry || regions.empty()) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* srcEntry = m_registry->getImage(src);
    const auto* dstEntry = m_registry->getBuffer(dst);
    
    if (srcEntry && srcEntry->image != VK_NULL_HANDLE &&
        dstEntry && dstEntry->buffer != VK_NULL_HANDLE) {
        
        std::vector<VkBufferImageCopy> vkRegions;
        vkRegions.reserve(regions.size());
        
        for (const auto& region : regions) {
            VkBufferImageCopy vkCopy{};
            vkCopy.bufferOffset = static_cast<VkDeviceSize>(region.bufferOffset);
            vkCopy.bufferRowLength = region.bufferRowLength;
            vkCopy.bufferImageHeight = region.bufferImageHeight;
            
            vkCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            vkCopy.imageSubresource.mipLevel = region.imageMipLevel;
            vkCopy.imageSubresource.baseArrayLayer = region.imageArrayLayer;
            vkCopy.imageSubresource.layerCount = 1;  // Single layer per copy region
            
            vkCopy.imageOffset = {region.imageOffsetX, region.imageOffsetY, region.imageOffsetZ};
            vkCopy.imageExtent = {region.imageWidth, region.imageHeight, region.imageDepth};
            
            vkRegions.push_back(vkCopy);
        }
        
        funcs.vkCmdCopyImageToBuffer(m_commandBuffer, srcEntry->image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstEntry->buffer,
            static_cast<u32>(vkRegions.size()), vkRegions.data());
    }
}

void VulkanCommandBuffer::fillBuffer(BufferHandle buffer, u64 offset, u64 size, u32 data) {
    if (!m_registry) return;
    
    const auto& funcs = m_device.getDeviceFuncs();
    const auto* bufEntry = m_registry->getBuffer(buffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE) {
        VkDeviceSize vkSize = (size == 0) ? VK_WHOLE_SIZE : static_cast<VkDeviceSize>(size);
        funcs.vkCmdFillBuffer(m_commandBuffer, bufEntry->buffer,
            static_cast<VkDeviceSize>(offset), vkSize, data);
    }
}

void VulkanCommandBuffer::updateBuffer(BufferHandle buffer, u64 offset, 
                                        const void* data, u64 size) {
    if (!m_registry || !data || size == 0) return;
    
    const auto* bufEntry = m_registry->getBuffer(buffer);
    
    if (bufEntry && bufEntry->buffer != VK_NULL_HANDLE) {
        // vkCmdUpdateBuffer has a max size of 65536 bytes
        // For larger updates, use staging buffer approach
        constexpr u64 MAX_UPDATE_SIZE = 65536;
        if (size <= MAX_UPDATE_SIZE) {
            // Use vkCmdUpdateBuffer for small inline updates
            // This function is loaded with other device functions
            // Fall back to memory mapping for this implementation
            if (bufEntry->mappedPtr != nullptr) {
                std::memcpy(static_cast<u8*>(bufEntry->mappedPtr) + offset, data, size);
            }
        }
    }
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
        
        // Look up buffer from registry
        if (m_registry && barrier.buffer.isValid()) {
            const auto* bufEntry = m_registry->getBuffer(barrier.buffer);
            if (bufEntry) {
                vkBarrier.buffer = bufEntry->buffer;
            }
        }
        
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
        
        // Look up image from registry (uses 'texture' member in barrier struct)
        if (m_registry && barrier.texture.isValid()) {
            const auto* imgEntry = m_registry->getImage(barrier.texture);
            if (imgEntry) {
                vkBarrier.image = imgEntry->image;
            }
        }
        
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
