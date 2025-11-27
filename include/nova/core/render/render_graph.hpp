/**
 * @file render_graph.hpp
 * @brief Production-grade render graph system for NovaCore engine
 * 
 * Implements a frame graph / render graph architecture for:
 * - Automatic resource management and aliasing
 * - Parallel command buffer recording
 * - Automatic synchronization and barriers
 * - Transient resource allocation
 * - Pass culling for unused resources
 * - Multi-queue execution planning
 * 
 * Following NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md Phase 1: Core Rendering
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/render/render_types.hpp"
#include "nova/core/render/buffer.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <bitset>
#include <optional>
#include <variant>

namespace nova::render {

// Forward declarations
class RenderGraph;
class RenderGraphBuilder;
class RenderGraphPass;
class RenderGraphResource;

// ============================================================================
// Render Graph Configuration
// ============================================================================

/**
 * @brief Render graph configuration constants
 */
struct RenderGraphConfig {
    /// Maximum number of passes per frame
    static constexpr u32 MAX_PASSES = 256;
    
    /// Maximum number of resources per frame
    static constexpr u32 MAX_RESOURCES = 512;
    
    /// Maximum number of dependencies per pass
    static constexpr u32 MAX_DEPENDENCIES = 64;
    
    /// Maximum number of render targets per pass
    static constexpr u32 MAX_RENDER_TARGETS = 8;
    
    /// Maximum transient buffer pool size (MB)
    static constexpr u32 TRANSIENT_BUFFER_POOL_SIZE_MB = 256;
    
    /// Maximum transient texture pool size (MB)
    static constexpr u32 TRANSIENT_TEXTURE_POOL_SIZE_MB = 512;
    
    /// Enable automatic barrier insertion
    static constexpr bool AUTO_BARRIERS = true;
    
    /// Enable pass culling for unused outputs
    static constexpr bool PASS_CULLING = true;
    
    /// Enable resource aliasing for memory reuse
    static constexpr bool RESOURCE_ALIASING = true;
};

// ============================================================================
// Resource Types
// ============================================================================

/**
 * @brief Render graph resource type
 */
enum class ResourceType : u8 {
    Buffer,         ///< GPU buffer resource
    Texture,        ///< GPU texture resource
    Imported        ///< Externally managed resource
};

/**
 * @brief Resource access flags
 */
enum class ResourceAccess : u16 {
    None            = 0,
    Read            = 1 << 0,   ///< Read access
    Write           = 1 << 1,   ///< Write access
    ReadWrite       = Read | Write,
    
    // Specific pipeline stages
    VertexShader    = 1 << 2,
    FragmentShader  = 1 << 3,
    ComputeShader   = 1 << 4,
    Transfer        = 1 << 5,
    
    // Render target usage
    ColorAttachment = 1 << 6,
    DepthAttachment = 1 << 7,
    InputAttachment = 1 << 8,
    
    // Buffer usage
    VertexBuffer    = 1 << 9,
    IndexBuffer     = 1 << 10,
    UniformBuffer   = 1 << 11,
    StorageBuffer   = 1 << 12,
    IndirectBuffer  = 1 << 13
};

/**
 * @brief Bitwise operators for ResourceAccess
 */
inline ResourceAccess operator|(ResourceAccess a, ResourceAccess b) {
    return static_cast<ResourceAccess>(static_cast<u16>(a) | static_cast<u16>(b));
}

inline ResourceAccess operator&(ResourceAccess a, ResourceAccess b) {
    return static_cast<ResourceAccess>(static_cast<u16>(a) & static_cast<u16>(b));
}

inline ResourceAccess& operator|=(ResourceAccess& a, ResourceAccess b) {
    a = a | b;
    return a;
}

inline bool hasAccess(ResourceAccess access, ResourceAccess flag) {
    return (static_cast<u16>(access) & static_cast<u16>(flag)) != 0;
}

/**
 * @brief Render graph resource handle
 */
struct RGResourceHandle {
    u32 index = ~0u;        ///< Resource index in the graph
    u32 version = 0;        ///< Resource version (for validation)
    
    [[nodiscard]] bool isValid() const { return index != ~0u; }
    
    bool operator==(const RGResourceHandle& other) const {
        return index == other.index && version == other.version;
    }
    
    bool operator!=(const RGResourceHandle& other) const {
        return !(*this == other);
    }
    
