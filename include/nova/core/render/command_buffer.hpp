/**
 * @file command_buffer.hpp
 * @brief Command buffer interface for recording GPU commands
 * 
 * Command buffers record GPU commands for later submission.
 * They provide a backend-agnostic interface for drawing, compute,
 * and transfer operations.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include "render_pass.hpp"
#include <span>
#include <functional>

namespace nova::render {

/**
 * @brief Command buffer type
 */
enum class CommandBufferType : u8 {
    Graphics,   ///< Graphics and compute commands
    Compute,    ///< Compute commands only
    Transfer    ///< Transfer commands only
};

/**
 * @brief Command buffer state
 */
enum class CommandBufferState : u8 {
    Initial,    ///< Not yet begun
    Recording,  ///< Currently recording commands
    Executable, ///< Ready for submission
    Pending,    ///< Submitted and pending execution
    Invalid     ///< Invalid state (needs reset)
};

/**
 * @brief Index type for indexed drawing
 */
enum class IndexType : u8 {
    UInt16,
    UInt32
};

/**
 * @brief Vertex input binding
 */
struct VertexBufferBinding {
    BufferHandle buffer;
    u64 offset = 0;
};

/**
 * @brief Push constant update
 */
struct PushConstantUpdate {
    const void* data = nullptr;
    u32 size = 0;
    u32 offset = 0;
};

/**
 * @brief Draw command parameters
 */
struct DrawParams {
    u32 vertexCount = 0;
    u32 instanceCount = 1;
    u32 firstVertex = 0;
    u32 firstInstance = 0;
};

/**
 * @brief Indexed draw command parameters
 */
struct DrawIndexedParams {
    u32 indexCount = 0;
    u32 instanceCount = 1;
    u32 firstIndex = 0;
    i32 vertexOffset = 0;
    u32 firstInstance = 0;
};

/**
 * @brief Indirect draw command (matches GPU layout)
 */
struct DrawIndirectCommand {
    u32 vertexCount;
    u32 instanceCount;
    u32 firstVertex;
    u32 firstInstance;
};

/**
 * @brief Indexed indirect draw command (matches GPU layout)
 */
struct DrawIndexedIndirectCommand {
    u32 indexCount;
    u32 instanceCount;
    u32 firstIndex;
    i32 vertexOffset;
    u32 firstInstance;
};

/**
 * @brief Dispatch compute parameters
 */
struct DispatchParams {
    u32 groupCountX = 1;
    u32 groupCountY = 1;
    u32 groupCountZ = 1;
};

/**
 * @brief Buffer copy region
 */
struct BufferCopyRegion {
    u64 srcOffset = 0;
    u64 dstOffset = 0;
    u64 size = 0;
};

/**
 * @brief Image copy region
 */
struct ImageCopyRegion {
    u32 srcMipLevel = 0;
    u32 srcArrayLayer = 0;
    i32 srcOffsetX = 0;
    i32 srcOffsetY = 0;
    i32 srcOffsetZ = 0;
    
    u32 dstMipLevel = 0;
    u32 dstArrayLayer = 0;
    i32 dstOffsetX = 0;
    i32 dstOffsetY = 0;
    i32 dstOffsetZ = 0;
    
    u32 width = 0;
    u32 height = 0;
    u32 depth = 1;
};

/**
 * @brief Buffer to image copy region
 */
struct BufferImageCopyRegion {
    u64 bufferOffset = 0;
    u32 bufferRowLength = 0;    // 0 = tightly packed
    u32 bufferImageHeight = 0;  // 0 = tightly packed
    
    u32 imageMipLevel = 0;
    u32 imageArrayLayer = 0;
    i32 imageOffsetX = 0;
    i32 imageOffsetY = 0;
    i32 imageOffsetZ = 0;
    
    u32 imageWidth = 0;
    u32 imageHeight = 0;
    u32 imageDepth = 1;
};

/**
 * @brief Memory barrier
 */
struct MemoryBarrier {
    Access srcAccessMask = Access::None;
    Access dstAccessMask = Access::None;
};

/**
 * @brief Buffer memory barrier
 */
