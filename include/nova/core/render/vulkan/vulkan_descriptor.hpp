/**
 * @file vulkan_descriptor.hpp
 * @brief Nova GraphicsCore™ - Vulkan Descriptor System
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Implements the Vulkan descriptor set layout, pool, and allocation system.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "vulkan_types.hpp"
#include "nova/core/types/result.hpp"
#include <memory>
#include <vector>
#include <span>
#include <array>

namespace nova::render::vulkan {

// Forward declarations
class VulkanDevice;

// ============================================================================
// Descriptor Types
// ============================================================================

/**
 * @brief Descriptor type enumeration
 * 
 * Maps to VkDescriptorType for efficient resource binding.
 */
enum class DescriptorType : u8 {
    Sampler = 0,                    ///< Standalone sampler
    CombinedImageSampler,           ///< Combined image + sampler (most common)
    SampledImage,                   ///< Image only (separate sampler)
    StorageImage,                   ///< Read/write compute image
    UniformTexelBuffer,             ///< Texture buffer for uniform access
    StorageTexelBuffer,             ///< Texture buffer for storage access
    UniformBuffer,                  ///< Constant/uniform buffer
    StorageBuffer,                  ///< Read/write storage buffer
    UniformBufferDynamic,           ///< Dynamic offset uniform buffer
    StorageBufferDynamic,           ///< Dynamic offset storage buffer
    InputAttachment                 ///< Subpass input attachment
};

/**
 * @brief Convert NovaCore descriptor type to Vulkan descriptor type
 */
