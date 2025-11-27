/**
 * @file vulkan_resource_registry.hpp
 * @brief Nova GraphicsCore™ - Vulkan GPU Resource Registry
 * 
 * Provides complete resource tracking and handle-to-Vulkan-object mapping for:
 * - VkBuffer handles
 * - VkImage handles  
 * - VkPipeline handles
 * - VkRenderPass handles
 * - VkFramebuffer handles
 * - VkDescriptorSet handles
 * 
 * This enables the command buffer to properly execute commands using type-safe
 * handles that abstract away raw Vulkan pointers.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "vulkan_types.hpp"
#include "nova/core/types/types.hpp"
#include "nova/core/types/result.hpp"
#include "nova/core/render/render_types.hpp"
#include "nova/core/render/buffer.hpp"
#include "nova/core/render/texture.hpp"

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <memory>

namespace nova::render::vulkan {

// Forward declarations
class VulkanDevice;

// ============================================================================
// Resource Entry Structures
// ============================================================================

/**
 * @brief Buffer resource entry containing Vulkan buffer and memory info
 */
struct BufferEntry {
    VkBuffer buffer = VK_NULL_HANDLE;           ///< Vulkan buffer handle
    VkDeviceMemory memory = VK_NULL_HANDLE;     ///< Backing memory
    VkDeviceSize size = 0;                      ///< Size in bytes
    VkDeviceSize offset = 0;                    ///< Offset in memory allocation
    BufferUsage usage = BufferUsage::Vertex;    ///< Buffer usage flags
    void* mappedPtr = nullptr;                  ///< Persistently mapped pointer (if applicable)
    std::string name;                           ///< Debug name
    u64 creationFrame = 0;                      ///< Frame when created (for tracking)
    bool ownsMemory = true;                     ///< Whether to free memory on destruction
};

/**
 * @brief Image/Texture resource entry containing Vulkan image and view info
 */
struct ImageEntry {
    VkImage image = VK_NULL_HANDLE;             ///< Vulkan image handle
    VkImageView view = VK_NULL_HANDLE;          ///< Default image view
    VkDeviceMemory memory = VK_NULL_HANDLE;     ///< Backing memory
    VkFormat format = VK_FORMAT_UNDEFINED;      ///< Image format
    VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED; ///< Current layout
    u32 width = 0;                              ///< Image width
    u32 height = 0;                             ///< Image height
    u32 depth = 1;                              ///< Image depth
    u32 mipLevels = 1;                          ///< Number of mip levels
    u32 arrayLayers = 1;                        ///< Number of array layers
    TextureUsage usage = TextureUsage::Sampled; ///< Texture usage flags
    std::string name;                           ///< Debug name
    u64 creationFrame = 0;                      ///< Frame when created
    bool ownsImage = true;                      ///< Whether to destroy image on cleanup
};

/**
 * @brief Pipeline resource entry containing Vulkan pipeline and layout info
 */
struct PipelineEntry {
    VkPipeline pipeline = VK_NULL_HANDLE;       ///< Vulkan pipeline handle
    VkPipelineLayout layout = VK_NULL_HANDLE;   ///< Pipeline layout
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; ///< Graphics or compute
    std::string name;                           ///< Debug name
    u64 creationFrame = 0;                      ///< Frame when created
};

/**
 * @brief Render pass entry containing Vulkan render pass handle
 */
struct RenderPassEntry {
    VkRenderPass renderPass = VK_NULL_HANDLE;   ///< Vulkan render pass handle
    u32 attachmentCount = 0;                    ///< Number of attachments
    std::string name;                           ///< Debug name
};

/**
 * @brief Framebuffer entry containing Vulkan framebuffer handle
 */
struct FramebufferEntry {
    VkFramebuffer framebuffer = VK_NULL_HANDLE; ///< Vulkan framebuffer handle
    VkRenderPass compatibleRenderPass = VK_NULL_HANDLE; ///< Compatible render pass
    u32 width = 0;                              ///< Framebuffer width
    u32 height = 0;                             ///< Framebuffer height
    u32 layers = 1;                             ///< Framebuffer layers
    std::vector<TextureHandle> attachments;     ///< Attached images
    std::string name;                           ///< Debug name
};

