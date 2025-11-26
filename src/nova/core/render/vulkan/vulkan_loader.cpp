/**
 * @file vulkan_loader.cpp
 * @brief Nova GraphicsCore™ - Dynamic Vulkan function loader implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Platform-specific implementation for loading the Vulkan library dynamically.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_loader.hpp"

// Platform-specific library loading headers
#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#elif defined(__ANDROID__)
    #include <dlfcn.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <dlfcn.h>
#endif

namespace nova::render::vulkan {

// ============================================================================
// Static Member Definitions
// ============================================================================

void* VulkanLoader::s_vulkanLibrary = nullptr;
u32 VulkanLoader::s_maxSupportedVersion = 0;
bool VulkanLoader::s_initialized = false;

// Global function pointers
PFN_vkGetInstanceProcAddr VulkanLoader::vkGetInstanceProcAddr = nullptr;
PFN_vkEnumerateInstanceExtensionProperties VulkanLoader::vkEnumerateInstanceExtensionProperties = nullptr;
PFN_vkEnumerateInstanceLayerProperties VulkanLoader::vkEnumerateInstanceLayerProperties = nullptr;
PFN_vkEnumerateInstanceVersion VulkanLoader::vkEnumerateInstanceVersion = nullptr;
PFN_vkCreateInstance VulkanLoader::vkCreateInstance = nullptr;

// ============================================================================
// Platform-Specific Library Loading
// ============================================================================

void* VulkanLoader::loadVulkanLibrary() {
#if defined(_WIN32)
    // Windows: Try vulkan-1.dll
    HMODULE handle = LoadLibraryA("vulkan-1.dll");
    return static_cast<void*>(handle);
    
#elif defined(__ANDROID__)
    // Android: Try libvulkan.so (system library)
    void* handle = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        // Try with full path as fallback
        handle = dlopen("/system/lib64/libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    }
    return handle;
    
#elif defined(__APPLE__)
    // macOS/iOS: Try MoltenVK or system Vulkan
    void* handle = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        handle = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
    }
    if (!handle) {
        handle = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
    }
    return handle;
    
#elif defined(__linux__)
    // Linux: Try various library names
    void* handle = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        handle = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    }
    return handle;
    
#else
    // Unknown platform
    return nullptr;
#endif
}

void VulkanLoader::unloadVulkanLibrary() {
    if (!s_vulkanLibrary) {
        return;
    }
    
#if defined(_WIN32)
    FreeLibrary(static_cast<HMODULE>(s_vulkanLibrary));
#elif defined(__ANDROID__) || defined(__linux__) || defined(__APPLE__)
    dlclose(s_vulkanLibrary);
#endif
    
    s_vulkanLibrary = nullptr;
}

void* VulkanLoader::getLibraryFunction(const char* name) {
    if (!s_vulkanLibrary) {
        return nullptr;
    }
    
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(
        static_cast<HMODULE>(s_vulkanLibrary), name));
#elif defined(__ANDROID__) || defined(__linux__) || defined(__APPLE__)
    return dlsym(s_vulkanLibrary, name);
#else
    return nullptr;
#endif
}

// ============================================================================
// Public Interface Implementation
// ============================================================================

Result<void> VulkanLoader::initialize() {
    // Already initialized?
    if (s_initialized) {
        return {};
    }
    
    // Load the Vulkan library
    s_vulkanLibrary = loadVulkanLibrary();
    if (!s_vulkanLibrary) {
        return std::unexpected(errors::notSupported(
            "Failed to load Vulkan library. Vulkan may not be installed on this system."));
    }
    
    // Load vkGetInstanceProcAddr - the entry point for all other functions
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
        getLibraryFunction("vkGetInstanceProcAddr"));
    
    if (!vkGetInstanceProcAddr) {
        unloadVulkanLibrary();
        return std::unexpected(errors::notSupported(
            "Failed to load vkGetInstanceProcAddr. Vulkan library may be corrupt."));
    }
    
    // Load global functions (instance = VK_NULL_HANDLE)
    vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
        vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties"));
    
    vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
        vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceLayerProperties"));
    
    vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
        vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance"));
    
    // vkEnumerateInstanceVersion is Vulkan 1.1+, may not exist
    vkEnumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
        vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));
    
    // Verify we got the essential functions
    if (!vkEnumerateInstanceExtensionProperties || 
        !vkEnumerateInstanceLayerProperties || 
        !vkCreateInstance) {
        unloadVulkanLibrary();
        return std::unexpected(errors::notSupported(
            "Failed to load essential Vulkan global functions."));
    }
    
    // Determine maximum supported version
    if (vkEnumerateInstanceVersion) {
        VkResult result = vkEnumerateInstanceVersion(&s_maxSupportedVersion);
        if (result != VK_SUCCESS) {
            s_maxSupportedVersion = VK_API_VERSION_1_0;
        }
    } else {
        // vkEnumerateInstanceVersion doesn't exist, so only 1.0 is supported
        s_maxSupportedVersion = VK_API_VERSION_1_0;
    }
    
    s_initialized = true;
    return {};
}

void VulkanLoader::shutdown() {
    if (!s_initialized) {
        return;
    }
    
    // Clear function pointers
    vkGetInstanceProcAddr = nullptr;
    vkEnumerateInstanceExtensionProperties = nullptr;
    vkEnumerateInstanceLayerProperties = nullptr;
    vkEnumerateInstanceVersion = nullptr;
    vkCreateInstance = nullptr;
    
    // Unload library
    unloadVulkanLibrary();
    
    s_maxSupportedVersion = 0;
    s_initialized = false;
}

bool VulkanLoader::isAvailable() noexcept {
    return s_initialized && s_vulkanLibrary != nullptr;
}

u32 VulkanLoader::getMaxSupportedVersion() noexcept {
    return s_maxSupportedVersion;
}

// ============================================================================
// Function Loading Helpers
// ============================================================================

// Macro to simplify loading instance functions
#define NOVA_LOAD_INSTANCE_FUNC(funcs, name) \
    funcs.name = reinterpret_cast<PFN_##name>( \
        vkGetInstanceProcAddr(instance, #name))

// Macro to simplify loading device functions
#define NOVA_LOAD_DEVICE_FUNC(funcs, name, getDeviceProcAddr) \
    funcs.name = reinterpret_cast<PFN_##name>( \
        getDeviceProcAddr(device, #name))

Result<void> VulkanLoader::loadInstanceFunctions(VkInstance instance, VulkanInstanceFunctions& funcs) {
    if (!s_initialized || !vkGetInstanceProcAddr) {
        return std::unexpected(errors::runtime("Vulkan loader not initialized"));
    }
    
    // Core 1.0 instance functions
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkDestroyInstance);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkEnumeratePhysicalDevices);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceProperties);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceFeatures);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceQueueFamilyProperties);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceMemoryProperties);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceFormatProperties);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkCreateDevice);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkEnumerateDeviceExtensionProperties);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetDeviceProcAddr);
    
    // Core 1.1 / VK_KHR_get_physical_device_properties2
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceProperties2);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceFeatures2);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceMemoryProperties2);
    
    // Fallback to KHR versions if core functions not available
    if (!funcs.vkGetPhysicalDeviceProperties2) {
        funcs.vkGetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR"));
    }
    if (!funcs.vkGetPhysicalDeviceFeatures2) {
        funcs.vkGetPhysicalDeviceFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2KHR"));
    }
    
    // Surface extension
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkDestroySurfaceKHR);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceSurfaceSupportKHR);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceSurfaceFormatsKHR);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkGetPhysicalDeviceSurfacePresentModesKHR);
    
    // Debug utils extension (optional)
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkCreateDebugUtilsMessengerEXT);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkDestroyDebugUtilsMessengerEXT);
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkSetDebugUtilsObjectNameEXT);
    
    // Platform-specific surface creation
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkCreateAndroidSurfaceKHR);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkCreateXcbSurfaceKHR);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkCreateWaylandSurfaceKHR);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    NOVA_LOAD_INSTANCE_FUNC(funcs, vkCreateWin32SurfaceKHR);
#endif
    
    // Verify essential functions were loaded
    if (!funcs.vkDestroyInstance || 
        !funcs.vkEnumeratePhysicalDevices || 
        !funcs.vkGetPhysicalDeviceProperties ||
        !funcs.vkCreateDevice) {
        return std::unexpected(errors::notSupported(
            "Failed to load essential Vulkan instance functions"));
    }
    
    return {};
}

Result<void> VulkanLoader::loadDeviceFunctions(
    VkDevice device,
    VulkanDeviceFunctions& funcs,
    const VulkanInstanceFunctions& instanceFuncs
) {
    if (!instanceFuncs.vkGetDeviceProcAddr) {
        return std::unexpected(errors::runtime("vkGetDeviceProcAddr not available"));
    }
    
    auto getDeviceProcAddr = instanceFuncs.vkGetDeviceProcAddr;
    
    // Core 1.0 - Device
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyDevice, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkGetDeviceQueue, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDeviceWaitIdle, getDeviceProcAddr);
    
    // Core 1.0 - Queue
    NOVA_LOAD_DEVICE_FUNC(funcs, vkQueueSubmit, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkQueueWaitIdle, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkQueuePresentKHR, getDeviceProcAddr);
    
    // Core 1.0 - Memory
    NOVA_LOAD_DEVICE_FUNC(funcs, vkAllocateMemory, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkFreeMemory, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkMapMemory, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkUnmapMemory, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkFlushMappedMemoryRanges, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkInvalidateMappedMemoryRanges, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkBindBufferMemory, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkBindImageMemory, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkGetBufferMemoryRequirements, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkGetImageMemoryRequirements, getDeviceProcAddr);
    
    // Core 1.0 - Buffer
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateBuffer, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyBuffer, getDeviceProcAddr);
    
    // Core 1.0 - Image
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateImage, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyImage, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateImageView, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyImageView, getDeviceProcAddr);
    
    // Core 1.0 - Sampler
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateSampler, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroySampler, getDeviceProcAddr);
    
    // Core 1.0 - Shader
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateShaderModule, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyShaderModule, getDeviceProcAddr);
    
    // Core 1.0 - Pipeline
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateGraphicsPipelines, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateComputePipelines, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyPipeline, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreatePipelineLayout, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyPipelineLayout, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreatePipelineCache, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyPipelineCache, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkGetPipelineCacheData, getDeviceProcAddr);
    
    // Core 1.0 - Render Pass
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateRenderPass, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyRenderPass, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateFramebuffer, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyFramebuffer, getDeviceProcAddr);
    
    // Core 1.0 - Descriptor
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateDescriptorSetLayout, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyDescriptorSetLayout, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateDescriptorPool, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyDescriptorPool, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkResetDescriptorPool, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkAllocateDescriptorSets, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkFreeDescriptorSets, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkUpdateDescriptorSets, getDeviceProcAddr);
    
    // Core 1.0 - Command Buffer
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateCommandPool, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyCommandPool, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkResetCommandPool, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkAllocateCommandBuffers, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkFreeCommandBuffers, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkBeginCommandBuffer, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkEndCommandBuffer, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkResetCommandBuffer, getDeviceProcAddr);
    
    // Core 1.0 - Command Recording
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdBindPipeline, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdSetViewport, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdSetScissor, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdBindVertexBuffers, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdBindIndexBuffer, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdBindDescriptorSets, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdPushConstants, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdDraw, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdDrawIndexed, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdDrawIndirect, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdDrawIndexedIndirect, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdDispatch, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdDispatchIndirect, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdCopyBuffer, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdCopyImage, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdCopyBufferToImage, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdCopyImageToBuffer, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdBlitImage, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdClearColorImage, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdClearDepthStencilImage, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdFillBuffer, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdPipelineBarrier, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdBeginRenderPass, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdNextSubpass, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdEndRenderPass, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdExecuteCommands, getDeviceProcAddr);
    
    // Core 1.0 - Synchronization
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateFence, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroyFence, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkResetFences, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkGetFenceStatus, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkWaitForFences, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateSemaphore, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroySemaphore, getDeviceProcAddr);
    
    // Swapchain extension
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCreateSwapchainKHR, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkDestroySwapchainKHR, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkGetSwapchainImagesKHR, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkAcquireNextImageKHR, getDeviceProcAddr);
    
    // Core 1.2 / VK_KHR_timeline_semaphore
    NOVA_LOAD_DEVICE_FUNC(funcs, vkWaitSemaphores, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkSignalSemaphore, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkGetSemaphoreCounterValue, getDeviceProcAddr);
    
    // Fallback to KHR versions
    if (!funcs.vkWaitSemaphores) {
        funcs.vkWaitSemaphores = reinterpret_cast<PFN_vkWaitSemaphores>(
            getDeviceProcAddr(device, "vkWaitSemaphoresKHR"));
    }
    if (!funcs.vkSignalSemaphore) {
        funcs.vkSignalSemaphore = reinterpret_cast<PFN_vkSignalSemaphore>(
            getDeviceProcAddr(device, "vkSignalSemaphoreKHR"));
    }
    if (!funcs.vkGetSemaphoreCounterValue) {
        funcs.vkGetSemaphoreCounterValue = reinterpret_cast<PFN_vkGetSemaphoreCounterValue>(
            getDeviceProcAddr(device, "vkGetSemaphoreCounterValueKHR"));
    }
    
    // Core 1.3 / VK_KHR_dynamic_rendering
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdBeginRendering, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdEndRendering, getDeviceProcAddr);
    
    // Fallback to KHR versions
    if (!funcs.vkCmdBeginRendering) {
        funcs.vkCmdBeginRendering = reinterpret_cast<PFN_vkCmdBeginRendering>(
            getDeviceProcAddr(device, "vkCmdBeginRenderingKHR"));
    }
    if (!funcs.vkCmdEndRendering) {
        funcs.vkCmdEndRendering = reinterpret_cast<PFN_vkCmdEndRendering>(
            getDeviceProcAddr(device, "vkCmdEndRenderingKHR"));
    }
    
    // Core 1.3 / VK_KHR_synchronization2
    NOVA_LOAD_DEVICE_FUNC(funcs, vkCmdPipelineBarrier2, getDeviceProcAddr);
    NOVA_LOAD_DEVICE_FUNC(funcs, vkQueueSubmit2, getDeviceProcAddr);
    
    // Fallback to KHR versions
    if (!funcs.vkCmdPipelineBarrier2) {
        funcs.vkCmdPipelineBarrier2 = reinterpret_cast<PFN_vkCmdPipelineBarrier2>(
            getDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR"));
    }
    if (!funcs.vkQueueSubmit2) {
        funcs.vkQueueSubmit2 = reinterpret_cast<PFN_vkQueueSubmit2>(
            getDeviceProcAddr(device, "vkQueueSubmit2KHR"));
    }
    
    // Verify essential functions were loaded
    if (!funcs.vkDestroyDevice || 
        !funcs.vkGetDeviceQueue || 
        !funcs.vkQueueSubmit ||
        !funcs.vkCreateCommandPool ||
        !funcs.vkAllocateCommandBuffers ||
        !funcs.vkBeginCommandBuffer) {
        return std::unexpected(errors::notSupported(
            "Failed to load essential Vulkan device functions"));
    }
    
    return {};
}

#undef NOVA_LOAD_INSTANCE_FUNC
#undef NOVA_LOAD_DEVICE_FUNC

} // namespace nova::render::vulkan
