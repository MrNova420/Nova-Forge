/**
 * @file vulkan_types.hpp
 * @brief Nova GraphicsCore™ - Vulkan type definitions and utilities
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * This file provides Vulkan-specific type definitions and conversion utilities
 * for the Nova GraphicsCore™ rendering abstraction.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/render/render_types.hpp"

// Vulkan header configuration
#define VK_NO_PROTOTYPES
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_CONSTRUCTORS

#if defined(__ANDROID__)
    #define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__) && !defined(__ANDROID__)
    // Linux: Support X11 and Wayland
    #if defined(NOVA_USE_WAYLAND)
        #define VK_USE_PLATFORM_WAYLAND_KHR
    #else
        #define VK_USE_PLATFORM_XCB_KHR
    #endif
#elif defined(_WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

// Include Vulkan headers
#include <vulkan/vulkan.h>

namespace nova::render::vulkan {

// ============================================================================
// Nova GraphicsCore™ - Vulkan Configuration Constants
// ============================================================================

/// Maximum frames in flight for triple buffering
constexpr u32 MAX_FRAMES_IN_FLIGHT = 3;

/// Maximum number of descriptor sets that can be bound simultaneously
constexpr u32 MAX_BOUND_DESCRIPTOR_SETS = 8;

/// Maximum number of vertex input bindings
constexpr u32 MAX_VERTEX_BINDINGS = 16;

/// Maximum number of vertex attributes
constexpr u32 MAX_VERTEX_ATTRIBUTES = 16;

/// Maximum number of color attachments per render pass
constexpr u32 MAX_COLOR_ATTACHMENTS = 8;

/// Maximum number of push constant bytes
constexpr u32 MAX_PUSH_CONSTANT_SIZE = 128;

/// Preferred Vulkan API version (1.3 for modern features, 1.1 fallback for mobile)
constexpr u32 VULKAN_API_VERSION_PREFERRED = VK_API_VERSION_1_3;
constexpr u32 VULKAN_API_VERSION_MINIMUM = VK_API_VERSION_1_1;

// ============================================================================
// Vulkan Function Pointer Types (Dynamically Loaded)
// ============================================================================

/// Vulkan instance function pointers
struct VulkanInstanceFunctions {
    // Core 1.0
    PFN_vkDestroyInstance vkDestroyInstance = nullptr;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = nullptr;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = nullptr;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = nullptr;
    PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties = nullptr;
    PFN_vkCreateDevice vkCreateDevice = nullptr;
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties = nullptr;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;
    
    // Core 1.1 / KHR extensions
    PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2 = nullptr;
    
    // Surface extension
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
    
    // Debug utils extension
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    
    // Platform-specific surface creation
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR = nullptr;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = nullptr;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR = nullptr;
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = nullptr;
#endif
};

/// Vulkan device function pointers
struct VulkanDeviceFunctions {
    // Core 1.0 - Device
    PFN_vkDestroyDevice vkDestroyDevice = nullptr;
    PFN_vkGetDeviceQueue vkGetDeviceQueue = nullptr;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle = nullptr;
    
    // Core 1.0 - Queue
    PFN_vkQueueSubmit vkQueueSubmit = nullptr;
    PFN_vkQueueWaitIdle vkQueueWaitIdle = nullptr;
    PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;
    
    // Core 1.0 - Memory
    PFN_vkAllocateMemory vkAllocateMemory = nullptr;
    PFN_vkFreeMemory vkFreeMemory = nullptr;
    PFN_vkMapMemory vkMapMemory = nullptr;
    PFN_vkUnmapMemory vkUnmapMemory = nullptr;
    PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges = nullptr;
    PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges = nullptr;
    PFN_vkBindBufferMemory vkBindBufferMemory = nullptr;
    PFN_vkBindImageMemory vkBindImageMemory = nullptr;
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = nullptr;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = nullptr;
    
    // Core 1.0 - Buffer
    PFN_vkCreateBuffer vkCreateBuffer = nullptr;
    PFN_vkDestroyBuffer vkDestroyBuffer = nullptr;
    
    // Core 1.0 - Image
    PFN_vkCreateImage vkCreateImage = nullptr;
    PFN_vkDestroyImage vkDestroyImage = nullptr;
    PFN_vkCreateImageView vkCreateImageView = nullptr;
    PFN_vkDestroyImageView vkDestroyImageView = nullptr;
    
    // Core 1.0 - Sampler
    PFN_vkCreateSampler vkCreateSampler = nullptr;
    PFN_vkDestroySampler vkDestroySampler = nullptr;
    
    // Core 1.0 - Shader
    PFN_vkCreateShaderModule vkCreateShaderModule = nullptr;
    PFN_vkDestroyShaderModule vkDestroyShaderModule = nullptr;
    
    // Core 1.0 - Pipeline
    PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = nullptr;
    PFN_vkCreateComputePipelines vkCreateComputePipelines = nullptr;
    PFN_vkDestroyPipeline vkDestroyPipeline = nullptr;
    PFN_vkCreatePipelineLayout vkCreatePipelineLayout = nullptr;
    PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = nullptr;
    PFN_vkCreatePipelineCache vkCreatePipelineCache = nullptr;
    PFN_vkDestroyPipelineCache vkDestroyPipelineCache = nullptr;
    PFN_vkGetPipelineCacheData vkGetPipelineCacheData = nullptr;
    
    // Core 1.0 - Render Pass
    PFN_vkCreateRenderPass vkCreateRenderPass = nullptr;
    PFN_vkDestroyRenderPass vkDestroyRenderPass = nullptr;
    PFN_vkCreateFramebuffer vkCreateFramebuffer = nullptr;
    PFN_vkDestroyFramebuffer vkDestroyFramebuffer = nullptr;
    
    // Core 1.0 - Descriptor
    PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = nullptr;
    PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = nullptr;
    PFN_vkCreateDescriptorPool vkCreateDescriptorPool = nullptr;
    PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool = nullptr;
    PFN_vkResetDescriptorPool vkResetDescriptorPool = nullptr;
    PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets = nullptr;
    PFN_vkFreeDescriptorSets vkFreeDescriptorSets = nullptr;
    PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets = nullptr;
    
    // Core 1.0 - Command Buffer
    PFN_vkCreateCommandPool vkCreateCommandPool = nullptr;
    PFN_vkDestroyCommandPool vkDestroyCommandPool = nullptr;
    PFN_vkResetCommandPool vkResetCommandPool = nullptr;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = nullptr;
    PFN_vkFreeCommandBuffers vkFreeCommandBuffers = nullptr;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer = nullptr;
    PFN_vkEndCommandBuffer vkEndCommandBuffer = nullptr;
    PFN_vkResetCommandBuffer vkResetCommandBuffer = nullptr;
    
    // Core 1.0 - Command Recording
    PFN_vkCmdBindPipeline vkCmdBindPipeline = nullptr;
    PFN_vkCmdSetViewport vkCmdSetViewport = nullptr;
    PFN_vkCmdSetScissor vkCmdSetScissor = nullptr;
    PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = nullptr;
    PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer = nullptr;
    PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets = nullptr;
    PFN_vkCmdPushConstants vkCmdPushConstants = nullptr;
    PFN_vkCmdDraw vkCmdDraw = nullptr;
    PFN_vkCmdDrawIndexed vkCmdDrawIndexed = nullptr;
    PFN_vkCmdDrawIndirect vkCmdDrawIndirect = nullptr;
    PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect = nullptr;
    PFN_vkCmdDispatch vkCmdDispatch = nullptr;
    PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect = nullptr;
    PFN_vkCmdCopyBuffer vkCmdCopyBuffer = nullptr;
    PFN_vkCmdCopyImage vkCmdCopyImage = nullptr;
    PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage = nullptr;
    PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer = nullptr;
    PFN_vkCmdBlitImage vkCmdBlitImage = nullptr;
    PFN_vkCmdClearColorImage vkCmdClearColorImage = nullptr;
    PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage = nullptr;
    PFN_vkCmdFillBuffer vkCmdFillBuffer = nullptr;
    PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = nullptr;
    PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = nullptr;
    PFN_vkCmdNextSubpass vkCmdNextSubpass = nullptr;
    PFN_vkCmdEndRenderPass vkCmdEndRenderPass = nullptr;
    PFN_vkCmdExecuteCommands vkCmdExecuteCommands = nullptr;
    
    // Core 1.0 - Synchronization
    PFN_vkCreateFence vkCreateFence = nullptr;
    PFN_vkDestroyFence vkDestroyFence = nullptr;
    PFN_vkResetFences vkResetFences = nullptr;
    PFN_vkGetFenceStatus vkGetFenceStatus = nullptr;
    PFN_vkWaitForFences vkWaitForFences = nullptr;
    PFN_vkCreateSemaphore vkCreateSemaphore = nullptr;
    PFN_vkDestroySemaphore vkDestroySemaphore = nullptr;
    
    // Swapchain extension
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
    
    // Core 1.2 / KHR Timeline Semaphores
    PFN_vkWaitSemaphores vkWaitSemaphores = nullptr;
    PFN_vkSignalSemaphore vkSignalSemaphore = nullptr;
    PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue = nullptr;
    
    // Core 1.3 / KHR Dynamic Rendering
    PFN_vkCmdBeginRendering vkCmdBeginRendering = nullptr;
    PFN_vkCmdEndRendering vkCmdEndRendering = nullptr;
    
    // Core 1.3 / KHR Synchronization2
    PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2 = nullptr;
    PFN_vkQueueSubmit2 vkQueueSubmit2 = nullptr;
};

// ============================================================================
// Format Conversion Utilities
// ============================================================================

/**
 * @brief Convert NovaCore texture format to Vulkan format
 * @param format NovaCore texture format
 * @return Vulkan format equivalent
 */