/**
 * @brief Descriptor set entry containing Vulkan descriptor set and pool info
 */
struct DescriptorSetEntry {
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE; ///< Vulkan descriptor set
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;  ///< Layout used to create
    VkDescriptorPool pool = VK_NULL_HANDLE;         ///< Pool allocated from
    std::string name;                               ///< Debug name
};

/**
 * @brief Sampler entry containing Vulkan sampler handle
 */
struct SamplerEntry {
    VkSampler sampler = VK_NULL_HANDLE;         ///< Vulkan sampler handle
    std::string name;                           ///< Debug name
};

// ============================================================================
// Resource Registry Statistics
// ============================================================================

/**
 * @brief Statistics about registered resources
 */
struct ResourceRegistryStats {
    u32 bufferCount = 0;            ///< Number of registered buffers
    u32 imageCount = 0;             ///< Number of registered images
    u32 pipelineCount = 0;          ///< Number of registered pipelines
    u32 renderPassCount = 0;        ///< Number of registered render passes
    u32 framebufferCount = 0;       ///< Number of registered framebuffers
    u32 descriptorSetCount = 0;     ///< Number of registered descriptor sets
    u32 samplerCount = 0;           ///< Number of registered samplers
    
    VkDeviceSize totalBufferMemory = 0;  ///< Total buffer memory allocated
    VkDeviceSize totalImageMemory = 0;   ///< Total image memory allocated
    
    u64 handleGenerations = 0;      ///< Total handles ever generated
    u64 destroyedResources = 0;     ///< Total resources destroyed
};

// ============================================================================
// Vulkan Resource Registry
// ============================================================================

/**
 * @brief Central registry for all Vulkan GPU resources
 * 
 * Provides thread-safe handle-to-resource mapping, enabling the command buffer
 * and other systems to use type-safe handles instead of raw Vulkan pointers.
 * 
 * Features:
 * - O(1) lookup from handle to resource
 * - Thread-safe read/write operations
 * - Automatic handle generation with collision avoidance
 * - Resource lifecycle tracking
 * - Memory usage statistics
 */
class VulkanResourceRegistry {
public:
    /**
     * @brief Construct resource registry
     * @param device Vulkan device reference
     */
    explicit VulkanResourceRegistry(VulkanDevice& device);
    ~VulkanResourceRegistry();
    
    // Non-copyable
    VulkanResourceRegistry(const VulkanResourceRegistry&) = delete;
    VulkanResourceRegistry& operator=(const VulkanResourceRegistry&) = delete;
    
    // =========================================================================
    // Buffer Registration
    // =========================================================================
    
    /**
     * @brief Register a buffer and get a handle
     * @param buffer Vulkan buffer handle
     * @param memory Backing memory
     * @param size Buffer size
     * @param offset Offset in memory
     * @param usage Buffer usage
     * @param name Debug name
     * @return Handle to the registered buffer
     */
    BufferHandle registerBuffer(
        VkBuffer buffer,
        VkDeviceMemory memory,
        VkDeviceSize size,
        VkDeviceSize offset = 0,
        BufferUsage usage = BufferUsage::Vertex,
        const std::string& name = "");
    
    /**
     * @brief Get buffer entry by handle
     * @param handle Buffer handle
     * @return Pointer to buffer entry or nullptr if not found
     */
    const BufferEntry* getBuffer(BufferHandle handle) const;
    
    /**
     * @brief Get mutable buffer entry by handle
     * @param handle Buffer handle
     * @return Pointer to buffer entry or nullptr if not found
     */
    BufferEntry* getBufferMutable(BufferHandle handle);
    
    /**
     * @brief Unregister and optionally destroy a buffer
     * @param handle Buffer handle
     * @param destroy Whether to destroy the Vulkan resources
     */
    void unregisterBuffer(BufferHandle handle, bool destroy = true);
    
    // =========================================================================
    // Image Registration
    // =========================================================================
    
