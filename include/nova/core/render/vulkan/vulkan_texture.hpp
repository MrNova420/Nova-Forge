/**
 * @file vulkan_texture.hpp
 * @brief Nova GraphicsCore™ - Vulkan Texture System
 * 
 * Provides complete GPU texture management for the Vulkan backend including:
 * - VulkanTexture: GPU texture with automatic memory allocation
 * - VulkanSampler: Texture sampler configuration
 * - VulkanTextureView: Texture view for shader access
 * - Mipmap generation utilities
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "../texture.hpp"
#include "vulkan_types.hpp"
#include "vulkan_buffer.hpp"
#include <memory>
#include <vector>
#include <string>

namespace nova::render::vulkan {
// Forward declarations
class VulkanDevice;
class VulkanCommandBuffer;
}

namespace nova::render {

// Bring in vulkan types for convenience
using vulkan::VulkanDevice;
using vulkan::VulkanCommandBuffer;

/**
 * @brief Vulkan texture wrapper with automatic memory management
 * 
 * Provides complete texture lifecycle management including:
 * - Image creation with proper memory allocation
 * - Image views for shader binding
 * - Layout transitions
 * - Mipmap generation
 * - Upload from CPU data
 */
class VulkanTexture {
public:
    /**
     * @brief Create a new Vulkan texture
     * @param device The Vulkan device
     * @param desc Texture creation descriptor
     * @return Created texture or nullptr on failure
     */
    static std::unique_ptr<VulkanTexture> create(
        VulkanDevice& device,
        const TextureDesc& desc);
    
    /**
     * @brief Create a texture from existing Vulkan image (e.g., swap chain images)
     * @param device The Vulkan device
     * @param image Existing VkImage handle
     * @param format Image format
     * @param width Image width
     * @param height Image height
     * @param ownsImage Whether this wrapper should destroy the image
     * @return Created texture wrapper or nullptr on failure
     */
    static std::unique_ptr<VulkanTexture> createFromImage(
        VulkanDevice& device,
        VkImage image,
        TextureFormat format,
        u32 width,
        u32 height,
        bool ownsImage = false);
    
    ~VulkanTexture();
    
    // Non-copyable
    VulkanTexture(const VulkanTexture&) = delete;
    VulkanTexture& operator=(const VulkanTexture&) = delete;
    
    // Movable
    VulkanTexture(VulkanTexture&& other) noexcept;
    VulkanTexture& operator=(VulkanTexture&& other) noexcept;
    
    /**
     * @brief Upload texture data from CPU memory
     * @param data Pointer to source data
     * @param size Size of data in bytes
     * @param mipLevel Target mip level (0 = base)
     * @param arrayLayer Target array layer (0 = first)
     * @return true if upload succeeded
     */
    bool upload(const void* data, usize size, u32 mipLevel = 0, u32 arrayLayer = 0);
    
    /**
     * @brief Upload texture data for a specific region
     * @param desc Update descriptor with region and data
     * @return true if upload succeeded
     */
    bool upload(const TextureUpdateDesc& desc);
    
    /**
     * @brief Generate mipmaps for this texture
     * @param cmdBuffer Command buffer to record mipmap generation commands
     * @return true if mipmap generation was recorded successfully
     * 
     * @note The command buffer must be in recording state
     * @note The texture must have been created with multiple mip levels
     */
    bool generateMipmaps(VulkanCommandBuffer& cmdBuffer);
    
