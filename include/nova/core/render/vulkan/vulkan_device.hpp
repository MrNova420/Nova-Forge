/**
 * @file vulkan_device.hpp
 * @brief Nova GraphicsCore™ - Vulkan render device implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * This is the core Vulkan backend implementation for Nova GraphicsCore™,
 * providing AAA-quality rendering on Android, Linux, and Windows.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "vulkan_types.hpp"
#include "vulkan_loader.hpp"
#include "nova/core/render/render_device.hpp"
#include <vector>
#include <array>

namespace nova::render::vulkan {

/**
 * @brief Queue family indices for the Vulkan device
 */
struct QueueFamilyIndices {
    u32 graphics = UINT32_MAX;
    u32 compute = UINT32_MAX;
    u32 transfer = UINT32_MAX;
    u32 present = UINT32_MAX;
    
    [[nodiscard]] bool isComplete() const noexcept {
        return graphics != UINT32_MAX && present != UINT32_MAX;
    }
    
    [[nodiscard]] bool hasAsyncCompute() const noexcept {
        return compute != UINT32_MAX && compute != graphics;
    }
    
    [[nodiscard]] bool hasAsyncTransfer() const noexcept {
        return transfer != UINT32_MAX && transfer != graphics && transfer != compute;
    }
};

/**
 * @brief Per-frame synchronization primitives
 */
struct FrameSyncObjects {
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkSemaphore renderFinished = VK_NULL_HANDLE;
    VkFence inFlight = VK_NULL_HANDLE;
};

/**
 * @brief Nova GraphicsCore™ - Vulkan Render Device
 * 
 * The primary Vulkan backend implementation for NovaCore rendering.
 * Designed for:
 * - Mobile-first optimization (Android, with ARM Mali/Adreno focus)
 * - Desktop support (Linux, Windows)
 * - Future-proof architecture (Vulkan 1.3 preferred, 1.1 minimum)
 */
class VulkanDevice : public RenderDevice {
public:
    /**
     * @brief Create a Vulkan render device
     * @param desc Device creation descriptor
     * @return Result containing the device or an error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanDevice>> create(const DeviceDesc& desc);
    
    ~VulkanDevice() override;
    
    // Non-copyable, non-movable (owns Vulkan resources)
    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;
    VulkanDevice(VulkanDevice&&) = delete;
    VulkanDevice& operator=(VulkanDevice&&) = delete;
    
    // =========================================================================
    // RenderDevice Interface Implementation
    // =========================================================================
    
    [[nodiscard]] GraphicsBackend getBackend() const override { 
        return GraphicsBackend::Vulkan; 
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
        return m_deviceInfo.recommendedTier; 
    }
    
    // Resource creation
    [[nodiscard]] BufferHandle createBuffer(const BufferDesc& desc) override;
    [[nodiscard]] TextureHandle createTexture(const TextureDesc& desc) override;
    [[nodiscard]] SamplerHandle createSampler(const SamplerDesc& desc) override;
    [[nodiscard]] ShaderHandle createShader(const ShaderDesc& desc) override;
    [[nodiscard]] PipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    [[nodiscard]] PipelineHandle createComputePipeline(const ComputePipelineDesc& desc) override;
    [[nodiscard]] RenderPassHandle createRenderPass(const RenderPassDesc& desc) override;
    [[nodiscard]] FramebufferHandle createFramebuffer(const FramebufferDesc& desc) override;
    
    // Resource destruction
    void destroyBuffer(BufferHandle handle) override;
    void destroyTexture(TextureHandle handle) override;
    void destroySampler(SamplerHandle handle) override;
    void destroyShader(ShaderHandle handle) override;
    void destroyPipeline(PipelineHandle handle) override;
    void destroyRenderPass(RenderPassHandle handle) override;
    void destroyFramebuffer(FramebufferHandle handle) override;
    
    // Buffer operations
    [[nodiscard]] void* mapBuffer(BufferHandle handle) override;
    void unmapBuffer(BufferHandle handle) override;
    void updateBuffer(BufferHandle handle, const void* data, usize size, usize offset) override;
    
    // Texture operations
    void updateTexture(TextureHandle handle, const TextureUpdateDesc& updateDesc) override;
    
    // Swap chain
    [[nodiscard]] std::unique_ptr<SwapChain> createSwapChain(const SwapChainDesc& desc) override;
    
    // Command submission
    [[nodiscard]] RenderContext* getRenderContext() override;
    void beginFrame() override;
    void endFrame() override;
    void waitIdle() override;
    
    // Statistics
    [[nodiscard]] FrameStats getFrameStats() const override { return m_frameStats; }
    
    // =========================================================================
    // Vulkan-Specific Accessors
    // =========================================================================
    
    [[nodiscard]] VkInstance getInstance() const noexcept { return m_instance; }
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const noexcept { return m_physicalDevice; }
    [[nodiscard]] VkDevice getDevice() const noexcept { return m_device; }
    [[nodiscard]] const QueueFamilyIndices& getQueueFamilies() const noexcept { return m_queueFamilies; }
    [[nodiscard]] VkQueue getGraphicsQueue() const noexcept { return m_graphicsQueue; }
    [[nodiscard]] VkQueue getPresentQueue() const noexcept { return m_presentQueue; }
    [[nodiscard]] VkQueue getComputeQueue() const noexcept { return m_computeQueue; }
    [[nodiscard]] VkQueue getTransferQueue() const noexcept { return m_transferQueue; }
    [[nodiscard]] const VulkanInstanceFunctions& getInstanceFuncs() const noexcept { return m_instanceFuncs; }
    [[nodiscard]] const VulkanDeviceFunctions& getDeviceFuncs() const noexcept { return m_deviceFuncs; }
    [[nodiscard]] u32 getApiVersion() const noexcept { return m_apiVersion; }
    
private:
    VulkanDevice() = default;
    
    /**
     * @brief Initialize the Vulkan instance
     */
    Result<void> createInstance(const DeviceDesc& desc);
    