    /**
     * @brief Register an image and get a handle
     * @param image Vulkan image handle
     * @param view Default image view
     * @param memory Backing memory
     * @param format Image format
     * @param width Width in pixels
     * @param height Height in pixels
     * @param usage Texture usage
     * @param name Debug name
     * @return Handle to the registered image
     */
    TextureHandle registerImage(
        VkImage image,
        VkImageView view,
        VkDeviceMemory memory,
        VkFormat format,
        u32 width,
        u32 height,
        TextureUsage usage = TextureUsage::Sampled,
        const std::string& name = "");
    
    /**
     * @brief Get image entry by handle
     * @param handle Texture handle
     * @return Pointer to image entry or nullptr if not found
     */
    const ImageEntry* getImage(TextureHandle handle) const;
    
    /**
     * @brief Get mutable image entry by handle
     * @param handle Texture handle
     * @return Pointer to image entry or nullptr if not found
     */
    ImageEntry* getImageMutable(TextureHandle handle);
    
    /**
     * @brief Unregister and optionally destroy an image
     * @param handle Texture handle
     * @param destroy Whether to destroy the Vulkan resources
     */
    void unregisterImage(TextureHandle handle, bool destroy = true);
    
    // =========================================================================
    // Pipeline Registration
    // =========================================================================
    
    /**
     * @brief Register a pipeline and get a handle
     * @param pipeline Vulkan pipeline
     * @param layout Pipeline layout
     * @param bindPoint Graphics or compute
     * @param name Debug name
     * @return Handle to the registered pipeline
     */
    PipelineHandle registerPipeline(
        VkPipeline pipeline,
        VkPipelineLayout layout,
        VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        const std::string& name = "");
    
    /**
     * @brief Get pipeline entry by handle
     * @param handle Pipeline handle
     * @return Pointer to pipeline entry or nullptr if not found
     */
    const PipelineEntry* getPipeline(PipelineHandle handle) const;
    
    /**
     * @brief Unregister and optionally destroy a pipeline
     * @param handle Pipeline handle
     * @param destroy Whether to destroy the Vulkan resources
     */
    void unregisterPipeline(PipelineHandle handle, bool destroy = true);
    
    // =========================================================================
    // Render Pass Registration
    // =========================================================================
    
    /**
     * @brief Register a render pass and get a handle
     * @param renderPass Vulkan render pass
     * @param attachmentCount Number of attachments
     * @param name Debug name
     * @return Handle to the registered render pass
     */
    RenderPassHandle registerRenderPass(
        VkRenderPass renderPass,
        u32 attachmentCount = 0,
        const std::string& name = "");
    
    /**
     * @brief Get render pass entry by handle
     * @param handle Render pass handle
     * @return Pointer to render pass entry or nullptr if not found
     */
    const RenderPassEntry* getRenderPass(RenderPassHandle handle) const;
    
    /**
     * @brief Unregister and optionally destroy a render pass
     * @param handle Render pass handle
     * @param destroy Whether to destroy the Vulkan resource
     */
    void unregisterRenderPass(RenderPassHandle handle, bool destroy = true);
    
    // =========================================================================
    // Framebuffer Registration
    // =========================================================================
    
    /**
     * @brief Register a framebuffer and get a handle
     * @param framebuffer Vulkan framebuffer
     * @param renderPass Compatible render pass
     * @param width Framebuffer width
     * @param height Framebuffer height
     * @param name Debug name
     * @return Handle to the registered framebuffer
     */
    FramebufferHandle registerFramebuffer(
        VkFramebuffer framebuffer,
        VkRenderPass renderPass,
        u32 width,
        u32 height,
        const std::string& name = "");
    
    /**
     * @brief Get framebuffer entry by handle
     * @param handle Framebuffer handle
     * @return Pointer to framebuffer entry or nullptr if not found
     */
    const FramebufferEntry* getFramebuffer(FramebufferHandle handle) const;
    
    /**
     * @brief Unregister and optionally destroy a framebuffer
     * @param handle Framebuffer handle
     * @param destroy Whether to destroy the Vulkan resource
     */
    void unregisterFramebuffer(FramebufferHandle handle, bool destroy = true);
    
    // =========================================================================
    // Descriptor Set Registration
    // =========================================================================
    
