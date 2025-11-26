/**
 * @file buffer.hpp
 * @brief GPU buffer types and descriptors
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include <string>

namespace nova::render {

/**
 * @brief Buffer usage flags
 */
enum class BufferUsage : u32 {
    None = 0,
    VertexBuffer = 1 << 0,
    IndexBuffer = 1 << 1,
    UniformBuffer = 1 << 2,
    StorageBuffer = 1 << 3,
    IndirectBuffer = 1 << 4,
    TransferSrc = 1 << 5,
    TransferDst = 1 << 6,
    
    // Convenience combinations
    Vertex = VertexBuffer | TransferDst,
    Index = IndexBuffer | TransferDst,
    Uniform = UniformBuffer,
    Storage = StorageBuffer,
    Staging = TransferSrc
};

inline BufferUsage operator|(BufferUsage a, BufferUsage b) {
    return static_cast<BufferUsage>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline BufferUsage operator&(BufferUsage a, BufferUsage b) {
    return static_cast<BufferUsage>(static_cast<u32>(a) & static_cast<u32>(b));
}

/**
 * @brief Memory usage hint
 */
enum class MemoryUsage : u8 {
    GPUOnly,        ///< GPU-only memory (fastest, no CPU access)
    CPUToGPU,       ///< CPU writable, GPU readable (staging, dynamic uniforms)
    GPUToCPU,       ///< GPU writable, CPU readable (readback)
    CPUOnly         ///< CPU-only memory (for staging)
};

/**
 * @brief Buffer creation descriptor
 */
struct BufferDesc {
    /// Size in bytes
    u64 size = 0;
    
    /// Usage flags
    BufferUsage usage = BufferUsage::None;
    
    /// Memory usage hint
    MemoryUsage memoryUsage = MemoryUsage::GPUOnly;
    
    /// Create mapped for persistent mapping
    bool persistentlyMapped = false;
    
    /// Initial data (optional)
    const void* initialData = nullptr;
    
    /// Debug name
    std::string name;
    
    // Factory methods
    static BufferDesc vertex(u64 size, const void* data = nullptr) {
        BufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Vertex;
        desc.memoryUsage = MemoryUsage::GPUOnly;
        desc.initialData = data;
        return desc;
    }
    
    static BufferDesc index(u64 size, const void* data = nullptr) {
        BufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Index;
        desc.memoryUsage = MemoryUsage::GPUOnly;
        desc.initialData = data;
        return desc;
    }
    
    static BufferDesc uniform(u64 size) {
        BufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Uniform;
        desc.memoryUsage = MemoryUsage::CPUToGPU;
        desc.persistentlyMapped = true;
        return desc;
    }
    
    static BufferDesc storage(u64 size) {
        BufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Storage;
        desc.memoryUsage = MemoryUsage::GPUOnly;
        return desc;
    }
    
    static BufferDesc staging(u64 size) {
        BufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Staging;
        desc.memoryUsage = MemoryUsage::CPUOnly;
        return desc;
    }
};

} // namespace nova::render
