/**
 * @file render_context.hpp
 * @brief Render context for frame rendering
 * 
 * The render context manages per-frame rendering state and
 * provides the main interface for recording rendering commands.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include "command_buffer.hpp"
#include "render_pass.hpp"
#include "buffer.hpp"
#include <memory>

namespace nova::render {

// Forward declarations
class RenderDevice;
class SwapChain;

/**
 * @brief Per-frame rendering context
 * 
 * The RenderContext provides a high-level interface for recording
 * rendering commands for a single frame. It manages command buffer
 * allocation, synchronization, and submission.
 */
class RenderContext {
public:
    virtual ~RenderContext() = default;
    
    // =========================================================================
    // Frame Management
    // =========================================================================
    
    /// Get the current frame index (0 to MAX_FRAMES_IN_FLIGHT-1)
    [[nodiscard]] virtual u32 getFrameIndex() const = 0;
    
    /// Get the absolute frame number
    [[nodiscard]] virtual u64 getFrameNumber() const = 0;
    
    // =========================================================================
    // Command Buffer Access
    // =========================================================================
    
    /// Get the primary graphics command buffer for this frame
    [[nodiscard]] virtual CommandBuffer* getCommandBuffer() = 0;
    
    /// Get a secondary command buffer for parallel recording
    [[nodiscard]] virtual CommandBuffer* getSecondaryCommandBuffer() = 0;
    
    /// Get a compute command buffer (may be same as graphics on some platforms)
    [[nodiscard]] virtual CommandBuffer* getComputeCommandBuffer() = 0;
    
    /// Get a transfer command buffer (may be same as graphics on some platforms)
    [[nodiscard]] virtual CommandBuffer* getTransferCommandBuffer() = 0;
    
    // =========================================================================
    // High-Level Rendering
    // =========================================================================
    
    /**
     * @brief Begin rendering to a swap chain
     * @param swapChain The swap chain to render to
     * @return The swap chain image to render to, or an error
     */
    [[nodiscard]] virtual Result<SwapChainImage> beginSwapChainRendering(SwapChain* swapChain) = 0;
    
    /**
     * @brief End swap chain rendering and present
     * @param swapChain The swap chain
     */
    virtual void endSwapChainRendering(SwapChain* swapChain) = 0;
    
    /**
     * @brief Begin a render pass with automatic framebuffer
     * @param desc Render pass description
     * @param targetTexture Target texture to render to
     */
    virtual void beginRenderPass(const RenderPassDesc& desc, 
                                  TextureHandle targetTexture) = 0;
    
    /**
     * @brief Begin a render pass with explicit begin info
     */
    virtual void beginRenderPass(const RenderPassBeginInfo& beginInfo) = 0;
    
    /**
     * @brief End the current render pass
     */
    virtual void endRenderPass() = 0;
    
    // =========================================================================
    // Resource Management
    // =========================================================================
    
    /**
     * @brief Allocate temporary buffer memory for this frame
     * @param size Size in bytes
     * @param alignment Alignment requirement
     * @return Pointer to allocated memory, valid until end of frame
     */
    [[nodiscard]] virtual void* allocateFrameMemory(usize size, usize alignment = 16) = 0;
    
    /**
     * @brief Get a temporary buffer for this frame
     * @param size Size in bytes
     * @param usage Buffer usage
     * @return Temporary buffer handle, valid until end of frame
     */
    [[nodiscard]] virtual BufferHandle getTemporaryBuffer(usize size, BufferUsage usage) = 0;
    
    /**
     * @brief Upload data to a temporary buffer
     * @param data Data to upload
     * @param size Size in bytes
     * @return Temporary buffer with the data
     */
    [[nodiscard]] virtual BufferHandle uploadTemporaryData(const void* data, usize size) = 0;
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    /// Get number of draw calls recorded this frame
    [[nodiscard]] virtual u64 getDrawCallCount() const = 0;
    
    /// Get number of triangles rendered this frame
    [[nodiscard]] virtual u64 getTriangleCount() const = 0;
    
    /// Get bytes uploaded this frame
    [[nodiscard]] virtual u64 getUploadedBytes() const = 0;
};

} // namespace nova::render
