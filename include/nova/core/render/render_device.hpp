/**
 * @file render_device.hpp
 * @brief GPU device abstraction for NovaCore rendering
 * 
 * Provides a backend-agnostic interface for GPU device operations.
 * Supports automatic backend selection and quality tier detection.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include "nova/core/types/result.hpp"
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace nova::render {

// Forward declarations
class SwapChain;
class RenderContext;

/**
 * @brief GPU device capabilities and limits
 */
struct DeviceLimits {
    // Texture limits
    u32 maxTextureSize1D = 0;
    u32 maxTextureSize2D = 0;
    u32 maxTextureSize3D = 0;
    u32 maxTextureSizeCube = 0;
    u32 maxTextureArrayLayers = 0;
    
    // Buffer limits
    u64 maxBufferSize = 0;
    u32 maxUniformBufferSize = 0;
    u32 maxStorageBufferSize = 0;
    u32 maxPushConstantSize = 0;
    
    // Sampler limits
    u32 maxSamplers = 0;
    f32 maxAnisotropy = 0.0f;
    
    // Pipeline limits
    u32 maxVertexInputAttributes = 0;
    u32 maxVertexInputBindings = 0;
    u32 maxVertexInputStride = 0;
    u32 maxColorAttachments = 0;
    
    // Compute limits
    u32 maxComputeWorkGroupCount[3] = {};
    u32 maxComputeWorkGroupSize[3] = {};
    u32 maxComputeWorkGroupInvocations = 0;
    u32 maxComputeSharedMemorySize = 0;
    
    // Descriptor limits
    u32 maxBoundDescriptorSets = 0;
    u32 maxDescriptorSetSamplers = 0;
    u32 maxDescriptorSetUniformBuffers = 0;
    u32 maxDescriptorSetStorageBuffers = 0;
    u32 maxDescriptorSetSampledImages = 0;
    u32 maxDescriptorSetStorageImages = 0;
    
    // Memory
    u64 totalDeviceMemory = 0;
    u64 availableDeviceMemory = 0;
    bool hasUnifiedMemory = false;
};

/**
 * @brief GPU device features
 */
struct DeviceFeatures {
    // Geometry features
    bool geometryShader = false;
    bool tessellationShader = false;
    bool meshShader = false;
    bool multiDrawIndirect = false;
    bool drawIndirectCount = false;
    
    // Texture features
    bool samplerAnisotropy = false;
    bool textureCompressionBC = false;
    bool textureCompressionETC2 = false;
    bool textureCompressionASTC = false;
    
    // Buffer features
    bool storageBuffer16BitAccess = false;
    bool storageBuffer8BitAccess = false;
    
    // Pipeline features
    bool multiViewport = false;
    bool fillModeNonSolid = false;
    bool wideLines = false;
    bool depthClamp = false;
    bool depthBiasClamp = false;
    bool independentBlend = false;
    bool dualSrcBlend = false;
    
    // Advanced features
    bool rayTracing = false;
    bool rayQuery = false;
    bool variableRateShading = false;
    bool bindlessResources = false;
    bool dynamicRendering = false;
    bool timelineSemaphores = false;
    
    // Compute features
    bool computeShader = true;
    bool asyncCompute = false;
    bool asyncTransfer = false;
    
    // Mobile-specific
    bool subgroupOperations = false;
    bool fragmentShaderInterlock = false;
};

/**
 * @brief Physical GPU device information
 */
struct PhysicalDeviceInfo {
    std::string name;
    std::string vendorName;
    std::string driverVersion;
    std::string apiVersion;
    
    u32 vendorId = 0;
    u32 deviceId = 0;
    
    enum class DeviceType : u8 {
        Unknown,
        IntegratedGPU,
        DiscreteGPU,
        VirtualGPU,
        CPU
    } deviceType = DeviceType::Unknown;
    
    DeviceLimits limits;
    DeviceFeatures features;
    QualityTier recommendedTier = QualityTier::Standard;
};

/**
 * @brief Device creation descriptor
 */
struct DeviceDesc {
    /// Preferred graphics backend (auto-detect if None)
    GraphicsBackend preferredBackend = GraphicsBackend::None;
    
    /// Enable validation layers (debug mode)
    bool enableValidation = false;
    
    /// Enable GPU profiling
    bool enableProfiling = false;
    
    /// Preferred physical device index (-1 for auto-select)
    i32 preferredDeviceIndex = -1;
    
    /// Enable ray tracing if available
    bool enableRayTracing = true;
    
    /// Enable bindless resources if available
    bool enableBindless = true;
    
    /// Application name for driver hints
    std::string appName = "NovaCore";
    
    /// Application version
    u32 appVersion = 1;
};

/**
 * @brief Abstract GPU device interface
 * 
 * The RenderDevice is the main interface for GPU operations.
 * It manages resource creation, command submission, and synchronization.
 */
class RenderDevice {
public:
    virtual ~RenderDevice() = default;
    
