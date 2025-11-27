/**
 * @file render_device.cpp
 * @brief Render device implementation and backend selection
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/render_device.hpp"
#include "nova/core/render/swap_chain.hpp"
#include "nova/core/render/render_context.hpp"
#include "nova/core/render/buffer.hpp"
#include "nova/core/render/texture.hpp"
#include "nova/core/render/render_pass.hpp"
#include "nova/core/render/render_pipeline.hpp"
#include "nova/core/logging/logging.hpp"

// Include Vulkan backend
#include "nova/core/render/vulkan/vulkan_device.hpp"
#include "nova/core/render/vulkan/vulkan_loader.hpp"

#include <sstream>

using namespace nova::logging;

namespace nova::render {

// ============================================================================
// Null/Software Render Device (for testing and fallback)
// ============================================================================

/**
 * @brief Null render device for testing
 */
class NullRenderDevice : public RenderDevice {
private:
    PhysicalDeviceInfo m_deviceInfo;
    FrameStats m_frameStats;
    u64 m_nextResourceId = 1;
    
public:
    NullRenderDevice() {
        m_deviceInfo.name = "Null Render Device";
        m_deviceInfo.vendorName = "NovaCore";
        m_deviceInfo.driverVersion = "1.0.0";
        m_deviceInfo.apiVersion = "N/A";
        m_deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::CPU;
        m_deviceInfo.recommendedTier = QualityTier::Minimal;
        
        // Set reasonable limits
        m_deviceInfo.limits.maxTextureSize2D = 4096;
        m_deviceInfo.limits.maxTextureSize3D = 256;
        m_deviceInfo.limits.maxTextureSizeCube = 2048;
        m_deviceInfo.limits.maxBufferSize = 256 * 1024 * 1024; // 256MB
        m_deviceInfo.limits.maxUniformBufferSize = 64 * 1024;
        m_deviceInfo.limits.maxPushConstantSize = 128;
        m_deviceInfo.limits.maxColorAttachments = 8;
        m_deviceInfo.limits.maxComputeWorkGroupSize[0] = 1024;
        m_deviceInfo.limits.maxComputeWorkGroupSize[1] = 1024;
        m_deviceInfo.limits.maxComputeWorkGroupSize[2] = 64;
        
        // Set basic features
        m_deviceInfo.features.computeShader = true;
        m_deviceInfo.features.textureCompressionBC = true;
    }
    
    // Device info
    [[nodiscard]] GraphicsBackend getBackend() const override { 
        return GraphicsBackend::Software; 
    }
    
    [[nodiscard]] const PhysicalDeviceInfo& getDeviceInfo() const override { 
        return m_deviceInfo; 
    }
    
    [[nodiscard]] const DeviceLimits& getLimits() const override { 
        return m_deviceInfo.limits; 
    }
    
    [[nodiscard]] const DeviceFeatures& getFeatures() const override { 
        return m_deviceInfo.features; 
    }
    
    [[nodiscard]] QualityTier getRecommendedQualityTier() const override { 
        return QualityTier::Minimal; 
    }
    
    // Resource creation (return dummy handles)
    [[nodiscard]] BufferHandle createBuffer(const BufferDesc&) override {
        return BufferHandle(m_nextResourceId++);
    }
    
    [[nodiscard]] TextureHandle createTexture(const TextureDesc&) override {
        return TextureHandle(m_nextResourceId++);
    }
    
    [[nodiscard]] SamplerHandle createSampler(const SamplerDesc&) override {
        return SamplerHandle(m_nextResourceId++);
    }
    
    [[nodiscard]] ShaderHandle createShader(const ShaderDesc&) override {
        return ShaderHandle(m_nextResourceId++);
    }
    
