/**
 * @file vulkan_texture.cpp
 * @brief Nova GraphicsCore™ - Vulkan Texture System Implementation
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_texture.hpp"
#include "nova/core/render/vulkan/vulkan_device.hpp"
#include "nova/core/render/vulkan/vulkan_command_buffer.hpp"
#include <cstring>
#include <algorithm>

namespace nova::render {

using namespace nova::render::vulkan;

// =============================================================================
// VulkanTexture Implementation
// =============================================================================

VulkanTexture::VulkanTexture(VulkanDevice& device, const TextureDesc& desc)
    : m_device(device)
    , m_desc(desc)
{
    m_vkFormat = toVkFormat(desc.format);
}

VulkanTexture::~VulkanTexture() {
    cleanup();
}

VulkanTexture::VulkanTexture(VulkanTexture&& other) noexcept
    : m_device(other.m_device)
    , m_desc(std::move(other.m_desc))
    , m_image(other.m_image)
    , m_imageView(other.m_imageView)
    , m_memory(other.m_memory)
    , m_vkFormat(other.m_vkFormat)
    , m_currentLayout(other.m_currentLayout)
    , m_ownsImage(other.m_ownsImage)
    , m_memorySize(other.m_memorySize)
{
    other.m_image = VK_NULL_HANDLE;
    other.m_imageView = VK_NULL_HANDLE;
    other.m_memory = VK_NULL_HANDLE;
}

VulkanTexture& VulkanTexture::operator=(VulkanTexture&& other) noexcept {
    if (this != &other) {
        cleanup();
        
        m_desc = std::move(other.m_desc);
        m_image = other.m_image;
        m_imageView = other.m_imageView;
        m_memory = other.m_memory;
        m_vkFormat = other.m_vkFormat;
        m_currentLayout = other.m_currentLayout;
        m_ownsImage = other.m_ownsImage;
        m_memorySize = other.m_memorySize;
        
        other.m_image = VK_NULL_HANDLE;
        other.m_imageView = VK_NULL_HANDLE;
        other.m_memory = VK_NULL_HANDLE;
    }
    return *this;
}

std::unique_ptr<VulkanTexture> VulkanTexture::create(
    VulkanDevice& device,
    const TextureDesc& desc)
{
    auto texture = std::unique_ptr<VulkanTexture>(new VulkanTexture(device, desc));
    
    // Validate format
    if (texture->m_vkFormat == VK_FORMAT_UNDEFINED) {
        return nullptr;
    }
    
    // Create image
    if (!texture->createImage()) {
        return nullptr;
    }
    
    // Allocate memory
    if (!texture->allocateMemory()) {
        return nullptr;
    }
    
    // Create default image view
    if (!texture->createImageView()) {
        return nullptr;
    }
    
    // Upload initial data if provided
    if (desc.initialData) {
        u32 bytesPerPixel = getBytesPerPixel(desc.format);
        usize dataSize = static_cast<usize>(desc.width) * desc.height * bytesPerPixel;
        if (!texture->upload(desc.initialData, dataSize)) {
            return nullptr;
        }
    }
    
    return texture;
}

std::unique_ptr<VulkanTexture> VulkanTexture::createFromImage(
    VulkanDevice& device,
    VkImage image,
    TextureFormat format,
    u32 width,
    u32 height,
    bool ownsImage)
{
    TextureDesc desc;
    desc.type = TextureType::Texture2D;
    desc.format = format;
    desc.width = width;
    desc.height = height;
    desc.depth = 1;
    desc.mipLevels = 1;
    desc.arrayLayers = 1;
    desc.samples = 1;
    desc.usage = TextureUsage::Sampled;
    
    auto texture = std::unique_ptr<VulkanTexture>(new VulkanTexture(device, desc));
    texture->m_image = image;
    texture->m_ownsImage = ownsImage;
    
    if (!texture->createImageView()) {
        texture->m_image = VK_NULL_HANDLE; // Don't destroy if we don't own it
        return nullptr;
    }
    
    return texture;
}

bool VulkanTexture::createImage() {
    auto& funcs = m_device.getDeviceFuncs();
    VkDevice device = m_device.getDevice();
    
    // Determine image type
    VkImageType imageType = VK_IMAGE_TYPE_2D;
    switch (m_desc.type) {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            imageType = VK_IMAGE_TYPE_1D;
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            imageType = VK_IMAGE_TYPE_2D;
            break;
        case TextureType::Texture3D:
            imageType = VK_IMAGE_TYPE_3D;
            break;
    }
    
    // Build usage flags
    VkImageUsageFlags usageFlags = 0;
    if ((static_cast<u32>(m_desc.usage) & static_cast<u32>(TextureUsage::Sampled)) != 0) {
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if ((static_cast<u32>(m_desc.usage) & static_cast<u32>(TextureUsage::Storage)) != 0) {
        usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if ((static_cast<u32>(m_desc.usage) & static_cast<u32>(TextureUsage::ColorAttachment)) != 0) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if ((static_cast<u32>(m_desc.usage) & static_cast<u32>(TextureUsage::DepthStencilAttachment)) != 0) {
        usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if ((static_cast<u32>(m_desc.usage) & static_cast<u32>(TextureUsage::InputAttachment)) != 0) {
        usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if ((static_cast<u32>(m_desc.usage) & static_cast<u32>(TextureUsage::TransferSrc)) != 0) {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if ((static_cast<u32>(m_desc.usage) & static_cast<u32>(TextureUsage::TransferDst)) != 0) {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    
    // Always allow transfer for data upload
    usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    
    // If generating mipmaps, need transfer src
    if (m_desc.mipLevels > 1) {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    
    // Determine flags
    VkImageCreateFlags createFlags = 0;
    if (m_desc.type == TextureType::TextureCube || m_desc.type == TextureType::TextureCubeArray) {
        createFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    
    // Actual mip levels (0 means calculate full chain)
    u32 mipLevels = m_desc.mipLevels;
    if (mipLevels == 0) {
        mipLevels = TextureUtils::calculateMipLevels(m_desc.width, m_desc.height);
    }
    
    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.flags = createFlags;
    createInfo.imageType = imageType;
    createInfo.format = m_vkFormat;
    createInfo.extent.width = m_desc.width;
    createInfo.extent.height = m_desc.height;
    createInfo.extent.depth = m_desc.depth;
    createInfo.mipLevels = mipLevels;
    createInfo.arrayLayers = m_desc.arrayLayers;
    createInfo.samples = static_cast<VkSampleCountFlagBits>(m_desc.samples);
    createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = usageFlags;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    VkResult result = funcs.vkCreateImage(device, &createInfo, nullptr, &m_image);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    // Update desc with actual mip levels
    m_desc.mipLevels = mipLevels;
    
    return true;
}

bool VulkanTexture::allocateMemory() {
    auto& funcs = m_device.getDeviceFuncs();
    VkDevice device = m_device.getDevice();
    
    // Get memory requirements
    VkMemoryRequirements memRequirements;
    funcs.vkGetImageMemoryRequirements(device, m_image, &memRequirements);
    
    // Find suitable memory type
    VkPhysicalDeviceMemoryProperties memProps;
    m_device.getInstanceFuncs().vkGetPhysicalDeviceMemoryProperties(
        m_device.getPhysicalDevice(), &memProps);
    
    u32 memoryTypeIndex = UINT32_MAX;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    for (u32 i = 0; i < memProps.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            memoryTypeIndex = i;
            break;
        }
    }
    
    if (memoryTypeIndex == UINT32_MAX) {
        return false;
    }
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    VkResult result = funcs.vkAllocateMemory(device, &allocInfo, nullptr, &m_memory);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    m_memorySize = memRequirements.size;
    
    // Bind memory to image
    result = funcs.vkBindImageMemory(device, m_image, m_memory, 0);
    if (result != VK_SUCCESS) {
        funcs.vkFreeMemory(device, m_memory, nullptr);
        m_memory = VK_NULL_HANDLE;
        return false;
    }
    
    return true;
}

bool VulkanTexture::createImageView() {
    auto& funcs = m_device.getDeviceFuncs();
    VkDevice device = m_device.getDevice();
    
    // Determine view type
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    switch (m_desc.type) {
        case TextureType::Texture1D:
            viewType = VK_IMAGE_VIEW_TYPE_1D;
            break;
        case TextureType::Texture2D:
            viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case TextureType::Texture3D:
            viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        case TextureType::TextureCube:
            viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        case TextureType::Texture1DArray:
            viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            break;
        case TextureType::Texture2DArray:
            viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            break;
        case TextureType::TextureCubeArray:
            viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            break;
    }
    
    VkImageAspectFlags aspectFlags = TextureUtils::getImageAspect(m_desc.format);
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = viewType;
    viewInfo.format = m_vkFormat;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_desc.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = m_desc.arrayLayers;
    
    VkResult result = funcs.vkCreateImageView(device, &viewInfo, nullptr, &m_imageView);
    return result == VK_SUCCESS;
}

void VulkanTexture::cleanup() {
    auto& funcs = m_device.getDeviceFuncs();
    VkDevice device = m_device.getDevice();
    
    if (m_imageView != VK_NULL_HANDLE) {
        funcs.vkDestroyImageView(device, m_imageView, nullptr);
        m_imageView = VK_NULL_HANDLE;
    }
    
    if (m_image != VK_NULL_HANDLE && m_ownsImage) {
        funcs.vkDestroyImage(device, m_image, nullptr);
        m_image = VK_NULL_HANDLE;
    }
    
    if (m_memory != VK_NULL_HANDLE) {
        funcs.vkFreeMemory(device, m_memory, nullptr);
        m_memory = VK_NULL_HANDLE;
    }
}

u32 VulkanTexture::getBytesPerPixel(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8_UNORM:
        case TextureFormat::R8_SNORM:
        case TextureFormat::R8_UINT:
        case TextureFormat::R8_SINT:
            return 1;
            
        case TextureFormat::R16_FLOAT:
        case TextureFormat::R16_UNORM:
        case TextureFormat::R16_SNORM:
        case TextureFormat::R16_UINT:
        case TextureFormat::R16_SINT:
        case TextureFormat::RG8_UNORM:
        case TextureFormat::RG8_SNORM:
        case TextureFormat::RG8_UINT:
        case TextureFormat::RG8_SINT:
        case TextureFormat::D16_UNORM:
            return 2;
            
        case TextureFormat::R32_FLOAT:
        case TextureFormat::R32_UINT:
        case TextureFormat::R32_SINT:
        case TextureFormat::RG16_FLOAT:
        case TextureFormat::RG16_UNORM:
        case TextureFormat::RG16_SNORM:
        case TextureFormat::RG16_UINT:
        case TextureFormat::RG16_SINT:
        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::RGBA8_SNORM:
        case TextureFormat::RGBA8_UINT:
        case TextureFormat::RGBA8_SINT:
        case TextureFormat::RGBA8_SRGB:
        case TextureFormat::BGRA8_UNORM:
        case TextureFormat::BGRA8_SRGB:
        case TextureFormat::RGB10A2_UNORM:
        case TextureFormat::RGB10A2_UINT:
        case TextureFormat::D32_FLOAT:
        case TextureFormat::D24_UNORM_S8_UINT:
            return 4;
            
        case TextureFormat::RG32_FLOAT:
        case TextureFormat::RG32_UINT:
        case TextureFormat::RG32_SINT:
        case TextureFormat::RGBA16_FLOAT:
        case TextureFormat::RGBA16_UNORM:
        case TextureFormat::RGBA16_SNORM:
        case TextureFormat::RGBA16_UINT:
        case TextureFormat::RGBA16_SINT:
        case TextureFormat::D32_FLOAT_S8_UINT:
            return 8;
            
        case TextureFormat::RGBA32_FLOAT:
        case TextureFormat::RGBA32_UINT:
        case TextureFormat::RGBA32_SINT:
            return 16;
            
        default:
            // Compressed formats and unknown - calculate based on block size
            return 4;
    }
}

u32 VulkanTexture::getMipLevelSize(u32 mipLevel) const {
    u32 width, height;
    TextureUtils::getMipLevelDimensions(m_desc.width, m_desc.height, mipLevel, width, height);
    return width * height * getBytesPerPixel(m_desc.format);
}

bool VulkanTexture::upload(const void* data, usize size, u32 mipLevel, u32 arrayLayer) {
    TextureUpdateDesc desc;
    desc.mipLevel = mipLevel;
    desc.arrayLayer = arrayLayer;
    desc.offsetX = 0;
    desc.offsetY = 0;
    desc.offsetZ = 0;
    
    TextureUtils::getMipLevelDimensions(m_desc.width, m_desc.height, mipLevel,
                                         desc.width, desc.height);
    desc.depth = 1;
    desc.data = data;
    desc.rowPitch = 0;
    desc.slicePitch = 0;
    
    return upload(desc);
}

bool VulkanTexture::upload(const TextureUpdateDesc& desc) {
    if (!desc.data) {
        return false;
    }
    
    auto& funcs = m_device.getDeviceFuncs();
    VkDevice device = m_device.getDevice();
    
    // Calculate data size
    u32 bytesPerPixel = getBytesPerPixel(m_desc.format);
    u32 rowPitch = desc.rowPitch > 0 ? desc.rowPitch : desc.width * bytesPerPixel;
    usize dataSize = static_cast<usize>(rowPitch) * desc.height * desc.depth;
    
    // Create staging buffer
    auto stagingBufferResult = VulkanBuffer::create(m_device, VulkanBufferDesc::staging(dataSize));
    if (!stagingBufferResult) {
        return false;
    }
    auto& stagingBuffer = *stagingBufferResult;
    
    // Copy data to staging buffer
    void* mapped = stagingBuffer->map();
    if (!mapped) {
        return false;
    }
    
    // Copy row by row in case of padding
    if (desc.rowPitch == 0 || desc.rowPitch == desc.width * bytesPerPixel) {
        std::memcpy(mapped, desc.data, dataSize);
    } else {
        const u8* srcPtr = static_cast<const u8*>(desc.data);
        u8* dstPtr = static_cast<u8*>(mapped);
        u32 srcRowPitch = desc.width * bytesPerPixel;
        for (u32 y = 0; y < desc.height; y++) {
            std::memcpy(dstPtr, srcPtr, srcRowPitch);
            srcPtr += desc.rowPitch;
            dstPtr += srcRowPitch;
        }
    }
    
    stagingBuffer->unmap();
    
    // Create temporary command buffer for the copy
    VkCommandPool commandPool = m_device.getTransferCommandPool();
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer cmdBuffer;
    VkResult result = funcs.vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    funcs.vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    
    // Transition image to transfer dst layout
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = m_currentLayout;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = TextureUtils::getImageAspect(m_desc.format);
    barrier.subresourceRange.baseMipLevel = desc.mipLevel;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = desc.arrayLayer;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
    funcs.vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
    
    // Copy buffer to image
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = TextureUtils::getImageAspect(m_desc.format);
    region.imageSubresource.mipLevel = desc.mipLevel;
    region.imageSubresource.baseArrayLayer = desc.arrayLayer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { static_cast<i32>(desc.offsetX), static_cast<i32>(desc.offsetY), static_cast<i32>(desc.offsetZ) };
    region.imageExtent = { desc.width, desc.height, desc.depth };
    
    funcs.vkCmdCopyBufferToImage(
        cmdBuffer,
        stagingBuffer->getHandle(),
        m_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region);
    
    // Transition to shader read layout
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    funcs.vkCmdPipelineBarrier(
        cmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
    
    m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    funcs.vkEndCommandBuffer(cmdBuffer);
    
    // Submit and wait
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    
    VkQueue queue = m_device.getGraphicsQueue();
    funcs.vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    funcs.vkQueueWaitIdle(queue);
    
    funcs.vkFreeCommandBuffers(device, commandPool, 1, &cmdBuffer);
    
    return true;
}

bool VulkanTexture::generateMipmaps(VulkanCommandBuffer& cmdBuffer) {
    if (m_desc.mipLevels <= 1) {
        return true; // No mipmaps to generate
    }
    
    auto& funcs = m_device.getDeviceFuncs();
    VkCommandBuffer vkCmdBuffer = cmdBuffer.getCommandBuffer();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = m_image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = m_desc.arrayLayers;
    barrier.subresourceRange.levelCount = 1;
    
    i32 mipWidth = static_cast<i32>(m_desc.width);
    i32 mipHeight = static_cast<i32>(m_desc.height);
    
    for (u32 i = 1; i < m_desc.mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        
        funcs.vkCmdPipelineBarrier(
            vkCmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        
        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = m_desc.arrayLayers;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { 
            mipWidth > 1 ? mipWidth / 2 : 1, 
            mipHeight > 1 ? mipHeight / 2 : 1, 
            1 
        };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = m_desc.arrayLayers;
        
        funcs.vkCmdBlitImage(
            vkCmdBuffer,
            m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        funcs.vkCmdPipelineBarrier(
            vkCmdBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    
    // Transition last mip level
    barrier.subresourceRange.baseMipLevel = m_desc.mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    funcs.vkCmdPipelineBarrier(
        vkCmdBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
    
    m_currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    return true;
}

void VulkanTexture::transitionLayout(
    VulkanCommandBuffer& cmdBuffer,
    VkImageLayout newLayout,
    VkPipelineStageFlags srcStage,
    VkPipelineStageFlags dstStage)
{
    if (m_currentLayout == newLayout) {
        return;
    }
    
    auto& funcs = m_device.getDeviceFuncs();
    VkCommandBuffer vkCmdBuffer = cmdBuffer.getCommandBuffer();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = m_currentLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = TextureUtils::getImageAspect(m_desc.format);
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = m_desc.mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = m_desc.arrayLayers;
    
    // Determine access masks based on layouts
    switch (m_currentLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.srcAccessMask = 0;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        default:
            barrier.srcAccessMask = 0;
            break;
    }
    
    switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.dstAccessMask = 0;
            break;
        default:
            barrier.dstAccessMask = 0;
            break;
    }
    
    funcs.vkCmdPipelineBarrier(
        vkCmdBuffer,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
    
    m_currentLayout = newLayout;
}

// Factory methods

std::unique_ptr<VulkanTexture> VulkanTexture::texture2D(
    VulkanDevice& device,
    u32 width, u32 height,
    TextureFormat format,
    u32 mipLevels,
    const void* initialData)
{
    TextureDesc desc = TextureDesc::texture2D(width, height, format, mipLevels);
    desc.initialData = initialData;
    return create(device, desc);
}

std::unique_ptr<VulkanTexture> VulkanTexture::depthTexture(
    VulkanDevice& device,
    u32 width, u32 height,
    TextureFormat format)
{
    TextureDesc desc = TextureDesc::depthTarget(width, height, format);
    return create(device, desc);
}

std::unique_ptr<VulkanTexture> VulkanTexture::renderTarget(
    VulkanDevice& device,
    u32 width, u32 height,
    TextureFormat format,
    u32 samples)
{
    TextureDesc desc = TextureDesc::renderTarget(width, height, format);
    desc.samples = samples;
    return create(device, desc);
}

std::unique_ptr<VulkanTexture> VulkanTexture::cubemap(
    VulkanDevice& device,
    u32 size,
    TextureFormat format,
    u32 mipLevels)
{
    TextureDesc desc = TextureDesc::cubemap(size, format, mipLevels);
    return create(device, desc);
}

// =============================================================================
// VulkanSampler Implementation
// =============================================================================

VulkanSampler::VulkanSampler(VulkanDevice& device)
    : m_device(device)
{
}

VulkanSampler::~VulkanSampler() {
    if (m_sampler != VK_NULL_HANDLE) {
        m_device.getDeviceFuncs().vkDestroySampler(
            m_device.getDevice(), m_sampler, nullptr);
    }
}

VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept
    : m_device(other.m_device)
    , m_sampler(other.m_sampler)
    , m_name(std::move(other.m_name))
{
    other.m_sampler = VK_NULL_HANDLE;
}

VulkanSampler& VulkanSampler::operator=(VulkanSampler&& other) noexcept {
    if (this != &other) {
        if (m_sampler != VK_NULL_HANDLE) {
            m_device.getDeviceFuncs().vkDestroySampler(
                m_device.getDevice(), m_sampler, nullptr);
        }
        
        m_sampler = other.m_sampler;
        m_name = std::move(other.m_name);
        
        other.m_sampler = VK_NULL_HANDLE;
    }
    return *this;
}

std::unique_ptr<VulkanSampler> VulkanSampler::create(
    VulkanDevice& device,
    const Desc& desc)
{
    auto sampler = std::unique_ptr<VulkanSampler>(new VulkanSampler(device));
    sampler->m_name = desc.name;
    
    auto& funcs = device.getDeviceFuncs();
    VkDevice vkDevice = device.getDevice();
    
    // Clamp max anisotropy to device limits
    f32 maxAnisotropy = desc.maxAnisotropy;
    const auto& limits = device.getDeviceInfo().limits;
    if (maxAnisotropy > limits.maxAnisotropy) {
        maxAnisotropy = limits.maxAnisotropy;
    }
    
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = desc.magFilter;
    samplerInfo.minFilter = desc.minFilter;
    samplerInfo.mipmapMode = desc.mipmapMode;
    samplerInfo.addressModeU = desc.addressModeU;
    samplerInfo.addressModeV = desc.addressModeV;
    samplerInfo.addressModeW = desc.addressModeW;
    samplerInfo.mipLodBias = desc.mipLodBias;
    samplerInfo.anisotropyEnable = desc.anisotropyEnable ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = maxAnisotropy;
    samplerInfo.compareEnable = desc.compareEnable ? VK_TRUE : VK_FALSE;
    samplerInfo.compareOp = desc.compareOp;
    samplerInfo.minLod = desc.minLod;
    samplerInfo.maxLod = desc.maxLod;
    samplerInfo.borderColor = desc.borderColor;
    samplerInfo.unnormalizedCoordinates = desc.unnormalizedCoordinates ? VK_TRUE : VK_FALSE;
    
    VkResult result = funcs.vkCreateSampler(vkDevice, &samplerInfo, nullptr, &sampler->m_sampler);
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    
    return sampler;
}

// =============================================================================
// VulkanTextureView Implementation
// =============================================================================

VulkanTextureView::VulkanTextureView(VulkanDevice& device)
    : m_device(device)
{
}

VulkanTextureView::~VulkanTextureView() {
    if (m_imageView != VK_NULL_HANDLE) {
        m_device.getDeviceFuncs().vkDestroyImageView(
            m_device.getDevice(), m_imageView, nullptr);
    }
}

VulkanTextureView::VulkanTextureView(VulkanTextureView&& other) noexcept
    : m_device(other.m_device)
    , m_imageView(other.m_imageView)
{
    other.m_imageView = VK_NULL_HANDLE;
}

VulkanTextureView& VulkanTextureView::operator=(VulkanTextureView&& other) noexcept {
    if (this != &other) {
        if (m_imageView != VK_NULL_HANDLE) {
            m_device.getDeviceFuncs().vkDestroyImageView(
                m_device.getDevice(), m_imageView, nullptr);
        }
        
        m_imageView = other.m_imageView;
        other.m_imageView = VK_NULL_HANDLE;
    }
    return *this;
}

std::unique_ptr<VulkanTextureView> VulkanTextureView::create(
    VulkanDevice& device,
    const VulkanTexture& texture,
    const TextureViewDesc& desc)
{
    auto view = std::unique_ptr<VulkanTextureView>(new VulkanTextureView(device));
    
    // Determine view type
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    switch (desc.viewType) {
        case TextureType::Texture1D:
            viewType = VK_IMAGE_VIEW_TYPE_1D;
            break;
        case TextureType::Texture2D:
            viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case TextureType::Texture3D:
            viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        case TextureType::TextureCube:
            viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        case TextureType::Texture1DArray:
            viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            break;
        case TextureType::Texture2DArray:
            viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            break;
        case TextureType::TextureCubeArray:
            viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            break;
    }
    
    // Use texture format if not specified
    VkFormat format = texture.getVkFormat();
    if (desc.format != TextureFormat::Unknown) {
        format = toVkFormat(desc.format);
    }
    
    VkImageAspectFlags aspectFlags = TextureUtils::getImageAspect(texture.getFormat());
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture.getImage();
    viewInfo.viewType = viewType;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = desc.baseMipLevel;
    viewInfo.subresourceRange.levelCount = desc.mipLevelCount;
    viewInfo.subresourceRange.baseArrayLayer = desc.baseArrayLayer;
    viewInfo.subresourceRange.layerCount = desc.arrayLayerCount;
    
    VkResult result = device.getDeviceFuncs().vkCreateImageView(
        device.getDevice(), &viewInfo, nullptr, &view->m_imageView);
    
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    
    return view;
}

std::unique_ptr<VulkanTextureView> VulkanTextureView::createDefault(
    VulkanDevice& device,
    const VulkanTexture& texture)
{
    TextureViewDesc desc;
    desc.viewType = texture.getType();
    desc.format = TextureFormat::Unknown;
    desc.baseMipLevel = 0;
    desc.mipLevelCount = texture.getMipLevels();
    desc.baseArrayLayer = 0;
    desc.arrayLayerCount = texture.getArrayLayers();
    
    return create(device, texture, desc);
}

} // namespace nova::render