    // =========================================================================
    // Device Info
    // =========================================================================
    
    /// Get the graphics backend being used
    [[nodiscard]] virtual GraphicsBackend getBackend() const = 0;
    
    /// Get physical device info
    [[nodiscard]] virtual const PhysicalDeviceInfo& getDeviceInfo() const = 0;
    
    /// Get device limits
    [[nodiscard]] virtual const DeviceLimits& getLimits() const = 0;
    
    /// Get device features
    [[nodiscard]] virtual const DeviceFeatures& getFeatures() const = 0;
    
    /// Get recommended quality tier
    [[nodiscard]] virtual QualityTier getRecommendedQualityTier() const = 0;
    
    // =========================================================================
    // Resource Creation
    // =========================================================================
    
    /// Create a buffer
    [[nodiscard]] virtual BufferHandle createBuffer(const struct BufferDesc& desc) = 0;
    
    /// Create a texture
    [[nodiscard]] virtual TextureHandle createTexture(const struct TextureDesc& desc) = 0;
    
    /// Create a sampler
    [[nodiscard]] virtual SamplerHandle createSampler(const SamplerDesc& desc) = 0;
    
    /// Create a shader module
    [[nodiscard]] virtual ShaderHandle createShader(const struct ShaderDesc& desc) = 0;
    
    /// Create a graphics pipeline
    [[nodiscard]] virtual PipelineHandle createGraphicsPipeline(const struct GraphicsPipelineDesc& desc) = 0;
    
    /// Create a compute pipeline
    [[nodiscard]] virtual PipelineHandle createComputePipeline(const struct ComputePipelineDesc& desc) = 0;
    
    /// Create a render pass
    [[nodiscard]] virtual RenderPassHandle createRenderPass(const struct RenderPassDesc& desc) = 0;
    
    /// Create a framebuffer
    [[nodiscard]] virtual FramebufferHandle createFramebuffer(const struct FramebufferDesc& desc) = 0;
    
    // =========================================================================
    // Resource Destruction
    // =========================================================================
    
    virtual void destroyBuffer(BufferHandle handle) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;
    virtual void destroySampler(SamplerHandle handle) = 0;
    virtual void destroyShader(ShaderHandle handle) = 0;
    virtual void destroyPipeline(PipelineHandle handle) = 0;
    virtual void destroyRenderPass(RenderPassHandle handle) = 0;
    virtual void destroyFramebuffer(FramebufferHandle handle) = 0;
    
    // =========================================================================
    // Buffer Operations
    // =========================================================================
    
    /// Map a buffer for CPU access
    [[nodiscard]] virtual void* mapBuffer(BufferHandle handle) = 0;
    
    /// Unmap a previously mapped buffer
    virtual void unmapBuffer(BufferHandle handle) = 0;
    
    /// Update buffer data
    virtual void updateBuffer(BufferHandle handle, const void* data, usize size, usize offset = 0) = 0;
    
    // =========================================================================
    // Texture Operations
    // =========================================================================
    
    /// Update texture data
    virtual void updateTexture(TextureHandle handle, const struct TextureUpdateDesc& updateDesc) = 0;
    
    // =========================================================================
    // Swap Chain
    // =========================================================================
    
    /// Create a swap chain for a window
    [[nodiscard]] virtual std::unique_ptr<SwapChain> createSwapChain(const struct SwapChainDesc& desc) = 0;
    
    // =========================================================================
    // Command Submission
    // =========================================================================
    
    /// Get a render context for the current frame
    [[nodiscard]] virtual RenderContext* getRenderContext() = 0;
    
    /// Begin a new frame
    virtual void beginFrame() = 0;
    
    /// End the current frame and submit commands
    virtual void endFrame() = 0;
    
    /// Wait for all GPU operations to complete
    virtual void waitIdle() = 0;
    
    // =========================================================================
    // Statistics
    // =========================================================================
    
    struct FrameStats {
        u64 frameNumber = 0;
        f64 gpuTimeMs = 0.0;
        u64 drawCalls = 0;
        u64 triangles = 0;
        u64 uploadedBytes = 0;
        u64 allocatedMemory = 0;
    };
    
    [[nodiscard]] virtual FrameStats getFrameStats() const = 0;
};

/**
 * @brief Create a render device with the specified backend
 * @param desc Device creation descriptor
 * @return Result containing the device or an error
 */
[[nodiscard]] Result<std::unique_ptr<RenderDevice>> createRenderDevice(const DeviceDesc& desc);

/**
 * @brief Enumerate available physical devices
 * @param backend Backend to query
 * @return List of available physical devices
 */
[[nodiscard]] std::vector<PhysicalDeviceInfo> enumeratePhysicalDevices(GraphicsBackend backend);

/**
 * @brief Get the best available graphics backend for this platform
 */
[[nodiscard]] GraphicsBackend getBestAvailableBackend();

} // namespace nova::render
