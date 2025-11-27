/**
 * @file vulkan_pipeline.hpp
 * @brief Nova GraphicsCore™ - Vulkan pipeline implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * This file provides the Vulkan graphics and compute pipeline implementation.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "vulkan_types.hpp"
#include "vulkan_device.hpp"
#include "nova/core/render/render_pipeline.hpp"
#include <vector>
#include <string>

namespace nova::render::vulkan {

/**
 * @brief Nova GraphicsCore™ - Vulkan Graphics Pipeline
 * 
 * Encapsulates a Vulkan graphics pipeline with all required state.
 * Supports:
 * - Vertex input configuration
 * - Shader stages (vertex, fragment, etc.)
 * - Dynamic state
 * - Render pass compatibility
 */
class VulkanGraphicsPipeline {
public:
    /**
     * @brief Create a Vulkan graphics pipeline
     * @param device The Vulkan device
     * @param desc Pipeline description
     * @param renderPass Compatible render pass
     * @return Result containing the pipeline or an error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanGraphicsPipeline>> create(
        VulkanDevice& device,
        const GraphicsPipelineDesc& desc,
        VkRenderPass renderPass
    );
    
    ~VulkanGraphicsPipeline();
    
    // Non-copyable, non-movable
    VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) = delete;
    VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;
    VulkanGraphicsPipeline(VulkanGraphicsPipeline&&) = delete;
    VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&) = delete;
    
    // =========================================================================
    // Accessors
    // =========================================================================
    
    [[nodiscard]] VkPipeline getPipeline() const noexcept { return m_pipeline; }
    [[nodiscard]] VkPipelineLayout getLayout() const noexcept { return m_layout; }
    [[nodiscard]] const std::string& getName() const noexcept { return m_name; }
    
private:
    VulkanGraphicsPipeline(VulkanDevice& device);
    
    /**
     * @brief Create the pipeline layout
     */
    Result<void> createLayout();
    
    /**
     * @brief Create the pipeline
     */
    Result<void> createPipeline(const GraphicsPipelineDesc& desc, VkRenderPass renderPass);
    
    /**
     * @brief Convert vertex format to Vulkan format
     */
    [[nodiscard]] static VkFormat toVkVertexFormat(VertexFormat format) noexcept;
    
    /**
     * @brief Convert primitive topology to Vulkan topology
     */
    [[nodiscard]] static VkPrimitiveTopology toVkPrimitiveTopology(PrimitiveTopology topology) noexcept;
    
    // =========================================================================
    // Member Variables
    // =========================================================================
    
    VulkanDevice& m_device;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    std::string m_name;
};

/**
 * @brief Nova GraphicsCore™ - Vulkan Compute Pipeline
 * 
 * Encapsulates a Vulkan compute pipeline.
 */
class VulkanComputePipeline {
public:
    /**
     * @brief Create a Vulkan compute pipeline
     * @param device The Vulkan device
     * @param desc Pipeline description
     * @return Result containing the pipeline or an error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanComputePipeline>> create(
        VulkanDevice& device,
        const ComputePipelineDesc& desc
    );
    
    ~VulkanComputePipeline();
    
    // Non-copyable, non-movable
    VulkanComputePipeline(const VulkanComputePipeline&) = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;
    VulkanComputePipeline(VulkanComputePipeline&&) = delete;
    VulkanComputePipeline& operator=(VulkanComputePipeline&&) = delete;
    
    // =========================================================================
    // Accessors
    // =========================================================================
    
    [[nodiscard]] VkPipeline getPipeline() const noexcept { return m_pipeline; }
    [[nodiscard]] VkPipelineLayout getLayout() const noexcept { return m_layout; }
    [[nodiscard]] const std::string& getName() const noexcept { return m_name; }
    
private:
    VulkanComputePipeline(VulkanDevice& device);
    
    /**
     * @brief Create the pipeline layout
     */
    Result<void> createLayout();
    
    /**
     * @brief Create the pipeline
     */
    Result<void> createPipeline(const ComputePipelineDesc& desc);
    
    // =========================================================================
    // Member Variables
    // =========================================================================
    
    VulkanDevice& m_device;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    std::string m_name;
};

/**
 * @brief Pipeline cache for faster pipeline creation
 */
class VulkanPipelineCache {
public:
    /**
     * @brief Create a pipeline cache
     * @param device The Vulkan device
     * @param initialData Optional initial cache data
     * @return Result containing the cache or an error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanPipelineCache>> create(
        VulkanDevice& device,
        const void* initialData = nullptr,
        u64 initialDataSize = 0
    );
    
    ~VulkanPipelineCache();
    
    // Non-copyable, non-movable
    VulkanPipelineCache(const VulkanPipelineCache&) = delete;
    VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;
    VulkanPipelineCache(VulkanPipelineCache&&) = delete;
    VulkanPipelineCache& operator=(VulkanPipelineCache&&) = delete;
    
    /**
     * @brief Get the cache data for saving
     * @return Result containing the cache data or an error
     */
    [[nodiscard]] Result<std::vector<u8>> getData() const;
    
    [[nodiscard]] VkPipelineCache getCache() const noexcept { return m_cache; }
    
private:
    VulkanPipelineCache(VulkanDevice& device);
    
    VulkanDevice& m_device;
    VkPipelineCache m_cache = VK_NULL_HANDLE;
};

} // namespace nova::render::vulkan