    /**
     * @brief Transition image layout
     * @param cmdBuffer Command buffer to record transition
     * @param newLayout Target layout
     * @param srcStage Source pipeline stage
     * @param dstStage Destination pipeline stage
     */
    void transitionLayout(
        VulkanCommandBuffer& cmdBuffer,
        VkImageLayout newLayout,
        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    
    // Accessors
    [[nodiscard]] VkImage getImage() const noexcept { return m_image; }
    [[nodiscard]] VkImageView getImageView() const noexcept { return m_imageView; }
    [[nodiscard]] VkDeviceMemory getMemory() const noexcept { return m_memory; }
    [[nodiscard]] VkImageLayout getLayout() const noexcept { return m_currentLayout; }
    [[nodiscard]] VkFormat getVkFormat() const noexcept { return m_vkFormat; }
    
    [[nodiscard]] TextureFormat getFormat() const noexcept { return m_desc.format; }
    [[nodiscard]] TextureType getType() const noexcept { return m_desc.type; }
    [[nodiscard]] u32 getWidth() const noexcept { return m_desc.width; }
    [[nodiscard]] u32 getHeight() const noexcept { return m_desc.height; }
    [[nodiscard]] u32 getDepth() const noexcept { return m_desc.depth; }
    [[nodiscard]] u32 getMipLevels() const noexcept { return m_desc.mipLevels; }
    [[nodiscard]] u32 getArrayLayers() const noexcept { return m_desc.arrayLayers; }
    [[nodiscard]] u32 getSamples() const noexcept { return m_desc.samples; }
    [[nodiscard]] const std::string& getName() const noexcept { return m_desc.name; }
    [[nodiscard]] const TextureDesc& getDesc() const noexcept { return m_desc; }
    
    // Factory methods for common texture configurations
    
    /**
     * @brief Create a 2D texture
     */
    static std::unique_ptr<VulkanTexture> texture2D(
        VulkanDevice& device,
        u32 width, u32 height,
        TextureFormat format = TextureFormat::RGBA8_UNORM,
        u32 mipLevels = 1,
        const void* initialData = nullptr);
    
    /**
     * @brief Create a depth texture
     */
    static std::unique_ptr<VulkanTexture> depthTexture(
        VulkanDevice& device,
        u32 width, u32 height,
        TextureFormat format = TextureFormat::D32_FLOAT);
    
    /**
     * @brief Create a render target texture
     */
    static std::unique_ptr<VulkanTexture> renderTarget(
        VulkanDevice& device,
        u32 width, u32 height,
        TextureFormat format = TextureFormat::RGBA8_UNORM,
        u32 samples = 1);
    
    /**
     * @brief Create a cubemap texture
     */
    static std::unique_ptr<VulkanTexture> cubemap(
        VulkanDevice& device,
        u32 size,
        TextureFormat format = TextureFormat::RGBA8_UNORM,
        u32 mipLevels = 1);

private:
    VulkanTexture(VulkanDevice& device, const TextureDesc& desc);
    
    bool createImage();
    bool createImageView();
    bool allocateMemory();
    void cleanup();
    
    /**
     * @brief Calculate the number of bytes per pixel for a format
     */
    static u32 getBytesPerPixel(TextureFormat format);
    
    /**
     * @brief Calculate the size in bytes for a mip level
     */
    u32 getMipLevelSize(u32 mipLevel) const;
    
    VulkanDevice& m_device;
    TextureDesc m_desc;
    
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    bool m_ownsImage = true;
    VkDeviceSize m_memorySize = 0;
};

/**
 * @brief Vulkan sampler configuration
 * 
 * Provides texture sampling parameters including:
 * - Filtering (linear, nearest, anisotropic)
 * - Address modes (repeat, clamp, mirror)
 * - LOD clamping
 * - Border color
 */
class VulkanSampler {
public:
    /**
     * @brief Sampler creation descriptor
     */
    struct Desc {
        /// Minification filter
        VkFilter minFilter = VK_FILTER_LINEAR;
        
        /// Magnification filter
        VkFilter magFilter = VK_FILTER_LINEAR;
        
        /// Mipmap mode
        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        
        /// Address mode for U coordinate
        VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        
        /// Address mode for V coordinate
        VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        
        /// Address mode for W coordinate
        VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        
        /// Enable anisotropic filtering
        bool anisotropyEnable = true;
        
        /// Maximum anisotropy level
        f32 maxAnisotropy = 16.0f;
        
