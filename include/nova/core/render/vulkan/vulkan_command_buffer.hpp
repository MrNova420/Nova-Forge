/**
 * @file vulkan_command_buffer.hpp
 * @brief Nova GraphicsCore™ - Vulkan command buffer implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * This file provides the Vulkan command buffer implementation for recording
 * GPU commands for rendering, compute, and transfer operations.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "vulkan_types.hpp"
#include "vulkan_device.hpp"
#include "nova/core/render/command_buffer.hpp"
#include <vector>

namespace nova::render::vulkan {

// Forward declaration
class VulkanResourceRegistry;

/**
 * @brief Nova GraphicsCore™ - Vulkan Command Buffer
 * 
 * Records GPU commands for later submission to queues.
 * Supports:
 * - Graphics, compute, and transfer operations
 * - Render pass recording
 * - Dynamic state updates
 * - Debug markers for profiling
 */
class VulkanCommandBuffer : public CommandBuffer {
public:
    /**
     * @brief Create a Vulkan command buffer
     * @param device The Vulkan device
     * @param type Command buffer type (Graphics, Compute, Transfer)
     * @param pool The command pool to allocate from
     * @param registry Resource registry for handle lookups (optional, can be nullptr for basic usage)
     * @return Result containing the command buffer or an error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanCommandBuffer>> create(
        VulkanDevice& device,
        CommandBufferType type,
        VkCommandPool pool,
        VulkanResourceRegistry* registry = nullptr
    );
    
    ~VulkanCommandBuffer() override;
    
    // Non-copyable, non-movable
    VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer(VulkanCommandBuffer&&) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) = delete;
    
    // =========================================================================
    // State
    // =========================================================================
    
    [[nodiscard]] CommandBufferType getType() const override { return m_type; }
    [[nodiscard]] CommandBufferState getState() const override { return m_state; }
    
    // =========================================================================
    // Recording Control
    // =========================================================================
    
    void begin() override;
    void end() override;
    void reset() override;
    
    // =========================================================================
    // Render Pass Commands
    // =========================================================================
    
    void beginRenderPass(const RenderPassBeginInfo& beginInfo) override;
    void endRenderPass() override;
    void nextSubpass() override;
    
    // =========================================================================
    // Pipeline State
    // =========================================================================
    
    void bindPipeline(PipelineHandle pipeline) override;
    void setViewport(const Viewport& viewport) override;
    void setViewports(std::span<const Viewport> viewports) override;
    void setScissor(const Scissor& scissor) override;
    void setScissors(std::span<const Scissor> scissors) override;
    void setBlendConstants(f32 r, f32 g, f32 b, f32 a) override;
    void setDepthBounds(f32 minDepth, f32 maxDepth) override;
    void setStencilReference(u32 reference) override;
    void setLineWidth(f32 width) override;
    
    // =========================================================================
    // Resource Binding
    // =========================================================================
    
    void bindVertexBuffers(u32 firstBinding, 
                           std::span<const VertexBufferBinding> bindings) override;
    void bindVertexBuffer(u32 binding, BufferHandle buffer, u64 offset = 0) override;
    void bindIndexBuffer(BufferHandle buffer, u64 offset, IndexType indexType) override;
    void bindDescriptorSets(u32 firstSet, 
                            std::span<const DescriptorSetHandle> sets,
                            std::span<const u32> dynamicOffsets = {}) override;
    void pushConstants(const PushConstantUpdate& update) override;
    
    // =========================================================================
    // Draw Commands
    // =========================================================================
    
    void draw(const DrawParams& params) override;
    void draw(u32 vertexCount, u32 instanceCount = 1, 
              u32 firstVertex = 0, u32 firstInstance = 0) override;
    void drawIndexed(const DrawIndexedParams& params) override;
    void drawIndexed(u32 indexCount, u32 instanceCount = 1,
                     u32 firstIndex = 0, i32 vertexOffset = 0,
                     u32 firstInstance = 0) override;
    void drawIndirect(BufferHandle buffer, u64 offset, 
                      u32 drawCount, u32 stride) override;
    void drawIndexedIndirect(BufferHandle buffer, u64 offset,
                             u32 drawCount, u32 stride) override;
    void drawIndirectCount(BufferHandle buffer, u64 offset,
                           BufferHandle countBuffer, u64 countOffset,
                           u32 maxDrawCount, u32 stride) override;
    void drawIndexedIndirectCount(BufferHandle buffer, u64 offset,
                                  BufferHandle countBuffer, u64 countOffset,
                                  u32 maxDrawCount, u32 stride) override;
    
    // =========================================================================
    // Compute Commands
    // =========================================================================
    
    void dispatch(const DispatchParams& params) override;
    void dispatch(u32 groupCountX, u32 groupCountY = 1, u32 groupCountZ = 1) override;
    void dispatchIndirect(BufferHandle buffer, u64 offset) override;
    
    // =========================================================================
    // Transfer Commands
    // =========================================================================
    
    void copyBuffer(BufferHandle src, BufferHandle dst,
                    std::span<const BufferCopyRegion> regions) override;
    void copyBuffer(BufferHandle src, BufferHandle dst,
                    u64 srcOffset, u64 dstOffset, u64 size) override;
    void copyTexture(TextureHandle src, TextureHandle dst,
                     std::span<const ImageCopyRegion> regions) override;
    void copyBufferToTexture(BufferHandle src, TextureHandle dst,
                             std::span<const BufferImageCopyRegion> regions) override;
    void copyTextureToBuffer(TextureHandle src, BufferHandle dst,
                             std::span<const BufferImageCopyRegion> regions) override;
    void fillBuffer(BufferHandle buffer, u64 offset, u64 size, u32 data) override;
    void updateBuffer(BufferHandle buffer, u64 offset, 
                      const void* data, u64 size) override;
    
    // =========================================================================
    // Synchronization
    // =========================================================================
    
    void pipelineBarrier(PipelineStage srcStage, PipelineStage dstStage,
                         std::span<const MemoryBarrier> memoryBarriers = {},
                         std::span<const BufferMemoryBarrier> bufferBarriers = {},
                         std::span<const ImageMemoryBarrier> imageBarriers = {}) override;
    
    // =========================================================================
    // Debug
    // =========================================================================
    
    void beginDebugLabel(const char* name, f32 r = 1.0f, f32 g = 1.0f, 
                         f32 b = 1.0f, f32 a = 1.0f) override;
    void endDebugLabel() override;
    void insertDebugLabel(const char* name, f32 r = 1.0f, f32 g = 1.0f,
                          f32 b = 1.0f, f32 a = 1.0f) override;
    
    // =========================================================================
    // Vulkan-Specific Accessors
    // =========================================================================
    
    [[nodiscard]] VkCommandBuffer getCommandBuffer() const noexcept { return m_commandBuffer; }
    
private:
    VulkanCommandBuffer(VulkanDevice& device, CommandBufferType type, VkCommandPool pool, 
                        VulkanResourceRegistry* registry);
    
    /**
     * @brief Convert pipeline stage to Vulkan stage flags
     */
    [[nodiscard]] static VkPipelineStageFlags toVkStageFlags(PipelineStage stage) noexcept;
    
    /**
     * @brief Convert access flags to Vulkan access flags
     */
    [[nodiscard]] static VkAccessFlags toVkAccessFlags(Access access) noexcept;
    
    /**
     * @brief Convert image layout to Vulkan image layout
     */
    [[nodiscard]] static VkImageLayout toVkImageLayout(ImageLayout layout) noexcept;
    
    /**
     * @brief Convert index type to Vulkan index type
     */
    [[nodiscard]] static VkIndexType toVkIndexType(IndexType type) noexcept;
    
    // =========================================================================
    // Member Variables
    // =========================================================================
    
    VulkanDevice& m_device;
    VulkanResourceRegistry* m_registry = nullptr;  ///< Resource registry for handle lookups
    VkCommandPool m_pool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    CommandBufferType m_type;
    CommandBufferState m_state = CommandBufferState::Initial;
    
    // Current bound state (for validation)
    VkPipeline m_boundPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_boundPipelineLayout = VK_NULL_HANDLE;
    VkPipelineBindPoint m_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    bool m_inRenderPass = false;
};

} // namespace nova::render::vulkan