[[nodiscard]] constexpr VkFormat toVkFormat(TextureFormat format) noexcept {
    switch (format) {
        // 8-bit formats
        case TextureFormat::R8_UNORM:     return VK_FORMAT_R8_UNORM;
        case TextureFormat::R8_SNORM:     return VK_FORMAT_R8_SNORM;
        case TextureFormat::R8_UINT:      return VK_FORMAT_R8_UINT;
        case TextureFormat::R8_SINT:      return VK_FORMAT_R8_SINT;
        
        case TextureFormat::RG8_UNORM:    return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RG8_SNORM:    return VK_FORMAT_R8G8_SNORM;
        
        case TextureFormat::RGBA8_UNORM:  return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA8_SNORM:  return VK_FORMAT_R8G8B8A8_SNORM;
        case TextureFormat::RGBA8_SRGB:   return VK_FORMAT_R8G8B8A8_SRGB;
        
        case TextureFormat::BGRA8_UNORM:  return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::BGRA8_SRGB:   return VK_FORMAT_B8G8R8A8_SRGB;
        
        // 16-bit formats
        case TextureFormat::R16_FLOAT:    return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::R16_UINT:     return VK_FORMAT_R16_UINT;
        case TextureFormat::R16_SINT:     return VK_FORMAT_R16_SINT;
        
        case TextureFormat::RG16_FLOAT:   return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RG16_UINT:    return VK_FORMAT_R16G16_UINT;
        
        case TextureFormat::RGBA16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RGBA16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
        
        // 32-bit formats
        case TextureFormat::R32_UINT:     return VK_FORMAT_R32_UINT;
        case TextureFormat::R32_SINT:     return VK_FORMAT_R32_SINT;
        case TextureFormat::R32_FLOAT:    return VK_FORMAT_R32_SFLOAT;
        
        case TextureFormat::RG32_FLOAT:   return VK_FORMAT_R32G32_SFLOAT;
        
        case TextureFormat::RGBA32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        
        // Packed formats
        case TextureFormat::RGB10A2_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case TextureFormat::RG11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        
        // Depth/Stencil formats
        case TextureFormat::D16_UNORM:         return VK_FORMAT_D16_UNORM;
        case TextureFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32_FLOAT:         return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::D32_FLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        
        // Compressed formats - BC
        case TextureFormat::BC1_UNORM:    return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case TextureFormat::BC1_SRGB:     return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case TextureFormat::BC2_UNORM:    return VK_FORMAT_BC2_UNORM_BLOCK;
        case TextureFormat::BC2_SRGB:     return VK_FORMAT_BC2_SRGB_BLOCK;
        case TextureFormat::BC3_UNORM:    return VK_FORMAT_BC3_UNORM_BLOCK;
        case TextureFormat::BC3_SRGB:     return VK_FORMAT_BC3_SRGB_BLOCK;
        case TextureFormat::BC4_UNORM:    return VK_FORMAT_BC4_UNORM_BLOCK;
        case TextureFormat::BC4_SNORM:    return VK_FORMAT_BC4_SNORM_BLOCK;
        case TextureFormat::BC5_UNORM:    return VK_FORMAT_BC5_UNORM_BLOCK;
        case TextureFormat::BC5_SNORM:    return VK_FORMAT_BC5_SNORM_BLOCK;
        case TextureFormat::BC6H_UFLOAT:  return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case TextureFormat::BC6H_SFLOAT:  return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case TextureFormat::BC7_UNORM:    return VK_FORMAT_BC7_UNORM_BLOCK;
        case TextureFormat::BC7_SRGB:     return VK_FORMAT_BC7_SRGB_BLOCK;
        
        // Compressed formats - ETC2 (Mobile)
        case TextureFormat::ETC2_RGB8_UNORM:   return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case TextureFormat::ETC2_RGB8_SRGB:    return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case TextureFormat::ETC2_RGBA8_UNORM:  return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case TextureFormat::ETC2_RGBA8_SRGB:   return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        
        // Compressed formats - ASTC (Mobile)
        case TextureFormat::ASTC_4x4_UNORM:   return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case TextureFormat::ASTC_4x4_SRGB:    return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case TextureFormat::ASTC_5x5_UNORM:   return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case TextureFormat::ASTC_5x5_SRGB:    return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case TextureFormat::ASTC_6x6_UNORM:   return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case TextureFormat::ASTC_6x6_SRGB:    return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case TextureFormat::ASTC_8x8_UNORM:   return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case TextureFormat::ASTC_8x8_SRGB:    return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        
        default:
            return VK_FORMAT_UNDEFINED;
    }
}

