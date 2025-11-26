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
// Resource Creation (Stubs - to be implemented)
// ============================================================================

BufferHandle VulkanDevice::createBuffer(const BufferDesc&) {
    // TODO: Implement buffer creation
    return BufferHandle(m_nextResourceId++);
}

TextureHandle VulkanDevice::createTexture(const TextureDesc&) {
    // TODO: Implement texture creation
    return TextureHandle(m_nextResourceId++);
}

SamplerHandle VulkanDevice::createSampler(const SamplerDesc&) {
    // TODO: Implement sampler creation
    return SamplerHandle(m_nextResourceId++);
}

ShaderHandle VulkanDevice::createShader(const ShaderDesc&) {
    // TODO: Implement shader creation
    return ShaderHandle(m_nextResourceId++);
}

PipelineHandle VulkanDevice::createGraphicsPipeline(const GraphicsPipelineDesc&) {
    // TODO: Implement pipeline creation
    return PipelineHandle(m_nextResourceId++);
}

PipelineHandle VulkanDevice::createComputePipeline(const ComputePipelineDesc&) {
    // TODO: Implement pipeline creation
    return PipelineHandle(m_nextResourceId++);
}

RenderPassHandle VulkanDevice::createRenderPass(const RenderPassDesc&) {
    // TODO: Implement render pass creation
    return RenderPassHandle(m_nextResourceId++);
}

FramebufferHandle VulkanDevice::createFramebuffer(const FramebufferDesc&) {
    // TODO: Implement framebuffer creation
    return FramebufferHandle(m_nextResourceId++);
}

// ============================================================================
// Resource Destruction (Stubs)
// ============================================================================

void VulkanDevice::destroyBuffer(BufferHandle) { /* TODO */ }
void VulkanDevice::destroyTexture(TextureHandle) { /* TODO */ }
void VulkanDevice::destroySampler(SamplerHandle) { /* TODO */ }
void VulkanDevice::destroyShader(ShaderHandle) { /* TODO */ }
void VulkanDevice::destroyPipeline(PipelineHandle) { /* TODO */ }
void VulkanDevice::destroyRenderPass(RenderPassHandle) { /* TODO */ }
void VulkanDevice::destroyFramebuffer(FramebufferHandle) { /* TODO */ }

// ============================================================================
// Buffer Operations (Stubs)
// ============================================================================

void* VulkanDevice::mapBuffer(BufferHandle) { return nullptr; }
void VulkanDevice::unmapBuffer(BufferHandle) { /* TODO */ }
void VulkanDevice::updateBuffer(BufferHandle, const void*, usize, usize) { /* TODO */ }

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