struct BufferMemoryBarrier {
    BufferHandle buffer;
    Access srcAccessMask = Access::None;
    Access dstAccessMask = Access::None;
    u64 offset = 0;
    u64 size = 0; // 0 = whole buffer
};

/**
 * @brief Image memory barrier
 */
struct ImageMemoryBarrier {
    TextureHandle texture;
    Access srcAccessMask = Access::None;
    Access dstAccessMask = Access::None;
    ImageLayout oldLayout = ImageLayout::Undefined;
    ImageLayout newLayout = ImageLayout::General;
    u32 baseMipLevel = 0;
    u32 mipLevelCount = 1;
    u32 baseArrayLayer = 0;
    u32 arrayLayerCount = 1;
};

/**
 * @brief Abstract command buffer interface
 */
class CommandBuffer {
public:
    virtual ~CommandBuffer() = default;
    
    // =========================================================================
    // State
    // =========================================================================
    
    /// Get command buffer type
    [[nodiscard]] virtual CommandBufferType getType() const = 0;
    
    /// Get current state
    [[nodiscard]] virtual CommandBufferState getState() const = 0;
    
    // =========================================================================
    // Recording Control
    // =========================================================================
    
    /// Begin recording commands
    virtual void begin() = 0;
    
    /// End recording commands
    virtual void end() = 0;
    
    /// Reset the command buffer for reuse
    virtual void reset() = 0;
    
    // =========================================================================
    // Render Pass Commands
    // =========================================================================
    
    /// Begin a render pass
    virtual void beginRenderPass(const RenderPassBeginInfo& beginInfo) = 0;
    
    /// End the current render pass
    virtual void endRenderPass() = 0;
    
    /// Advance to the next subpass
    virtual void nextSubpass() = 0;
    
    // =========================================================================
    // Pipeline State
    // =========================================================================
    
    /// Bind a graphics or compute pipeline
    virtual void bindPipeline(PipelineHandle pipeline) = 0;
    
    /// Set viewport
    virtual void setViewport(const Viewport& viewport) = 0;
    
    /// Set multiple viewports
    virtual void setViewports(std::span<const Viewport> viewports) = 0;
    
    /// Set scissor rectangle
    virtual void setScissor(const Scissor& scissor) = 0;
    
    /// Set multiple scissor rectangles
    virtual void setScissors(std::span<const Scissor> scissors) = 0;
    
    /// Set blend constants
    virtual void setBlendConstants(f32 r, f32 g, f32 b, f32 a) = 0;
    
    /// Set depth bounds
    virtual void setDepthBounds(f32 minDepth, f32 maxDepth) = 0;
    
    /// Set stencil reference value
    virtual void setStencilReference(u32 reference) = 0;
    
    /// Set line width
    virtual void setLineWidth(f32 width) = 0;
    
    // =========================================================================
    // Resource Binding
    // =========================================================================
    
    /// Bind vertex buffers
    virtual void bindVertexBuffers(u32 firstBinding, 
                                    std::span<const VertexBufferBinding> bindings) = 0;
    
    /// Bind a single vertex buffer
    virtual void bindVertexBuffer(u32 binding, BufferHandle buffer, u64 offset = 0) = 0;
    
    /// Bind index buffer
    virtual void bindIndexBuffer(BufferHandle buffer, u64 offset, IndexType indexType) = 0;
    
    /// Bind descriptor sets
    virtual void bindDescriptorSets(u32 firstSet, 
                                     std::span<const DescriptorSetHandle> sets,
                                     std::span<const u32> dynamicOffsets = {}) = 0;
    
    /// Push constants
    virtual void pushConstants(const PushConstantUpdate& update) = 0;
    
    // =========================================================================
    // Draw Commands
    // =========================================================================
    
    /// Draw non-indexed primitives
    virtual void draw(const DrawParams& params) = 0;
    
    /// Draw non-indexed primitives (convenience)
    virtual void draw(u32 vertexCount, u32 instanceCount = 1, 
                      u32 firstVertex = 0, u32 firstInstance = 0) = 0;
    
