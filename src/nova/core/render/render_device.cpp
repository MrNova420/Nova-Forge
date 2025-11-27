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

// Include Vulkan backend
#include "nova/core/render/vulkan/vulkan_device.hpp"
#include "nova/core/render/vulkan/vulkan_loader.hpp"

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
            // Enumerate Vulkan physical devices
            if (!vulkan::VulkanLoader::isAvailable()) {
                break;
            }
            
            // Get instance for enumeration (would need a temporary instance)
            // In full implementation, this would use VulkanLoader to enumerate devices
            
            // Add a placeholder entry indicating Vulkan is available
            PhysicalDeviceInfo vulkanDevice;
            vulkanDevice.name = "Vulkan Device";
            vulkanDevice.vendorName = "Unknown";
            vulkanDevice.deviceType = PhysicalDeviceInfo::DeviceType::DiscreteGPU;
            vulkanDevice.recommendedTier = QualityTier::High;
            // Feature support would be populated from actual device query
            devices.push_back(vulkanDevice);
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
