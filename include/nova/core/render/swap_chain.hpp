/**
 * @file swap_chain.hpp
 * @brief Swap chain management for presenting rendered frames
 * 
 * The swap chain manages the presentation of rendered frames to the screen.
 * Supports triple buffering, vsync, and HDR output.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include "nova/core/types/result.hpp"
#include <vector>

namespace nova::render {

/**
 * @brief VSync mode
 */
enum class VSyncMode : u8 {
    Off,            ///< No vsync, may cause tearing
    On,             ///< Standard vsync (wait for vblank)
    Mailbox,        ///< Triple buffering (low latency, no tearing)
    Adaptive        ///< Adaptive sync (VRR, FreeSync, G-Sync)
};

/**
 * @brief Present mode
 */
enum class PresentMode : u8 {
    Immediate,      ///< No synchronization
    FIFO,           ///< Wait for vblank (vsync)
    FIFORelaxed,    ///< Wait for vblank, but present immediately if late
    Mailbox         ///< Triple buffering
};

/**
 * @brief Swap chain creation descriptor
 */
struct SwapChainDesc {
    /// Width of the swap chain images (0 = use window size)
    u32 width = 0;
    
    /// Height of the swap chain images (0 = use window size)
    u32 height = 0;
    
    /// Preferred format for swap chain images
    TextureFormat format = TextureFormat::BGRA8_SRGB;
    
    /// Number of swap chain images (2 = double buffering, 3 = triple buffering)
    u32 imageCount = 3;
    
    /// VSync mode
    VSyncMode vsync = VSyncMode::Mailbox;
    
    /// Enable HDR output if available
    bool enableHDR = false;
    
    /// Window handle (platform-specific)
    void* windowHandle = nullptr;
    
    /// Display handle (platform-specific, e.g., X11 display)
    void* displayHandle = nullptr;
};

/**
 * @brief Swap chain image for rendering
 */
struct SwapChainImage {
    TextureHandle texture;
    u32 width = 0;
    u32 height = 0;
    u32 imageIndex = 0;
};

/**
 * @brief Abstract swap chain interface
 */
class SwapChain {
public:
    virtual ~SwapChain() = default;
    
    // =========================================================================
    // Properties
    // =========================================================================
    
    /// Get swap chain width
    [[nodiscard]] virtual u32 getWidth() const = 0;
    
    /// Get swap chain height
    [[nodiscard]] virtual u32 getHeight() const = 0;
    
    /// Get swap chain format
    [[nodiscard]] virtual TextureFormat getFormat() const = 0;
    
    /// Get number of images in the swap chain
    [[nodiscard]] virtual u32 getImageCount() const = 0;
    
    /// Get current present mode
    [[nodiscard]] virtual PresentMode getPresentMode() const = 0;
    
    /// Check if HDR is enabled
    [[nodiscard]] virtual bool isHDREnabled() const = 0;
    
    // =========================================================================
    // Operations
    // =========================================================================
    
    /**
     * @brief Acquire the next image to render to
     * @param timeoutNs Timeout in nanoseconds (UINT64_MAX = infinite)
     * @return The acquired image or an error
     */
    [[nodiscard]] virtual Result<SwapChainImage> acquireNextImage(u64 timeoutNs = UINT64_MAX) = 0;
    
    /**
     * @brief Present the current image to the screen
     * @return Success or an error
     */
    virtual VoidResult present() = 0;
    
    /**
     * @brief Resize the swap chain
     * @param width New width (0 = use window size)
     * @param height New height (0 = use window size)
     * @return Success or an error
     */
    virtual VoidResult resize(u32 width = 0, u32 height = 0) = 0;
    
    /**
     * @brief Set VSync mode
     * @param mode The new VSync mode
     */
    virtual void setVSyncMode(VSyncMode mode) = 0;
    
    /**
     * @brief Get all swap chain images
     * @return Vector of swap chain images
     */
    [[nodiscard]] virtual std::vector<SwapChainImage> getImages() const = 0;
    
    /**
     * @brief Get the current image index
     */
    [[nodiscard]] virtual u32 getCurrentImageIndex() const = 0;
};

} // namespace nova::render
