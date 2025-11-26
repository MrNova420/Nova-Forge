/**
 * @file texture.hpp
 * @brief GPU texture types and descriptors
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include <string>

namespace nova::render {

/**
 * @brief Texture type/dimension
 */
enum class TextureType : u8 {
    Texture1D,
    Texture2D,
    Texture3D,
    TextureCube,
    Texture1DArray,
    Texture2DArray,
    TextureCubeArray
};

/**
 * @brief Texture usage flags
 */
enum class TextureUsage : u32 {
    None = 0,
    Sampled = 1 << 0,           ///< Can be sampled in shaders
    Storage = 1 << 1,           ///< Can be used as storage image
    ColorAttachment = 1 << 2,   ///< Can be used as color attachment
    DepthStencilAttachment = 1 << 3, ///< Can be used as depth/stencil attachment
    InputAttachment = 1 << 4,   ///< Can be used as input attachment
    TransferSrc = 1 << 5,       ///< Can be used as transfer source
    TransferDst = 1 << 6,       ///< Can be used as transfer destination
    
    // Convenience combinations
    RenderTarget = ColorAttachment | Sampled,
    DepthTarget = DepthStencilAttachment | Sampled,
    ShaderResource = Sampled | TransferDst
};

inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline TextureUsage operator&(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<u32>(a) & static_cast<u32>(b));
}

/**
 * @brief Texture creation descriptor
 */
struct TextureDesc {
    /// Texture type
    TextureType type = TextureType::Texture2D;
    
    /// Texture format
    TextureFormat format = TextureFormat::RGBA8_UNORM;
    
    /// Width in pixels
    u32 width = 1;
    
    /// Height in pixels
    u32 height = 1;
    
    /// Depth in pixels (for 3D textures)
    u32 depth = 1;
    
    /// Number of mip levels (0 = full mip chain)
    u32 mipLevels = 1;
    
    /// Number of array layers
    u32 arrayLayers = 1;
    
    /// Number of samples (for MSAA)
    u32 samples = 1;
    
    /// Usage flags
    TextureUsage usage = TextureUsage::Sampled;
    
    /// Initial data (optional, row-major)
    const void* initialData = nullptr;
    
    /// Debug name
    std::string name;
    
    // Factory methods
    static TextureDesc texture2D(u32 width, u32 height, 
                                 TextureFormat format = TextureFormat::RGBA8_UNORM,
                                 u32 mipLevels = 1) {
        TextureDesc desc;
        desc.type = TextureType::Texture2D;
        desc.width = width;
        desc.height = height;
        desc.format = format;
        desc.mipLevels = mipLevels;
        desc.usage = TextureUsage::ShaderResource;
        return desc;
    }
    
    static TextureDesc renderTarget(u32 width, u32 height,
                                    TextureFormat format = TextureFormat::RGBA8_UNORM) {
        TextureDesc desc;
        desc.type = TextureType::Texture2D;
        desc.width = width;
        desc.height = height;
        desc.format = format;
        desc.usage = TextureUsage::RenderTarget;
        return desc;
    }
    
    static TextureDesc depthTarget(u32 width, u32 height,
                                   TextureFormat format = TextureFormat::D32_FLOAT) {
        TextureDesc desc;
        desc.type = TextureType::Texture2D;
        desc.width = width;
        desc.height = height;
        desc.format = format;
        desc.usage = TextureUsage::DepthTarget;
        return desc;
    }
    
    static TextureDesc cubemap(u32 size, TextureFormat format = TextureFormat::RGBA8_UNORM,
                               u32 mipLevels = 1) {
        TextureDesc desc;
        desc.type = TextureType::TextureCube;
        desc.width = size;
        desc.height = size;
        desc.format = format;
        desc.mipLevels = mipLevels;
        desc.arrayLayers = 6;
        desc.usage = TextureUsage::ShaderResource;
        return desc;
    }
};

/**
 * @brief Texture update descriptor
 */
struct TextureUpdateDesc {
    /// Mip level to update
    u32 mipLevel = 0;
    
    /// Array layer to update
    u32 arrayLayer = 0;
    
    /// X offset in texels
    u32 offsetX = 0;
    
    /// Y offset in texels
    u32 offsetY = 0;
    
    /// Z offset in texels (3D textures)
    u32 offsetZ = 0;
    
    /// Width of the region to update
    u32 width = 0;
    
    /// Height of the region to update
    u32 height = 0;
    
    /// Depth of the region to update
    u32 depth = 1;
    
    /// Source data
    const void* data = nullptr;
    
    /// Row pitch in bytes (0 = tightly packed)
    u32 rowPitch = 0;
    
    /// Slice pitch in bytes (0 = tightly packed, for 3D textures)
    u32 slicePitch = 0;
};

/**
 * @brief Texture view descriptor
 */
struct TextureViewDesc {
    /// Texture to create view for
    TextureHandle texture;
    
    /// View type (can differ from texture type)
    TextureType viewType = TextureType::Texture2D;
    
    /// View format (can differ for compatible formats)
    TextureFormat format = TextureFormat::Unknown; // Unknown = use texture format
    
    /// First mip level
    u32 baseMipLevel = 0;
    
    /// Number of mip levels
    u32 mipLevelCount = 1;
    
    /// First array layer
    u32 baseArrayLayer = 0;
    
    /// Number of array layers
    u32 arrayLayerCount = 1;
};

} // namespace nova::render
