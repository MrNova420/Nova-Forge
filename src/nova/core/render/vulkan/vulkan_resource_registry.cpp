/**
 * @file vulkan_resource_registry.cpp
 * @brief Nova GraphicsCore™ - Vulkan GPU Resource Registry Implementation
 * 
 * Implements comprehensive resource tracking and handle-to-Vulkan-object mapping.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_resource_registry.hpp"
#include "nova/core/render/vulkan/vulkan_device.hpp"

namespace nova::render::vulkan {

// ============================================================================
// Construction / Destruction
// ============================================================================

VulkanResourceRegistry::VulkanResourceRegistry(VulkanDevice& device)
    : m_device(device)
{
}

VulkanResourceRegistry::~VulkanResourceRegistry() {
    clear(true);
}

// ============================================================================
// Handle Generation
// ============================================================================

u64 VulkanResourceRegistry::generateHandle() {
    return m_nextHandle.fetch_add(1, std::memory_order_relaxed);
}

// ============================================================================
// Buffer Registration
// ============================================================================

BufferHandle VulkanResourceRegistry::registerBuffer(
    VkBuffer buffer,
    VkDeviceMemory memory,
    VkDeviceSize size,
    VkDeviceSize offset,
    BufferUsage usage,
    const std::string& name)
{
    u64 handleValue = generateHandle();
    
    BufferEntry entry;
    entry.buffer = buffer;
    entry.memory = memory;
    entry.size = size;
    entry.offset = offset;
    entry.usage = usage;
    entry.name = name;
    entry.creationFrame = m_currentFrame.load(std::memory_order_relaxed);
    entry.ownsMemory = true;
    
    {
        std::unique_lock<std::shared_mutex> lock(m_bufferMutex);
        m_buffers[handleValue] = std::move(entry);
    }
    
    return BufferHandle(handleValue);
}

const BufferEntry* VulkanResourceRegistry::getBuffer(BufferHandle handle) const {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_bufferMutex);
    auto it = m_buffers.find(handle.id());
    return it != m_buffers.end() ? &it->second : nullptr;
}

BufferEntry* VulkanResourceRegistry::getBufferMutable(BufferHandle handle) {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_bufferMutex);
    auto it = m_buffers.find(handle.id());
    return it != m_buffers.end() ? &it->second : nullptr;
}

void VulkanResourceRegistry::unregisterBuffer(BufferHandle handle, bool destroy) {
    if (!handle.isValid()) return;
    
    BufferEntry entry;
    {
        std::unique_lock<std::shared_mutex> lock(m_bufferMutex);
        auto it = m_buffers.find(handle.id());
        if (it == m_buffers.end()) return;
        entry = std::move(it->second);
        m_buffers.erase(it);
    }
    
    if (destroy && entry.buffer != VK_NULL_HANDLE) {
        const auto& funcs = m_device.getDeviceFuncs();
        
        // Destroy buffer
        funcs.vkDestroyBuffer(m_device.getDevice(), entry.buffer, nullptr);
        
        // Free memory if we own it
        if (entry.ownsMemory && entry.memory != VK_NULL_HANDLE) {
            funcs.vkFreeMemory(m_device.getDevice(), entry.memory, nullptr);
        }
    }
    
    m_destroyedCount.fetch_add(1, std::memory_order_relaxed);
}

bool VulkanResourceRegistry::isValidBuffer(BufferHandle handle) const {
    if (!handle.isValid()) return false;
    
    std::shared_lock<std::shared_mutex> lock(m_bufferMutex);
    return m_buffers.find(handle.id()) != m_buffers.end();
}

// ============================================================================
// Image Registration
// ============================================================================

TextureHandle VulkanResourceRegistry::registerImage(
    VkImage image,
    VkImageView view,
    VkDeviceMemory memory,
    VkFormat format,
    u32 width,
    u32 height,
    TextureUsage usage,
    const std::string& name)
{
    u64 handleValue = generateHandle();
    
    ImageEntry entry;
    entry.image = image;
    entry.view = view;
    entry.memory = memory;
    entry.format = format;
    entry.width = width;
    entry.height = height;
    entry.usage = usage;
    entry.name = name;
    entry.creationFrame = m_currentFrame.load(std::memory_order_relaxed);
    entry.ownsImage = true;
    entry.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    {
        std::unique_lock<std::shared_mutex> lock(m_imageMutex);
        m_images[handleValue] = std::move(entry);
    }
    
    return TextureHandle(handleValue);
}

const ImageEntry* VulkanResourceRegistry::getImage(TextureHandle handle) const {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_imageMutex);
    auto it = m_images.find(handle.id());
    return it != m_images.end() ? &it->second : nullptr;
}

ImageEntry* VulkanResourceRegistry::getImageMutable(TextureHandle handle) {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_imageMutex);
    auto it = m_images.find(handle.id());
    return it != m_images.end() ? &it->second : nullptr;
}

void VulkanResourceRegistry::unregisterImage(TextureHandle handle, bool destroy) {
    if (!handle.isValid()) return;
    
    ImageEntry entry;
    {
        std::unique_lock<std::shared_mutex> lock(m_imageMutex);
        auto it = m_images.find(handle.id());
        if (it == m_images.end()) return;
        entry = std::move(it->second);
        m_images.erase(it);
    }
    
    if (destroy) {
        const auto& funcs = m_device.getDeviceFuncs();
        
        // Destroy image view
        if (entry.view != VK_NULL_HANDLE) {
            funcs.vkDestroyImageView(m_device.getDevice(), entry.view, nullptr);
        }
        
        // Destroy image if we own it
        if (entry.ownsImage && entry.image != VK_NULL_HANDLE) {
            funcs.vkDestroyImage(m_device.getDevice(), entry.image, nullptr);
        }
        
        // Free memory
        if (entry.memory != VK_NULL_HANDLE) {
            funcs.vkFreeMemory(m_device.getDevice(), entry.memory, nullptr);
        }
    }
    
    m_destroyedCount.fetch_add(1, std::memory_order_relaxed);
}

bool VulkanResourceRegistry::isValidImage(TextureHandle handle) const {
    if (!handle.isValid()) return false;
    
    std::shared_lock<std::shared_mutex> lock(m_imageMutex);
    return m_images.find(handle.id()) != m_images.end();
}

// ============================================================================
// Pipeline Registration
// ============================================================================

PipelineHandle VulkanResourceRegistry::registerPipeline(
    VkPipeline pipeline,
    VkPipelineLayout layout,
    VkPipelineBindPoint bindPoint,
    const std::string& name)
{
    u64 handleValue = generateHandle();
    
    PipelineEntry entry;
    entry.pipeline = pipeline;
    entry.layout = layout;
    entry.bindPoint = bindPoint;
    entry.name = name;
    entry.creationFrame = m_currentFrame.load(std::memory_order_relaxed);
    
    {
        std::unique_lock<std::shared_mutex> lock(m_pipelineMutex);
        m_pipelines[handleValue] = std::move(entry);
    }
    
    return PipelineHandle(handleValue);
}

const PipelineEntry* VulkanResourceRegistry::getPipeline(PipelineHandle handle) const {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_pipelineMutex);
    auto it = m_pipelines.find(handle.id());
    return it != m_pipelines.end() ? &it->second : nullptr;
}

void VulkanResourceRegistry::unregisterPipeline(PipelineHandle handle, bool destroy) {
    if (!handle.isValid()) return;
    
    PipelineEntry entry;
    {
        std::unique_lock<std::shared_mutex> lock(m_pipelineMutex);
        auto it = m_pipelines.find(handle.id());
        if (it == m_pipelines.end()) return;
        entry = std::move(it->second);
        m_pipelines.erase(it);
    }
    
    if (destroy) {
        const auto& funcs = m_device.getDeviceFuncs();
        
        if (entry.pipeline != VK_NULL_HANDLE) {
            funcs.vkDestroyPipeline(m_device.getDevice(), entry.pipeline, nullptr);
        }
        
        // Note: Pipeline layout is typically shared, don't destroy here
        // Layout destruction should be handled separately by the pipeline cache
    }
    
    m_destroyedCount.fetch_add(1, std::memory_order_relaxed);
}

bool VulkanResourceRegistry::isValidPipeline(PipelineHandle handle) const {
    if (!handle.isValid()) return false;
    
    std::shared_lock<std::shared_mutex> lock(m_pipelineMutex);
    return m_pipelines.find(handle.id()) != m_pipelines.end();
}

// ============================================================================
// Render Pass Registration
// ============================================================================

RenderPassHandle VulkanResourceRegistry::registerRenderPass(
    VkRenderPass renderPass,
    u32 attachmentCount,
    const std::string& name)
{
    u64 handleValue = generateHandle();
    
    RenderPassEntry entry;
    entry.renderPass = renderPass;
    entry.attachmentCount = attachmentCount;
    entry.name = name;
    
    {
        std::unique_lock<std::shared_mutex> lock(m_renderPassMutex);
        m_renderPasses[handleValue] = std::move(entry);
    }
    
    return RenderPassHandle(handleValue);
}

const RenderPassEntry* VulkanResourceRegistry::getRenderPass(RenderPassHandle handle) const {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_renderPassMutex);
    auto it = m_renderPasses.find(handle.id());
    return it != m_renderPasses.end() ? &it->second : nullptr;
}

void VulkanResourceRegistry::unregisterRenderPass(RenderPassHandle handle, bool destroy) {
    if (!handle.isValid()) return;
    
    RenderPassEntry entry;
    {
        std::unique_lock<std::shared_mutex> lock(m_renderPassMutex);
        auto it = m_renderPasses.find(handle.id());
        if (it == m_renderPasses.end()) return;
        entry = std::move(it->second);
        m_renderPasses.erase(it);
    }
    
    if (destroy && entry.renderPass != VK_NULL_HANDLE) {
        const auto& funcs = m_device.getDeviceFuncs();
        funcs.vkDestroyRenderPass(m_device.getDevice(), entry.renderPass, nullptr);
    }
    
    m_destroyedCount.fetch_add(1, std::memory_order_relaxed);
}

// ============================================================================
// Framebuffer Registration
// ============================================================================

FramebufferHandle VulkanResourceRegistry::registerFramebuffer(
    VkFramebuffer framebuffer,
    VkRenderPass renderPass,
    u32 width,
    u32 height,
    const std::string& name)
{
    u64 handleValue = generateHandle();
    
    FramebufferEntry entry;
    entry.framebuffer = framebuffer;
    entry.compatibleRenderPass = renderPass;
    entry.width = width;
    entry.height = height;
    entry.name = name;
    
    {
        std::unique_lock<std::shared_mutex> lock(m_framebufferMutex);
        m_framebuffers[handleValue] = std::move(entry);
    }
    
    return FramebufferHandle(handleValue);
}

const FramebufferEntry* VulkanResourceRegistry::getFramebuffer(FramebufferHandle handle) const {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_framebufferMutex);
    auto it = m_framebuffers.find(handle.id());
    return it != m_framebuffers.end() ? &it->second : nullptr;
}

void VulkanResourceRegistry::unregisterFramebuffer(FramebufferHandle handle, bool destroy) {
    if (!handle.isValid()) return;
    
    FramebufferEntry entry;
    {
        std::unique_lock<std::shared_mutex> lock(m_framebufferMutex);
        auto it = m_framebuffers.find(handle.id());
        if (it == m_framebuffers.end()) return;
        entry = std::move(it->second);
        m_framebuffers.erase(it);
    }
    
    if (destroy && entry.framebuffer != VK_NULL_HANDLE) {
        const auto& funcs = m_device.getDeviceFuncs();
        funcs.vkDestroyFramebuffer(m_device.getDevice(), entry.framebuffer, nullptr);
    }
    
    m_destroyedCount.fetch_add(1, std::memory_order_relaxed);
}

// ============================================================================
// Descriptor Set Registration
// ============================================================================

DescriptorSetHandle VulkanResourceRegistry::registerDescriptorSet(
    VkDescriptorSet descriptorSet,
    VkDescriptorSetLayout layout,
    VkDescriptorPool pool,
    const std::string& name)
{
    u64 handleValue = generateHandle();
    
    DescriptorSetEntry entry;
    entry.descriptorSet = descriptorSet;
    entry.layout = layout;
    entry.pool = pool;
    entry.name = name;
    
    {
        std::unique_lock<std::shared_mutex> lock(m_descriptorSetMutex);
        m_descriptorSets[handleValue] = std::move(entry);
    }
    
    return DescriptorSetHandle(handleValue);
}

const DescriptorSetEntry* VulkanResourceRegistry::getDescriptorSet(DescriptorSetHandle handle) const {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_descriptorSetMutex);
    auto it = m_descriptorSets.find(handle.id());
    return it != m_descriptorSets.end() ? &it->second : nullptr;
}

void VulkanResourceRegistry::unregisterDescriptorSet(DescriptorSetHandle handle) {
    if (!handle.isValid()) return;
    
    {
        std::unique_lock<std::shared_mutex> lock(m_descriptorSetMutex);
        m_descriptorSets.erase(handle.id());
    }
    
    // Note: Descriptor sets are typically freed back to the pool, not individually destroyed
    m_destroyedCount.fetch_add(1, std::memory_order_relaxed);
}

// ============================================================================
// Sampler Registration
// ============================================================================

SamplerHandle VulkanResourceRegistry::registerSampler(VkSampler sampler, const std::string& name) {
    u64 handleValue = generateHandle();
    
    SamplerEntry entry;
    entry.sampler = sampler;
    entry.name = name;
    
    {
        std::unique_lock<std::shared_mutex> lock(m_samplerMutex);
        m_samplers[handleValue] = std::move(entry);
    }
    
    return SamplerHandle(handleValue);
}

const SamplerEntry* VulkanResourceRegistry::getSampler(SamplerHandle handle) const {
    if (!handle.isValid()) return nullptr;
    
    std::shared_lock<std::shared_mutex> lock(m_samplerMutex);
    auto it = m_samplers.find(handle.id());
    return it != m_samplers.end() ? &it->second : nullptr;
}

void VulkanResourceRegistry::unregisterSampler(SamplerHandle handle, bool destroy) {
    if (!handle.isValid()) return;
    
    SamplerEntry entry;
    {
        std::unique_lock<std::shared_mutex> lock(m_samplerMutex);
        auto it = m_samplers.find(handle.id());
        if (it == m_samplers.end()) return;
        entry = std::move(it->second);
        m_samplers.erase(it);
    }
    
    if (destroy && entry.sampler != VK_NULL_HANDLE) {
        const auto& funcs = m_device.getDeviceFuncs();
        funcs.vkDestroySampler(m_device.getDevice(), entry.sampler, nullptr);
    }
    
    m_destroyedCount.fetch_add(1, std::memory_order_relaxed);
}

// ============================================================================
// Statistics
// ============================================================================

ResourceRegistryStats VulkanResourceRegistry::getStats() const {
    ResourceRegistryStats stats;
    
    // Buffer stats
    {
        std::shared_lock<std::shared_mutex> lock(m_bufferMutex);
        stats.bufferCount = static_cast<u32>(m_buffers.size());
        for (const auto& [handle, entry] : m_buffers) {
            stats.totalBufferMemory += entry.size;
        }
    }
    
    // Image stats
    {
        std::shared_lock<std::shared_mutex> lock(m_imageMutex);
        stats.imageCount = static_cast<u32>(m_images.size());
        for (const auto& [handle, entry] : m_images) {
            // Estimate memory based on image dimensions and format
            // This is an approximation; actual memory may vary
            u64 bpp = 4;  // Assume 4 bytes per pixel for most formats
            stats.totalImageMemory += static_cast<VkDeviceSize>(entry.width) * 
                                       entry.height * entry.depth * 
                                       entry.arrayLayers * bpp;
        }
    }
    
    // Other resource counts
    {
        std::shared_lock<std::shared_mutex> lock(m_pipelineMutex);
        stats.pipelineCount = static_cast<u32>(m_pipelines.size());
    }
    {
        std::shared_lock<std::shared_mutex> lock(m_renderPassMutex);
        stats.renderPassCount = static_cast<u32>(m_renderPasses.size());
    }
    {
        std::shared_lock<std::shared_mutex> lock(m_framebufferMutex);
        stats.framebufferCount = static_cast<u32>(m_framebuffers.size());
    }
    {
        std::shared_lock<std::shared_mutex> lock(m_descriptorSetMutex);
        stats.descriptorSetCount = static_cast<u32>(m_descriptorSets.size());
    }
    {
        std::shared_lock<std::shared_mutex> lock(m_samplerMutex);
        stats.samplerCount = static_cast<u32>(m_samplers.size());
    }
    
    stats.handleGenerations = m_nextHandle.load(std::memory_order_relaxed);
    stats.destroyedResources = m_destroyedCount.load(std::memory_order_relaxed);
    
    return stats;
}

// ============================================================================
// Clear All Resources
// ============================================================================

void VulkanResourceRegistry::clear(bool destroy) {
    const auto& funcs = m_device.getDeviceFuncs();
    VkDevice device = m_device.getDevice();
    
    // Clear samplers
    {
        std::unique_lock<std::shared_mutex> lock(m_samplerMutex);
        if (destroy) {
            for (auto& [handle, entry] : m_samplers) {
                if (entry.sampler != VK_NULL_HANDLE) {
                    funcs.vkDestroySampler(device, entry.sampler, nullptr);
                }
            }
        }
        m_samplers.clear();
    }
    
    // Clear descriptor sets (no Vulkan destroy needed - freed with pool)
    {
        std::unique_lock<std::shared_mutex> lock(m_descriptorSetMutex);
        m_descriptorSets.clear();
    }
    
    // Clear framebuffers
    {
        std::unique_lock<std::shared_mutex> lock(m_framebufferMutex);
        if (destroy) {
            for (auto& [handle, entry] : m_framebuffers) {
                if (entry.framebuffer != VK_NULL_HANDLE) {
                    funcs.vkDestroyFramebuffer(device, entry.framebuffer, nullptr);
                }
            }
        }
        m_framebuffers.clear();
    }
    
    // Clear render passes
    {
        std::unique_lock<std::shared_mutex> lock(m_renderPassMutex);
        if (destroy) {
            for (auto& [handle, entry] : m_renderPasses) {
                if (entry.renderPass != VK_NULL_HANDLE) {
                    funcs.vkDestroyRenderPass(device, entry.renderPass, nullptr);
                }
            }
        }
        m_renderPasses.clear();
    }
    
    // Clear pipelines
    {
        std::unique_lock<std::shared_mutex> lock(m_pipelineMutex);
        if (destroy) {
            for (auto& [handle, entry] : m_pipelines) {
                if (entry.pipeline != VK_NULL_HANDLE) {
                    funcs.vkDestroyPipeline(device, entry.pipeline, nullptr);
                }
            }
        }
        m_pipelines.clear();
    }
    
    // Clear images
    {
        std::unique_lock<std::shared_mutex> lock(m_imageMutex);
        if (destroy) {
            for (auto& [handle, entry] : m_images) {
                if (entry.view != VK_NULL_HANDLE) {
                    funcs.vkDestroyImageView(device, entry.view, nullptr);
                }
                if (entry.ownsImage && entry.image != VK_NULL_HANDLE) {
                    funcs.vkDestroyImage(device, entry.image, nullptr);
                }
                if (entry.memory != VK_NULL_HANDLE) {
                    funcs.vkFreeMemory(device, entry.memory, nullptr);
                }
            }
        }
        m_images.clear();
    }
    
    // Clear buffers
    {
        std::unique_lock<std::shared_mutex> lock(m_bufferMutex);
        if (destroy) {
            for (auto& [handle, entry] : m_buffers) {
                if (entry.buffer != VK_NULL_HANDLE) {
                    funcs.vkDestroyBuffer(device, entry.buffer, nullptr);
                }
                if (entry.ownsMemory && entry.memory != VK_NULL_HANDLE) {
                    funcs.vkFreeMemory(device, entry.memory, nullptr);
                }
            }
        }
        m_buffers.clear();
    }
}

} // namespace nova::render::vulkan
