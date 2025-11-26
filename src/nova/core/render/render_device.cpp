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
        case GraphicsBackend::Vulkan:
            // TODO: Implement Vulkan device
            return std::unexpected(errors::notSupported("Vulkan backend not yet implemented"));
            
        case GraphicsBackend::Metal:
            // TODO: Implement Metal device
            return std::unexpected(errors::notSupported("Metal backend not yet implemented"));
            
        case GraphicsBackend::WebGPU:
            // TODO: Implement WebGPU device
            return std::unexpected(errors::notSupported("WebGPU backend not yet implemented"));
            
        case GraphicsBackend::D3D12:
            // TODO: Implement D3D12 device
            return std::unexpected(errors::notSupported("D3D12 backend not yet implemented"));
            
        case GraphicsBackend::OpenGLES:
        case GraphicsBackend::OpenGL:
            // TODO: Implement OpenGL device
            return std::unexpected(errors::notSupported("OpenGL backend not yet implemented"));
            
        case GraphicsBackend::Software:
        case GraphicsBackend::None:
            // Use null device for testing
            return std::make_unique<NullRenderDevice>();
    }
    
    return std::unexpected(errors::notSupported("Unknown graphics backend"));
}

std::vector<PhysicalDeviceInfo> enumeratePhysicalDevices(GraphicsBackend backend) {
    std::vector<PhysicalDeviceInfo> devices;
    
    // TODO: Implement actual device enumeration for each backend
    // For now, return a single null device
    
    PhysicalDeviceInfo nullDevice;
    nullDevice.name = "Null Render Device";
    nullDevice.vendorName = "NovaCore";
    nullDevice.deviceType = PhysicalDeviceInfo::DeviceType::CPU;
    nullDevice.recommendedTier = QualityTier::Minimal;
    devices.push_back(nullDevice);
    
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