    static RGResourceHandle invalid() { return {}; }
};

/**
 * @brief Typed texture resource handle
 */
struct RGTextureHandle : RGResourceHandle {
    using RGResourceHandle::RGResourceHandle;
    
    static RGTextureHandle invalid() { return {}; }
};

/**
 * @brief Typed buffer resource handle
 */
struct RGBufferHandle : RGResourceHandle {
    using RGResourceHandle::RGResourceHandle;
    
    static RGBufferHandle invalid() { return {}; }
};

// ============================================================================
// Resource Descriptions
// ============================================================================

/**
 * @brief Texture resource description
 */
struct RGTextureDesc {
    std::string name;
    u32 width = 0;
    u32 height = 0;
    u32 depth = 1;
    u32 mipLevels = 1;
    u32 arrayLayers = 1;
    TextureFormat format = TextureFormat::RGBA8_UNORM;
    bool isRenderTarget = false;
    bool isDepthStencil = false;
    ClearColor clearColor{0.0f, 0.0f, 0.0f, 1.0f};
    ClearDepthStencil clearDepth{1.0f, 0};
    
    /**
     * @brief Create render target description
     */
    static RGTextureDesc renderTarget(const std::string& name, u32 w, u32 h, 
                                       TextureFormat fmt = TextureFormat::RGBA8_UNORM) {
        RGTextureDesc desc;
        desc.name = name;
        desc.width = w;
        desc.height = h;
        desc.format = fmt;
        desc.isRenderTarget = true;
        return desc;
    }
    
    /**
     * @brief Create depth buffer description
     */
    static RGTextureDesc depthBuffer(const std::string& name, u32 w, u32 h,
                                      TextureFormat fmt = TextureFormat::D32_FLOAT) {
        RGTextureDesc desc;
        desc.name = name;
        desc.width = w;
        desc.height = h;
        desc.format = fmt;
        desc.isRenderTarget = true;
        desc.isDepthStencil = true;
        return desc;
    }
    
    /**
     * @brief Create GBuffer texture description
     */
    static RGTextureDesc gBuffer(const std::string& name, u32 w, u32 h, TextureFormat fmt) {
        return renderTarget(name, w, h, fmt);
    }
    
    /**
     * @brief Compute size in bytes
     */
    [[nodiscard]] usize computeSizeBytes() const {
        u32 bpp = getFormatBytesPerPixel(format);
        if (bpp == 0) bpp = 4; // Default for compressed
        return static_cast<usize>(width) * height * depth * arrayLayers * bpp;
    }
};

/**
 * @brief Buffer resource description
 */
struct RGBufferDesc {
    std::string name;
    usize size = 0;
    BufferUsage usage = BufferUsage::Storage;
    bool cpuReadable = false;
    
    /**
     * @brief Create uniform buffer description
     */
    static RGBufferDesc uniform(const std::string& name, usize sz) {
        RGBufferDesc desc;
        desc.name = name;
        desc.size = sz;
        desc.usage = BufferUsage::Uniform;
        return desc;
    }
    
    /**
     * @brief Create storage buffer description
     */
    static RGBufferDesc storage(const std::string& name, usize sz) {
        RGBufferDesc desc;
        desc.name = name;
        desc.size = sz;
        desc.usage = BufferUsage::Storage;
        return desc;
    }
    
    /**
     * @brief Create indirect buffer description
     */
    static RGBufferDesc indirect(const std::string& name, usize sz) {
        RGBufferDesc desc;
        desc.name = name;
        desc.size = sz;
        desc.usage = BufferUsage::IndirectBuffer;
        return desc;
    }
};

// ============================================================================
// Pass Types
// ============================================================================

/**
 * @brief Render pass type
 */
enum class RGPassType : u8 {
    Graphics,       ///< Rasterization pass
    Compute,        ///< Compute shader pass
    Transfer,       ///< Copy/blit operations
    Present,        ///< Swap chain present
    AsyncCompute,   ///< Async compute queue
    RayTracing      ///< Ray tracing pass
};

/**
 * @brief Get pass type name
 */
constexpr const char* getPassTypeName(RGPassType type) {
    switch (type) {
        case RGPassType::Graphics: return "Graphics";
        case RGPassType::Compute: return "Compute";
        case RGPassType::Transfer: return "Transfer";
        case RGPassType::Present: return "Present";
        case RGPassType::AsyncCompute: return "AsyncCompute";
        case RGPassType::RayTracing: return "RayTracing";
    }
    return "Unknown";
}

