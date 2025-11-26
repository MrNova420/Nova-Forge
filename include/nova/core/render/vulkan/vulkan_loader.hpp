/**
 * @file vulkan_loader.hpp
 * @brief Nova GraphicsCore™ - Dynamic Vulkan function loader
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * This module handles dynamic loading of Vulkan library and function pointers
 * to support maximum portability across all target platforms.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "vulkan_types.hpp"
#include "nova/core/types/result.hpp"

namespace nova::render::vulkan {

/**
 * @brief Nova GraphicsCore™ - Vulkan Dynamic Loader
 * 
 * Handles dynamic loading of the Vulkan library and function pointers.
 * This approach allows NovaCore to:
 * - Run on systems without Vulkan installed (graceful fallback)
 * - Support multiple Vulkan versions dynamically
 * - Minimize binary size by not linking statically
 */
class VulkanLoader {
public:
    /**
     * @brief Initialize the Vulkan loader
     * 
     * Attempts to load the Vulkan library and the vkGetInstanceProcAddr function.
     * Call this once at application startup.
     * 
     * @return Result indicating success or failure reason
     */
    static Result<void> initialize();
    
    /**
     * @brief Shutdown the Vulkan loader
     * 
     * Unloads the Vulkan library. Call this at application shutdown.
     */
    static void shutdown();
    
    /**
     * @brief Check if Vulkan is available on this system
     * @return true if Vulkan library was loaded successfully
     */
    [[nodiscard]] static bool isAvailable() noexcept;
    
    /**
     * @brief Get the maximum supported Vulkan API version
     * @return Vulkan API version packed integer (e.g., VK_API_VERSION_1_3)
     */
    [[nodiscard]] static u32 getMaxSupportedVersion() noexcept;
    
    // =========================================================================
    // Global Function Pointers (loaded before instance creation)
    // =========================================================================
    
    /// Get instance procedure address (the entry point for all other functions)
    static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    
    /// Enumerate available instance extensions
    static PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
    
    /// Enumerate available layers
    static PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
    
    /// Get instance version
    static PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
    
    /// Create Vulkan instance
    static PFN_vkCreateInstance vkCreateInstance;
    
    // =========================================================================
    // Instance Function Loading
    // =========================================================================
    
    /**
     * @brief Load instance-level function pointers
     * 
     * Must be called after creating a VkInstance.
     * Populates the VulkanInstanceFunctions structure with function pointers.
     * 
     * @param instance The VkInstance to load functions for
     * @param funcs Output structure to populate with function pointers
     * @return Result indicating success or failure
     */
    static Result<void> loadInstanceFunctions(VkInstance instance, VulkanInstanceFunctions& funcs);
    
    /**
     * @brief Load device-level function pointers
     * 
     * Must be called after creating a VkDevice.
     * Populates the VulkanDeviceFunctions structure with function pointers.
     * 
     * @param device The VkDevice to load functions for
     * @param funcs Output structure to populate with function pointers
     * @param instanceFuncs Instance functions (for vkGetDeviceProcAddr)
     * @return Result indicating success or failure
     */
    static Result<void> loadDeviceFunctions(
        VkDevice device, 
        VulkanDeviceFunctions& funcs,
        const VulkanInstanceFunctions& instanceFuncs
    );
    
private:
    /// Platform-specific library handle
    static void* s_vulkanLibrary;
    
    /// Cached maximum supported version
    static u32 s_maxSupportedVersion;
    
    /// Has the loader been initialized?
    static bool s_initialized;
    
    /**
     * @brief Load the Vulkan library
     * @return Platform-specific library handle or nullptr on failure
     */
    static void* loadVulkanLibrary();
    
    /**
     * @brief Unload the Vulkan library
     */
    static void unloadVulkanLibrary();
    
    /**
     * @brief Get a function pointer from the library
     * @param name Function name to load
     * @return Function pointer or nullptr if not found
     */
    static void* getLibraryFunction(const char* name);
};

} // namespace nova::render::vulkan
