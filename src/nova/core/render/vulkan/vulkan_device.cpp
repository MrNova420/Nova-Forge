/**
 * @file vulkan_device.cpp
 * @brief Nova GraphicsCore™ - Vulkan render device implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Complete Vulkan backend implementation for AAA-quality rendering.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_device.hpp"
#include "nova/core/render/buffer.hpp"
#include "nova/core/render/texture.hpp"
#include "nova/core/render/shader.hpp"
#include "nova/core/render/render_pass.hpp"
#include "nova/core/render/render_pipeline.hpp"
#include "nova/core/render/swap_chain.hpp"
#include <algorithm>
#include <cstring>
#include <set>
#include <cstdio>

namespace nova::render::vulkan {

// ============================================================================
// Static Factory
// ============================================================================

Result<std::unique_ptr<VulkanDevice>> VulkanDevice::create(const DeviceDesc& desc) {
    // Initialize the Vulkan loader if not already done
    auto loaderResult = VulkanLoader::initialize();
    if (!loaderResult) {
        return std::unexpected(loaderResult.error());
    }
    
    if (!VulkanLoader::isAvailable()) {
        return std::unexpected(errors::notSupported(
            "Vulkan is not available on this system"));
    }
    
    // Create device instance
    auto device = std::unique_ptr<VulkanDevice>(new VulkanDevice());
    device->m_validationEnabled = desc.enableValidation;
    
    // Create Vulkan instance
    auto instanceResult = device->createInstance(desc);
    if (!instanceResult) {
        return std::unexpected(instanceResult.error());
    }
    
    // Select physical device
    auto physicalResult = device->selectPhysicalDevice(desc.preferredDeviceIndex);
    if (!physicalResult) {
        return std::unexpected(physicalResult.error());
    }
    
    // Create logical device
    auto logicalResult = device->createLogicalDevice(desc);
    if (!logicalResult) {
        return std::unexpected(logicalResult.error());
    }
    
    // Create synchronization objects
    auto syncResult = device->createSyncObjects();
    if (!syncResult) {
        return std::unexpected(syncResult.error());
    }
    
    // Create command pools
    auto poolResult = device->createCommandPools();
    if (!poolResult) {
        return std::unexpected(poolResult.error());
    }
    
    return device;
}

VulkanDevice::~VulkanDevice() {
    // Wait for device to be idle before destroying resources
    if (m_device && m_deviceFuncs.vkDeviceWaitIdle) {
        m_deviceFuncs.vkDeviceWaitIdle(m_device);
    }
    
    // Destroy synchronization objects
    for (auto& sync : m_frameSyncObjects) {
        if (sync.imageAvailable && m_deviceFuncs.vkDestroySemaphore) {
            m_deviceFuncs.vkDestroySemaphore(m_device, sync.imageAvailable, nullptr);
        }
        if (sync.renderFinished && m_deviceFuncs.vkDestroySemaphore) {
            m_deviceFuncs.vkDestroySemaphore(m_device, sync.renderFinished, nullptr);
        }
        if (sync.inFlight && m_deviceFuncs.vkDestroyFence) {
            m_deviceFuncs.vkDestroyFence(m_device, sync.inFlight, nullptr);
        }
    }
    
    // Destroy command pools
    for (auto pool : m_graphicsCommandPools) {
        if (pool && m_deviceFuncs.vkDestroyCommandPool) {
            m_deviceFuncs.vkDestroyCommandPool(m_device, pool, nullptr);
        }
    }
    if (m_computeCommandPool && m_deviceFuncs.vkDestroyCommandPool) {
        m_deviceFuncs.vkDestroyCommandPool(m_device, m_computeCommandPool, nullptr);
    }
    if (m_transferCommandPool && m_deviceFuncs.vkDestroyCommandPool) {
        m_deviceFuncs.vkDestroyCommandPool(m_device, m_transferCommandPool, nullptr);
    }
    
    // Destroy logical device
    if (m_device && m_deviceFuncs.vkDestroyDevice) {
        m_deviceFuncs.vkDestroyDevice(m_device, nullptr);
    }
    
    // Destroy debug messenger
    if (m_debugMessenger && m_instanceFuncs.vkDestroyDebugUtilsMessengerEXT) {
        m_instanceFuncs.vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }
    
    // Destroy instance
    if (m_instance && m_instanceFuncs.vkDestroyInstance) {
        m_instanceFuncs.vkDestroyInstance(m_instance, nullptr);
    }
}

// ============================================================================
// Instance Creation
// ============================================================================

Result<void> VulkanDevice::createInstance(const DeviceDesc& desc) {
    // Get available extensions
    u32 extensionCount = 0;
    VulkanLoader::vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    VulkanLoader::vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
    
    // Get available layers
    u32 layerCount = 0;
    VulkanLoader::vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    
    std::vector<VkLayerProperties> availableLayers(layerCount);
    VulkanLoader::vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    // Required extensions
    std::vector<const char*> requiredExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
    };
    
    // Platform-specific surface extension
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    requiredExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    requiredExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    requiredExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
    requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
    
    // Optional extensions
    auto hasExtension = [&availableExtensions](const char* name) {
        return std::any_of(availableExtensions.begin(), availableExtensions.end(),
            [name](const VkExtensionProperties& ext) {
                return std::strcmp(ext.extensionName, name) == 0;
            });
    };
    
    // Add VK_KHR_get_physical_device_properties2 if available (needed for many features)
    if (hasExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    
    // Validation layers and debug extension
    std::vector<const char*> enabledLayers;
    
    if (m_validationEnabled) {
        auto hasLayer = [&availableLayers](const char* name) {
            return std::any_of(availableLayers.begin(), availableLayers.end(),
                [name](const VkLayerProperties& layer) {
                    return std::strcmp(layer.layerName, name) == 0;
                });
        };
        
        if (hasLayer("VK_LAYER_KHRONOS_validation")) {
            enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
        }
        
        if (hasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
    }
    
    // Determine API version
    m_apiVersion = VulkanLoader::getMaxSupportedVersion();
    
    // Clamp to our preferred version
    if (m_apiVersion > VULKAN_API_VERSION_PREFERRED) {
        m_apiVersion = VULKAN_API_VERSION_PREFERRED;
    }
    
    // Ensure minimum version
    if (m_apiVersion < VULKAN_API_VERSION_MINIMUM) {
        return std::unexpected(errors::notSupported(
            "Vulkan 1.1 or higher is required. This system only supports Vulkan 1.0."));
    }
    
    // Application info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = desc.appName.c_str();
    appInfo.applicationVersion = desc.appVersion;
    appInfo.pEngineName = "NovaCore";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = m_apiVersion;
    
    // Instance create info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<u32>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledLayerCount = static_cast<u32>(enabledLayers.size());
    createInfo.ppEnabledLayerNames = enabledLayers.data();
    
    // Debug messenger create info (for instance creation/destruction debugging)
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_validationEnabled && hasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = this;
        
        createInfo.pNext = &debugCreateInfo;
    }
    
    // Create instance
    VkResult result = VulkanLoader::vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create Vulkan instance: " + std::string(vkResultToString(result))));
    }
    
    // Load instance functions
    auto funcResult = VulkanLoader::loadInstanceFunctions(m_instance, m_instanceFuncs);
    if (!funcResult) {
        return std::unexpected(funcResult.error());
    }
    
    // Create debug messenger
    if (m_validationEnabled && m_instanceFuncs.vkCreateDebugUtilsMessengerEXT) {
        result = m_instanceFuncs.vkCreateDebugUtilsMessengerEXT(
            m_instance, &debugCreateInfo, nullptr, &m_debugMessenger);
        // Non-fatal if this fails
    }
    
    return {};
}

// ============================================================================
// Physical Device Selection
// ============================================================================

Result<void> VulkanDevice::selectPhysicalDevice(i32 preferredIndex) {
    // Enumerate physical devices
    u32 deviceCount = 0;
    m_instanceFuncs.vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        return std::unexpected(errors::notSupported(
            "No Vulkan-capable GPU found on this system"));
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    m_instanceFuncs.vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    
    // If a specific device was requested, try to use it
    if (preferredIndex >= 0 && static_cast<u32>(preferredIndex) < deviceCount) {
        if (isDeviceSuitable(devices[preferredIndex])) {
            m_physicalDevice = devices[preferredIndex];
            m_queueFamilies = findQueueFamilies(m_physicalDevice);
            queryDeviceInfo();
            return {};
        }
    }
    
    // Otherwise, pick the best suitable device
    i32 bestScore = -1;
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            i32 score = rateDeviceSuitability(device);
            if (score > bestScore) {
                bestScore = score;
                m_physicalDevice = device;
            }
        }
    }
    
    if (m_physicalDevice == VK_NULL_HANDLE) {
        return std::unexpected(errors::notSupported(
            "No suitable Vulkan GPU found. Ensure your GPU supports Vulkan 1.1+."));
    }
    
    m_queueFamilies = findQueueFamilies(m_physicalDevice);
    queryDeviceInfo();
    
    return {};
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device) const {
    QueueFamilyIndices indices;
    
    u32 queueFamilyCount = 0;
    m_instanceFuncs.vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    m_instanceFuncs.vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    for (u32 i = 0; i < queueFamilyCount; ++i) {
        const auto& queueFamily = queueFamilies[i];
        
        // Graphics queue
        if (indices.graphics == UINT32_MAX && 
            (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphics = i;
            // Assume present support for now (will be verified with surface later)
            indices.present = i;
        }
        
        // Dedicated compute queue (different from graphics)
        if (indices.compute == UINT32_MAX &&
            (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.compute = i;
        }
        
        // Dedicated transfer queue (different from graphics and compute)
        if (indices.transfer == UINT32_MAX &&
            (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            indices.transfer = i;
        }
    }
    
    // Fallback: use graphics queue for compute if no dedicated one
    if (indices.compute == UINT32_MAX) {
        indices.compute = indices.graphics;
    }
    
    // Fallback: use graphics queue for transfer if no dedicated one
    if (indices.transfer == UINT32_MAX) {
        indices.transfer = indices.graphics;
    }
    
    return indices;
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) const {
    // Get queue families
    auto queueFamilies = findQueueFamilies(device);
    if (!queueFamilies.isComplete()) {
        return false;
    }
    
    // Check for required extensions
    u32 extensionCount = 0;
    m_instanceFuncs.vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    m_instanceFuncs.vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    
    // Required extensions
    std::set<std::string> requiredExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    for (const auto& ext : availableExtensions) {
        requiredExtensions.erase(ext.extensionName);
    }
    
    return requiredExtensions.empty();
}

i32 VulkanDevice::rateDeviceSuitability(VkPhysicalDevice device) const {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    
    m_instanceFuncs.vkGetPhysicalDeviceProperties(device, &properties);
    m_instanceFuncs.vkGetPhysicalDeviceFeatures(device, &features);
    
    i32 score = 0;
    
    // Discrete GPUs have a significant performance advantage
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 10000;
    } else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 1000;
    }
    
    // Higher Vulkan version is better
    score += VK_API_VERSION_MAJOR(properties.apiVersion) * 1000;
    score += VK_API_VERSION_MINOR(properties.apiVersion) * 100;
    
    // Maximum possible size of textures affects graphics quality
    score += properties.limits.maxImageDimension2D / 100;
    
    // Prefer devices with more memory
    VkPhysicalDeviceMemoryProperties memProperties;
    m_instanceFuncs.vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
    
    for (u32 i = 0; i < memProperties.memoryHeapCount; ++i) {
        if (memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            score += static_cast<i32>(memProperties.memoryHeaps[i].size / (1024 * 1024));
        }
    }
    
    // Check for dedicated compute/transfer queues (better for async operations)
    auto queueFamilies = findQueueFamilies(device);
    if (queueFamilies.hasAsyncCompute()) {
        score += 500;
    }
    if (queueFamilies.hasAsyncTransfer()) {
        score += 250;
    }
    
    return score;
}

void VulkanDevice::queryDeviceInfo() {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memProperties;
    
    m_instanceFuncs.vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
    m_instanceFuncs.vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);
    m_instanceFuncs.vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    
    // Basic info
    m_deviceInfo.name = properties.deviceName;
    m_deviceInfo.driverVersion = std::to_string(VK_API_VERSION_MAJOR(properties.driverVersion)) + "." +
                                  std::to_string(VK_API_VERSION_MINOR(properties.driverVersion)) + "." +
                                  std::to_string(VK_API_VERSION_PATCH(properties.driverVersion));
    m_deviceInfo.apiVersion = std::to_string(VK_API_VERSION_MAJOR(properties.apiVersion)) + "." +
                               std::to_string(VK_API_VERSION_MINOR(properties.apiVersion)) + "." +
                               std::to_string(VK_API_VERSION_PATCH(properties.apiVersion));
    m_deviceInfo.vendorId = properties.vendorID;
    m_deviceInfo.deviceId = properties.deviceID;
    
    // Vendor name
    switch (properties.vendorID) {
        case 0x1002: m_deviceInfo.vendorName = "AMD"; break;
        case 0x10DE: m_deviceInfo.vendorName = "NVIDIA"; break;
        case 0x8086: m_deviceInfo.vendorName = "Intel"; break;
        case 0x13B5: m_deviceInfo.vendorName = "ARM (Mali)"; break;
        case 0x5143: m_deviceInfo.vendorName = "Qualcomm (Adreno)"; break;
        case 0x1010: m_deviceInfo.vendorName = "ImgTec (PowerVR)"; break;
        default: m_deviceInfo.vendorName = "Unknown"; break;
    }
    
    // Device type
    switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            m_deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::DiscreteGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            m_deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::IntegratedGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            m_deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::VirtualGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            m_deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::CPU;
            break;
        default:
            m_deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::Unknown;
            break;
    }
    
    // Limits
    m_deviceInfo.limits.maxTextureSize1D = properties.limits.maxImageDimension1D;
    m_deviceInfo.limits.maxTextureSize2D = properties.limits.maxImageDimension2D;
    m_deviceInfo.limits.maxTextureSize3D = properties.limits.maxImageDimension3D;
    m_deviceInfo.limits.maxTextureSizeCube = properties.limits.maxImageDimensionCube;
    m_deviceInfo.limits.maxTextureArrayLayers = properties.limits.maxImageArrayLayers;
    m_deviceInfo.limits.maxBufferSize = properties.limits.maxStorageBufferRange;
    m_deviceInfo.limits.maxUniformBufferSize = properties.limits.maxUniformBufferRange;
    m_deviceInfo.limits.maxStorageBufferSize = properties.limits.maxStorageBufferRange;
    m_deviceInfo.limits.maxPushConstantSize = properties.limits.maxPushConstantsSize;
    m_deviceInfo.limits.maxSamplers = properties.limits.maxSamplerAllocationCount;
    m_deviceInfo.limits.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    m_deviceInfo.limits.maxVertexInputAttributes = properties.limits.maxVertexInputAttributes;
    m_deviceInfo.limits.maxVertexInputBindings = properties.limits.maxVertexInputBindings;
    m_deviceInfo.limits.maxVertexInputStride = properties.limits.maxVertexInputBindingStride;
    m_deviceInfo.limits.maxColorAttachments = properties.limits.maxColorAttachments;
    m_deviceInfo.limits.maxComputeWorkGroupCount[0] = properties.limits.maxComputeWorkGroupCount[0];
    m_deviceInfo.limits.maxComputeWorkGroupCount[1] = properties.limits.maxComputeWorkGroupCount[1];
    m_deviceInfo.limits.maxComputeWorkGroupCount[2] = properties.limits.maxComputeWorkGroupCount[2];
    m_deviceInfo.limits.maxComputeWorkGroupSize[0] = properties.limits.maxComputeWorkGroupSize[0];
    m_deviceInfo.limits.maxComputeWorkGroupSize[1] = properties.limits.maxComputeWorkGroupSize[1];
    m_deviceInfo.limits.maxComputeWorkGroupSize[2] = properties.limits.maxComputeWorkGroupSize[2];
    m_deviceInfo.limits.maxComputeWorkGroupInvocations = properties.limits.maxComputeWorkGroupInvocations;
    m_deviceInfo.limits.maxComputeSharedMemorySize = properties.limits.maxComputeSharedMemorySize;
    m_deviceInfo.limits.maxBoundDescriptorSets = properties.limits.maxBoundDescriptorSets;
    m_deviceInfo.limits.maxDescriptorSetSamplers = properties.limits.maxDescriptorSetSamplers;
    m_deviceInfo.limits.maxDescriptorSetUniformBuffers = properties.limits.maxDescriptorSetUniformBuffers;
    m_deviceInfo.limits.maxDescriptorSetStorageBuffers = properties.limits.maxDescriptorSetStorageBuffers;
    m_deviceInfo.limits.maxDescriptorSetSampledImages = properties.limits.maxDescriptorSetSampledImages;
    m_deviceInfo.limits.maxDescriptorSetStorageImages = properties.limits.maxDescriptorSetStorageImages;
    
    // Calculate total device memory
    m_deviceInfo.limits.totalDeviceMemory = 0;
    for (u32 i = 0; i < memProperties.memoryHeapCount; ++i) {
        if (memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            m_deviceInfo.limits.totalDeviceMemory += memProperties.memoryHeaps[i].size;
        }
    }
    m_deviceInfo.limits.availableDeviceMemory = m_deviceInfo.limits.totalDeviceMemory;
    
    // Features
    m_deviceInfo.features.geometryShader = features.geometryShader;
    m_deviceInfo.features.tessellationShader = features.tessellationShader;
    m_deviceInfo.features.multiDrawIndirect = features.multiDrawIndirect;
    m_deviceInfo.features.drawIndirectCount = false; // Check extension
    m_deviceInfo.features.samplerAnisotropy = features.samplerAnisotropy;
    m_deviceInfo.features.textureCompressionBC = features.textureCompressionBC;
    m_deviceInfo.features.textureCompressionETC2 = features.textureCompressionETC2;
    m_deviceInfo.features.textureCompressionASTC = features.textureCompressionASTC_LDR;
    m_deviceInfo.features.multiViewport = features.multiViewport;
    m_deviceInfo.features.fillModeNonSolid = features.fillModeNonSolid;
    m_deviceInfo.features.wideLines = features.wideLines;
    m_deviceInfo.features.depthClamp = features.depthClamp;
    m_deviceInfo.features.depthBiasClamp = features.depthBiasClamp;
    m_deviceInfo.features.independentBlend = features.independentBlend;
    m_deviceInfo.features.dualSrcBlend = features.dualSrcBlend;
    m_deviceInfo.features.computeShader = true; // Always supported in Vulkan
    m_deviceInfo.features.asyncCompute = m_queueFamilies.hasAsyncCompute();
    m_deviceInfo.features.asyncTransfer = m_queueFamilies.hasAsyncTransfer();
    
    // Determine quality tier
    m_deviceInfo.recommendedTier = determineQualityTier();
}

QualityTier VulkanDevice::determineQualityTier() const {
    // Based on device capabilities, determine the optimal quality tier
    // This follows the Nova VisualLOD™ quality scaling system
    
    const auto& limits = m_deviceInfo.limits;
    const auto& features = m_deviceInfo.features;
    
    // Ultra tier: High-end desktop GPUs or flagship mobile (2023+)
    // - 8K texture support
    // - Ray tracing capable
    // - >8GB VRAM
    if (limits.maxTextureSize2D >= 8192 &&
        features.rayTracing &&
        limits.totalDeviceMemory >= 8ULL * 1024 * 1024 * 1024) {
        return QualityTier::Ultra;
    }
    
    // High tier: Mid-high desktop or high-end mobile
    // - 4K texture support
    // - >4GB VRAM
    // - Bindless resources
    if (limits.maxTextureSize2D >= 4096 &&
        limits.totalDeviceMemory >= 4ULL * 1024 * 1024 * 1024) {
        return QualityTier::High;
    }
    
    // Standard tier: Mid-range devices (target for 60 FPS)
    // - 2K texture support
    // - >2GB VRAM
    // - Compute shaders
    if (limits.maxTextureSize2D >= 2048 &&
        limits.totalDeviceMemory >= 2ULL * 1024 * 1024 * 1024 &&
        features.computeShader) {
        return QualityTier::Standard;
    }
    
    // Basic tier: Low-end devices
    // - 1K textures
    // - >1GB VRAM
    if (limits.maxTextureSize2D >= 1024 &&
        limits.totalDeviceMemory >= 1ULL * 1024 * 1024 * 1024) {
        return QualityTier::Basic;
    }
    
    // Minimal tier: Ultra-low end devices
    return QualityTier::Minimal;
}

// ============================================================================
// Logical Device Creation
// ============================================================================

Result<void> VulkanDevice::createLogicalDevice(const DeviceDesc& desc) {
    // Collect unique queue families
    std::set<u32> uniqueQueueFamilies = {
        m_queueFamilies.graphics
    };
    
    if (m_queueFamilies.present != m_queueFamilies.graphics) {
        uniqueQueueFamilies.insert(m_queueFamilies.present);
    }
    if (m_queueFamilies.hasAsyncCompute()) {
        uniqueQueueFamilies.insert(m_queueFamilies.compute);
    }
    if (m_queueFamilies.hasAsyncTransfer()) {
        uniqueQueueFamilies.insert(m_queueFamilies.transfer);
    }
    
    // Create queue create infos
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    
    for (u32 queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // Get available device extensions
    u32 extensionCount = 0;
    m_instanceFuncs.vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
    
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    m_instanceFuncs.vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, availableExtensions.data());
    
    auto hasExtension = [&availableExtensions](const char* name) {
        return std::any_of(availableExtensions.begin(), availableExtensions.end(),
            [name](const VkExtensionProperties& ext) {
                return std::strcmp(ext.extensionName, name) == 0;
            });
    };
    
    // Required extensions
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    // Optional extensions based on API version and availability
    if (m_apiVersion < VK_API_VERSION_1_2) {
        if (hasExtension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)) {
            deviceExtensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
        }
    }
    
    if (m_apiVersion < VK_API_VERSION_1_3) {
        if (hasExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
            deviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        }
        if (hasExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)) {
            deviceExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        }
    }
    
    // Ray tracing extensions (optional)
    if (desc.enableRayTracing) {
        if (hasExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) &&
            hasExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)) {
            deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            m_deviceInfo.features.rayTracing = true;
        }
    }
    
    // Mesh shader extension (optional)
    if (hasExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME)) {
        deviceExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
        m_deviceInfo.features.meshShader = true;
    }
    
    // Request features
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = m_deviceInfo.features.samplerAnisotropy;
    deviceFeatures.textureCompressionBC = m_deviceInfo.features.textureCompressionBC;
    deviceFeatures.textureCompressionETC2 = m_deviceInfo.features.textureCompressionETC2;
    deviceFeatures.textureCompressionASTC_LDR = m_deviceInfo.features.textureCompressionASTC;
    deviceFeatures.fillModeNonSolid = m_deviceInfo.features.fillModeNonSolid;
    deviceFeatures.wideLines = m_deviceInfo.features.wideLines;
    deviceFeatures.multiDrawIndirect = m_deviceInfo.features.multiDrawIndirect;
    deviceFeatures.depthClamp = m_deviceInfo.features.depthClamp;
    deviceFeatures.depthBiasClamp = m_deviceInfo.features.depthBiasClamp;
    deviceFeatures.independentBlend = m_deviceInfo.features.independentBlend;
    deviceFeatures.dualSrcBlend = m_deviceInfo.features.dualSrcBlend;
    
    // Create device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<u32>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    
    VkResult result = m_instanceFuncs.vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            "Failed to create Vulkan logical device: " + std::string(vkResultToString(result))));
    }
    
    // Load device functions
    auto funcResult = VulkanLoader::loadDeviceFunctions(m_device, m_deviceFuncs, m_instanceFuncs);
    if (!funcResult) {
        return std::unexpected(funcResult.error());
    }
    
    // Get queues
    m_deviceFuncs.vkGetDeviceQueue(m_device, m_queueFamilies.graphics, 0, &m_graphicsQueue);
    m_deviceFuncs.vkGetDeviceQueue(m_device, m_queueFamilies.present, 0, &m_presentQueue);
    
    if (m_queueFamilies.hasAsyncCompute()) {
        m_deviceFuncs.vkGetDeviceQueue(m_device, m_queueFamilies.compute, 0, &m_computeQueue);
    } else {
        m_computeQueue = m_graphicsQueue;
    }
    
    if (m_queueFamilies.hasAsyncTransfer()) {
        m_deviceFuncs.vkGetDeviceQueue(m_device, m_queueFamilies.transfer, 0, &m_transferQueue);
    } else {
        m_transferQueue = m_graphicsQueue;
    }
    
    return {};
}

// ============================================================================
// Synchronization & Command Pools
// ============================================================================

Result<void> VulkanDevice::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // Start signaled
    
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkResult result = m_deviceFuncs.vkCreateSemaphore(
            m_device, &semaphoreInfo, nullptr, &m_frameSyncObjects[i].imageAvailable);
        if (result != VK_SUCCESS) {
            return std::unexpected(errors::graphics("Failed to create image available semaphore"));
        }
        
        result = m_deviceFuncs.vkCreateSemaphore(
            m_device, &semaphoreInfo, nullptr, &m_frameSyncObjects[i].renderFinished);
        if (result != VK_SUCCESS) {
            return std::unexpected(errors::graphics("Failed to create render finished semaphore"));
        }
        
        result = m_deviceFuncs.vkCreateFence(
            m_device, &fenceInfo, nullptr, &m_frameSyncObjects[i].inFlight);
        if (result != VK_SUCCESS) {
            return std::unexpected(errors::graphics("Failed to create in-flight fence"));
        }
    }
    
    return {};
}

Result<void> VulkanDevice::createCommandPools() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_queueFamilies.graphics;
    
    // Create graphics command pools (one per frame)
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VkResult result = m_deviceFuncs.vkCreateCommandPool(
            m_device, &poolInfo, nullptr, &m_graphicsCommandPools[i]);
        if (result != VK_SUCCESS) {
            return std::unexpected(errors::graphics("Failed to create graphics command pool"));
        }
    }
    
    // Create compute command pool (if async compute available)
    if (m_queueFamilies.hasAsyncCompute()) {
        poolInfo.queueFamilyIndex = m_queueFamilies.compute;
        VkResult result = m_deviceFuncs.vkCreateCommandPool(
            m_device, &poolInfo, nullptr, &m_computeCommandPool);
        if (result != VK_SUCCESS) {
            return std::unexpected(errors::graphics("Failed to create compute command pool"));
        }
    }
    
    // Create transfer command pool (if async transfer available)
    if (m_queueFamilies.hasAsyncTransfer()) {
        poolInfo.queueFamilyIndex = m_queueFamilies.transfer;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;  // Short-lived commands
        VkResult result = m_deviceFuncs.vkCreateCommandPool(
            m_device, &poolInfo, nullptr, &m_transferCommandPool);
        if (result != VK_SUCCESS) {
            return std::unexpected(errors::graphics("Failed to create transfer command pool"));
        }
    }
    
    return {};
}

// ============================================================================
// Debug Callback
// ============================================================================

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDevice::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    (void)pUserData;
    
    // Determine severity prefix
    const char* severityStr = "INFO";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        severityStr = "ERROR";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        severityStr = "WARNING";
    }
    
    // Determine type prefix
    const char* typeStr = "General";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        typeStr = "Validation";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        typeStr = "Performance";
    }
    
    // Log to stderr for now (TODO: integrate with NovaCore Logger)
    std::fprintf(stderr, "[Vulkan %s/%s] %s\n", severityStr, typeStr, pCallbackData->pMessage);
    
    return VK_FALSE;
}

// ============================================================================
// Memory Type Finder
// ============================================================================

u32 VulkanDevice::findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    
    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    // Fallback: try to find any suitable memory type
    for (u32 i = 0; i < memProperties.memoryTypeCount; ++i) {
        if (typeFilter & (1 << i)) {
            return i;
        }
    }
    
    return UINT32_MAX;
}

// ============================================================================
// Buffer Creation - Full Implementation
// ============================================================================

BufferHandle VulkanDevice::createBuffer(const BufferDesc& desc) {
    BufferResource resource;
    resource.size = desc.size;
    resource.usage = desc.usage;
    resource.memoryUsage = desc.memoryUsage;
    
    // Build Vulkan usage flags from NovaCore usage
    VkBufferUsageFlags vkUsage = 0;
    
    if (hasFlag(desc.usage, BufferUsage::Vertex)) {
        vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (hasFlag(desc.usage, BufferUsage::Index)) {
        vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (hasFlag(desc.usage, BufferUsage::Uniform)) {
        vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (hasFlag(desc.usage, BufferUsage::Storage)) {
        vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (hasFlag(desc.usage, BufferUsage::Indirect)) {
        vkUsage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if (hasFlag(desc.usage, BufferUsage::TransferSrc)) {
        vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (hasFlag(desc.usage, BufferUsage::TransferDst)) {
        vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
    // Create buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.size;
    bufferInfo.usage = vkUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (m_deviceFuncs.vkCreateBuffer(m_device, &bufferInfo, nullptr, &resource.buffer) != VK_SUCCESS) {
        return BufferHandle::invalid();
    }
    
    // Get memory requirements
    VkMemoryRequirements memRequirements;
    m_deviceFuncs.vkGetBufferMemoryRequirements(m_device, resource.buffer, &memRequirements);
    
    // Determine memory properties based on usage
    VkMemoryPropertyFlags memProperties = 0;
    switch (desc.memoryUsage) {
        case MemoryUsage::GpuOnly:
            memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case MemoryUsage::CpuOnly:
            memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        case MemoryUsage::CpuToGpu:
            memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            resource.persistentlyMapped = true;
            break;
        case MemoryUsage::GpuToCpu:
            memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            break;
    }
    
    // Allocate memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, memProperties);
    
    if (allocInfo.memoryTypeIndex == UINT32_MAX) {
        m_deviceFuncs.vkDestroyBuffer(m_device, resource.buffer, nullptr);
        return BufferHandle::invalid();
    }
    
    if (m_deviceFuncs.vkAllocateMemory(m_device, &allocInfo, nullptr, &resource.memory) != VK_SUCCESS) {
        m_deviceFuncs.vkDestroyBuffer(m_device, resource.buffer, nullptr);
        return BufferHandle::invalid();
    }
    
    // Bind memory to buffer
    m_deviceFuncs.vkBindBufferMemory(m_device, resource.buffer, resource.memory, 0);
    
    // Persistently map if requested
    if (resource.persistentlyMapped) {
        m_deviceFuncs.vkMapMemory(m_device, resource.memory, 0, desc.size, 0, &resource.mappedPtr);
    }
    
    // Upload initial data if provided
    if (desc.initialData && desc.size > 0) {
        void* mappedData = resource.mappedPtr;
        if (!mappedData) {
            m_deviceFuncs.vkMapMemory(m_device, resource.memory, 0, desc.size, 0, &mappedData);
        }
        if (mappedData) {
            std::memcpy(mappedData, desc.initialData, desc.size);
            if (!resource.persistentlyMapped) {
                m_deviceFuncs.vkUnmapMemory(m_device, resource.memory);
            }
        }
    }
    
    u64 id = m_nextResourceId++;
    m_buffers[id] = resource;
    
    m_frameStats.buffersCreated++;
    m_frameStats.bufferMemoryUsed += desc.size;
    
    return BufferHandle(id);
}

// ============================================================================
// Texture Creation - Full Implementation
// ============================================================================

TextureHandle VulkanDevice::createTexture(const TextureDesc& desc) {
    TextureResource resource;
    resource.width = desc.width;
    resource.height = desc.height;
    resource.depth = desc.depth;
    resource.mipLevels = desc.mipLevels;
    resource.arrayLayers = desc.arrayLayers;
    resource.type = desc.type;
    
    // Convert format
    resource.format = toVkFormat(desc.format);
    if (resource.format == VK_FORMAT_UNDEFINED) {
        return TextureHandle::invalid();
    }
    
    // Determine image type
    VkImageType imageType = VK_IMAGE_TYPE_2D;
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    
    switch (desc.type) {
        case TextureType::Texture1D:
            imageType = VK_IMAGE_TYPE_1D;
            viewType = VK_IMAGE_VIEW_TYPE_1D;
            break;
        case TextureType::Texture2D:
            imageType = VK_IMAGE_TYPE_2D;
            viewType = desc.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            break;
        case TextureType::Texture3D:
            imageType = VK_IMAGE_TYPE_3D;
            viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        case TextureType::TextureCube:
            imageType = VK_IMAGE_TYPE_2D;
            viewType = desc.arrayLayers > 6 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
            break;
    }
    
    // Build usage flags
    VkImageUsageFlags vkUsage = 0;
    if (hasFlag(desc.usage, TextureUsage::Sampled)) {
        vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (hasFlag(desc.usage, TextureUsage::Storage)) {
        vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (hasFlag(desc.usage, TextureUsage::RenderTarget)) {
        vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (hasFlag(desc.usage, TextureUsage::DepthStencil)) {
        vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (hasFlag(desc.usage, TextureUsage::TransferSrc)) {
        vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (hasFlag(desc.usage, TextureUsage::TransferDst)) {
        vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    
    // Create image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = imageType;
    imageInfo.extent.width = desc.width;
    imageInfo.extent.height = desc.height;
    imageInfo.extent.depth = desc.depth;
    imageInfo.mipLevels = desc.mipLevels;
    imageInfo.arrayLayers = desc.arrayLayers;
    imageInfo.format = resource.format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = vkUsage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = static_cast<VkSampleCountFlagBits>(desc.samples);
    
    if (desc.type == TextureType::TextureCube) {
        imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    
    if (m_deviceFuncs.vkCreateImage(m_device, &imageInfo, nullptr, &resource.image) != VK_SUCCESS) {
        return TextureHandle::invalid();
    }
    
    // Get memory requirements
    VkMemoryRequirements memRequirements;
    m_deviceFuncs.vkGetImageMemoryRequirements(m_device, resource.image, &memRequirements);
    
    // Allocate memory (GPU-only for textures)
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    if (allocInfo.memoryTypeIndex == UINT32_MAX || 
        m_deviceFuncs.vkAllocateMemory(m_device, &allocInfo, nullptr, &resource.memory) != VK_SUCCESS) {
        m_deviceFuncs.vkDestroyImage(m_device, resource.image, nullptr);
        return TextureHandle::invalid();
    }
    
    m_deviceFuncs.vkBindImageMemory(m_device, resource.image, resource.memory, 0);
    
    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = resource.image;
    viewInfo.viewType = viewType;
    viewInfo.format = resource.format;
    viewInfo.subresourceRange.aspectMask = hasFlag(desc.usage, TextureUsage::DepthStencil) 
        ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = desc.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = desc.arrayLayers;
    
    if (m_deviceFuncs.vkCreateImageView(m_device, &viewInfo, nullptr, &resource.view) != VK_SUCCESS) {
        m_deviceFuncs.vkFreeMemory(m_device, resource.memory, nullptr);
        m_deviceFuncs.vkDestroyImage(m_device, resource.image, nullptr);
        return TextureHandle::invalid();
    }
    
    u64 id = m_nextResourceId++;
    m_textures[id] = resource;
    
    m_frameStats.texturesCreated++;
    m_frameStats.textureMemoryUsed += memRequirements.size;
    
    return TextureHandle(id);
}

// ============================================================================
// Sampler Creation - Full Implementation
// ============================================================================

SamplerHandle VulkanDevice::createSampler(const SamplerDesc& desc) {
    SamplerResource resource;
    
    auto toVkFilter = [](FilterMode mode) -> VkFilter {
        switch (mode) {
            case FilterMode::Nearest: return VK_FILTER_NEAREST;
            case FilterMode::Linear: return VK_FILTER_LINEAR;
            default: return VK_FILTER_LINEAR;
        }
    };
    
    auto toVkMipmapMode = [](FilterMode mode) -> VkSamplerMipmapMode {
        switch (mode) {
            case FilterMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            case FilterMode::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            default: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    };
    
    auto toVkAddressMode = [](WrapMode mode) -> VkSamplerAddressMode {
        switch (mode) {
            case WrapMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case WrapMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case WrapMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case WrapMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    };
    
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = toVkFilter(desc.magFilter);
    samplerInfo.minFilter = toVkFilter(desc.minFilter);
    samplerInfo.mipmapMode = toVkMipmapMode(desc.mipFilter);
    samplerInfo.addressModeU = toVkAddressMode(desc.wrapU);
    samplerInfo.addressModeV = toVkAddressMode(desc.wrapV);
    samplerInfo.addressModeW = toVkAddressMode(desc.wrapW);
    samplerInfo.mipLodBias = desc.mipLodBias;
    samplerInfo.anisotropyEnable = desc.maxAnisotropy > 1.0f ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = desc.maxAnisotropy;
    samplerInfo.compareEnable = desc.compareEnable ? VK_TRUE : VK_FALSE;
    samplerInfo.compareOp = toVkCompareOp(desc.compareOp);
    samplerInfo.minLod = desc.minLod;
    samplerInfo.maxLod = desc.maxLod;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    
    if (m_deviceFuncs.vkCreateSampler(m_device, &samplerInfo, nullptr, &resource.sampler) != VK_SUCCESS) {
        return SamplerHandle::invalid();
    }
    
    u64 id = m_nextResourceId++;
    m_samplers[id] = resource;
    
    return SamplerHandle(id);
}

// ============================================================================
// Shader Creation - Full Implementation
// ============================================================================

ShaderHandle VulkanDevice::createShader(const ShaderDesc& desc) {
    ShaderResource resource;
    resource.stage = desc.stage;
    resource.entryPoint = desc.entryPoint.empty() ? "main" : std::string(desc.entryPoint);
    
    // Shader code must be SPIR-V
    if (desc.codeSize == 0 || !desc.code) {
        return ShaderHandle::invalid();
    }
    
    // SPIR-V must be aligned to 4 bytes
    if (desc.codeSize % 4 != 0) {
        return ShaderHandle::invalid();
    }
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = desc.codeSize;
    createInfo.pCode = reinterpret_cast<const u32*>(desc.code);
    
    if (m_deviceFuncs.vkCreateShaderModule(m_device, &createInfo, nullptr, &resource.module) != VK_SUCCESS) {
        return ShaderHandle::invalid();
    }
    
    u64 id = m_nextResourceId++;
    m_shaders[id] = resource;
    
    return ShaderHandle(id);
}

// ============================================================================
// Pipeline Creation - Full Implementation
// ============================================================================

PipelineHandle VulkanDevice::createGraphicsPipeline(const GraphicsPipelineDesc& desc) {
    PipelineResource resource;
    resource.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    
    // Shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    
    auto addShaderStage = [&](ShaderHandle handle, VkShaderStageFlagBits stage) {
        auto it = m_shaders.find(handle.id());
        if (it != m_shaders.end()) {
            VkPipelineShaderStageCreateInfo stageInfo{};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.stage = stage;
            stageInfo.module = it->second.module;
            stageInfo.pName = it->second.entryPoint.c_str();
            shaderStages.push_back(stageInfo);
        }
    };
    
    if (desc.vertexShader.isValid()) {
        addShaderStage(desc.vertexShader, VK_SHADER_STAGE_VERTEX_BIT);
    }
    if (desc.fragmentShader.isValid()) {
        addShaderStage(desc.fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    if (desc.geometryShader.isValid()) {
        addShaderStage(desc.geometryShader, VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    if (desc.tessControlShader.isValid()) {
        addShaderStage(desc.tessControlShader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    }
    if (desc.tessEvalShader.isValid()) {
        addShaderStage(desc.tessEvalShader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    }
    
    // Vertex input state
    std::vector<VkVertexInputBindingDescription> bindingDescs;
    std::vector<VkVertexInputAttributeDescription> attributeDescs;
    
    for (const auto& binding : desc.vertexBindings) {
        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = binding.binding;
        bindingDesc.stride = binding.stride;
        bindingDesc.inputRate = binding.perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescs.push_back(bindingDesc);
    }
    
    for (const auto& attr : desc.vertexAttributes) {
        VkVertexInputAttributeDescription attrDesc{};
        attrDesc.location = attr.location;
        attrDesc.binding = attr.binding;
        attrDesc.format = toVkFormat(attr.format);
        attrDesc.offset = attr.offset;
        attributeDescs.push_back(attrDesc);
    }
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<u32>(bindingDescs.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescs.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescs.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = toVkPrimitiveTopology(desc.primitiveTopology);
    inputAssembly.primitiveRestartEnable = desc.primitiveRestartEnable ? VK_TRUE : VK_FALSE;
    
    // Viewport and scissor (dynamic)
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    
    // Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = desc.rasterState.depthClampEnable ? VK_TRUE : VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = toVkPolygonMode(desc.rasterState.fillMode);
    rasterizer.cullMode = toVkCullMode(desc.rasterState.cullMode);
    rasterizer.frontFace = desc.rasterState.frontFaceCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = desc.rasterState.depthBiasEnable ? VK_TRUE : VK_FALSE;
    rasterizer.depthBiasConstantFactor = desc.rasterState.depthBiasConstant;
    rasterizer.depthBiasSlopeFactor = desc.rasterState.depthBiasSlope;
    rasterizer.lineWidth = 1.0f;
    
    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(desc.sampleCount);
    
    // Depth stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = desc.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = desc.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = toVkCompareOp(desc.depthStencilState.depthCompareOp);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = desc.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;
    
    // Color blending
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    for (const auto& attachment : desc.colorAttachments) {
        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.blendEnable = attachment.blendEnable ? VK_TRUE : VK_FALSE;
        blendAttachment.srcColorBlendFactor = toVkBlendFactor(attachment.srcColorBlendFactor);
        blendAttachment.dstColorBlendFactor = toVkBlendFactor(attachment.dstColorBlendFactor);
        blendAttachment.colorBlendOp = toVkBlendOp(attachment.colorBlendOp);
        blendAttachment.srcAlphaBlendFactor = toVkBlendFactor(attachment.srcAlphaBlendFactor);
        blendAttachment.dstAlphaBlendFactor = toVkBlendFactor(attachment.dstAlphaBlendFactor);
        blendAttachment.alphaBlendOp = toVkBlendOp(attachment.alphaBlendOp);
        colorBlendAttachments.push_back(blendAttachment);
    }
    
    if (colorBlendAttachments.empty()) {
        // Default attachment
        VkPipelineColorBlendAttachmentState defaultAttachment{};
        defaultAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        defaultAttachment.blendEnable = VK_FALSE;
        colorBlendAttachments.push_back(defaultAttachment);
    }
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = static_cast<u32>(colorBlendAttachments.size());
    colorBlending.pAttachments = colorBlendAttachments.data();
    
    // Dynamic states
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Pipeline layout
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    if (m_deviceFuncs.vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &resource.layout) != VK_SUCCESS) {
        return PipelineHandle::invalid();
    }
    
    // Look up render pass
    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    auto rpIt = m_renderPasses.find(desc.renderPass.id());
    if (rpIt != m_renderPasses.end()) {
        vkRenderPass = rpIt->second.renderPass;
    }
    
    // Create pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<u32>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = resource.layout;
    pipelineInfo.renderPass = vkRenderPass;
    pipelineInfo.subpass = 0;
    
    if (m_deviceFuncs.vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resource.pipeline) != VK_SUCCESS) {
        m_deviceFuncs.vkDestroyPipelineLayout(m_device, resource.layout, nullptr);
        return PipelineHandle::invalid();
    }
    
    u64 id = m_nextResourceId++;
    m_pipelines[id] = resource;
    
    m_frameStats.pipelinesCreated++;
    
    return PipelineHandle(id);
}

PipelineHandle VulkanDevice::createComputePipeline(const ComputePipelineDesc& desc) {
    PipelineResource resource;
    resource.bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    
    // Get compute shader
    auto shaderIt = m_shaders.find(desc.computeShader.id());
    if (shaderIt == m_shaders.end()) {
        return PipelineHandle::invalid();
    }
    
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = shaderIt->second.module;
    stageInfo.pName = shaderIt->second.entryPoint.c_str();
    
    // Pipeline layout
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    if (m_deviceFuncs.vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &resource.layout) != VK_SUCCESS) {
        return PipelineHandle::invalid();
    }
    
    // Create compute pipeline
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = resource.layout;
    
    if (m_deviceFuncs.vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resource.pipeline) != VK_SUCCESS) {
        m_deviceFuncs.vkDestroyPipelineLayout(m_device, resource.layout, nullptr);
        return PipelineHandle::invalid();
    }
    
    u64 id = m_nextResourceId++;
    m_pipelines[id] = resource;
    
    m_frameStats.pipelinesCreated++;
    
    return PipelineHandle(id);
}

// ============================================================================
// Render Pass Creation - Full Implementation
// ============================================================================

RenderPassHandle VulkanDevice::createRenderPass(const RenderPassDesc& desc) {
    RenderPassResource resource;
    resource.colorAttachmentCount = static_cast<u32>(desc.colorAttachments.size());
    resource.hasDepthStencil = desc.depthStencilAttachment.format != TextureFormat::Unknown;
    
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorRefs;
    VkAttachmentReference depthRef{};
    
    // Color attachments
    for (u32 i = 0; i < desc.colorAttachments.size(); ++i) {
        const auto& colorAttach = desc.colorAttachments[i];
        
        VkAttachmentDescription attachment{};
        attachment.format = toVkFormat(colorAttach.format);
        attachment.samples = static_cast<VkSampleCountFlagBits>(colorAttach.samples);
        attachment.loadOp = toVkLoadOp(colorAttach.loadOp);
        attachment.storeOp = toVkStoreOp(colorAttach.storeOp);
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments.push_back(attachment);
        
        VkAttachmentReference ref{};
        ref.attachment = i;
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorRefs.push_back(ref);
    }
    
    // Depth attachment
    if (resource.hasDepthStencil) {
        VkAttachmentDescription attachment{};
        attachment.format = toVkFormat(desc.depthStencilAttachment.format);
        attachment.samples = static_cast<VkSampleCountFlagBits>(desc.depthStencilAttachment.samples);
        attachment.loadOp = toVkLoadOp(desc.depthStencilAttachment.depthLoadOp);
        attachment.storeOp = toVkStoreOp(desc.depthStencilAttachment.depthStoreOp);
        attachment.stencilLoadOp = toVkLoadOp(desc.depthStencilAttachment.stencilLoadOp);
        attachment.stencilStoreOp = toVkStoreOp(desc.depthStencilAttachment.stencilStoreOp);
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(attachment);
        
        depthRef.attachment = static_cast<u32>(attachments.size() - 1);
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    
    // Subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<u32>(colorRefs.size());
    subpass.pColorAttachments = colorRefs.data();
    subpass.pDepthStencilAttachment = resource.hasDepthStencil ? &depthRef : nullptr;
    
    // Subpass dependencies
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    // Create render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<u32>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    if (m_deviceFuncs.vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &resource.renderPass) != VK_SUCCESS) {
        return RenderPassHandle::invalid();
    }
    
    u64 id = m_nextResourceId++;
    m_renderPasses[id] = resource;
    
    return RenderPassHandle(id);
}

// ============================================================================
// Framebuffer Creation - Full Implementation
// ============================================================================

FramebufferHandle VulkanDevice::createFramebuffer(const FramebufferDesc& desc) {
    FramebufferResource resource;
    resource.width = desc.width;
    resource.height = desc.height;
    resource.renderPass = desc.renderPass;
    
    // Get render pass
    auto rpIt = m_renderPasses.find(desc.renderPass.id());
    if (rpIt == m_renderPasses.end()) {
        return FramebufferHandle::invalid();
    }
    
    // Collect image views
    std::vector<VkImageView> attachments;
    
    for (const auto& texHandle : desc.colorAttachments) {
        auto texIt = m_textures.find(texHandle.id());
        if (texIt != m_textures.end()) {
            attachments.push_back(texIt->second.view);
        }
    }
    
    if (desc.depthStencilAttachment.isValid()) {
        auto texIt = m_textures.find(desc.depthStencilAttachment.id());
        if (texIt != m_textures.end()) {
            attachments.push_back(texIt->second.view);
        }
    }
    
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = rpIt->second.renderPass;
    framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = desc.width;
    framebufferInfo.height = desc.height;
    framebufferInfo.layers = 1;
    
    if (m_deviceFuncs.vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &resource.framebuffer) != VK_SUCCESS) {
        return FramebufferHandle::invalid();
    }
    
    u64 id = m_nextResourceId++;
    m_framebuffers[id] = resource;
    
    return FramebufferHandle(id);
}

// ============================================================================
// Resource Destruction - Full Implementation
// ============================================================================

void VulkanDevice::destroyBuffer(BufferHandle handle) {
    auto it = m_buffers.find(handle.id());
    if (it != m_buffers.end()) {
        if (it->second.mappedPtr) {
            m_deviceFuncs.vkUnmapMemory(m_device, it->second.memory);
        }
        m_deviceFuncs.vkDestroyBuffer(m_device, it->second.buffer, nullptr);
        m_deviceFuncs.vkFreeMemory(m_device, it->second.memory, nullptr);
        m_frameStats.bufferMemoryUsed -= it->second.size;
        m_buffers.erase(it);
    }
}

void VulkanDevice::destroyTexture(TextureHandle handle) {
    auto it = m_textures.find(handle.id());
    if (it != m_textures.end()) {
        m_deviceFuncs.vkDestroyImageView(m_device, it->second.view, nullptr);
        m_deviceFuncs.vkDestroyImage(m_device, it->second.image, nullptr);
        m_deviceFuncs.vkFreeMemory(m_device, it->second.memory, nullptr);
        m_textures.erase(it);
    }
}

void VulkanDevice::destroySampler(SamplerHandle handle) {
    auto it = m_samplers.find(handle.id());
    if (it != m_samplers.end()) {
        m_deviceFuncs.vkDestroySampler(m_device, it->second.sampler, nullptr);
        m_samplers.erase(it);
    }
}

void VulkanDevice::destroyShader(ShaderHandle handle) {
    auto it = m_shaders.find(handle.id());
    if (it != m_shaders.end()) {
        m_deviceFuncs.vkDestroyShaderModule(m_device, it->second.module, nullptr);
        m_shaders.erase(it);
    }
}

void VulkanDevice::destroyPipeline(PipelineHandle handle) {
    auto it = m_pipelines.find(handle.id());
    if (it != m_pipelines.end()) {
        m_deviceFuncs.vkDestroyPipeline(m_device, it->second.pipeline, nullptr);
        m_deviceFuncs.vkDestroyPipelineLayout(m_device, it->second.layout, nullptr);
        m_pipelines.erase(it);
    }
}

void VulkanDevice::destroyRenderPass(RenderPassHandle handle) {
    auto it = m_renderPasses.find(handle.id());
    if (it != m_renderPasses.end()) {
        m_deviceFuncs.vkDestroyRenderPass(m_device, it->second.renderPass, nullptr);
        m_renderPasses.erase(it);
    }
}

void VulkanDevice::destroyFramebuffer(FramebufferHandle handle) {
    auto it = m_framebuffers.find(handle.id());
    if (it != m_framebuffers.end()) {
        m_deviceFuncs.vkDestroyFramebuffer(m_device, it->second.framebuffer, nullptr);
        m_framebuffers.erase(it);
    }
}

// ============================================================================
// Buffer Operations - Full Implementation
// ============================================================================

void* VulkanDevice::mapBuffer(BufferHandle handle) {
    auto it = m_buffers.find(handle.id());
    if (it == m_buffers.end()) {
        return nullptr;
    }
    
    // Return persistent mapping if available
    if (it->second.mappedPtr) {
        return it->second.mappedPtr;
    }
    
    // Map memory
    void* data = nullptr;
    if (m_deviceFuncs.vkMapMemory(m_device, it->second.memory, 0, it->second.size, 0, &data) != VK_SUCCESS) {
        return nullptr;
    }
    
    return data;
}

void VulkanDevice::unmapBuffer(BufferHandle handle) {
    auto it = m_buffers.find(handle.id());
    if (it != m_buffers.end() && !it->second.persistentlyMapped) {
        m_deviceFuncs.vkUnmapMemory(m_device, it->second.memory);
    }
}

void VulkanDevice::updateBuffer(BufferHandle handle, const void* data, usize size, usize offset) {
    auto it = m_buffers.find(handle.id());
    if (it == m_buffers.end() || !data || size == 0) {
        return;
    }
    
    void* mappedData = it->second.mappedPtr;
    bool needsUnmap = false;
    
    if (!mappedData) {
        if (m_deviceFuncs.vkMapMemory(m_device, it->second.memory, offset, size, 0, &mappedData) != VK_SUCCESS) {
            return;
        }
        needsUnmap = true;
    } else {
        mappedData = static_cast<u8*>(mappedData) + offset;
    }
    
    std::memcpy(mappedData, data, size);
    
    if (needsUnmap) {
        m_deviceFuncs.vkUnmapMemory(m_device, it->second.memory);
    }
}

// ============================================================================
// Texture Operations (Stubs)
// ============================================================================

void VulkanDevice::updateTexture(TextureHandle, const TextureUpdateDesc&) { /* TODO */ }