/**
 * @brief Pass execution flags
 */
enum class RGPassFlags : u8 {
    None            = 0,
    NoCulling       = 1 << 0,   ///< Never cull this pass
    AsyncCapable    = 1 << 1,   ///< Can run on async compute
    ForceSerial     = 1 << 2,   ///< Must run serially
    SideEffects     = 1 << 3    ///< Has external side effects
};

inline RGPassFlags operator|(RGPassFlags a, RGPassFlags b) {
    return static_cast<RGPassFlags>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline bool hasFlag(RGPassFlags flags, RGPassFlags flag) {
    return (static_cast<u8>(flags) & static_cast<u8>(flag)) != 0;
}

// ============================================================================
// Resource Usage
// ============================================================================

/**
 * @brief Resource usage within a pass
 */
struct RGResourceUsage {
    RGResourceHandle handle;
    ResourceAccess access = ResourceAccess::None;
    u32 bindingIndex = 0;       ///< Shader binding slot
    u32 mipLevel = 0;           ///< Mip level for textures
    u32 arraySlice = 0;         ///< Array slice for texture arrays
};

// ============================================================================
// Render Graph Pass
// ============================================================================

/**
 * @brief Render graph pass data
 */
struct RGPassData {
    std::string name;
    RGPassType type = RGPassType::Graphics;
    RGPassFlags flags = RGPassFlags::None;
    
    // Resources
    std::vector<RGResourceUsage> reads;
    std::vector<RGResourceUsage> writes;
    
    // Render targets (for graphics passes)
    std::vector<RGTextureHandle> colorTargets;
    RGTextureHandle depthTarget;
    
    // Viewport/scissor
    Viewport viewport;
    Scissor scissor;
    
    // Execution order
    i32 executionOrder = -1;
    bool culled = false;
    
    // Dependency tracking
    std::vector<u32> dependencies;      ///< Pass indices this depends on
    std::vector<u32> dependents;        ///< Pass indices that depend on this
    
    /**
     * @brief Check if pass has color targets
     */
    [[nodiscard]] bool hasColorTargets() const { return !colorTargets.empty(); }
    
    /**
     * @brief Check if pass has depth target
     */
    [[nodiscard]] bool hasDepthTarget() const { return depthTarget.isValid(); }
    
    /**
     * @brief Check if pass is graphics type
     */
    [[nodiscard]] bool isGraphics() const { return type == RGPassType::Graphics; }
    
    /**
     * @brief Check if pass is compute type
     */
    [[nodiscard]] bool isCompute() const { 
        return type == RGPassType::Compute || type == RGPassType::AsyncCompute; 
    }
};

// ============================================================================
// Render Graph Builder
// ============================================================================

/**
 * @brief Builder for constructing render graph passes
 */
class RenderGraphBuilder {
public:
    /**
     * @brief Construct builder for a pass
     */
    RenderGraphBuilder(RenderGraph& graph, u32 passIndex);
    
    /**
     * @brief Create transient texture
     */
    [[nodiscard]] RGTextureHandle createTexture(const RGTextureDesc& desc);
    
    /**
     * @brief Create transient buffer
     */
    [[nodiscard]] RGBufferHandle createBuffer(const RGBufferDesc& desc);
    
    /**
     * @brief Read from texture
     */
    RGTextureHandle read(RGTextureHandle texture, ResourceAccess access = ResourceAccess::Read);
    
    /**
     * @brief Write to texture
     */
    RGTextureHandle write(RGTextureHandle texture, ResourceAccess access = ResourceAccess::Write);
    
    /**
     * @brief Read from buffer
     */
    RGBufferHandle read(RGBufferHandle buffer, ResourceAccess access = ResourceAccess::Read);
    
    /**
     * @brief Write to buffer
     */
    RGBufferHandle write(RGBufferHandle buffer, ResourceAccess access = ResourceAccess::Write);
    
    /**
     * @brief Set color render target
     */
    void setRenderTarget(u32 index, RGTextureHandle texture);
    
    /**
     * @brief Set depth render target
     */
    void setDepthTarget(RGTextureHandle texture);
    
    /**
     * @brief Set viewport
     */
    void setViewport(const Viewport& vp);
    
    /**
     * @brief Set scissor
     */
    void setScissor(const Scissor& sc);
    
    /**
     * @brief Set pass flags
     */
    void setFlags(RGPassFlags flags);
    
    /**
     * @brief Get the render graph
     */
    [[nodiscard]] RenderGraph& getGraph() { return m_graph; }
    
    /**
     * @brief Get pass index
     */
    [[nodiscard]] u32 getPassIndex() const { return m_passIndex; }

private:
    RenderGraph& m_graph;
    u32 m_passIndex;
};

// ============================================================================
// Pass Execution Context
// ============================================================================

/**
 * @brief Context passed to pass execution callbacks
 */
class RenderGraphContext {
public:
    /**
     * @brief Construct execution context
     */
    RenderGraphContext(RenderGraph& graph, const RGPassData& pass)
        : m_graph(graph), m_pass(pass) {}
    
    /**
     * @brief Get GPU texture for resource
     */
    [[nodiscard]] TextureHandle getTexture(RGTextureHandle handle) const;
    
    /**
     * @brief Get GPU buffer for resource
     */
    [[nodiscard]] BufferHandle getBuffer(RGBufferHandle handle) const;
    
    /**
     * @brief Get render graph
     */
    [[nodiscard]] const RenderGraph& getGraph() const { return m_graph; }
    
    /**
     * @brief Get pass data
     */
    [[nodiscard]] const RGPassData& getPass() const { return m_pass; }
    
    /**
     * @brief Get viewport
     */
    [[nodiscard]] const Viewport& getViewport() const { return m_pass.viewport; }
    
    /**
     * @brief Get scissor
     */
    [[nodiscard]] const Scissor& getScissor() const { return m_pass.scissor; }

private:
    RenderGraph& m_graph;
    const RGPassData& m_pass;
};

// ============================================================================
// Render Graph Resource Data
// ============================================================================

/**
 * @brief Internal resource data
 */
struct RGResourceData {
    std::string name;
    ResourceType type = ResourceType::Buffer;
    
    // Description (one of these is valid based on type)
    std::variant<RGTextureDesc, RGBufferDesc> desc;
    
    // Physical resource binding
    std::variant<TextureHandle, BufferHandle> physicalResource;
    
    // Lifetime tracking
    u32 firstPassUsage = ~0u;   ///< First pass that uses this
    u32 lastPassUsage = 0;      ///< Last pass that uses this
    
    // State tracking
    ResourceAccess currentState = ResourceAccess::None;
    bool isImported = false;    ///< External resource (not managed by graph)
    bool isTransient = true;    ///< Transient (can be aliased)
    u32 refCount = 0;           ///< Reference count
    
    /**
     * @brief Check if resource is texture
     */
    [[nodiscard]] bool isTexture() const { return type == ResourceType::Texture; }
    
    /**
     * @brief Check if resource is buffer
     */
    [[nodiscard]] bool isBuffer() const { return type == ResourceType::Buffer; }
    
    /**
     * @brief Get texture description
     */
    [[nodiscard]] const RGTextureDesc& getTextureDesc() const {
        return std::get<RGTextureDesc>(desc);
    }
    
    /**
     * @brief Get buffer description
     */
    [[nodiscard]] const RGBufferDesc& getBufferDesc() const {
        return std::get<RGBufferDesc>(desc);
    }
};

// ============================================================================
// Render Graph
// ============================================================================

/**
 * @brief Render graph for frame rendering
 * 
 * A render graph organizes rendering into passes with automatic:
 * - Resource lifetime management
 * - Memory aliasing
 * - Synchronization barriers
 * - Pass culling
 * - Parallel execution planning
 */
class RenderGraph {
public:
    /// Pass execution callback type
    using ExecuteCallback = std::function<void(RenderGraphContext&)>;
    
    /**
     * @brief Construct render graph
     */
    RenderGraph();
    
    /**
     * @brief Destructor
     */
    ~RenderGraph();
    
    // ========================================================================
    // Pass Creation
    // ========================================================================
    
    /**
     * @brief Add a graphics pass
     * @param name Pass name for debugging
     * @param setup Setup callback to configure the pass
     * @param execute Execution callback
     * @return Pass index
     */
    template<typename SetupFunc, typename ExecuteFunc>
    u32 addPass(const std::string& name, RGPassType type,
                SetupFunc&& setup, ExecuteFunc&& execute) {
        u32 passIndex = static_cast<u32>(m_passes.size());
        m_passes.emplace_back();
        m_passes.back().name = name;
        m_passes.back().type = type;
        
        RenderGraphBuilder builder(*this, passIndex);
        setup(builder);
        
        m_executeCallbacks.push_back(std::forward<ExecuteFunc>(execute));
        
        return passIndex;
    }
    
    /**
     * @brief Add a graphics pass (convenience)
     */
    template<typename SetupFunc, typename ExecuteFunc>
    u32 addGraphicsPass(const std::string& name, SetupFunc&& setup, ExecuteFunc&& execute) {
        return addPass(name, RGPassType::Graphics, 
                       std::forward<SetupFunc>(setup), 
                       std::forward<ExecuteFunc>(execute));
    }
    
    /**
     * @brief Add a compute pass
     */
    template<typename SetupFunc, typename ExecuteFunc>
    u32 addComputePass(const std::string& name, SetupFunc&& setup, ExecuteFunc&& execute) {
        return addPass(name, RGPassType::Compute,
                       std::forward<SetupFunc>(setup),
                       std::forward<ExecuteFunc>(execute));
    }
    
    /**
     * @brief Add a transfer pass
     */
    template<typename SetupFunc, typename ExecuteFunc>
    u32 addTransferPass(const std::string& name, SetupFunc&& setup, ExecuteFunc&& execute) {
        return addPass(name, RGPassType::Transfer,
                       std::forward<SetupFunc>(setup),
                       std::forward<ExecuteFunc>(execute));
    }
    
    // ========================================================================
    // Resource Management
    // ========================================================================
    
    /**
     * @brief Import external texture
     */
    [[nodiscard]] RGTextureHandle importTexture(const std::string& name, TextureHandle handle,
                                                 const RGTextureDesc& desc);
    
    /**
     * @brief Import external buffer
     */
    [[nodiscard]] RGBufferHandle importBuffer(const std::string& name, BufferHandle handle,
                                               const RGBufferDesc& desc);
    
    /**
     * @brief Import swap chain back buffer
     */
    [[nodiscard]] RGTextureHandle importBackBuffer(TextureHandle handle, u32 width, u32 height);
    
    /**
     * @brief Create transient texture
     */
    [[nodiscard]] RGTextureHandle createTexture(const RGTextureDesc& desc);
    
    /**
     * @brief Create transient buffer
     */
    [[nodiscard]] RGBufferHandle createBuffer(const RGBufferDesc& desc);
    
    /**
     * @brief Get resource data
     */
    [[nodiscard]] RGResourceData& getResource(RGResourceHandle handle);
    [[nodiscard]] const RGResourceData& getResource(RGResourceHandle handle) const;
    
    /**
     * @brief Get pass data
     */
    [[nodiscard]] RGPassData& getPass(u32 index);
    [[nodiscard]] const RGPassData& getPass(u32 index) const;
    
    // ========================================================================
    // Compilation and Execution
    // ========================================================================
    
    /**
     * @brief Compile the render graph
     * 
     * Performs:
     * 1. Topological sort of passes
     * 2. Dead pass elimination
     * 3. Resource lifetime analysis
     * 4. Memory aliasing
     * 5. Barrier computation
     */
    void compile();
    
    /**
     * @brief Execute all passes
     */
    void execute();
    
    /**
     * @brief Reset for next frame
     */
    void reset();
    
    // ========================================================================
    // Query Methods
    // ========================================================================
    
    /**
     * @brief Get number of passes
     */
    [[nodiscard]] usize getPassCount() const { return m_passes.size(); }
    
    /**
     * @brief Get number of resources
     */
    [[nodiscard]] usize getResourceCount() const { return m_resources.size(); }
    
    /**
     * @brief Get number of active (non-culled) passes
     */
    [[nodiscard]] usize getActivePassCount() const;
    
    /**
     * @brief Check if graph is compiled
     */
    [[nodiscard]] bool isCompiled() const { return m_compiled; }
    
    /**
     * @brief Get execution order
     */
    [[nodiscard]] const std::vector<u32>& getExecutionOrder() const { return m_executionOrder; }
    
    /**
     * @brief Get back buffer handle
     */
    [[nodiscard]] RGTextureHandle getBackBuffer() const { return m_backBuffer; }
    
    // ========================================================================
    // Debug
    // ========================================================================
    
    /**
     * @brief Dump graph to string for debugging
     */
    [[nodiscard]] std::string dump() const;
    
    /**
     * @brief Export to GraphViz DOT format
     */
    [[nodiscard]] std::string exportGraphViz() const;

private:
    // Internal compilation steps
    void buildDependencies();
    void topologicalSort();
    void cullUnusedPasses();
    void computeResourceLifetimes();
    void allocateResources();
    void computeBarriers();
    
    // Helper methods
    void markPassAsUsed(u32 passIndex);
    bool isResourceUsedAfter(RGResourceHandle handle, u32 passIndex) const;
    
    // Data
    std::vector<RGPassData> m_passes;
    std::vector<RGResourceData> m_resources;
    std::vector<ExecuteCallback> m_executeCallbacks;
    std::vector<u32> m_executionOrder;
    
    // Frame state
    RGTextureHandle m_backBuffer;
    bool m_compiled = false;
    u32 m_resourceVersion = 0;
};

// ============================================================================
// Render Graph Builder Implementation
// ============================================================================

inline RenderGraphBuilder::RenderGraphBuilder(RenderGraph& graph, u32 passIndex)
    : m_graph(graph), m_passIndex(passIndex) {}

inline RGTextureHandle RenderGraphBuilder::createTexture(const RGTextureDesc& desc) {
    return m_graph.createTexture(desc);
}

inline RGBufferHandle RenderGraphBuilder::createBuffer(const RGBufferDesc& desc) {
    return m_graph.createBuffer(desc);
}

inline RGTextureHandle RenderGraphBuilder::read(RGTextureHandle texture, ResourceAccess access) {
    auto& pass = m_graph.getPass(m_passIndex);
    RGResourceUsage usage;
    usage.handle = texture;
    usage.access = access | ResourceAccess::Read;
    pass.reads.push_back(usage);
    
    // Update resource lifetime
    auto& res = m_graph.getResource(texture);
    if (res.firstPassUsage > m_passIndex) res.firstPassUsage = m_passIndex;
    if (res.lastPassUsage < m_passIndex) res.lastPassUsage = m_passIndex;
    res.refCount++;
    
    return texture;
}

inline RGTextureHandle RenderGraphBuilder::write(RGTextureHandle texture, ResourceAccess access) {
    auto& pass = m_graph.getPass(m_passIndex);
    RGResourceUsage usage;
    usage.handle = texture;
    usage.access = access | ResourceAccess::Write;
    pass.writes.push_back(usage);
    
    // Update resource lifetime
    auto& res = m_graph.getResource(texture);
    if (res.firstPassUsage > m_passIndex) res.firstPassUsage = m_passIndex;
    if (res.lastPassUsage < m_passIndex) res.lastPassUsage = m_passIndex;
    res.refCount++;
    
    return texture;
}

inline RGBufferHandle RenderGraphBuilder::read(RGBufferHandle buffer, ResourceAccess access) {
    auto& pass = m_graph.getPass(m_passIndex);
    RGResourceUsage usage;
    usage.handle = buffer;
    usage.access = access | ResourceAccess::Read;
    pass.reads.push_back(usage);
    
    // Update resource lifetime
    auto& res = m_graph.getResource(buffer);
    if (res.firstPassUsage > m_passIndex) res.firstPassUsage = m_passIndex;
    if (res.lastPassUsage < m_passIndex) res.lastPassUsage = m_passIndex;
    res.refCount++;
    
    return buffer;
}

inline RGBufferHandle RenderGraphBuilder::write(RGBufferHandle buffer, ResourceAccess access) {
    auto& pass = m_graph.getPass(m_passIndex);
    RGResourceUsage usage;
    usage.handle = buffer;
    usage.access = access | ResourceAccess::Write;
    pass.writes.push_back(usage);
    
    // Update resource lifetime
    auto& res = m_graph.getResource(buffer);
    if (res.firstPassUsage > m_passIndex) res.firstPassUsage = m_passIndex;
    if (res.lastPassUsage < m_passIndex) res.lastPassUsage = m_passIndex;
    res.refCount++;
    
    return buffer;
}

inline void RenderGraphBuilder::setRenderTarget(u32 index, RGTextureHandle texture) {
    auto& pass = m_graph.getPass(m_passIndex);
    if (pass.colorTargets.size() <= index) {
        pass.colorTargets.resize(index + 1);
    }
    pass.colorTargets[index] = texture;
    
    // Mark as write (color attachment)
    write(texture, ResourceAccess::ColorAttachment);
}

inline void RenderGraphBuilder::setDepthTarget(RGTextureHandle texture) {
    auto& pass = m_graph.getPass(m_passIndex);
    pass.depthTarget = texture;
    
    // Mark as write (depth attachment)
    write(texture, ResourceAccess::DepthAttachment);
}

inline void RenderGraphBuilder::setViewport(const Viewport& vp) {
    m_graph.getPass(m_passIndex).viewport = vp;
}

inline void RenderGraphBuilder::setScissor(const Scissor& sc) {
    m_graph.getPass(m_passIndex).scissor = sc;
}

inline void RenderGraphBuilder::setFlags(RGPassFlags flags) {
    m_graph.getPass(m_passIndex).flags = flags;
}

// ============================================================================
// Render Graph Context Implementation
// ============================================================================

inline TextureHandle RenderGraphContext::getTexture(RGTextureHandle handle) const {
    const auto& res = m_graph.getResource(handle);
    return std::get<TextureHandle>(res.physicalResource);
}

inline BufferHandle RenderGraphContext::getBuffer(RGBufferHandle handle) const {
    const auto& res = m_graph.getResource(handle);
    return std::get<BufferHandle>(res.physicalResource);
}

// ============================================================================
// Common Render Graph Patterns
// ============================================================================

/**
 * @brief Helper for creating GBuffer pass
 */
struct GBufferSetup {
    RGTextureHandle albedo;
    RGTextureHandle normal;
    RGTextureHandle material;  ///< Metallic/roughness
    RGTextureHandle depth;
    
    /**
     * @brief Create GBuffer textures
     */
    static GBufferSetup create(RenderGraphBuilder& builder, u32 width, u32 height) {
        GBufferSetup setup;
        
        setup.albedo = builder.createTexture(
            RGTextureDesc::gBuffer("GBuffer_Albedo", width, height, TextureFormat::RGBA8_SRGB));
        setup.normal = builder.createTexture(
            RGTextureDesc::gBuffer("GBuffer_Normal", width, height, TextureFormat::RGB10A2_UNORM));
        setup.material = builder.createTexture(
            RGTextureDesc::gBuffer("GBuffer_Material", width, height, TextureFormat::RGBA8_UNORM));
        setup.depth = builder.createTexture(
            RGTextureDesc::depthBuffer("GBuffer_Depth", width, height));
        
        builder.setRenderTarget(0, setup.albedo);
        builder.setRenderTarget(1, setup.normal);
        builder.setRenderTarget(2, setup.material);
        builder.setDepthTarget(setup.depth);
        
        return setup;
    }
};

/**
 * @brief Helper for shadow map pass
 */
struct ShadowMapSetup {
    RGTextureHandle shadowMap;
    u32 resolution;
    
    /**
     * @brief Create shadow map texture
     */
    static ShadowMapSetup create(RenderGraphBuilder& builder, u32 res = 2048) {
        ShadowMapSetup setup;
        setup.resolution = res;
        setup.shadowMap = builder.createTexture(
            RGTextureDesc::depthBuffer("ShadowMap", res, res, TextureFormat::D32_FLOAT));
        builder.setDepthTarget(setup.shadowMap);
        return setup;
    }
};

/**
 * @brief Helper for post-process pass
 */
struct PostProcessSetup {
    RGTextureHandle input;
    RGTextureHandle output;
    
    /**
     * @brief Setup post-process pass
     */
    static PostProcessSetup create(RenderGraphBuilder& builder, 
                                    RGTextureHandle inputTex,
                                    const std::string& outputName,
                                    u32 width, u32 height) {
        PostProcessSetup setup;
        setup.input = builder.read(inputTex, ResourceAccess::FragmentShader);
        setup.output = builder.createTexture(
            RGTextureDesc::renderTarget(outputName, width, height));
        builder.setRenderTarget(0, setup.output);
        return setup;
    }
};

} // namespace nova::render
