/**
 * @file render_graph.cpp
 * @brief Render graph system implementation
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/render_graph.hpp"
#include <algorithm>
#include <queue>
#include <sstream>
#include <stack>

namespace nova::render {

// ============================================================================
// RenderGraph Implementation
// ============================================================================

RenderGraph::RenderGraph() {
    m_passes.reserve(RenderGraphConfig::MAX_PASSES);
    m_resources.reserve(RenderGraphConfig::MAX_RESOURCES);
    m_executeCallbacks.reserve(RenderGraphConfig::MAX_PASSES);
}

RenderGraph::~RenderGraph() = default;

// ============================================================================
// Resource Management
// ============================================================================

RGTextureHandle RenderGraph::importTexture(const std::string& name, TextureHandle handle,
                                            const RGTextureDesc& desc) {
    RGResourceData data;
    data.name = name;
    data.type = ResourceType::Imported;
    data.desc = desc;
    data.physicalResource = handle;
    data.isImported = true;
    data.isTransient = false;
    
    RGTextureHandle result;
    result.index = static_cast<u32>(m_resources.size());
    result.version = ++m_resourceVersion;
    
    m_resources.push_back(std::move(data));
    return result;
}

RGBufferHandle RenderGraph::importBuffer(const std::string& name, BufferHandle handle,
                                          const RGBufferDesc& desc) {
    RGResourceData data;
    data.name = name;
    data.type = ResourceType::Imported;
    data.desc = desc;
    data.physicalResource = handle;
    data.isImported = true;
    data.isTransient = false;
    
    RGBufferHandle result;
    result.index = static_cast<u32>(m_resources.size());
    result.version = ++m_resourceVersion;
    
    m_resources.push_back(std::move(data));
    return result;
}

RGTextureHandle RenderGraph::importBackBuffer(TextureHandle handle, u32 width, u32 height) {
    RGTextureDesc desc;
    desc.name = "BackBuffer";
    desc.width = width;
    desc.height = height;
    desc.format = TextureFormat::BGRA8_SRGB;
    desc.isRenderTarget = true;
    
    m_backBuffer = importTexture("BackBuffer", handle, desc);
    return m_backBuffer;
}

RGTextureHandle RenderGraph::createTexture(const RGTextureDesc& desc) {
    RGResourceData data;
    data.name = desc.name;
    data.type = ResourceType::Texture;
    data.desc = desc;
    data.isImported = false;
    data.isTransient = true;
    
    RGTextureHandle result;
    result.index = static_cast<u32>(m_resources.size());
    result.version = ++m_resourceVersion;
    
    m_resources.push_back(std::move(data));
    return result;
}

RGBufferHandle RenderGraph::createBuffer(const RGBufferDesc& desc) {
    RGResourceData data;
    data.name = desc.name;
    data.type = ResourceType::Buffer;
    data.desc = desc;
    data.isImported = false;
    data.isTransient = true;
    
    RGBufferHandle result;
    result.index = static_cast<u32>(m_resources.size());
    result.version = ++m_resourceVersion;
    
    m_resources.push_back(std::move(data));
    return result;
}

RGResourceData& RenderGraph::getResource(RGResourceHandle handle) {
    return m_resources[handle.index];
}

const RGResourceData& RenderGraph::getResource(RGResourceHandle handle) const {
    return m_resources[handle.index];
}

RGPassData& RenderGraph::getPass(u32 index) {
    return m_passes[index];
}

const RGPassData& RenderGraph::getPass(u32 index) const {
    return m_passes[index];
}

// ============================================================================
// Compilation
// ============================================================================

void RenderGraph::compile() {
    if (m_compiled) return;
    
    // Step 1: Build dependency graph
    buildDependencies();
    
    // Step 2: Topological sort for execution order
    topologicalSort();
    
    // Step 3: Cull unused passes (if enabled)
    if (RenderGraphConfig::PASS_CULLING) {
        cullUnusedPasses();
    }
    
    // Step 4: Compute resource lifetimes
    computeResourceLifetimes();
    
    // Step 5: Allocate physical resources (with aliasing if enabled)
    allocateResources();
    
    // Step 6: Compute barriers for synchronization
    if (RenderGraphConfig::AUTO_BARRIERS) {
        computeBarriers();
    }
    
    m_compiled = true;
}

void RenderGraph::buildDependencies() {
    // For each pass, find dependencies based on resource reads/writes
    for (u32 passIdx = 0; passIdx < m_passes.size(); ++passIdx) {
        auto& pass = m_passes[passIdx];
        
        // Check what this pass reads
        for (const auto& read : pass.reads) {
            // Find which pass wrote this resource
            for (u32 otherIdx = 0; otherIdx < passIdx; ++otherIdx) {
                const auto& otherPass = m_passes[otherIdx];
                for (const auto& write : otherPass.writes) {
                    if (write.handle == read.handle) {
                        // Found a dependency: passIdx depends on otherIdx
                        if (std::find(pass.dependencies.begin(), 
                                      pass.dependencies.end(), 
                                      otherIdx) == pass.dependencies.end()) {
                            pass.dependencies.push_back(otherIdx);
                            m_passes[otherIdx].dependents.push_back(passIdx);
                        }
                    }
                }
            }
        }
    }
}

void RenderGraph::topologicalSort() {
    m_executionOrder.clear();
    m_executionOrder.reserve(m_passes.size());
    
    // Kahn's algorithm for topological sort
    std::vector<u32> inDegree(m_passes.size(), 0);
    
    // Count incoming edges
    for (u32 i = 0; i < m_passes.size(); ++i) {
        inDegree[i] = static_cast<u32>(m_passes[i].dependencies.size());
    }
    
    // Start with passes that have no dependencies
    std::queue<u32> ready;
    for (u32 i = 0; i < m_passes.size(); ++i) {
        if (inDegree[i] == 0) {
            ready.push(i);
        }
    }
    
    // Process passes in topological order
    i32 executionOrder = 0;
    while (!ready.empty()) {
        u32 passIdx = ready.front();
        ready.pop();
        
        m_executionOrder.push_back(passIdx);
        m_passes[passIdx].executionOrder = executionOrder++;
        
        // Reduce in-degree for dependents
        for (u32 dependent : m_passes[passIdx].dependents) {
            inDegree[dependent]--;
            if (inDegree[dependent] == 0) {
                ready.push(dependent);
            }
        }
    }
    
    // Check for cycles (shouldn't happen in a well-formed graph)
    if (m_executionOrder.size() != m_passes.size()) {
        // Cycle detected - add remaining passes anyway
        for (u32 i = 0; i < m_passes.size(); ++i) {
            if (m_passes[i].executionOrder < 0) {
                m_executionOrder.push_back(i);
                m_passes[i].executionOrder = executionOrder++;
            }
        }
    }
}

void RenderGraph::cullUnusedPasses() {
    // Mark all passes as potentially cullable
    for (auto& pass : m_passes) {
        pass.culled = !hasFlag(pass.flags, RGPassFlags::NoCulling) &&
                      !hasFlag(pass.flags, RGPassFlags::SideEffects);
    }
    
    // Start from passes that write to the back buffer or have side effects
    std::stack<u32> toVisit;
    for (u32 i = 0; i < m_passes.size(); ++i) {
        const auto& pass = m_passes[i];
        
        // Check if this pass writes to the back buffer
        bool writesToBackBuffer = false;
        for (const auto& write : pass.writes) {
            if (write.handle.index == m_backBuffer.index) {
                writesToBackBuffer = true;
                break;
            }
        }
        for (const auto& rt : pass.colorTargets) {
            if (rt.index == m_backBuffer.index) {
                writesToBackBuffer = true;
                break;
            }
        }
        
        if (writesToBackBuffer || hasFlag(pass.flags, RGPassFlags::NoCulling) ||
            hasFlag(pass.flags, RGPassFlags::SideEffects)) {
            markPassAsUsed(i);
            toVisit.push(i);
        }
    }
    
    // Propagate backwards through dependencies
    while (!toVisit.empty()) {
        u32 passIdx = toVisit.top();
        toVisit.pop();
        
        for (u32 depIdx : m_passes[passIdx].dependencies) {
            if (m_passes[depIdx].culled) {
                markPassAsUsed(depIdx);
                toVisit.push(depIdx);
            }
        }
    }
    
    // Remove culled passes from execution order
    m_executionOrder.erase(
        std::remove_if(m_executionOrder.begin(), m_executionOrder.end(),
                       [this](u32 idx) { return m_passes[idx].culled; }),
        m_executionOrder.end());
}

void RenderGraph::markPassAsUsed(u32 passIndex) {
    m_passes[passIndex].culled = false;
}

void RenderGraph::computeResourceLifetimes() {
    // Reset lifetimes
    for (auto& res : m_resources) {
        res.firstPassUsage = ~0u;
        res.lastPassUsage = 0;
    }
    
    // Scan execution order to determine actual lifetimes
    for (usize orderIdx = 0; orderIdx < m_executionOrder.size(); ++orderIdx) {
        u32 passIdx = m_executionOrder[orderIdx];
        const auto& pass = m_passes[passIdx];
        
        // Update lifetimes for read resources
        for (const auto& usage : pass.reads) {
            auto& res = m_resources[usage.handle.index];
            if (res.firstPassUsage > static_cast<u32>(orderIdx)) {
                res.firstPassUsage = static_cast<u32>(orderIdx);
            }
            if (res.lastPassUsage < static_cast<u32>(orderIdx)) {
                res.lastPassUsage = static_cast<u32>(orderIdx);
            }
        }
        
        // Update lifetimes for write resources
        for (const auto& usage : pass.writes) {
            auto& res = m_resources[usage.handle.index];
            if (res.firstPassUsage > static_cast<u32>(orderIdx)) {
                res.firstPassUsage = static_cast<u32>(orderIdx);
            }
            if (res.lastPassUsage < static_cast<u32>(orderIdx)) {
                res.lastPassUsage = static_cast<u32>(orderIdx);
            }
        }
    }
}

void RenderGraph::allocateResources() {
    // For now, allocate each transient resource individually
    // Future: implement memory aliasing based on lifetimes
    
    for (auto& res : m_resources) {
        if (res.isImported) continue;  // Already has physical resource
        
        // Create physical resource based on type
        if (res.isTexture()) {
            // In a real implementation, this would call the render device
            // to allocate the actual GPU texture
            res.physicalResource = TextureHandle(res.firstPassUsage + 1);
        } else if (res.isBuffer()) {
            res.physicalResource = BufferHandle(res.firstPassUsage + 1);
        }
    }
}

void RenderGraph::computeBarriers() {
    // Track resource states and insert barriers as needed
    // This is a simplified implementation - a full implementation would
    // track pipeline stages and access masks more precisely
    
    for (u32 orderIdx = 0; orderIdx < m_executionOrder.size(); ++orderIdx) {
        u32 passIdx = m_executionOrder[orderIdx];
        const auto& pass = m_passes[passIdx];
        
        // Check reads - need barrier if previous state was write
        for (const auto& usage : pass.reads) {
            auto& res = m_resources[usage.handle.index];
            if (hasAccess(res.currentState, ResourceAccess::Write)) {
                // Would insert read barrier here
            }
            res.currentState = usage.access;
        }
        
        // Check writes
        for (const auto& usage : pass.writes) {
            auto& res = m_resources[usage.handle.index];
            // Would insert write barrier here if needed
            res.currentState = usage.access;
        }
    }
}

bool RenderGraph::isResourceUsedAfter(RGResourceHandle handle, u32 passIndex) const {
    const auto& res = m_resources[handle.index];
    return res.lastPassUsage > passIndex;
}

// ============================================================================
// Execution
// ============================================================================

void RenderGraph::execute() {
    if (!m_compiled) {
        compile();
    }
    
    // Execute passes in order
    for (u32 orderIdx : m_executionOrder) {
        const auto& pass = m_passes[orderIdx];
        
        if (pass.culled) continue;
        
        // Create execution context
        RenderGraphContext ctx(*this, pass);
        
        // Execute the pass
        if (orderIdx < m_executeCallbacks.size() && m_executeCallbacks[orderIdx]) {
            m_executeCallbacks[orderIdx](ctx);
        }
    }
}

void RenderGraph::reset() {
    m_passes.clear();
    m_resources.clear();
    m_executeCallbacks.clear();
    m_executionOrder.clear();
    m_backBuffer = RGTextureHandle::invalid();
    m_compiled = false;
}

// ============================================================================
// Query Methods
// ============================================================================

usize RenderGraph::getActivePassCount() const {
    usize count = 0;
    for (const auto& pass : m_passes) {
        if (!pass.culled) count++;
    }
    return count;
}

// ============================================================================
// Debug Output
// ============================================================================

std::string RenderGraph::dump() const {
    std::ostringstream ss;
    
    ss << "=== Render Graph ===\n";
    ss << "Passes: " << m_passes.size() << " (" << getActivePassCount() << " active)\n";
    ss << "Resources: " << m_resources.size() << "\n\n";
    
    ss << "--- Passes ---\n";
    for (u32 i = 0; i < m_passes.size(); ++i) {
        const auto& pass = m_passes[i];
        ss << "[" << i << "] " << pass.name;
        ss << " (" << getPassTypeName(pass.type) << ")";
        if (pass.culled) ss << " [CULLED]";
        ss << "\n";
        
        if (!pass.reads.empty()) {
            ss << "  Reads: ";
            for (const auto& r : pass.reads) {
                ss << m_resources[r.handle.index].name << " ";
            }
            ss << "\n";
        }
        
        if (!pass.writes.empty()) {
            ss << "  Writes: ";
            for (const auto& w : pass.writes) {
                ss << m_resources[w.handle.index].name << " ";
            }
            ss << "\n";
        }
    }
    
    ss << "\n--- Resources ---\n";
    for (u32 i = 0; i < m_resources.size(); ++i) {
        const auto& res = m_resources[i];
        ss << "[" << i << "] " << res.name;
        ss << " (lifetime: " << res.firstPassUsage << "-" << res.lastPassUsage << ")";
        if (res.isImported) ss << " [IMPORTED]";
        ss << "\n";
    }
    
    ss << "\n--- Execution Order ---\n";
    for (u32 orderIdx : m_executionOrder) {
        ss << m_passes[orderIdx].name << " -> ";
    }
    ss << "END\n";
    
    return ss.str();
}

std::string RenderGraph::exportGraphViz() const {
    std::ostringstream ss;
    
    ss << "digraph RenderGraph {\n";
    ss << "  rankdir=LR;\n";
    ss << "  node [shape=box];\n\n";
    
    // Passes as nodes
    for (u32 i = 0; i < m_passes.size(); ++i) {
        const auto& pass = m_passes[i];
        ss << "  pass" << i << " [label=\"" << pass.name << "\"";
        if (pass.culled) {
            ss << ", style=dashed, color=gray";
        } else {
            switch (pass.type) {
                case RGPassType::Graphics: ss << ", color=blue"; break;
                case RGPassType::Compute: ss << ", color=green"; break;
                case RGPassType::Transfer: ss << ", color=orange"; break;
                default: break;
            }
        }
        ss << "];\n";
    }
    
    ss << "\n";
    
    // Resources as nodes (different shape)
    for (u32 i = 0; i < m_resources.size(); ++i) {
        const auto& res = m_resources[i];
        ss << "  res" << i << " [label=\"" << res.name << "\"";
        ss << ", shape=ellipse";
        if (res.isImported) {
            ss << ", style=filled, fillcolor=lightgray";
        }
        ss << "];\n";
    }
    
    ss << "\n";
    
    // Dependencies
    for (u32 i = 0; i < m_passes.size(); ++i) {
        const auto& pass = m_passes[i];
        for (u32 dep : pass.dependencies) {
            ss << "  pass" << dep << " -> pass" << i << ";\n";
        }
    }
    
    ss << "}\n";
    
    return ss.str();
}

} // namespace nova::render
