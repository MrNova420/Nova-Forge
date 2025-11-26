/**
 * @file render_pass.hpp
 * @brief Render pass definitions for NovaCore rendering
 * 
 * Render passes define the structure of rendering operations,
 * including attachments, subpasses, and dependencies.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include <vector>
#include <string>

namespace nova::render {

/**
 * @brief Attachment load operation
 */
enum class LoadOp : u8 {
    Load,       ///< Load previous contents
    Clear,      ///< Clear to specified value
    DontCare    ///< Contents undefined (for performance)
};

/**
 * @brief Attachment store operation
 */
enum class StoreOp : u8 {
    Store,      ///< Store results
    DontCare    ///< Contents can be discarded (for performance)
};

/**
 * @brief Image layout
 */
enum class ImageLayout : u8 {
    Undefined,
    General,
    ColorAttachment,
    DepthStencilAttachment,
    DepthStencilReadOnly,
    ShaderReadOnly,
    TransferSrc,
    TransferDst,
    Present
};

/**
 * @brief Render pass attachment description
 */
struct AttachmentDesc {
    /// Attachment format
    TextureFormat format = TextureFormat::RGBA8_UNORM;
    
    /// Number of samples (1 = no MSAA)
    u32 samples = 1;
    
    /// Load operation at render pass begin
    LoadOp loadOp = LoadOp::Clear;
    
    /// Store operation at render pass end
    StoreOp storeOp = StoreOp::Store;
    
    /// Stencil load operation
    LoadOp stencilLoadOp = LoadOp::DontCare;
    
    /// Stencil store operation
    StoreOp stencilStoreOp = StoreOp::DontCare;
    
    /// Initial image layout
    ImageLayout initialLayout = ImageLayout::Undefined;
    
    /// Final image layout
    ImageLayout finalLayout = ImageLayout::ColorAttachment;
    
    // Factory methods
    static AttachmentDesc color(TextureFormat format = TextureFormat::RGBA8_SRGB,
                                LoadOp load = LoadOp::Clear,
                                StoreOp store = StoreOp::Store) {
        AttachmentDesc desc;
        desc.format = format;
        desc.loadOp = load;
        desc.storeOp = store;
        desc.finalLayout = ImageLayout::ColorAttachment;
        return desc;
    }
    
    static AttachmentDesc depth(TextureFormat format = TextureFormat::D32_FLOAT,
                                LoadOp load = LoadOp::Clear,
                                StoreOp store = StoreOp::DontCare) {
        AttachmentDesc desc;
        desc.format = format;
        desc.loadOp = load;
        desc.storeOp = store;
        desc.finalLayout = ImageLayout::DepthStencilAttachment;
        return desc;
    }
    
    static AttachmentDesc present(TextureFormat format = TextureFormat::BGRA8_SRGB) {
        AttachmentDesc desc;
        desc.format = format;
        desc.loadOp = LoadOp::Clear;
        desc.storeOp = StoreOp::Store;
        desc.finalLayout = ImageLayout::Present;
        return desc;
    }
};

/**
 * @brief Attachment reference in a subpass
 */
struct AttachmentReference {
    u32 attachment = 0;                     ///< Attachment index
    ImageLayout layout = ImageLayout::ColorAttachment;
    
    AttachmentReference() = default;
    AttachmentReference(u32 idx, ImageLayout imageLayout = ImageLayout::ColorAttachment)
        : attachment(idx), layout(imageLayout) {}
    
    static constexpr u32 UNUSED = ~0u;
};

/**
 * @brief Subpass description
 */
struct SubpassDesc {
    /// Color attachments used by this subpass
    std::vector<AttachmentReference> colorAttachments;
    
    /// Input attachments (from previous subpass)
    std::vector<AttachmentReference> inputAttachments;
    
    /// Resolve attachments (for MSAA resolve)
    std::vector<AttachmentReference> resolveAttachments;
    
    /// Depth/stencil attachment
    AttachmentReference depthStencilAttachment = {AttachmentReference::UNUSED};
    
    /// Attachments to preserve (not used but contents must be preserved)
    std::vector<u32> preserveAttachments;
};

/**
 * @brief Pipeline stage flags for subpass dependencies
 */
enum class PipelineStage : u32 {
    TopOfPipe = 1 << 0,
    DrawIndirect = 1 << 1,
    VertexInput = 1 << 2,
    VertexShader = 1 << 3,
    TessellationControl = 1 << 4,
    TessellationEvaluation = 1 << 5,
    GeometryShader = 1 << 6,
    FragmentShader = 1 << 7,
    EarlyFragmentTests = 1 << 8,
    LateFragmentTests = 1 << 9,
    ColorAttachmentOutput = 1 << 10,
    ComputeShader = 1 << 11,
    Transfer = 1 << 12,
    BottomOfPipe = 1 << 13,
    Host = 1 << 14,
    AllGraphics = 1 << 15,
    AllCommands = 1 << 16
};

inline PipelineStage operator|(PipelineStage a, PipelineStage b) {
    return static_cast<PipelineStage>(static_cast<u32>(a) | static_cast<u32>(b));
}

/**
 * @brief Memory access flags
 */
