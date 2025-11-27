/**
 * @file vulkan_buffer.cpp
 * @brief Nova GraphicsCore™ - Vulkan Buffer and Memory Implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Implements buffer creation and memory allocation for Vulkan.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_buffer.hpp"
#include "nova/core/render/vulkan/vulkan_device.hpp"
#include <cstring>
#include <algorithm>

namespace nova::render::vulkan {

// ============================================================================
// Helper Functions
// ============================================================================

static VkBufferUsageFlags toVkBufferUsage(BufferUsage usage) noexcept {
    VkBufferUsageFlags flags = 0;
    
    if (static_cast<u32>(usage) & static_cast<u32>(BufferUsage::VertexBuffer)) {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (static_cast<u32>(usage) & static_cast<u32>(BufferUsage::IndexBuffer)) {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (static_cast<u32>(usage) & static_cast<u32>(BufferUsage::UniformBuffer)) {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (static_cast<u32>(usage) & static_cast<u32>(BufferUsage::StorageBuffer)) {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (static_cast<u32>(usage) & static_cast<u32>(BufferUsage::IndirectBuffer)) {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if (static_cast<u32>(usage) & static_cast<u32>(BufferUsage::TransferSrc)) {
        flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (static_cast<u32>(usage) & static_cast<u32>(BufferUsage::TransferDst)) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
    // Always allow transfer for GPU-only buffers (for staging uploads)
    if (flags & (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
    return flags;
}

static VkMemoryPropertyFlags getMemoryProperties(MemoryUsage usage) noexcept {
    switch (usage) {
        case MemoryUsage::GpuOnly:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        case MemoryUsage::CpuOnly:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                   VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        case MemoryUsage::CpuToGpu:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        case MemoryUsage::GpuToCpu:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                   VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        case MemoryUsage::Auto:
        default:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
}

// ============================================================================
// VulkanBuffer Implementation
// ============================================================================

VulkanBuffer::VulkanBuffer(VulkanDevice& device)
    : m_device(&device) {
}

VulkanBuffer::~VulkanBuffer() {
    if (m_device) {
        const auto& funcs = m_device->getDeviceFuncs();
        
        if (m_mappedPtr && !m_persistentlyMapped) {
            funcs.vkUnmapMemory(m_device->getDevice(), m_memory);
        }
        
        if (m_buffer != VK_NULL_HANDLE) {
            funcs.vkDestroyBuffer(m_device->getDevice(), m_buffer, nullptr);
        }
        
        if (m_memory != VK_NULL_HANDLE) {
            funcs.vkFreeMemory(m_device->getDevice(), m_memory, nullptr);
        }
    }
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
    : m_device(other.m_device)
    , m_buffer(other.m_buffer)
    , m_memory(other.m_memory)
    , m_size(other.m_size)
    , m_usage(other.m_usage)
    , m_memoryUsage(other.m_memoryUsage)
    , m_mappedPtr(other.m_mappedPtr)
    , m_persistentlyMapped(other.m_persistentlyMapped) {
    other.m_device = nullptr;
    other.m_buffer = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
    other.m_mappedPtr = nullptr;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept {
    if (this != &other) {
        // Destroy existing resources
        if (m_device) {
            const auto& funcs = m_device->getDeviceFuncs();
            
            if (m_mappedPtr && !m_persistentlyMapped) {
                funcs.vkUnmapMemory(m_device->getDevice(), m_memory);
            }
            if (m_buffer != VK_NULL_HANDLE) {
                funcs.vkDestroyBuffer(m_device->getDevice(), m_buffer, nullptr);
            }
            if (m_memory != VK_NULL_HANDLE) {
                funcs.vkFreeMemory(m_device->getDevice(), m_memory, nullptr);
            }
        }
        
        m_device = other.m_device;
        m_buffer = other.m_buffer;
        m_memory = other.m_memory;
        m_size = other.m_size;
        m_usage = other.m_usage;
        m_memoryUsage = other.m_memoryUsage;
        m_mappedPtr = other.m_mappedPtr;
        m_persistentlyMapped = other.m_persistentlyMapped;
        
        other.m_device = nullptr;
        other.m_buffer = VK_NULL_HANDLE;
        other.m_memory = VK_NULL_HANDLE;
        other.m_mappedPtr = nullptr;
    }
    return *this;
}

Result<std::unique_ptr<VulkanBuffer>, Error>
VulkanBuffer::create(VulkanDevice& device, const VulkanBufferDesc& desc) {
    auto buffer = std::unique_ptr<VulkanBuffer>(new VulkanBuffer(device));
    
    buffer->m_size = desc.size;
    buffer->m_usage = desc.usage;
    buffer->m_memoryUsage = desc.memoryUsage;
    
    const auto& funcs = device.getDeviceFuncs();
    
    // Create buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.size;
    bufferInfo.usage = toVkBufferUsage(desc.usage);
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = funcs.vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &buffer->m_buffer);
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            std::string("Failed to create buffer: ") + vkResultToString(result)));
    }
    
    // Get memory requirements
    VkMemoryRequirements memReqs;
    funcs.vkGetBufferMemoryRequirements(device.getDevice(), buffer->m_buffer, &memReqs);
    
    // Determine memory usage if auto
    MemoryUsage actualUsage = desc.memoryUsage;
    if (actualUsage == MemoryUsage::Auto) {
        // Choose based on buffer usage
        if (static_cast<u32>(desc.usage) & static_cast<u32>(BufferUsage::Uniform)) {
            actualUsage = MemoryUsage::CpuToGpu;
        } else if (static_cast<u32>(desc.usage) & static_cast<u32>(BufferUsage::TransferSrc)) {
            actualUsage = MemoryUsage::CpuOnly;
        } else {
            actualUsage = MemoryUsage::GpuOnly;
        }
    }
    buffer->m_memoryUsage = actualUsage;
    
    // Find memory type
    VkPhysicalDeviceMemoryProperties memProps;
    device.getInstanceFuncs().vkGetPhysicalDeviceMemoryProperties(
        device.getPhysicalDevice(), &memProps);
    
    VkMemoryPropertyFlags desiredProps = getMemoryProperties(actualUsage);
    u32 memoryTypeIndex = UINT32_MAX;
    
    for (u32 i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & desiredProps) == desiredProps) {
            memoryTypeIndex = i;
            break;
        }
    }
    
    // If not found, try without cached bit
    if (memoryTypeIndex == UINT32_MAX && actualUsage == MemoryUsage::CpuOnly) {
        desiredProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        for (u32 i = 0; i < memProps.memoryTypeCount; ++i) {
            if ((memReqs.memoryTypeBits & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags & desiredProps) == desiredProps) {
                memoryTypeIndex = i;
                break;
            }
        }
    }
    
    if (memoryTypeIndex == UINT32_MAX) {
        funcs.vkDestroyBuffer(device.getDevice(), buffer->m_buffer, nullptr);
        return std::unexpected(errors::graphics("Failed to find suitable memory type for buffer"));
    }
    
    // Allocate memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    result = funcs.vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &buffer->m_memory);
    if (result != VK_SUCCESS) {
        funcs.vkDestroyBuffer(device.getDevice(), buffer->m_buffer, nullptr);
        return std::unexpected(errors::graphics(
            std::string("Failed to allocate buffer memory: ") + vkResultToString(result)));
    }
    
    // Bind memory to buffer
    result = funcs.vkBindBufferMemory(device.getDevice(), buffer->m_buffer, buffer->m_memory, 0);
    if (result != VK_SUCCESS) {
        funcs.vkFreeMemory(device.getDevice(), buffer->m_memory, nullptr);
        funcs.vkDestroyBuffer(device.getDevice(), buffer->m_buffer, nullptr);
        return std::unexpected(errors::graphics(
            std::string("Failed to bind buffer memory: ") + vkResultToString(result)));
    }
    
    // Persistently map CPU-visible buffers
    if (actualUsage != MemoryUsage::GpuOnly) {
        result = funcs.vkMapMemory(device.getDevice(), buffer->m_memory, 0, 
                                   VK_WHOLE_SIZE, 0, &buffer->m_mappedPtr);
        if (result == VK_SUCCESS) {
            buffer->m_persistentlyMapped = true;
        }
    }
    
    // Set debug name
    if (!desc.name.empty() && device.isValidationEnabled()) {
        device.setDebugName(VK_OBJECT_TYPE_BUFFER,
                           reinterpret_cast<u64>(buffer->m_buffer),
                           desc.name);
    }
    
    return buffer;
}

void* VulkanBuffer::map() {
    if (m_mappedPtr) {
        return m_mappedPtr;
    }
    
    if (m_memoryUsage == MemoryUsage::GpuOnly) {
        return nullptr;
    }
    
    const auto& funcs = m_device->getDeviceFuncs();
    VkResult result = funcs.vkMapMemory(m_device->getDevice(), m_memory, 0, 
                                        VK_WHOLE_SIZE, 0, &m_mappedPtr);
    
    return (result == VK_SUCCESS) ? m_mappedPtr : nullptr;
}

void VulkanBuffer::unmap() {
    if (m_mappedPtr && !m_persistentlyMapped) {
        const auto& funcs = m_device->getDeviceFuncs();
        funcs.vkUnmapMemory(m_device->getDevice(), m_memory);
        m_mappedPtr = nullptr;
    }
}

void VulkanBuffer::flush(VkDeviceSize offset, VkDeviceSize size) {
    if (!m_mappedPtr) return;
    
    VkMappedMemoryRange range{};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = m_memory;
    range.offset = offset;
    range.size = size;
    
    const auto& funcs = m_device->getDeviceFuncs();
    funcs.vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &range);
}

void VulkanBuffer::invalidate(VkDeviceSize offset, VkDeviceSize size) {
    if (!m_mappedPtr) return;
    
    VkMappedMemoryRange range{};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = m_memory;
    range.offset = offset;
    range.size = size;
    
    const auto& funcs = m_device->getDeviceFuncs();
    funcs.vkInvalidateMappedMemoryRanges(m_device->getDevice(), 1, &range);
}

void VulkanBuffer::upload(const void* data, VkDeviceSize size, VkDeviceSize offset) {
    if (!m_mappedPtr) {
        if (!map()) return;
    }
    
    std::memcpy(static_cast<u8*>(m_mappedPtr) + offset, data, size);
}

void VulkanBuffer::download(void* data, VkDeviceSize size, VkDeviceSize offset) {
    if (!m_mappedPtr) {
        if (!map()) return;
    }
    
    invalidate(offset, size);
    std::memcpy(data, static_cast<u8*>(m_mappedPtr) + offset, size);
}

// ============================================================================
// VulkanMemoryAllocator Implementation
// ============================================================================

VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanDevice& device)
    : m_device(&device) {
    device.getInstanceFuncs().vkGetPhysicalDeviceMemoryProperties(
        device.getPhysicalDevice(), &m_memProps);
}

Result<MemoryAllocation, Error>
VulkanMemoryAllocator::allocateBufferMemory(VkBuffer buffer, MemoryUsage usage) {
    const auto& funcs = m_device->getDeviceFuncs();
    
    VkMemoryRequirements memReqs;
    funcs.vkGetBufferMemoryRequirements(m_device->getDevice(), buffer, &memReqs);
    
    auto typeResult = findMemoryType(memReqs.memoryTypeBits, usage);
    if (!typeResult) {
        return std::unexpected(typeResult.error());
    }
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = *typeResult;
    
    MemoryAllocation allocation;
    allocation.size = memReqs.size;
    allocation.memoryTypeIndex = *typeResult;
    allocation.usage = usage;
    
    VkResult result = funcs.vkAllocateMemory(m_device->getDevice(), &allocInfo, 
                                             nullptr, &allocation.memory);
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            std::string("Failed to allocate buffer memory: ") + vkResultToString(result)));
    }
    
    return allocation;
}

Result<MemoryAllocation, Error>
VulkanMemoryAllocator::allocateImageMemory(VkImage image, MemoryUsage usage) {
    const auto& funcs = m_device->getDeviceFuncs();
    
    VkMemoryRequirements memReqs;
    funcs.vkGetImageMemoryRequirements(m_device->getDevice(), image, &memReqs);
    
    auto typeResult = findMemoryType(memReqs.memoryTypeBits, usage);
    if (!typeResult) {
        return std::unexpected(typeResult.error());
    }
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = *typeResult;
    
    MemoryAllocation allocation;
    allocation.size = memReqs.size;
    allocation.memoryTypeIndex = *typeResult;
    allocation.usage = usage;
    
    VkResult result = funcs.vkAllocateMemory(m_device->getDevice(), &allocInfo, 
                                             nullptr, &allocation.memory);
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            std::string("Failed to allocate image memory: ") + vkResultToString(result)));
    }
    
    return allocation;
}

void VulkanMemoryAllocator::freeMemory(const MemoryAllocation& allocation) {
    if (allocation.memory != VK_NULL_HANDLE) {
        const auto& funcs = m_device->getDeviceFuncs();
        funcs.vkFreeMemory(m_device->getDevice(), allocation.memory, nullptr);
    }
}

Result<u32, Error>
VulkanMemoryAllocator::findMemoryType(u32 typeBits, MemoryUsage usage) {
    VkMemoryPropertyFlags desiredProps = getMemoryProperties(usage);
    
    for (u32 i = 0; i < m_memProps.memoryTypeCount; ++i) {
        if ((typeBits & (1 << i)) &&
            (m_memProps.memoryTypes[i].propertyFlags & desiredProps) == desiredProps) {
            return i;
        }
    }
    
    // Fallback: try without cached bit for CPU memory
    if (usage == MemoryUsage::CpuOnly || usage == MemoryUsage::GpuToCpu) {
        desiredProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        for (u32 i = 0; i < m_memProps.memoryTypeCount; ++i) {
            if ((typeBits & (1 << i)) &&
                (m_memProps.memoryTypes[i].propertyFlags & desiredProps) == desiredProps) {
                return i;
            }
        }
    }
    
    return std::unexpected(errors::graphics("Failed to find suitable memory type"));
}

// ============================================================================
// VulkanStagingManager Implementation
// ============================================================================

VulkanStagingManager::VulkanStagingManager(VulkanDevice& device)
    : m_device(&device) {
    const auto& funcs = device.getDeviceFuncs();
    
    // Create command pool for transfer operations
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = device.getQueueFamilies().graphics;
    
    funcs.vkCreateCommandPool(device.getDevice(), &poolInfo, nullptr, &m_commandPool);
    
    // Allocate command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    funcs.vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &m_commandBuffer);
    
    // Create fence for synchronization
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    
    funcs.vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &m_fence);
}

VulkanStagingManager::~VulkanStagingManager() {
    if (m_device) {
        const auto& funcs = m_device->getDeviceFuncs();
        
        // Wait for any pending operations
        if (m_hasWork) {
            funcs.vkWaitForFences(m_device->getDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);
        }
        
        funcs.vkDestroyFence(m_device->getDevice(), m_fence, nullptr);
        funcs.vkDestroyCommandPool(m_device->getDevice(), m_commandPool, nullptr);
        
        m_stagingBuffers.clear();
    }
}

Result<VulkanBuffer*, Error> VulkanStagingManager::acquireStagingBuffer(VkDeviceSize size) {
    // Find existing buffer with enough space
    for (auto& staging : m_stagingBuffers) {
        if (staging.buffer->getSize() - staging.usedSize >= size) {
            return staging.buffer.get();
        }
    }
    
    // Create new staging buffer
    VkDeviceSize newSize = std::max(size, DEFAULT_STAGING_SIZE);
    auto bufferResult = VulkanBuffer::create(*m_device, VulkanBufferDesc::staging(newSize));
    if (!bufferResult) {
        return std::unexpected(bufferResult.error());
    }
    
    m_stagingBuffers.push_back({std::move(*bufferResult), 0});
    return m_stagingBuffers.back().buffer.get();
}

void VulkanStagingManager::submitAndWait() {
    if (!m_hasWork) return;
    
    const auto& funcs = m_device->getDeviceFuncs();
    
    // End command buffer
    funcs.vkEndCommandBuffer(m_commandBuffer);
    
    // Submit
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    
    funcs.vkQueueSubmit(m_device->getGraphicsQueue(), 1, &submitInfo, m_fence);
    
    // Wait for completion
    funcs.vkWaitForFences(m_device->getDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);
    funcs.vkResetFences(m_device->getDevice(), 1, &m_fence);
    
    // Reset command pool
    funcs.vkResetCommandPool(m_device->getDevice(), m_commandPool, 0);
    
    // Reset staging buffer usage
    for (auto& staging : m_stagingBuffers) {
        staging.usedSize = 0;
    }
    
    m_hasWork = false;
}

Result<void, Error>
VulkanStagingManager::copyToBuffer(VkBuffer dstBuffer, const void* data, 
                                   VkDeviceSize size, VkDeviceSize dstOffset) {
    // Get staging buffer
    auto stagingResult = acquireStagingBuffer(size);
    if (!stagingResult) {
        return std::unexpected(stagingResult.error());
    }
    
    VulkanBuffer* staging = *stagingResult;
    
    // Find the staging buffer entry to track usage
    for (auto& entry : m_stagingBuffers) {
        if (entry.buffer.get() == staging) {
            VkDeviceSize srcOffset = entry.usedSize;
            entry.usedSize += size;
            
            // Copy data to staging buffer
            staging->upload(data, size, srcOffset);
            
            // Begin command buffer if not started
            if (!m_hasWork) {
                const auto& funcs = m_device->getDeviceFuncs();
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                funcs.vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
                m_hasWork = true;
            }
            
            // Record copy command
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = srcOffset;
            copyRegion.dstOffset = dstOffset;
            copyRegion.size = size;
            
            const auto& funcs = m_device->getDeviceFuncs();
            funcs.vkCmdCopyBuffer(m_commandBuffer, staging->getHandle(), dstBuffer, 1, &copyRegion);
            
            break;
        }
    }
    
    return {};
}

Result<void, Error>
VulkanStagingManager::copyToImage(VkImage dstImage, const void* data, VkDeviceSize size,
                                  u32 width, u32 height, u32 layerCount, u32 mipLevel) {
    // Get staging buffer
    auto stagingResult = acquireStagingBuffer(size);
    if (!stagingResult) {
        return std::unexpected(stagingResult.error());
    }
    
    VulkanBuffer* staging = *stagingResult;
    
    // Find the staging buffer entry
    for (auto& entry : m_stagingBuffers) {
        if (entry.buffer.get() == staging) {
            VkDeviceSize srcOffset = entry.usedSize;
            entry.usedSize += size;
            
            // Copy data to staging buffer
            staging->upload(data, size, srcOffset);
            
            // Begin command buffer if not started
            if (!m_hasWork) {
                const auto& funcs = m_device->getDeviceFuncs();
                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                funcs.vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
                m_hasWork = true;
            }
            
            const auto& funcs = m_device->getDeviceFuncs();
            
            // Transition image to transfer dst
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = dstImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = mipLevel;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = layerCount;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            
            funcs.vkCmdPipelineBarrier(m_commandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier);
            
            // Copy buffer to image
            VkBufferImageCopy region{};
            region.bufferOffset = srcOffset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mipLevel;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = layerCount;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {width, height, 1};
            
            funcs.vkCmdCopyBufferToImage(m_commandBuffer, staging->getHandle(), dstImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            
            // Transition image to shader read
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            
            funcs.vkCmdPipelineBarrier(m_commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0, 0, nullptr, 0, nullptr, 1, &barrier);
            
            break;
        }
    }
    
    return {};
}

void VulkanStagingManager::flush() {
    submitAndWait();
}

} // namespace nova::render::vulkan
