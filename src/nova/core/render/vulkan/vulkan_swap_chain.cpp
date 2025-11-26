/**
 * @file vulkan_swap_chain.cpp
 * @brief Nova GraphicsCore™ - Vulkan swap chain implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_swap_chain.hpp"
#include <algorithm>
#include <limits>

namespace nova::render::vulkan {

// ============================================================================
// Static Factory
// ============================================================================

Result<std::unique_ptr<VulkanSwapChain>> VulkanSwapChain::create(
    VulkanDevice& device,
    VkSurfaceKHR surface,
    const SwapChainDesc& desc
) {
    auto swapChain = std::unique_ptr<VulkanSwapChain>(new VulkanSwapChain(device, surface));
    
    // Create the swap chain
    auto result = swapChain->createSwapChain(desc);
    if (!result) {
        return std::unexpected(result.error());
    }
    
    // Create image views
    result = swapChain->createImageViews();
    if (!result) {
        return std::unexpected(result.error());
    }
    
    // Create a compatible render pass for simple use cases
    result = swapChain->createRenderPass();
    if (!result) {
        return std::unexpected(result.error());
    }
    
    // Create framebuffers
    result = swapChain->createFramebuffers();
    if (!result) {
        return std::unexpected(result.error());
    }
    
    return swapChain;
}

VulkanSwapChain::VulkanSwapChain(VulkanDevice& device, VkSurfaceKHR surface)
    : m_device(device)
    , m_surface(surface)
{
}

VulkanSwapChain::~VulkanSwapChain() {
    cleanup();
}

// ============================================================================
// SwapChain Interface Implementation
// ============================================================================

Result<SwapChainImage> VulkanSwapChain::acquireNextImage(u64 timeoutNs) {
    const auto& funcs = m_device.getDeviceFuncs();
    
    VkResult result = funcs.vkAcquireNextImageKHR(
        m_device.getDevice(),
        m_swapChain,
        timeoutNs,
        VK_NULL_HANDLE,  // Would use imageAvailable semaphore
        VK_NULL_HANDLE,  // fence
        &m_currentImageIndex
    );
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_needsRecreation = true;
        return std::unexpected(errors::graphics("Swap chain out of date"));
    }
    
    if (result == VK_SUBOPTIMAL_KHR) {
        // Swap chain still usable but should be recreated
        m_needsRecreation = true;
    } else if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to acquire swap chain image: " + std::string(vkResultToString(result))));
    }
    
    // Return the swap chain image
    SwapChainImage image;
    image.texture = m_frames[m_currentImageIndex].textureHandle;
    image.width = m_extent.width;
    image.height = m_extent.height;
    image.imageIndex = m_currentImageIndex;
    
    return image;
}

VoidResult VulkanSwapChain::present() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.pWaitSemaphores = nullptr;  // Would wait on renderFinished semaphore
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapChain;
    presentInfo.pImageIndices = &m_currentImageIndex;
    presentInfo.pResults = nullptr;
    
    VkResult result = funcs.vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        m_needsRecreation = true;
    } else if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to present swap chain image: " + std::string(vkResultToString(result))));
    }
    
    ++m_currentFrame;
    
    return {};
}

VoidResult VulkanSwapChain::resize(u32 width, u32 height) {
    // Wait for device to be idle
    m_device.waitIdle();
    
    // Cleanup old resources
    cleanup();
    
    // Create new swap chain
    SwapChainDesc desc;
    desc.width = width;
    desc.height = height;
    desc.vsync = m_vsyncMode;
    desc.enableHDR = m_hdrEnabled;
    
    auto result = createSwapChain(desc);
    if (!result) {
        return std::unexpected(result.error());
    }
    
    result = createImageViews();
    if (!result) {
        return std::unexpected(result.error());
    }
    
    result = createRenderPass();
    if (!result) {
        return std::unexpected(result.error());
    }
    
    result = createFramebuffers();
    if (!result) {
        return std::unexpected(result.error());
    }
    
    m_needsRecreation = false;
    m_currentImageIndex = 0;
    
    return {};
}

std::vector<SwapChainImage> VulkanSwapChain::getImages() const {
    std::vector<SwapChainImage> images;
    images.reserve(m_frames.size());
    
    for (u32 i = 0; i < m_frames.size(); ++i) {
        SwapChainImage image;
        image.texture = m_frames[i].textureHandle;
        image.width = m_extent.width;
        image.height = m_extent.height;
        image.imageIndex = i;
        images.push_back(image);
    }
    
    return images;
}

void VulkanSwapChain::setVSyncMode(VSyncMode mode) {
    if (m_vsyncMode != mode) {
        m_vsyncMode = mode;
        m_needsRecreation = true;
    }
}

// ============================================================================
// Swap Chain Creation
// ============================================================================

SwapChainSupportDetails VulkanSwapChain::querySwapChainSupport() const {
    return nova::render::vulkan::querySwapChainSupport(
        m_device.getPhysicalDevice(),
        m_surface,
        m_device.getInstanceFuncs()
    );
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats,
    bool preferHDR
) const {
    // If HDR is requested and available, try to use it
    if (preferHDR) {
        for (const auto& format : formats) {
            // HDR10 format
            if (format.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 &&
                format.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT) {
                return format;
            }
        }
    }
    
    // Prefer BGRA8 SRGB (most common on desktop)
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    
    // Fallback to BGRA8 UNORM
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    
    // Fallback to RGBA8
    for (const auto& format : formats) {
        if ((format.format == VK_FORMAT_R8G8B8A8_SRGB ||
             format.format == VK_FORMAT_R8G8B8A8_UNORM) &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    
    // Just use the first available format
    return formats[0];
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& modes,
    VSyncMode requestedMode
) const {
    VkPresentModeKHR preferred = toVkPresentMode(requestedMode);
    
    // Check if preferred mode is available
    for (const auto& mode : modes) {
        if (mode == preferred) {
            return mode;
        }
    }
    
    // Mailbox is a good fallback for low latency
    for (const auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    
    // FIFO is guaranteed to be available (VSync)
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    u32 requestedWidth,
    u32 requestedHeight
) const {
    // If the surface size is defined, we must match it
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
        return capabilities.currentExtent;
    }
    
    // Otherwise, set to requested size within bounds
    VkExtent2D extent = {requestedWidth, requestedHeight};
    
    extent.width = std::clamp(
        extent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width
    );
    
    extent.height = std::clamp(
        extent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height
    );
    
    return extent;
}

Result<void> VulkanSwapChain::createSwapChain(const SwapChainDesc& desc) {
    SwapChainSupportDetails support = querySwapChainSupport();
    
    if (!support.isAdequate()) {
        return std::unexpected(errors::graphics("Swap chain support is not adequate"));
    }
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats, desc.enableHDR);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(support.presentModes, desc.vsync);
    VkExtent2D extent = chooseSwapExtent(support.capabilities, desc.width, desc.height);
    
    // Determine image count (triple buffering if possible)
    u32 imageCount = support.capabilities.minImageCount + 1;
    if (support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
        imageCount = support.capabilities.maxImageCount;
    }
    
    // Clamp to max frames in flight
    imageCount = std::min(imageCount, MAX_FRAMES_IN_FLIGHT);
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    // Handle queue family indices
    const auto& queueFamilies = m_device.getQueueFamilies();
    u32 queueFamilyIndices[] = {queueFamilies.graphics, queueFamilies.present};
    
    if (queueFamilies.graphics != queueFamilies.present) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    
    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    const auto& funcs = m_device.getDeviceFuncs();
    VkResult result = funcs.vkCreateSwapchainKHR(
        m_device.getDevice(), &createInfo, nullptr, &m_swapChain);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create swap chain: " + std::string(vkResultToString(result))));
    }
    
    // Store swap chain properties
    m_extent = extent;
    m_vkFormat = surfaceFormat.format;
    m_colorSpace = surfaceFormat.colorSpace;
    m_format = fromVkFormat(surfaceFormat.format);
    m_vsyncMode = desc.vsync;
    m_presentMode = fromVkPresentMode(presentMode);
    m_hdrEnabled = (surfaceFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT);
    
    // Get swap chain images
    u32 actualImageCount = 0;
    funcs.vkGetSwapchainImagesKHR(m_device.getDevice(), m_swapChain, &actualImageCount, nullptr);
    
    std::vector<VkImage> images(actualImageCount);
    funcs.vkGetSwapchainImagesKHR(m_device.getDevice(), m_swapChain, &actualImageCount, images.data());
    
    m_frames.resize(actualImageCount);
    for (u32 i = 0; i < actualImageCount; ++i) {
        m_frames[i].image = images[i];
        m_frames[i].textureHandle = TextureHandle(m_nextTextureId++);
    }
    
    return {};
}

Result<void> VulkanSwapChain::createImageViews() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    for (auto& frame : m_frames) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = frame.image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_vkFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        VkResult result = funcs.vkCreateImageView(
            m_device.getDevice(), &createInfo, nullptr, &frame.imageView);
        
        if (result != VK_SUCCESS) {
            return std::unexpected(errors::graphics(
                "Failed to create swap chain image view: " + std::string(vkResultToString(result))));
        }
    }
    
    return {};
}

Result<void> VulkanSwapChain::createRenderPass() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_vkFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    // Subpass dependency
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    VkResult result = funcs.vkCreateRenderPass(
        m_device.getDevice(), &renderPassInfo, nullptr, &m_renderPass);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create render pass: " + std::string(vkResultToString(result))));
    }
    
    return {};
}

Result<void> VulkanSwapChain::createFramebuffers() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    for (auto& frame : m_frames) {
        VkImageView attachments[] = {frame.imageView};
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_extent.width;
        framebufferInfo.height = m_extent.height;
        framebufferInfo.layers = 1;
        
        VkResult result = funcs.vkCreateFramebuffer(
            m_device.getDevice(), &framebufferInfo, nullptr, &frame.framebuffer);
        
        if (result != VK_SUCCESS) {
            return std::unexpected(errors::graphics(
                "Failed to create framebuffer: " + std::string(vkResultToString(result))));
        }
    }
    
    return {};
}

void VulkanSwapChain::cleanup() {
    const auto& funcs = m_device.getDeviceFuncs();
    
    // Destroy framebuffers
    for (auto& frame : m_frames) {
        if (frame.framebuffer) {
            funcs.vkDestroyFramebuffer(m_device.getDevice(), frame.framebuffer, nullptr);
            frame.framebuffer = VK_NULL_HANDLE;
        }
    }
    
    // Destroy render pass
    if (m_renderPass) {
        funcs.vkDestroyRenderPass(m_device.getDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }
    
    // Destroy image views
    for (auto& frame : m_frames) {
        if (frame.imageView) {
            funcs.vkDestroyImageView(m_device.getDevice(), frame.imageView, nullptr);
            frame.imageView = VK_NULL_HANDLE;
        }
        // Images are owned by the swap chain, don't destroy them
        frame.image = VK_NULL_HANDLE;
    }
    
    m_frames.clear();
    
    // Destroy swap chain
    if (m_swapChain) {
        funcs.vkDestroySwapchainKHR(m_device.getDevice(), m_swapChain, nullptr);
        m_swapChain = VK_NULL_HANDLE;
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

VkPresentModeKHR VulkanSwapChain::toVkPresentMode(VSyncMode mode) noexcept {
    switch (mode) {
        case VSyncMode::Off:      return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case VSyncMode::On:       return VK_PRESENT_MODE_FIFO_KHR;
        case VSyncMode::Mailbox:  return VK_PRESENT_MODE_MAILBOX_KHR;
        case VSyncMode::Adaptive: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        default:                  return VK_PRESENT_MODE_FIFO_KHR;
    }
}

PresentMode VulkanSwapChain::fromVkPresentMode(VkPresentModeKHR mode) noexcept {
    switch (mode) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:    return PresentMode::Immediate;
        case VK_PRESENT_MODE_FIFO_KHR:         return PresentMode::FIFO;
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR: return PresentMode::FIFORelaxed;
        case VK_PRESENT_MODE_MAILBOX_KHR:      return PresentMode::Mailbox;
        default:                               return PresentMode::FIFO;
    }
}

TextureFormat VulkanSwapChain::fromVkFormat(VkFormat format) noexcept {
    switch (format) {
        case VK_FORMAT_B8G8R8A8_UNORM: return TextureFormat::BGRA8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:  return TextureFormat::BGRA8_SRGB;
        case VK_FORMAT_R8G8B8A8_UNORM: return TextureFormat::RGBA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:  return TextureFormat::RGBA8_SRGB;
        default:                       return TextureFormat::Unknown;
    }
}

// ============================================================================
// Helper Functions
// ============================================================================

SwapChainSupportDetails querySwapChainSupport(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VulkanInstanceFunctions& funcs
) {
    SwapChainSupportDetails details;
    
    // Get surface capabilities
    funcs.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    
    // Get supported formats
    u32 formatCount = 0;
    funcs.vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    
    if (formatCount > 0) {
        details.formats.resize(formatCount);
        funcs.vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }
    
    // Get supported present modes
    u32 presentModeCount = 0;
    funcs.vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    
    if (presentModeCount > 0) {
        details.presentModes.resize(presentModeCount);
        funcs.vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    
    return details;
}

} // namespace nova::render::vulkan