        /// Enable comparison operations
        bool compareEnable = false;
        
        /// Comparison operation
        VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
        
        /// Minimum LOD
        f32 minLod = 0.0f;
        
        /// Maximum LOD
        f32 maxLod = VK_LOD_CLAMP_NONE;
        
        /// LOD bias
        f32 mipLodBias = 0.0f;
        
        /// Border color
        VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        
        /// Use unnormalized coordinates
        bool unnormalizedCoordinates = false;
        
        /// Debug name
        std::string name;
        
        // Preset factory methods
        static Desc linear() {
            Desc desc;
            desc.minFilter = VK_FILTER_LINEAR;
            desc.magFilter = VK_FILTER_LINEAR;
            desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            return desc;
        }
        
        static Desc nearest() {
            Desc desc;
            desc.minFilter = VK_FILTER_NEAREST;
            desc.magFilter = VK_FILTER_NEAREST;
            desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            desc.anisotropyEnable = false;
            return desc;
        }
        
        static Desc shadow() {
            Desc desc;
            desc.minFilter = VK_FILTER_LINEAR;
            desc.magFilter = VK_FILTER_LINEAR;
            desc.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            desc.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            desc.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            desc.compareEnable = true;
            desc.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            desc.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            return desc;
        }
        
        static Desc clampToEdge() {
            Desc desc = linear();
            desc.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            desc.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            desc.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            return desc;
        }
    };
    
    /**
     * @brief Create a new sampler
     * @param device The Vulkan device
     * @param desc Sampler creation descriptor
     * @return Created sampler or nullptr on failure
     */
    static std::unique_ptr<VulkanSampler> create(
        VulkanDevice& device,
        const Desc& desc);
    
    ~VulkanSampler();
    
    // Non-copyable
    VulkanSampler(const VulkanSampler&) = delete;
    VulkanSampler& operator=(const VulkanSampler&) = delete;
    
    // Movable
    VulkanSampler(VulkanSampler&& other) noexcept;
    VulkanSampler& operator=(VulkanSampler&& other) noexcept;
    
    [[nodiscard]] VkSampler getSampler() const noexcept { return m_sampler; }
    [[nodiscard]] const std::string& getName() const noexcept { return m_name; }

private:
    VulkanSampler(VulkanDevice& device);
    
    VulkanDevice& m_device;
    VkSampler m_sampler = VK_NULL_HANDLE;
    std::string m_name;
};

/**
 * @brief Vulkan texture view for shader access
 * 
 * Allows creating different views of a texture:
 * - Different format (for compatible formats)
 * - Subset of mip levels
 * - Subset of array layers
 * - Different view type (e.g., single layer of array as 2D)
 */
class VulkanTextureView {
public:
    /**
     * @brief Create a texture view
     * @param device The Vulkan device
     * @param texture Source texture
     * @param desc View descriptor
     * @return Created view or nullptr on failure
     */
    static std::unique_ptr<VulkanTextureView> create(
        VulkanDevice& device,
        const VulkanTexture& texture,
        const TextureViewDesc& desc);
    
    /**
     * @brief Create a default view of a texture
     * @param device The Vulkan device
     * @param texture Source texture
     * @return Created view or nullptr on failure
     */
    static std::unique_ptr<VulkanTextureView> createDefault(
        VulkanDevice& device,
        const VulkanTexture& texture);
    
    ~VulkanTextureView();
    
    // Non-copyable
    VulkanTextureView(const VulkanTextureView&) = delete;
    VulkanTextureView& operator=(const VulkanTextureView&) = delete;
    
    // Movable
    VulkanTextureView(VulkanTextureView&& other) noexcept;
    VulkanTextureView& operator=(VulkanTextureView&& other) noexcept;
    
    [[nodiscard]] VkImageView getImageView() const noexcept { return m_imageView; }

private:
    VulkanTextureView(VulkanDevice& device);
    