    /**
     * @brief Select a physical device
     */
    Result<void> selectPhysicalDevice(i32 preferredIndex);
    
    /**
     * @brief Create the logical device and queues
     */
    Result<void> createLogicalDevice(const DeviceDesc& desc);
    
    /**
     * @brief Create per-frame synchronization objects
     */
    Result<void> createSyncObjects();
    
    /**
     * @brief Create command pools for each queue family
     */
    Result<void> createCommandPools();
    
    /**
     * @brief Populate PhysicalDeviceInfo from Vulkan queries
     */
    void queryDeviceInfo();
    
    /**
     * @brief Determine recommended quality tier based on device capabilities
     */
    QualityTier determineQualityTier() const;
    
    /**
     * @brief Find queue family indices for the selected physical device
     */
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    
    /**
     * @brief Check if a physical device is suitable for NovaCore rendering
     */
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    
    /**
     * @brief Rate a physical device for selection
     */
    i32 rateDeviceSuitability(VkPhysicalDevice device) const;
    
    /**
     * @brief Debug callback for validation layers
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    
    // =========================================================================
    // Member Variables
    // =========================================================================
    
    // Vulkan handles
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    
    // Queues
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkQueue m_computeQueue = VK_NULL_HANDLE;
    VkQueue m_transferQueue = VK_NULL_HANDLE;
    QueueFamilyIndices m_queueFamilies;
    
    // Command pools (one per frame in flight, per queue type)
    std::array<VkCommandPool, MAX_FRAMES_IN_FLIGHT> m_graphicsCommandPools{};
    VkCommandPool m_computeCommandPool = VK_NULL_HANDLE;
    VkCommandPool m_transferCommandPool = VK_NULL_HANDLE;
    
    // Per-frame synchronization
    std::array<FrameSyncObjects, MAX_FRAMES_IN_FLIGHT> m_frameSyncObjects{};
    u32 m_currentFrame = 0;
    
    // Function pointers
    VulkanInstanceFunctions m_instanceFuncs{};
    VulkanDeviceFunctions m_deviceFuncs{};
    
    // Device info
    PhysicalDeviceInfo m_deviceInfo{};
    u32 m_apiVersion = VK_API_VERSION_1_0;
    
    // Statistics
    FrameStats m_frameStats{};
    
    // Configuration
    bool m_validationEnabled = false;
    
    // Resource ID tracking
    u64 m_nextResourceId = 1;
};

/**
 * @brief Enumerate available Vulkan physical devices
 * @return List of physical device information
 */
[[nodiscard]] std::vector<PhysicalDeviceInfo> enumerateVulkanDevices();

} // namespace nova::render::vulkan