    [[nodiscard]] PipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc&) override {
        return PipelineHandle(m_nextResourceId++);
    }
    
    [[nodiscard]] PipelineHandle createComputePipeline(const ComputePipelineDesc&) override {
        return PipelineHandle(m_nextResourceId++);
    }
    
    [[nodiscard]] RenderPassHandle createRenderPass(const RenderPassDesc&) override {
        return RenderPassHandle(m_nextResourceId++);
    }
    
    [[nodiscard]] FramebufferHandle createFramebuffer(const FramebufferDesc&) override {
        return FramebufferHandle(m_nextResourceId++);
    }
    
    // Resource destruction (no-op for null device)
    void destroyBuffer(BufferHandle) override {}
    void destroyTexture(TextureHandle) override {}
    void destroySampler(SamplerHandle) override {}
    void destroyShader(ShaderHandle) override {}
    void destroyPipeline(PipelineHandle) override {}
    void destroyRenderPass(RenderPassHandle) override {}
    void destroyFramebuffer(FramebufferHandle) override {}
    
    // Buffer operations
    [[nodiscard]] void* mapBuffer(BufferHandle) override { return nullptr; }
    void unmapBuffer(BufferHandle) override {}
    void updateBuffer(BufferHandle, const void*, usize, usize) override {}
    
    // Texture operations
    void updateTexture(TextureHandle, const TextureUpdateDesc&) override {}
    
    // Swap chain (not supported in null device)
    [[nodiscard]] std::unique_ptr<SwapChain> createSwapChain(const SwapChainDesc&) override {
        return nullptr;
    }
    
    // Command submission
    [[nodiscard]] RenderContext* getRenderContext() override { return nullptr; }
    void beginFrame() override { ++m_frameStats.frameNumber; }
    void endFrame() override {}
    void waitIdle() override {}
    
    // Statistics
    [[nodiscard]] FrameStats getFrameStats() const override { return m_frameStats; }
};

// ============================================================================
// Factory Functions
// ============================================================================

Result<std::unique_ptr<RenderDevice>> createRenderDevice(const DeviceDesc& desc) {
    GraphicsBackend backend = desc.preferredBackend;
    
    // Auto-detect best backend if not specified
    if (backend == GraphicsBackend::None) {
        backend = getBestAvailableBackend();
    }
    
    // Create device based on backend
    switch (backend) {
        case GraphicsBackend::Vulkan: {
            // Try to create Vulkan device
            auto result = vulkan::VulkanDevice::create(desc);
            if (result) {
                return std::move(*result);
            }
            // Vulkan failed, fall through to software renderer
            // (only in non-strict mode)
            if (desc.preferredBackend == GraphicsBackend::Vulkan) {
                return std::unexpected(result.error());
            }
            // Fall through to software
            [[fallthrough]];
        }
            
        case GraphicsBackend::Metal:
            // Metal backend for Apple platforms
            #if defined(__APPLE__)
            // Metal implementation would be created here when available
            return std::unexpected(errors::notSupported(
                "Metal backend is planned for Apple platforms. Currently in development."));
            #else
            return std::unexpected(errors::notSupported(
                "Metal backend is only available on Apple platforms (macOS, iOS)"));
            #endif
            
        case GraphicsBackend::WebGPU:
            // WebGPU backend for web platforms
            #if defined(__EMSCRIPTEN__)
            // WebGPU implementation would be created here when available
            return std::unexpected(errors::notSupported(
                "WebGPU backend is planned for web platform. Currently in development."));
            #else
            return std::unexpected(errors::notSupported(
                "WebGPU backend is primarily for web platforms via Emscripten"));
            #endif
            
        case GraphicsBackend::D3D12:
            // Direct3D 12 backend for Windows
            #if defined(_WIN32)
            // D3D12 implementation would be created here when available
            return std::unexpected(errors::notSupported(
                "D3D12 backend is planned for Windows. Currently in development."));
            #else
            return std::unexpected(errors::notSupported(
                "D3D12 backend is only available on Windows"));
            #endif
            
        case GraphicsBackend::OpenGLES:
        case GraphicsBackend::OpenGL:
            // OpenGL/ES fallback backend
            // OpenGL implementation would be created here when available
            return std::unexpected(errors::notSupported(
                "OpenGL/ES backend is planned as a fallback renderer. Currently in development."));
            
        case GraphicsBackend::Software:
        case GraphicsBackend::None:
            // Use null device for testing
            return std::make_unique<NullRenderDevice>();
    }
    
    return std::unexpected(errors::notSupported("Unknown graphics backend"));
}