enum class Access : u32 {
    None = 0,
    IndirectCommandRead = 1 << 0,
    IndexRead = 1 << 1,
    VertexAttributeRead = 1 << 2,
    UniformRead = 1 << 3,
    InputAttachmentRead = 1 << 4,
    ShaderRead = 1 << 5,
    ShaderWrite = 1 << 6,
    ColorAttachmentRead = 1 << 7,
    ColorAttachmentWrite = 1 << 8,
    DepthStencilAttachmentRead = 1 << 9,
    DepthStencilAttachmentWrite = 1 << 10,
    TransferRead = 1 << 11,
    TransferWrite = 1 << 12,
    HostRead = 1 << 13,
    HostWrite = 1 << 14,
    MemoryRead = 1 << 15,
    MemoryWrite = 1 << 16
};

inline Access operator|(Access a, Access b) {
    return static_cast<Access>(static_cast<u32>(a) | static_cast<u32>(b));
}

/**
 * @brief Subpass dependency
 */
struct SubpassDependency {
    u32 srcSubpass = 0;                 ///< Source subpass (EXTERNAL = before render pass)
    u32 dstSubpass = 0;                 ///< Destination subpass (EXTERNAL = after render pass)
    PipelineStage srcStageMask = PipelineStage::AllGraphics;
    PipelineStage dstStageMask = PipelineStage::AllGraphics;
    Access srcAccessMask = Access::None;
    Access dstAccessMask = Access::None;
    bool byRegion = false;
    
    static constexpr u32 EXTERNAL = ~0u;
    
    /// Create external-to-first dependency
    static SubpassDependency externalToFirst() {
        SubpassDependency dep;
        dep.srcSubpass = EXTERNAL;
        dep.dstSubpass = 0;
        dep.srcStageMask = PipelineStage::ColorAttachmentOutput;
        dep.dstStageMask = PipelineStage::ColorAttachmentOutput;
        dep.srcAccessMask = Access::None;
        dep.dstAccessMask = Access::ColorAttachmentWrite;
        return dep;
    }
    
    /// Create last-to-external dependency
    static SubpassDependency lastToExternal(u32 lastSubpass = 0) {
        SubpassDependency dep;
        dep.srcSubpass = lastSubpass;
        dep.dstSubpass = EXTERNAL;
        dep.srcStageMask = PipelineStage::ColorAttachmentOutput;
        dep.dstStageMask = PipelineStage::BottomOfPipe;
        dep.srcAccessMask = Access::ColorAttachmentWrite;
        dep.dstAccessMask = Access::None;
        return dep;
    }
};

/**
 * @brief Render pass creation descriptor
 */
struct RenderPassDesc {
    /// Debug name
    std::string name;
    
    /// Attachment descriptions
    std::vector<AttachmentDesc> attachments;
    
    /// Subpass descriptions
    std::vector<SubpassDesc> subpasses;
    
    /// Subpass dependencies
    std::vector<SubpassDependency> dependencies;
    
    /**
     * @brief Create a simple render pass with one color attachment
     */
    static RenderPassDesc simple(TextureFormat colorFormat = TextureFormat::BGRA8_SRGB,
                                 TextureFormat depthFormat = TextureFormat::D32_FLOAT) {
        RenderPassDesc desc;
        desc.name = "SimpleRenderPass";
        
        // Color attachment
        desc.attachments.push_back(AttachmentDesc::present(colorFormat));
        
        // Depth attachment (if specified)
        if (depthFormat != TextureFormat::Unknown) {
            desc.attachments.push_back(AttachmentDesc::depth(depthFormat));
        }
        
        // Single subpass
        SubpassDesc subpass;
        subpass.colorAttachments.push_back({0, ImageLayout::ColorAttachment});
        if (depthFormat != TextureFormat::Unknown) {
            subpass.depthStencilAttachment = {1, ImageLayout::DepthStencilAttachment};
        }
        desc.subpasses.push_back(std::move(subpass));
        
        // Dependencies
        desc.dependencies.push_back(SubpassDependency::externalToFirst());
        desc.dependencies.push_back(SubpassDependency::lastToExternal(0));
        
        return desc;
    }
};

/**
 * @brief Framebuffer creation descriptor
 */
struct FramebufferDesc {
    /// Render pass this framebuffer is compatible with
    RenderPassHandle renderPass;
    
    /// Attachment textures
    std::vector<TextureHandle> attachments;
    
    /// Framebuffer width
    u32 width = 0;
    
    /// Framebuffer height
    u32 height = 0;
    
    /// Number of layers (for multi-view rendering)
    u32 layers = 1;
    
    /// Debug name
    std::string name;
};

/**
 * @brief Render pass begin info
 */
struct RenderPassBeginInfo {
    /// Render pass to begin
    RenderPassHandle renderPass;
    
    /// Framebuffer to render to
    FramebufferHandle framebuffer;
    
    /// Render area offset
    i32 renderAreaX = 0;
    i32 renderAreaY = 0;
    
    /// Render area extent
    u32 renderAreaWidth = 0;
    u32 renderAreaHeight = 0;
    
    /// Clear values for each attachment
    std::vector<ClearColor> clearColors;
    
    /// Depth/stencil clear value
    ClearDepthStencil clearDepthStencil;
};

} // namespace nova::render