constexpr VkDescriptorType toVkDescriptorType(DescriptorType type) noexcept {
    switch (type) {
        case DescriptorType::Sampler:               return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::CombinedImageSampler:  return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::SampledImage:          return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::StorageImage:          return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::UniformTexelBuffer:    return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case DescriptorType::StorageTexelBuffer:    return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case DescriptorType::UniformBuffer:         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::StorageBuffer:         return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::UniformBufferDynamic:  return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case DescriptorType::StorageBufferDynamic:  return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case DescriptorType::InputAttachment:       return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:                                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}

// ============================================================================
// Descriptor Binding
// ============================================================================

/**
 * @brief Shader stage visibility flags
 */
enum class ShaderStage : u32 {
    None            = 0,
    Vertex          = VK_SHADER_STAGE_VERTEX_BIT,
    Fragment        = VK_SHADER_STAGE_FRAGMENT_BIT,
    Compute         = VK_SHADER_STAGE_COMPUTE_BIT,
    Geometry        = VK_SHADER_STAGE_GEOMETRY_BIT,
    TessControl     = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    TessEvaluation  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    All             = VK_SHADER_STAGE_ALL
};

inline ShaderStage operator|(ShaderStage a, ShaderStage b) noexcept {
    return static_cast<ShaderStage>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline ShaderStage operator&(ShaderStage a, ShaderStage b) noexcept {
    return static_cast<ShaderStage>(static_cast<u32>(a) & static_cast<u32>(b));
}

/**
 * @brief Descriptor binding description
 */
struct DescriptorBinding {
    u32 binding = 0;                            ///< Binding index
    DescriptorType type = DescriptorType::UniformBuffer; ///< Descriptor type
    u32 count = 1;                              ///< Array count (1 for non-arrays)
    ShaderStage stages = ShaderStage::All;      ///< Visible shader stages
    
    // Factory methods for common binding types
    static DescriptorBinding uniformBuffer(u32 bindingIndex, ShaderStage stages = ShaderStage::All) {
        return {bindingIndex, DescriptorType::UniformBuffer, 1, stages};
    }
    
    static DescriptorBinding storageBuffer(u32 bindingIndex, ShaderStage stages = ShaderStage::All) {
        return {bindingIndex, DescriptorType::StorageBuffer, 1, stages};
    }
    
    static DescriptorBinding combinedImageSampler(u32 bindingIndex, ShaderStage stages = ShaderStage::Fragment) {
        return {bindingIndex, DescriptorType::CombinedImageSampler, 1, stages};
    }
    
    static DescriptorBinding sampledImage(u32 bindingIndex, ShaderStage stages = ShaderStage::Fragment) {
        return {bindingIndex, DescriptorType::SampledImage, 1, stages};
    }
    
    static DescriptorBinding sampler(u32 bindingIndex, ShaderStage stages = ShaderStage::Fragment) {
        return {bindingIndex, DescriptorType::Sampler, 1, stages};
    }
    
    static DescriptorBinding storageImage(u32 bindingIndex, ShaderStage stages = ShaderStage::Compute) {
        return {bindingIndex, DescriptorType::StorageImage, 1, stages};
    }
    
    static DescriptorBinding imageArray(u32 bindingIndex, u32 arraySize, ShaderStage stages = ShaderStage::Fragment) {
        return {bindingIndex, DescriptorType::CombinedImageSampler, arraySize, stages};
    }
};

// ============================================================================
// Descriptor Set Layout
// ============================================================================

/**
 * @brief Descriptor set layout creation descriptor
 */
struct DescriptorSetLayoutDesc {
    std::vector<DescriptorBinding> bindings;    ///< All bindings in this layout
    std::string name;                           ///< Debug name
    
    /// Add a binding to the layout
    DescriptorSetLayoutDesc& addBinding(const DescriptorBinding& binding) {
        bindings.push_back(binding);
        return *this;
    }
    
    /// Factory: Create a simple material layout (diffuse + normal + specular)
    static DescriptorSetLayoutDesc material() {
        DescriptorSetLayoutDesc desc;
        desc.name = "Material";
        desc.addBinding(DescriptorBinding::combinedImageSampler(0, ShaderStage::Fragment))  // Diffuse
            .addBinding(DescriptorBinding::combinedImageSampler(1, ShaderStage::Fragment))  // Normal
            .addBinding(DescriptorBinding::combinedImageSampler(2, ShaderStage::Fragment)); // Specular
        return desc;
    }
    
    /// Factory: Create a per-object layout (transform UBO)
    static DescriptorSetLayoutDesc perObject() {
        DescriptorSetLayoutDesc desc;
        desc.name = "PerObject";
        desc.addBinding(DescriptorBinding::uniformBuffer(0, ShaderStage::Vertex));
        return desc;
    }
    
    /// Factory: Create a per-frame layout (view/projection UBO)
    static DescriptorSetLayoutDesc perFrame() {
        DescriptorSetLayoutDesc desc;
        desc.name = "PerFrame";
        desc.addBinding(DescriptorBinding::uniformBuffer(0, ShaderStage::Vertex | ShaderStage::Fragment));
        return desc;
    }
};

/**
 * @brief Vulkan descriptor set layout wrapper
 * 
 * Immutable after creation. Thread-safe.
 */
class VulkanDescriptorSetLayout {
public:
    ~VulkanDescriptorSetLayout();
    
    // Non-copyable
    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
    VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;
    
    // Moveable
    VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept;
    VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) noexcept;
    
    /**
     * @brief Create a descriptor set layout
     * @param device Vulkan device
     * @param desc Layout descriptor
     * @return Layout or error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanDescriptorSetLayout>, Error>
    create(VulkanDevice& device, const DescriptorSetLayoutDesc& desc);
    
    /// Get Vulkan handle
    [[nodiscard]] VkDescriptorSetLayout getHandle() const noexcept { return m_layout; }
    
    /// Get binding info
    [[nodiscard]] const std::vector<DescriptorBinding>& getBindings() const noexcept { return m_bindings; }
    
    /// Get debug name
    [[nodiscard]] const std::string& getName() const noexcept { return m_name; }

private:
    VulkanDescriptorSetLayout(VulkanDevice& device);
    
    VulkanDevice* m_device = nullptr;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    std::vector<DescriptorBinding> m_bindings;
    std::string m_name;
};

// ============================================================================
// Descriptor Pool
// ============================================================================

/**
 * @brief Descriptor pool size specification
 */
struct DescriptorPoolSize {
    DescriptorType type = DescriptorType::UniformBuffer;
    u32 count = 0;
};

/**
 * @brief Descriptor pool creation descriptor
 */
struct DescriptorPoolDesc {
    std::vector<DescriptorPoolSize> sizes;      ///< Pool size for each descriptor type
    u32 maxSets = 1000;                         ///< Maximum allocatable sets
    bool freeIndividual = false;                ///< Allow individual set freeing
    std::string name;                           ///< Debug name
    
    /// Add pool size for a descriptor type
    DescriptorPoolDesc& addPoolSize(DescriptorType type, u32 count) {
        sizes.push_back({type, count});
        return *this;
    }
    
    /// Factory: Create a general-purpose pool
    static DescriptorPoolDesc general(u32 maxSets = 1000) {
        DescriptorPoolDesc desc;
        desc.name = "GeneralPool";
        desc.maxSets = maxSets;
        desc.freeIndividual = true;
        desc.addPoolSize(DescriptorType::UniformBuffer, maxSets * 4)
            .addPoolSize(DescriptorType::StorageBuffer, maxSets * 2)
            .addPoolSize(DescriptorType::CombinedImageSampler, maxSets * 8)
            .addPoolSize(DescriptorType::SampledImage, maxSets * 4)
            .addPoolSize(DescriptorType::Sampler, maxSets * 4)
            .addPoolSize(DescriptorType::StorageImage, maxSets * 2);
        return desc;
    }
    
    /// Factory: Create a per-frame pool (no individual freeing, reset each frame)
    static DescriptorPoolDesc perFrame(u32 maxSets = 100) {
        DescriptorPoolDesc desc;
        desc.name = "PerFramePool";
        desc.maxSets = maxSets;
        desc.freeIndividual = false;
        desc.addPoolSize(DescriptorType::UniformBuffer, maxSets * 2)
            .addPoolSize(DescriptorType::CombinedImageSampler, maxSets * 4);
        return desc;
    }
};

/**
 * @brief Vulkan descriptor pool wrapper
 * 
 * Manages descriptor set allocation. Thread-safe for allocation with external sync.
 */
class VulkanDescriptorPool {
public:
    ~VulkanDescriptorPool();
    
    // Non-copyable
    VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
    VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;
    
    // Moveable
    VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept;
    VulkanDescriptorPool& operator=(VulkanDescriptorPool&& other) noexcept;
    
    /**
     * @brief Create a descriptor pool
     * @param device Vulkan device
     * @param desc Pool descriptor
     * @return Pool or error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanDescriptorPool>, Error>
    create(VulkanDevice& device, const DescriptorPoolDesc& desc);
    
    /**
     * @brief Allocate descriptor sets from this pool
     * @param layout Layout to use
     * @param count Number of sets to allocate
     * @return Allocated set handles or error
     */
    [[nodiscard]] Result<std::vector<VkDescriptorSet>, Error>
    allocateSets(const VulkanDescriptorSetLayout& layout, u32 count = 1);
    
    /**
     * @brief Free descriptor sets back to pool
     * @param sets Sets to free
     * @note Only valid if pool was created with freeIndividual = true
     */
    void freeSets(std::span<const VkDescriptorSet> sets);
    
    /**
     * @brief Reset entire pool, freeing all allocated sets
     */
    void reset();
    
    /// Get Vulkan handle
    [[nodiscard]] VkDescriptorPool getHandle() const noexcept { return m_pool; }
    
    /// Get maximum sets
    [[nodiscard]] u32 getMaxSets() const noexcept { return m_maxSets; }
    
    /// Get currently allocated set count
    [[nodiscard]] u32 getAllocatedCount() const noexcept { return m_allocatedCount; }
    
    /// Check if pool supports individual set freeing
    [[nodiscard]] bool canFreeIndividual() const noexcept { return m_freeIndividual; }

private:
    VulkanDescriptorPool(VulkanDevice& device);
    
    VulkanDevice* m_device = nullptr;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    u32 m_maxSets = 0;
    u32 m_allocatedCount = 0;
    bool m_freeIndividual = false;
};

// ============================================================================
// Descriptor Set Writer
// ============================================================================

/**
 * @brief Buffer info for descriptor updates
 */
struct DescriptorBufferInfo {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    VkDeviceSize range = VK_WHOLE_SIZE;
};

/**
 * @brief Image info for descriptor updates
 */
struct DescriptorImageInfo {
    VkSampler sampler = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
};

/**
 * @brief Descriptor set update builder
 * 
 * Provides a fluent API for updating descriptor sets.
 */
class VulkanDescriptorWriter {
public:
    explicit VulkanDescriptorWriter(VulkanDevice& device);
    
    /// Clear all pending writes
    VulkanDescriptorWriter& clear();
    
    /// Write a buffer descriptor
    VulkanDescriptorWriter& writeBuffer(
        VkDescriptorSet set,
        u32 binding,
        DescriptorType type,
        const DescriptorBufferInfo& bufferInfo,
        u32 arrayElement = 0);
    
    /// Write multiple buffer descriptors (array)
    VulkanDescriptorWriter& writeBuffers(
        VkDescriptorSet set,
        u32 binding,
        DescriptorType type,
        std::span<const DescriptorBufferInfo> bufferInfos,
        u32 arrayElement = 0);
    
    /// Write an image descriptor
    VulkanDescriptorWriter& writeImage(
        VkDescriptorSet set,
        u32 binding,
        DescriptorType type,
        const DescriptorImageInfo& imageInfo,
        u32 arrayElement = 0);
    
    /// Write multiple image descriptors (array)
    VulkanDescriptorWriter& writeImages(
        VkDescriptorSet set,
        u32 binding,
        DescriptorType type,
        std::span<const DescriptorImageInfo> imageInfos,
        u32 arrayElement = 0);
    
    /// Execute all pending writes
    void update();

private:
    VulkanDevice* m_device;
    std::vector<VkWriteDescriptorSet> m_writes;
    std::vector<VkDescriptorBufferInfo> m_bufferInfos;
    std::vector<VkDescriptorImageInfo> m_imageInfos;
};

// ============================================================================
// Bindless Descriptor Support (Optional, requires VK_EXT_descriptor_indexing)
// ============================================================================

/**
 * @brief Bindless descriptor manager for unbounded texture arrays
 * 
 * Provides GPU-driven indexing into massive texture arrays.
 * Requires VK_EXT_descriptor_indexing extension.
 */
class VulkanBindlessDescriptor {
public:
    ~VulkanBindlessDescriptor();
    
    /**
     * @brief Create bindless descriptor manager
     * @param device Vulkan device
     * @param maxTextures Maximum number of textures in the bindless array
     * @return Manager or error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanBindlessDescriptor>, Error>
    create(VulkanDevice& device, u32 maxTextures = 16384);
    
    /**
     * @brief Add a texture to the bindless array
     * @param imageView Image view to add
     * @param sampler Sampler to use
     * @return Index into the bindless array or error
     */
    [[nodiscard]] Result<u32, Error> addTexture(VkImageView imageView, VkSampler sampler);
    
    /**
     * @brief Remove a texture from the bindless array
     * @param index Index to remove
     */
    void removeTexture(u32 index);
    
    /// Get the descriptor set for binding
    [[nodiscard]] VkDescriptorSet getDescriptorSet() const noexcept { return m_set; }
    
    /// Get the descriptor set layout
    [[nodiscard]] VkDescriptorSetLayout getLayout() const noexcept { return m_layout; }
    
    /// Get current texture count
    [[nodiscard]] u32 getTextureCount() const noexcept { return m_textureCount; }
    
    /// Get maximum texture count
    [[nodiscard]] u32 getMaxTextures() const noexcept { return m_maxTextures; }

private:
    VulkanBindlessDescriptor(VulkanDevice& device);
    
    VulkanDevice* m_device = nullptr;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    VkDescriptorSet m_set = VK_NULL_HANDLE;
    u32 m_maxTextures = 0;
    u32 m_textureCount = 0;
    std::vector<u32> m_freeIndices;
};

} // namespace nova::render::vulkan