// ============================================================================
// Swap Chain (Stub)
// ============================================================================

std::unique_ptr<SwapChain> VulkanDevice::createSwapChain(const SwapChainDesc&) {
    // TODO: Implement swap chain creation
    return nullptr;
}

// ============================================================================
// Command Submission
// ============================================================================

RenderContext* VulkanDevice::getRenderContext() {
    // TODO: Implement render context
    return nullptr;
}

void VulkanDevice::beginFrame() {
    // Wait for the current frame's fence
    m_deviceFuncs.vkWaitForFences(
        m_device, 
        1, 
        &m_frameSyncObjects[m_currentFrame].inFlight, 
        VK_TRUE, 
        UINT64_MAX);
    
    ++m_frameStats.frameNumber;
}

void VulkanDevice::endFrame() {
    // Reset fence and advance frame counter
    m_deviceFuncs.vkResetFences(m_device, 1, &m_frameSyncObjects[m_currentFrame].inFlight);
    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanDevice::waitIdle() {
    if (m_device && m_deviceFuncs.vkDeviceWaitIdle) {
        m_deviceFuncs.vkDeviceWaitIdle(m_device);
    }
}

void VulkanDevice::setDebugName(VkObjectType objectType, u64 object, const std::string& name) {
    if (!m_validationEnabled || !m_instanceFuncs.vkSetDebugUtilsObjectNameEXT) {
        return;
    }
    
    VkDebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = object;
    nameInfo.pObjectName = name.c_str();
    
    m_instanceFuncs.vkSetDebugUtilsObjectNameEXT(m_device, &nameInfo);
}

// ============================================================================
// Device Enumeration Helper
// ============================================================================

std::vector<PhysicalDeviceInfo> enumerateVulkanDevices() {
    std::vector<PhysicalDeviceInfo> devices;
    
    // Initialize loader if needed
    auto result = VulkanLoader::initialize();
    if (!result || !VulkanLoader::isAvailable()) {
        return devices;
    }
    
    // Create temporary instance for enumeration
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "NovaCore Device Enumeration";
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    VkInstance instance;
    VkResult vkResult = VulkanLoader::vkCreateInstance(&createInfo, nullptr, &instance);
    if (vkResult != VK_SUCCESS) {
        return devices;
    }
    
    // Load instance functions
    VulkanInstanceFunctions funcs;
    auto funcResult = VulkanLoader::loadInstanceFunctions(instance, funcs);
    if (!funcResult) {
        // Use loader's vkGetInstanceProcAddr to get destroy function directly
        auto destroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
            VulkanLoader::vkGetInstanceProcAddr(instance, "vkDestroyInstance"));
        if (destroyInstance) {
            destroyInstance(instance, nullptr);
        }
        return devices;
    }
    
    // Enumerate physical devices
    u32 deviceCount = 0;
    funcs.vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    funcs.vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
    
    for (VkPhysicalDevice physDevice : physicalDevices) {
        VkPhysicalDeviceProperties props;
        funcs.vkGetPhysicalDeviceProperties(physDevice, &props);
        
        PhysicalDeviceInfo info;
        info.name = props.deviceName;
        info.apiVersion = std::to_string(VK_API_VERSION_MAJOR(props.apiVersion)) + "." +
                          std::to_string(VK_API_VERSION_MINOR(props.apiVersion));
        info.vendorId = props.vendorID;
        info.deviceId = props.deviceID;
        
        switch (props.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                info.deviceType = PhysicalDeviceInfo::DeviceType::DiscreteGPU;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                info.deviceType = PhysicalDeviceInfo::DeviceType::IntegratedGPU;
                break;
            default:
                info.deviceType = PhysicalDeviceInfo::DeviceType::Unknown;
                break;
        }
        
        devices.push_back(info);
    }
    
    // Cleanup
    funcs.vkDestroyInstance(instance, nullptr);
    
    return devices;
}

} // namespace nova::render::vulkan