/**
 * @brief Convert primitive topology to Vulkan topology
 */
[[nodiscard]] constexpr VkPrimitiveTopology toVkTopology(PrimitiveTopology topology) noexcept {
    switch (topology) {
        case PrimitiveTopology::PointList:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::TriangleList:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default:                               return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

/**
 * @brief Convert blend factor to Vulkan blend factor
 */
[[nodiscard]] constexpr VkBlendFactor toVkBlendFactor(BlendFactor factor) noexcept {
    switch (factor) {
        case BlendFactor::Zero:             return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One:              return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:         return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor:    return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha:    return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        default:                            return VK_BLEND_FACTOR_ONE;
    }
}

/**
 * @brief Convert blend operation to Vulkan blend op
 */
[[nodiscard]] constexpr VkBlendOp toVkBlendOp(BlendOp op) noexcept {
    switch (op) {
        case BlendOp::Add:             return VK_BLEND_OP_ADD;
        case BlendOp::Subtract:        return VK_BLEND_OP_SUBTRACT;
        case BlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::Min:             return VK_BLEND_OP_MIN;
        case BlendOp::Max:             return VK_BLEND_OP_MAX;
        default:                       return VK_BLEND_OP_ADD;
    }
}

/**
 * @brief Convert compare operation to Vulkan compare op
 */
[[nodiscard]] constexpr VkCompareOp toVkCompareOp(CompareOp op) noexcept {
    switch (op) {
        case CompareOp::Never:          return VK_COMPARE_OP_NEVER;
        case CompareOp::Less:           return VK_COMPARE_OP_LESS;
        case CompareOp::Equal:          return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater:        return VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Always:         return VK_COMPARE_OP_ALWAYS;
        default:                        return VK_COMPARE_OP_LESS;
    }
}

/**
 * @brief Convert stencil operation to Vulkan stencil op
 */
[[nodiscard]] constexpr VkStencilOp toVkStencilOp(StencilOp op) noexcept {
    switch (op) {
        case StencilOp::Keep:           return VK_STENCIL_OP_KEEP;
        case StencilOp::Zero:           return VK_STENCIL_OP_ZERO;
        case StencilOp::Replace:        return VK_STENCIL_OP_REPLACE;
        case StencilOp::IncrementClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp::DecrementClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp::Invert:         return VK_STENCIL_OP_INVERT;
        case StencilOp::IncrementWrap:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp::DecrementWrap:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default:                        return VK_STENCIL_OP_KEEP;
    }
}

/**
 * @brief Convert cull mode to Vulkan cull mode flags
 */
[[nodiscard]] constexpr VkCullModeFlags toVkCullMode(CullMode mode) noexcept {
    switch (mode) {
        case CullMode::None:         return VK_CULL_MODE_NONE;
        case CullMode::Front:        return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back:         return VK_CULL_MODE_BACK_BIT;
        case CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
        default:                     return VK_CULL_MODE_BACK_BIT;
    }
}

/**
 * @brief Convert front face to Vulkan front face
 */
[[nodiscard]] constexpr VkFrontFace toVkFrontFace(FrontFace face) noexcept {
    switch (face) {
        case FrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case FrontFace::Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
        default:                          return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

/**
 * @brief Convert polygon mode to Vulkan polygon mode
 */
[[nodiscard]] constexpr VkPolygonMode toVkPolygonMode(PolygonMode mode) noexcept {
    switch (mode) {
        case PolygonMode::Fill:  return VK_POLYGON_MODE_FILL;
        case PolygonMode::Line:  return VK_POLYGON_MODE_LINE;
        case PolygonMode::Point: return VK_POLYGON_MODE_POINT;
        default:                 return VK_POLYGON_MODE_FILL;
    }
}

/**
 * @brief Convert filter mode to Vulkan filter
 */
[[nodiscard]] constexpr VkFilter toVkFilter(FilterMode mode) noexcept {
    switch (mode) {
        case FilterMode::Nearest: return VK_FILTER_NEAREST;
        case FilterMode::Linear:  return VK_FILTER_LINEAR;
        default:                  return VK_FILTER_LINEAR;
    }
}

/**
 * @brief Convert sampler mipmap mode
 */
[[nodiscard]] constexpr VkSamplerMipmapMode toVkMipmapMode(FilterMode mode) noexcept {
    switch (mode) {
        case FilterMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case FilterMode::Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:                  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

/**
 * @brief Convert address mode to Vulkan sampler address mode
 */
[[nodiscard]] constexpr VkSamplerAddressMode toVkAddressMode(AddressMode mode) noexcept {
    switch (mode) {
        case AddressMode::Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case AddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case AddressMode::ClampToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case AddressMode::ClampToBorder:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:                          return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
}

/**
 * @brief Vulkan result check helper
 * @param result Vulkan result code
 * @return true if result is VK_SUCCESS
 */
[[nodiscard]] inline bool vkSucceeded(VkResult result) noexcept {
    return result == VK_SUCCESS;
}

/**
 * @brief Convert Vulkan result to string
 */
[[nodiscard]] inline const char* vkResultToString(VkResult result) noexcept {
    switch (result) {
        case VK_SUCCESS:                        return "VK_SUCCESS";
        case VK_NOT_READY:                      return "VK_NOT_READY";
        case VK_TIMEOUT:                        return "VK_TIMEOUT";
        case VK_EVENT_SET:                      return "VK_EVENT_SET";
        case VK_EVENT_RESET:                    return "VK_EVENT_RESET";
        case VK_INCOMPLETE:                     return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:       return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:     return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:    return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:              return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:        return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:        return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:    return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:      return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:      return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:         return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:     return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:          return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_SURFACE_LOST_KHR:         return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:                 return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:          return "VK_ERROR_OUT_OF_DATE_KHR";
        default:                                return "VK_UNKNOWN_ERROR";
    }
}

} // namespace nova::render::vulkan