std::vector<PhysicalDeviceInfo> enumeratePhysicalDevices(GraphicsBackend backend) {
    std::vector<PhysicalDeviceInfo> devices;
    
    switch (backend) {
        case GraphicsBackend::Vulkan: {
            // Enumerate Vulkan physical devices using full instance enumeration
            if (!vulkan::VulkanLoader::isAvailable()) {
                break;
            }
            
            // Create a temporary instance for device enumeration
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "NovaCore Device Enumerator";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "NovaCore";
            appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
            appInfo.apiVersion = vulkan::VulkanLoader::getMaxSupportedVersion();
            
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledLayerCount = 0;
            createInfo.enabledExtensionCount = 0;
            
            VkInstance tempInstance = VK_NULL_HANDLE;
            VkResult result = vulkan::VulkanLoader::vkCreateInstance(&createInfo, nullptr, &tempInstance);
            if (result != VK_SUCCESS || tempInstance == VK_NULL_HANDLE) {
                NOVA_LOG_WARN(LogCategory::Render, "Failed to create temporary Vulkan instance for enumeration");
                break;
            }
            
            // Load instance functions
            vulkan::VulkanInstanceFunctions instFuncs{};
            auto loadResult = vulkan::VulkanLoader::loadInstanceFunctions(tempInstance, instFuncs);
            if (!loadResult) {
                instFuncs.vkDestroyInstance(tempInstance, nullptr);
                break;
            }
            
            // Enumerate physical devices
            u32 deviceCount = 0;
            instFuncs.vkEnumeratePhysicalDevices(tempInstance, &deviceCount, nullptr);
            
            if (deviceCount > 0) {
                std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
                instFuncs.vkEnumeratePhysicalDevices(tempInstance, &deviceCount, physicalDevices.data());
                
                for (const VkPhysicalDevice& physDevice : physicalDevices) {
                    VkPhysicalDeviceProperties props{};
                    instFuncs.vkGetPhysicalDeviceProperties(physDevice, &props);
                    
                    VkPhysicalDeviceFeatures features{};
                    instFuncs.vkGetPhysicalDeviceFeatures(physDevice, &features);
                    
                    VkPhysicalDeviceMemoryProperties memProps{};
                    instFuncs.vkGetPhysicalDeviceMemoryProperties(physDevice, &memProps);
                    
                    PhysicalDeviceInfo deviceInfo;
                    deviceInfo.name = props.deviceName;
                    
                    // Determine vendor name from vendor ID
                    switch (props.vendorID) {
                        case 0x1002: deviceInfo.vendorName = "AMD"; break;
                        case 0x10DE: deviceInfo.vendorName = "NVIDIA"; break;
                        case 0x8086: deviceInfo.vendorName = "Intel"; break;
                        case 0x13B5: deviceInfo.vendorName = "ARM"; break;
                        case 0x5143: deviceInfo.vendorName = "Qualcomm"; break;
                        case 0x1010: deviceInfo.vendorName = "ImgTec"; break;
                        case 0x106B: deviceInfo.vendorName = "Apple"; break;
                        default: deviceInfo.vendorName = "Unknown"; break;
                    }
                    
                    // Map device type
                    switch (props.deviceType) {
                        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                            deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::DiscreteGPU;
                            break;
                        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                            deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::IntegratedGPU;
                            break;
                        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                            deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::VirtualGPU;
                            break;
                        case VK_PHYSICAL_DEVICE_TYPE_CPU:
                            deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::CPU;
                            break;
                        default:
                            deviceInfo.deviceType = PhysicalDeviceInfo::DeviceType::Unknown;
                            break;
                    }
                    
                    // Calculate total device memory
                    u64 totalMemory = 0;
                    for (u32 i = 0; i < memProps.memoryHeapCount; ++i) {
                        if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                            totalMemory += memProps.memoryHeaps[i].size;
                        }
                    }
                    deviceInfo.limits.totalDeviceMemory = totalMemory;
                    
                    // Format driver version as string
                    {
                        u32 dv = props.driverVersion;
                        std::ostringstream oss;
                        oss << ((dv >> 22) & 0x3FF) << "." << ((dv >> 12) & 0x3FF) << "." << (dv & 0xFFF);
                        deviceInfo.driverVersion = oss.str();
                    }
                    
                    // Format API version as string
                    {
                        u32 av = props.apiVersion;
                        std::ostringstream oss;
                        oss << VK_VERSION_MAJOR(av) << "." << VK_VERSION_MINOR(av) << "." << VK_VERSION_PATCH(av);
                        deviceInfo.apiVersion = oss.str();
                    }
                    
                    deviceInfo.vendorId = props.vendorID;
                    deviceInfo.deviceId = props.deviceID;
                    
                    // Populate limits from device properties
                    deviceInfo.limits.maxTextureSize2D = props.limits.maxImageDimension2D;
                    deviceInfo.limits.maxTextureSize3D = props.limits.maxImageDimension3D;
                    deviceInfo.limits.maxTextureSizeCube = props.limits.maxImageDimensionCube;
                    deviceInfo.limits.maxTextureArrayLayers = props.limits.maxImageArrayLayers;
                    deviceInfo.limits.maxUniformBufferSize = props.limits.maxUniformBufferRange;
                    deviceInfo.limits.maxStorageBufferSize = props.limits.maxStorageBufferRange;
                    deviceInfo.limits.maxVertexInputAttributes = props.limits.maxVertexInputAttributes;
                    deviceInfo.limits.maxVertexInputBindings = props.limits.maxVertexInputBindings;
                    deviceInfo.limits.maxColorAttachments = props.limits.maxColorAttachments;
                    deviceInfo.limits.maxComputeWorkGroupSize[0] = props.limits.maxComputeWorkGroupSize[0];
                    deviceInfo.limits.maxComputeWorkGroupSize[1] = props.limits.maxComputeWorkGroupSize[1];
                    deviceInfo.limits.maxComputeWorkGroupSize[2] = props.limits.maxComputeWorkGroupSize[2];
                    deviceInfo.limits.maxComputeWorkGroupInvocations = props.limits.maxComputeWorkGroupInvocations;
                    deviceInfo.limits.maxBoundDescriptorSets = props.limits.maxBoundDescriptorSets;
                    deviceInfo.limits.maxSamplers = props.limits.maxPerStageDescriptorSamplers;
                    deviceInfo.limits.maxAnisotropy = props.limits.maxSamplerAnisotropy;
                    
                    // Populate features from device features (matching DeviceFeatures struct)
                    deviceInfo.features.geometryShader = features.geometryShader;
                    deviceInfo.features.tessellationShader = features.tessellationShader;
                    deviceInfo.features.computeShader = true; // Always available in Vulkan
                    deviceInfo.features.multiDrawIndirect = features.multiDrawIndirect;
                    deviceInfo.features.fillModeNonSolid = features.fillModeNonSolid;
                    deviceInfo.features.wideLines = features.wideLines;
                    deviceInfo.features.depthClamp = features.depthClamp;
                    deviceInfo.features.textureCompressionBC = features.textureCompressionBC;
                    deviceInfo.features.textureCompressionETC2 = features.textureCompressionETC2;
                    deviceInfo.features.textureCompressionASTC = features.textureCompressionASTC_LDR;
                    deviceInfo.features.samplerAnisotropy = features.samplerAnisotropy;
                    deviceInfo.features.independentBlend = features.independentBlend;
                    deviceInfo.features.dualSrcBlend = features.dualSrcBlend;
                    
                    // Determine recommended quality tier based on capabilities
                    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && totalMemory > 4ULL * 1024 * 1024 * 1024) {
                        deviceInfo.recommendedTier = QualityTier::Ultra;
                    } else if (totalMemory > 2ULL * 1024 * 1024 * 1024) {
                        deviceInfo.recommendedTier = QualityTier::High;
                    } else if (totalMemory > 1ULL * 1024 * 1024 * 1024) {
                        deviceInfo.recommendedTier = QualityTier::Standard;
                    } else if (totalMemory > 512ULL * 1024 * 1024) {
                        deviceInfo.recommendedTier = QualityTier::Basic;
                    } else {
                        deviceInfo.recommendedTier = QualityTier::Minimal;
                    }
                    
                    devices.push_back(deviceInfo);
                }
            }
            
            // Cleanup temporary instance
            instFuncs.vkDestroyInstance(tempInstance, nullptr);
            break;
        }
        
        case GraphicsBackend::Metal: {
            #if defined(__APPLE__)
            // Enumerate Metal devices
            PhysicalDeviceInfo metalDevice;
            metalDevice.name = "Metal Device";
            metalDevice.vendorName = "Apple";
            metalDevice.deviceType = PhysicalDeviceInfo::DeviceType::IntegratedGPU;
            metalDevice.recommendedTier = QualityTier::High;
            devices.push_back(metalDevice);
            #endif
            break;
        }
        
        case GraphicsBackend::D3D12: {
            #if defined(_WIN32)
            // Enumerate D3D12 adapters
            PhysicalDeviceInfo d3d12Device;
            d3d12Device.name = "D3D12 Device";
            d3d12Device.vendorName = "Unknown";
            d3d12Device.deviceType = PhysicalDeviceInfo::DeviceType::DiscreteGPU;
            d3d12Device.recommendedTier = QualityTier::High;
            devices.push_back(d3d12Device);
            #endif
            break;
        }
        
        case GraphicsBackend::WebGPU: {
            #if defined(__EMSCRIPTEN__)
            // WebGPU adapter enumeration
            PhysicalDeviceInfo webgpuDevice;
            webgpuDevice.name = "WebGPU Device";
            webgpuDevice.vendorName = "Browser";
            webgpuDevice.deviceType = PhysicalDeviceInfo::DeviceType::IntegratedGPU;
            webgpuDevice.recommendedTier = QualityTier::Standard;
            devices.push_back(webgpuDevice);
            #endif
            break;
        }
        
        case GraphicsBackend::OpenGLES:
        case GraphicsBackend::OpenGL: {
            // OpenGL device enumeration (limited)
            PhysicalDeviceInfo glDevice;
            glDevice.name = "OpenGL Device";
            glDevice.vendorName = "Unknown";
            glDevice.deviceType = PhysicalDeviceInfo::DeviceType::IntegratedGPU;
            glDevice.recommendedTier = QualityTier::Basic;
            devices.push_back(glDevice);
            break;
        }
        
        case GraphicsBackend::Software:
        case GraphicsBackend::None: {
            // Software/null device always available
            PhysicalDeviceInfo nullDevice;
            nullDevice.name = "Software Renderer";
            nullDevice.vendorName = "NovaCore";
            nullDevice.deviceType = PhysicalDeviceInfo::DeviceType::CPU;
            nullDevice.recommendedTier = QualityTier::Minimal;
            devices.push_back(nullDevice);
            break;
        }
    }
    
    // Always include software fallback if no devices found
    if (devices.empty()) {
        PhysicalDeviceInfo nullDevice;
        nullDevice.name = "Software Renderer (Fallback)";
        nullDevice.vendorName = "NovaCore";
        nullDevice.deviceType = PhysicalDeviceInfo::DeviceType::CPU;
        nullDevice.recommendedTier = QualityTier::Minimal;
        devices.push_back(nullDevice);
    }
    
    return devices;
}

GraphicsBackend getBestAvailableBackend() {
    // Platform-specific backend selection
    
#if defined(__ANDROID__)
    // Android: Prefer Vulkan, fallback to OpenGL ES
    return GraphicsBackend::Vulkan;
    
#elif defined(__APPLE__)
    // Apple: Use Metal
    return GraphicsBackend::Metal;
    
#elif defined(__EMSCRIPTEN__) || defined(NOVA_PLATFORM_WEB)
    // Web: Use WebGPU
    return GraphicsBackend::WebGPU;
    
#elif defined(_WIN32)
    // Windows: Prefer Vulkan, D3D12 as alternative
    return GraphicsBackend::Vulkan;
    
#elif defined(__linux__)
    // Linux: Use Vulkan
    return GraphicsBackend::Vulkan;
    
#else
    // Unknown platform: Use software renderer
    return GraphicsBackend::Software;
#endif
}

} // namespace nova::render