    /// Draw indexed primitives
    virtual void drawIndexed(const DrawIndexedParams& params) = 0;
    
    /// Draw indexed primitives (convenience)
    virtual void drawIndexed(u32 indexCount, u32 instanceCount = 1,
                             u32 firstIndex = 0, i32 vertexOffset = 0,
                             u32 firstInstance = 0) = 0;
    
    /// Draw with indirect buffer
    virtual void drawIndirect(BufferHandle buffer, u64 offset, 
                              u32 drawCount, u32 stride) = 0;
    
    /// Draw indexed with indirect buffer
    virtual void drawIndexedIndirect(BufferHandle buffer, u64 offset,
                                      u32 drawCount, u32 stride) = 0;
    
    /// Draw with indirect count buffer
    virtual void drawIndirectCount(BufferHandle buffer, u64 offset,
                                    BufferHandle countBuffer, u64 countOffset,
                                    u32 maxDrawCount, u32 stride) = 0;
    
    /// Draw indexed with indirect count buffer
    virtual void drawIndexedIndirectCount(BufferHandle buffer, u64 offset,
                                           BufferHandle countBuffer, u64 countOffset,
                                           u32 maxDrawCount, u32 stride) = 0;
    
    // =========================================================================
    // Compute Commands
    // =========================================================================
    
    /// Dispatch compute work
    virtual void dispatch(const DispatchParams& params) = 0;
    
    /// Dispatch compute work (convenience)
    virtual void dispatch(u32 groupCountX, u32 groupCountY = 1, u32 groupCountZ = 1) = 0;
    
    /// Dispatch with indirect buffer
    virtual void dispatchIndirect(BufferHandle buffer, u64 offset) = 0;
    
    // =========================================================================
    // Transfer Commands
    // =========================================================================
    
    /// Copy buffer to buffer
    virtual void copyBuffer(BufferHandle src, BufferHandle dst,
                            std::span<const BufferCopyRegion> regions) = 0;
    
    /// Copy buffer to buffer (single region convenience)
    virtual void copyBuffer(BufferHandle src, BufferHandle dst,
                            u64 srcOffset, u64 dstOffset, u64 size) = 0;
    
    /// Copy texture to texture
    virtual void copyTexture(TextureHandle src, TextureHandle dst,
                             std::span<const ImageCopyRegion> regions) = 0;
    
    /// Copy buffer to texture
    virtual void copyBufferToTexture(BufferHandle src, TextureHandle dst,
                                      std::span<const BufferImageCopyRegion> regions) = 0;
    
    /// Copy texture to buffer
    virtual void copyTextureToBuffer(TextureHandle src, BufferHandle dst,
                                      std::span<const BufferImageCopyRegion> regions) = 0;
    
    /// Fill buffer with a value
    virtual void fillBuffer(BufferHandle buffer, u64 offset, u64 size, u32 data) = 0;
    
    /// Update buffer data
    virtual void updateBuffer(BufferHandle buffer, u64 offset, 
                              const void* data, u64 size) = 0;
    
    // =========================================================================
    // Synchronization
    // =========================================================================
    
    /// Insert a pipeline barrier
    virtual void pipelineBarrier(PipelineStage srcStage, PipelineStage dstStage,
                                  std::span<const MemoryBarrier> memoryBarriers = {},
                                  std::span<const BufferMemoryBarrier> bufferBarriers = {},
                                  std::span<const ImageMemoryBarrier> imageBarriers = {}) = 0;
    
    // =========================================================================
    // Debug
    // =========================================================================
    
    /// Begin a debug label region
    virtual void beginDebugLabel(const char* name, f32 r = 1.0f, f32 g = 1.0f, 
                                  f32 b = 1.0f, f32 a = 1.0f) = 0;
    
    /// End the current debug label region
    virtual void endDebugLabel() = 0;
    
    /// Insert a debug label
    virtual void insertDebugLabel(const char* name, f32 r = 1.0f, f32 g = 1.0f,
                                   f32 b = 1.0f, f32 a = 1.0f) = 0;
};

} // namespace nova::render
