/**
 * @file vulkan_swap_chain.hpp
 * @brief Nova GraphicsCore™ - Vulkan swap chain implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * This file provides the Vulkan swap chain implementation for presenting
 * rendered frames to the display surface.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "vulkan_types.hpp"
#include "vulkan_device.hpp"
#include "nova/core/render/swap_chain.hpp"
#include <vector>

namespace nova::render::vulkan {

/**
 * @brief Swap chain support details
 */
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    
    [[nodiscard]] bool isAdequate() const noexcept {
        return !formats.empty() && !presentModes.empty();
    }
};

/**
 * @brief Per-frame swap chain image resources
 */
struct SwapChainFrame {
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    TextureHandle textureHandle;
};

/**
 * @brief Nova GraphicsCore™ - Vulkan Swap Chain
 * 
 * Manages the presentation of rendered frames to the display surface.
 * Supports:
 * - Triple buffering for smooth presentation
 * - Multiple VSync modes (immediate, fifo, mailbox)
 * - Automatic recreation on resize
 * - HDR support (when available)
 */
class VulkanSwapChain : public SwapChain {
public:
    /**
     * @brief Create a Vulkan swap chain
     * @param device The Vulkan device
     * @param surface The window surface
     * @param desc Swap chain configuration
     * @return Result containing the swap chain or an error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanSwapChain>> create(
        VulkanDevice& device,
        VkSurfaceKHR surface,
        const SwapChainDesc& desc
    );
    
    ~VulkanSwapChain() override;
    
    // Non-copyable, non-movable
    VulkanSwapChain(const VulkanSwapChain&) = delete;
    VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;
    VulkanSwapChain(VulkanSwapChain&&) = delete;
    VulkanSwapChain& operator=(VulkanSwapChain&&) = delete;
    
    // =========================================================================
    // SwapChain Interface Implementation
    // =========================================================================
    
    [[nodiscard]] u32 getWidth() const override { return m_extent.width; }
    [[nodiscard]] u32 getHeight() const override { return m_extent.height; }
    [[nodiscard]] TextureFormat getFormat() const override { return m_format; }
    [[nodiscard]] u32 getImageCount() const override { return static_cast<u32>(m_frames.size()); }
    [[nodiscard]] u32 getCurrentImageIndex() const override { return m_currentImageIndex; }
    [[nodiscard]] PresentMode getPresentMode() const override { return m_presentMode; }
    [[nodiscard]] bool isHDREnabled() const override { return m_hdrEnabled; }
    
    [[nodiscard]] Result<SwapChainImage> acquireNextImage(u64 timeoutNs = UINT64_MAX) override;
    VoidResult present() override;
    VoidResult resize(u32 width = 0, u32 height = 0) override;
    void setVSyncMode(VSyncMode mode) override;
    [[nodiscard]] std::vector<SwapChainImage> getImages() const override;
    
    // =========================================================================
    // Vulkan-Specific Accessors
    // =========================================================================
    
    [[nodiscard]] VkSwapchainKHR getSwapChain() const noexcept { return m_swapChain; }
    [[nodiscard]] VkSurfaceKHR getSurface() const noexcept { return m_surface; }
    [[nodiscard]] VkExtent2D getExtent() const noexcept { return m_extent; }
    [[nodiscard]] VkFormat getVkFormat() const noexcept { return m_vkFormat; }
    [[nodiscard]] VkColorSpaceKHR getColorSpace() const noexcept { return m_colorSpace; }
    [[nodiscard]] VSyncMode getVSyncMode() const noexcept { return m_vsyncMode; }
    
    [[nodiscard]] const SwapChainFrame& getFrame(u32 index) const { return m_frames[index]; }
    [[nodiscard]] const SwapChainFrame& getCurrentFrame() const { return m_frames[m_currentImageIndex]; }
    
    [[nodiscard]] VkRenderPass getCompatibleRenderPass() const noexcept { return m_renderPass; }
    
private:
    VulkanSwapChain(VulkanDevice& device, VkSurfaceKHR surface);
    
    /**
     * @brief Query swap chain support details
     */
    [[nodiscard]] SwapChainSupportDetails querySwapChainSupport() const;
    
    /**
     * @brief Choose the best surface format
     */
    [[nodiscard]] VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& formats,
        bool preferHDR
    ) const;
    
    /**
     * @brief Choose the best present mode
     */
    [[nodiscard]] VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& modes,
        VSyncMode requestedMode
    ) const;
    
    /**
     * @brief Choose the swap extent
     */
    [[nodiscard]] VkExtent2D chooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& capabilities,
        u32 requestedWidth,
        u32 requestedHeight
    ) const;
    
    /**
     * @brief Create the swap chain
     */
    Result<void> createSwapChain(const SwapChainDesc& desc);
    
    /**
     * @brief Create image views
     */
    Result<void> createImageViews();
    
    /**
     * @brief Create a compatible render pass
     */
    Result<void> createRenderPass();
    
    /**
     * @brief Create framebuffers
     */
    Result<void> createFramebuffers();
    
    /**
     * @brief Cleanup swap chain resources
     */
    void cleanup();
    
    /**
     * @brief Convert VSyncMode to Vulkan present mode
     */
    [[nodiscard]] static VkPresentModeKHR toVkPresentMode(VSyncMode mode) noexcept;
    
    /**
     * @brief Convert Vulkan present mode to PresentMode
     */
    [[nodiscard]] static PresentMode fromVkPresentMode(VkPresentModeKHR mode) noexcept;
    
    /**
     * @brief Convert Vulkan format to NovaCore format
     */
    [[nodiscard]] static TextureFormat fromVkFormat(VkFormat format) noexcept;
    
    // =========================================================================
    // Member Variables
    // =========================================================================
    
    // References
    VulkanDevice& m_device;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    
    // Swap chain
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    VkExtent2D m_extent{};
    VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    TextureFormat m_format = TextureFormat::Unknown;
    
    // Frames
    std::vector<SwapChainFrame> m_frames;
    u32 m_currentImageIndex = 0;
    u32 m_currentFrame = 0;
    
    // Render pass (for simple use cases)
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    
    // Configuration
    VSyncMode m_vsyncMode = VSyncMode::On;
    PresentMode m_presentMode = PresentMode::FIFO;
    bool m_hdrEnabled = false;
    bool m_needsRecreation = false;
    
    // Resource ID counter for texture handles
    u64 m_nextTextureId = 1;
};

/**
 * @brief Query swap chain support for a physical device and surface
 */
[[nodiscard]] SwapChainSupportDetails querySwapChainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VulkanInstanceFunctions& funcs
);

} // namespace nova::render::vulkan