    VulkanDevice& m_device;
    VkImageView m_imageView = VK_NULL_HANDLE;
};

/**
 * @brief Utility functions for texture operations
 */
namespace TextureUtils {

/**
 * @brief Calculate the maximum number of mip levels for a given resolution
 * @param width Texture width
 * @param height Texture height
 * @return Maximum mip level count
 */
[[nodiscard]] inline u32 calculateMipLevels(u32 width, u32 height) {
    u32 maxDim = (width > height) ? width : height;
    u32 levels = 1;
    while (maxDim > 1) {
        maxDim >>= 1;
        levels++;
    }
    return levels;
}

/**
 * @brief Calculate the dimensions of a specific mip level
 * @param baseWidth Base width
 * @param baseHeight Base height
 * @param mipLevel Mip level (0 = base)
 * @param outWidth Output width
 * @param outHeight Output height
 */
inline void getMipLevelDimensions(
    u32 baseWidth, u32 baseHeight, u32 mipLevel,
    u32& outWidth, u32& outHeight) {
    outWidth = baseWidth >> mipLevel;
    outHeight = baseHeight >> mipLevel;
    if (outWidth == 0) outWidth = 1;
    if (outHeight == 0) outHeight = 1;
}

/**
 * @brief Check if a format is a depth format
 * @param format Texture format
 * @return true if format contains depth
 */
[[nodiscard]] inline bool isDepthFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::D16_UNORM:
        case TextureFormat::D32_FLOAT:
        case TextureFormat::D24_UNORM_S8_UINT:
        case TextureFormat::D32_FLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Check if a format has a stencil component
 * @param format Texture format
 * @return true if format contains stencil
 */
[[nodiscard]] inline bool hasStencil(TextureFormat format) {
    switch (format) {
        case TextureFormat::D24_UNORM_S8_UINT:
        case TextureFormat::D32_FLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Check if a format is compressed
 * @param format Texture format
 * @return true if format is block-compressed
 */
[[nodiscard]] inline bool isCompressedFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::BC1_UNORM:
        case TextureFormat::BC1_SRGB:
        case TextureFormat::BC2_UNORM:
        case TextureFormat::BC2_SRGB:
        case TextureFormat::BC3_UNORM:
        case TextureFormat::BC3_SRGB:
        case TextureFormat::BC4_UNORM:
        case TextureFormat::BC4_SNORM:
        case TextureFormat::BC5_UNORM:
        case TextureFormat::BC5_SNORM:
        case TextureFormat::BC6H_UFLOAT:
        case TextureFormat::BC6H_SFLOAT:
        case TextureFormat::BC7_UNORM:
        case TextureFormat::BC7_SRGB:
        case TextureFormat::ASTC_4x4_UNORM:
        case TextureFormat::ASTC_4x4_SRGB:
        case TextureFormat::ASTC_5x5_UNORM:
        case TextureFormat::ASTC_5x5_SRGB:
        case TextureFormat::ASTC_6x6_UNORM:
        case TextureFormat::ASTC_6x6_SRGB:
        case TextureFormat::ASTC_8x8_UNORM:
        case TextureFormat::ASTC_8x8_SRGB:
        case TextureFormat::ETC2_RGB8_UNORM:
        case TextureFormat::ETC2_RGB8_SRGB:
        case TextureFormat::ETC2_RGBA8_UNORM:
        case TextureFormat::ETC2_RGBA8_SRGB:
            return true;
        default:
            return false;
    }
}

/**
 * @brief Get the appropriate VkImageAspectFlags for a format
 * @param format Texture format
 * @return Image aspect flags
 */
[[nodiscard]] inline VkImageAspectFlags getImageAspect(TextureFormat format) {
    if (TextureUtils::isDepthFormat(format)) {
        if (TextureUtils::hasStencil(format)) {
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    return VK_IMAGE_ASPECT_COLOR_BIT;
}

} // namespace TextureUtils

} // namespace nova::render