    /**
     * @brief Register a descriptor set and get a handle
     * @param descriptorSet Vulkan descriptor set
     * @param layout Descriptor set layout
     * @param pool Pool allocated from
     * @param name Debug name
     * @return Handle to the registered descriptor set
     */
    DescriptorSetHandle registerDescriptorSet(
        VkDescriptorSet descriptorSet,
        VkDescriptorSetLayout layout,
        VkDescriptorPool pool,
        const std::string& name = "");
    
    /**
     * @brief Get descriptor set entry by handle
     * @param handle Descriptor set handle
     * @return Pointer to descriptor set entry or nullptr if not found
     */
    const DescriptorSetEntry* getDescriptorSet(DescriptorSetHandle handle) const;
    
    /**
     * @brief Unregister a descriptor set (typically freed back to pool)
     * @param handle Descriptor set handle
     */
    void unregisterDescriptorSet(DescriptorSetHandle handle);
    
    // =========================================================================
    // Sampler Registration
    // =========================================================================
    
    /**
     * @brief Register a sampler and get a handle
     * @param sampler Vulkan sampler
     * @param name Debug name
     * @return Handle to the registered sampler
     */
    SamplerHandle registerSampler(VkSampler sampler, const std::string& name = "");
    
    /**
     * @brief Get sampler entry by handle
     * @param handle Sampler handle
     * @return Pointer to sampler entry or nullptr if not found
     */
    const SamplerEntry* getSampler(SamplerHandle handle) const;
    
    /**
     * @brief Unregister and optionally destroy a sampler
     * @param handle Sampler handle
     * @param destroy Whether to destroy the Vulkan resource
     */
    void unregisterSampler(SamplerHandle handle, bool destroy = true);
    
    // =========================================================================
    // Utility Functions
    // =========================================================================
    
    /**
     * @brief Get current statistics
     * @return Resource registry statistics
     */
    ResourceRegistryStats getStats() const;
    
    /**
     * @brief Clear all registered resources
     * @param destroy Whether to destroy Vulkan resources
     */
    void clear(bool destroy = true);
    
    /**
     * @brief Set current frame number for tracking
     * @param frame Current frame number
     */
    void setCurrentFrame(u64 frame) { m_currentFrame = frame; }
    
    /**
     * @brief Check if a buffer handle is valid
     * @param handle Buffer handle to check
     * @return true if handle maps to a valid buffer
     */
    bool isValidBuffer(BufferHandle handle) const;
    
    /**
     * @brief Check if an image handle is valid
     * @param handle Texture handle to check
     * @return true if handle maps to a valid image
     */
    bool isValidImage(TextureHandle handle) const;
    
    /**
     * @brief Check if a pipeline handle is valid
     * @param handle Pipeline handle to check
     * @return true if handle maps to a valid pipeline
     */
    bool isValidPipeline(PipelineHandle handle) const;

private:
    /**
     * @brief Generate a unique handle value
     * @return New unique handle value
     */
    u64 generateHandle();
    
    VulkanDevice& m_device;                     ///< Device reference
    
    // Resource maps (handle value -> entry)
    mutable std::shared_mutex m_bufferMutex;
    std::unordered_map<u64, BufferEntry> m_buffers;
    
    mutable std::shared_mutex m_imageMutex;
    std::unordered_map<u64, ImageEntry> m_images;
    
    mutable std::shared_mutex m_pipelineMutex;
    std::unordered_map<u64, PipelineEntry> m_pipelines;
    
    mutable std::shared_mutex m_renderPassMutex;
    std::unordered_map<u64, RenderPassEntry> m_renderPasses;
    
    mutable std::shared_mutex m_framebufferMutex;
    std::unordered_map<u64, FramebufferEntry> m_framebuffers;
    
    mutable std::shared_mutex m_descriptorSetMutex;
    std::unordered_map<u64, DescriptorSetEntry> m_descriptorSets;
    
    mutable std::shared_mutex m_samplerMutex;
    std::unordered_map<u64, SamplerEntry> m_samplers;
    
    // Handle generation
    std::atomic<u64> m_nextHandle{1};           ///< Next handle value to generate
    std::atomic<u64> m_destroyedCount{0};       ///< Count of destroyed resources
    std::atomic<u64> m_currentFrame{0};         ///< Current frame number
};

} // namespace nova::render::vulkan
