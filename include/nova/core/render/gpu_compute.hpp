/**
 * @file gpu_compute.hpp
 * @brief GPU Compute Abstraction System for NovaCore Engine
 *
 * Provides a comprehensive abstraction over GPU compute capabilities including:
 * - Compute shader management and dispatch
 * - Structured buffer operations (append/consume, counter buffers)
 * - Indirect dispatch and argument buffers
 * - GPU-driven workload distribution
 * - Async compute queue management
 * - Memory barriers and synchronization
 * - Performance profiling and debugging
 *
 * Copyright (c) NovaForge Platform - All rights reserved.
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <memory>
#include <unordered_map>
#include <atomic>

namespace nova {

// ============================================================================
// Configuration Constants
// ============================================================================

/**
 * @brief Configuration constants for compute system
 */
struct ComputeConfig {
    // Workgroup limits
    static constexpr u32 MAX_WORKGROUP_SIZE_X = 1024;
    static constexpr u32 MAX_WORKGROUP_SIZE_Y = 1024;
    static constexpr u32 MAX_WORKGROUP_SIZE_Z = 64;
    static constexpr u32 MAX_WORKGROUP_INVOCATIONS = 1024;
    static constexpr u32 MAX_WORKGROUP_COUNT_X = 65535;
    static constexpr u32 MAX_WORKGROUP_COUNT_Y = 65535;
    static constexpr u32 MAX_WORKGROUP_COUNT_Z = 65535;
    
    // Resource limits
    static constexpr u32 MAX_COMPUTE_BUFFERS = 16;
    static constexpr u32 MAX_COMPUTE_TEXTURES = 16;
    static constexpr u32 MAX_COMPUTE_SAMPLERS = 8;
    static constexpr u32 MAX_PUSH_CONSTANT_SIZE = 256;
    static constexpr u32 MAX_SHARED_MEMORY = 49152;  // 48KB typical
    
    // Queue configuration
    static constexpr u32 MAX_ASYNC_COMPUTE_QUEUES = 4;
    static constexpr u32 COMMAND_BUFFER_POOL_SIZE = 64;
    
    // Profiling
    static constexpr u32 MAX_TIMESTAMP_QUERIES = 256;
    static constexpr u32 MAX_PIPELINE_STATISTICS = 64;
};

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Compute queue type for workload scheduling
 */
enum class ComputeQueueType : u8 {
    Graphics,       ///< Graphics queue (supports compute)
    AsyncCompute,   ///< Dedicated async compute queue
    Transfer,       ///< Transfer queue (for buffer operations)
    
    Count
};

/**
 * @brief Get name of compute queue type
 */
inline constexpr const char* getComputeQueueTypeName(ComputeQueueType type) {
    switch (type) {
        case ComputeQueueType::Graphics: return "Graphics";
        case ComputeQueueType::AsyncCompute: return "AsyncCompute";
        case ComputeQueueType::Transfer: return "Transfer";
        default: return "Unknown";
    }
}

/**
 * @brief Memory barrier scope for synchronization
 */
enum class BarrierScope : u8 {
    None = 0,
    Compute = 1 << 0,       ///< Compute shader access
    Graphics = 1 << 1,      ///< Graphics shader access
    Transfer = 1 << 2,      ///< Transfer operations
    Host = 1 << 3,          ///< CPU access
    All = Compute | Graphics | Transfer | Host
};

inline BarrierScope operator|(BarrierScope a, BarrierScope b) {
    return static_cast<BarrierScope>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline BarrierScope operator&(BarrierScope a, BarrierScope b) {
    return static_cast<BarrierScope>(static_cast<u8>(a) & static_cast<u8>(b));
}

inline bool hasScope(BarrierScope flags, BarrierScope scope) {
    return (static_cast<u8>(flags) & static_cast<u8>(scope)) != 0;
}

/**
 * @brief Buffer access type for barriers
 */
enum class BufferAccess : u8 {
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    ReadWrite = Read | Write,
    Atomic = 1 << 2
};

inline BufferAccess operator|(BufferAccess a, BufferAccess b) {
    return static_cast<BufferAccess>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline bool hasAccess(BufferAccess flags, BufferAccess access) {
    return (static_cast<u8>(flags) & static_cast<u8>(access)) != 0;
}

/**
 * @brief Compute buffer type
 */
enum class ComputeBufferType : u8 {
    Storage,            ///< Standard storage buffer (SSBO)
    Uniform,            ///< Uniform buffer (constant data)
    Indirect,           ///< Indirect dispatch arguments
    Counter,            ///< Atomic counter buffer
    AppendConsume,      ///< Append/consume structured buffer
    Staging,            ///< CPU-GPU staging buffer
    
    Count
};

inline constexpr const char* getComputeBufferTypeName(ComputeBufferType type) {
    switch (type) {
        case ComputeBufferType::Storage: return "Storage";
        case ComputeBufferType::Uniform: return "Uniform";
        case ComputeBufferType::Indirect: return "Indirect";
        case ComputeBufferType::Counter: return "Counter";
        case ComputeBufferType::AppendConsume: return "AppendConsume";
        case ComputeBufferType::Staging: return "Staging";
        default: return "Unknown";
    }
}

/**
 * @brief Image/texture access type in compute shaders
 */
enum class ImageAccess : u8 {
    SampleOnly,     ///< Read via sampler
    LoadOnly,       ///< Direct load (imageLoad)
    StoreOnly,      ///< Direct store (imageStore)
    LoadStore,      ///< Both load and store
    
    Count
};

inline constexpr const char* getImageAccessName(ImageAccess access) {
    switch (access) {
        case ImageAccess::SampleOnly: return "SampleOnly";
        case ImageAccess::LoadOnly: return "LoadOnly";
        case ImageAccess::StoreOnly: return "StoreOnly";
        case ImageAccess::LoadStore: return "LoadStore";
        default: return "Unknown";
    }
}

/**
 * @brief Dispatch type
 */
enum class DispatchType : u8 {
    Direct,         ///< Direct dispatch with explicit workgroup counts
    Indirect,       ///< Indirect dispatch from buffer
    IndirectCount,  ///< Indirect dispatch with count buffer
    
    Count
};

// ============================================================================
// Core Structures
// ============================================================================

/**
 * @brief 3D workgroup size specification
 */
struct WorkgroupSize {
    u32 x = 1;
    u32 y = 1;
    u32 z = 1;
    
    constexpr WorkgroupSize() = default;
    constexpr WorkgroupSize(u32 x_, u32 y_ = 1, u32 z_ = 1) : x(x_), y(y_), z(z_) {}
    
    [[nodiscard]] constexpr u32 totalInvocations() const { return x * y * z; }
    
    [[nodiscard]] constexpr bool isValid() const {
        return x > 0 && y > 0 && z > 0 &&
               x <= ComputeConfig::MAX_WORKGROUP_SIZE_X &&
               y <= ComputeConfig::MAX_WORKGROUP_SIZE_Y &&
               z <= ComputeConfig::MAX_WORKGROUP_SIZE_Z &&
               totalInvocations() <= ComputeConfig::MAX_WORKGROUP_INVOCATIONS;
    }
    
    // Common presets
    static constexpr WorkgroupSize linear64() { return {64, 1, 1}; }
    static constexpr WorkgroupSize linear128() { return {128, 1, 1}; }
    static constexpr WorkgroupSize linear256() { return {256, 1, 1}; }
    static constexpr WorkgroupSize tile8x8() { return {8, 8, 1}; }
    static constexpr WorkgroupSize tile16x16() { return {16, 16, 1}; }
    static constexpr WorkgroupSize tile32x32() { return {32, 32, 1}; }
    static constexpr WorkgroupSize volume4x4x4() { return {4, 4, 4}; }
    static constexpr WorkgroupSize volume8x8x8() { return {8, 8, 8}; }
};

/**
 * @brief Dispatch arguments for compute work
 */
struct DispatchArgs {
    u32 groupCountX = 1;
    u32 groupCountY = 1;
    u32 groupCountZ = 1;
    
    constexpr DispatchArgs() = default;
    constexpr DispatchArgs(u32 x, u32 y = 1, u32 z = 1) 
        : groupCountX(x), groupCountY(y), groupCountZ(z) {}
    
    [[nodiscard]] constexpr u64 totalGroups() const {
        return static_cast<u64>(groupCountX) * groupCountY * groupCountZ;
    }
    
    [[nodiscard]] constexpr u64 totalThreads(const WorkgroupSize& wgSize) const {
        return totalGroups() * wgSize.totalInvocations();
    }
    
    [[nodiscard]] constexpr bool isValid() const {
        return groupCountX > 0 && groupCountY > 0 && groupCountZ > 0 &&
               groupCountX <= ComputeConfig::MAX_WORKGROUP_COUNT_X &&
               groupCountY <= ComputeConfig::MAX_WORKGROUP_COUNT_Y &&
               groupCountZ <= ComputeConfig::MAX_WORKGROUP_COUNT_Z;
    }
    
    /**
     * @brief Calculate dispatch args for a 1D problem size
     */
    static DispatchArgs for1D(u32 elementCount, u32 workgroupSizeX) {
        return DispatchArgs((elementCount + workgroupSizeX - 1) / workgroupSizeX);
    }
    
    /**
     * @brief Calculate dispatch args for a 2D problem size
     */
    static DispatchArgs for2D(u32 width, u32 height, const WorkgroupSize& wgSize) {
        return DispatchArgs(
            (width + wgSize.x - 1) / wgSize.x,
            (height + wgSize.y - 1) / wgSize.y
        );
    }
    
    /**
     * @brief Calculate dispatch args for a 3D problem size
     */
    static DispatchArgs for3D(u32 width, u32 height, u32 depth, const WorkgroupSize& wgSize) {
        return DispatchArgs(
            (width + wgSize.x - 1) / wgSize.x,
            (height + wgSize.y - 1) / wgSize.y,
            (depth + wgSize.z - 1) / wgSize.z
        );
    }
};

/**
 * @brief Indirect dispatch arguments (GPU-readable format)
 */
struct IndirectDispatchArgs {
    u32 groupCountX;
    u32 groupCountY;
    u32 groupCountZ;
    
    static constexpr usize byteSize() { return sizeof(IndirectDispatchArgs); }
};

static_assert(sizeof(IndirectDispatchArgs) == 12, "IndirectDispatchArgs must be 12 bytes");

/**
 * @brief Compute buffer descriptor
 */
struct ComputeBufferDesc {
    std::string name;
    ComputeBufferType type = ComputeBufferType::Storage;
    u64 size = 0;
    u32 stride = 0;  // For structured buffers
    BufferAccess access = BufferAccess::ReadWrite;
    bool cpuVisible = false;
    bool persistentMap = false;
    
    [[nodiscard]] u64 elementCount() const {
        return stride > 0 ? size / stride : size;
    }
    
    [[nodiscard]] bool isStructured() const {
        return stride > 0;
    }
    
    // Factory methods
    static ComputeBufferDesc storage(std::string name, u64 size, u32 stride = 0) {
        ComputeBufferDesc desc;
        desc.name = std::move(name);
        desc.type = ComputeBufferType::Storage;
        desc.size = size;
        desc.stride = stride;
        return desc;
    }
    
    static ComputeBufferDesc uniform(std::string name, u64 size) {
        ComputeBufferDesc desc;
        desc.name = std::move(name);
        desc.type = ComputeBufferType::Uniform;
        desc.size = size;
        desc.access = BufferAccess::Read;
        return desc;
    }
    
    static ComputeBufferDesc indirect(std::string name, u32 maxDispatches = 1) {
        ComputeBufferDesc desc;
        desc.name = std::move(name);
        desc.type = ComputeBufferType::Indirect;
        desc.size = maxDispatches * sizeof(IndirectDispatchArgs);
        desc.stride = sizeof(IndirectDispatchArgs);
        desc.access = BufferAccess::Read;
        return desc;
    }
    
    static ComputeBufferDesc counter(std::string name, u32 numCounters = 1) {
        ComputeBufferDesc desc;
        desc.name = std::move(name);
        desc.type = ComputeBufferType::Counter;
        desc.size = numCounters * sizeof(u32);
        desc.stride = sizeof(u32);
        desc.access = BufferAccess::Atomic | BufferAccess::ReadWrite;
        return desc;
    }
    
    static ComputeBufferDesc appendConsume(std::string name, u64 maxElements, u32 stride) {
        ComputeBufferDesc desc;
        desc.name = std::move(name);
        desc.type = ComputeBufferType::AppendConsume;
        desc.size = maxElements * stride;
        desc.stride = stride;
        return desc;
    }
    
    static ComputeBufferDesc staging(std::string name, u64 size) {
        ComputeBufferDesc desc;
        desc.name = std::move(name);
        desc.type = ComputeBufferType::Staging;
        desc.size = size;
        desc.cpuVisible = true;
        desc.persistentMap = true;
        return desc;
    }
};

/**
 * @brief Compute image descriptor
 */
struct ComputeImageDesc {
    std::string name;
    u32 width = 0;
    u32 height = 0;
    u32 depth = 1;
    u32 mipLevels = 1;
    u32 arrayLayers = 1;
    ImageAccess access = ImageAccess::LoadStore;
    // Format would be TextureFormat from render_types.hpp
    
    [[nodiscard]] bool is3D() const { return depth > 1; }
    [[nodiscard]] bool isArray() const { return arrayLayers > 1; }
    [[nodiscard]] bool hasMips() const { return mipLevels > 1; }
};

/**
 * @brief Memory barrier specification
 */
struct MemoryBarrier {
    BarrierScope srcScope = BarrierScope::None;
    BarrierScope dstScope = BarrierScope::None;
    BufferAccess srcAccess = BufferAccess::None;
    BufferAccess dstAccess = BufferAccess::None;
    
    static MemoryBarrier computeToCompute() {
        return {BarrierScope::Compute, BarrierScope::Compute, 
                BufferAccess::Write, BufferAccess::Read};
    }
    
    static MemoryBarrier computeToGraphics() {
        return {BarrierScope::Compute, BarrierScope::Graphics,
                BufferAccess::Write, BufferAccess::Read};
    }
    
    static MemoryBarrier graphicsToCompute() {
        return {BarrierScope::Graphics, BarrierScope::Compute,
                BufferAccess::Write, BufferAccess::Read};
    }
    
    static MemoryBarrier computeToTransfer() {
        return {BarrierScope::Compute, BarrierScope::Transfer,
                BufferAccess::Write, BufferAccess::Read};
    }
    
    static MemoryBarrier computeToHost() {
        return {BarrierScope::Compute, BarrierScope::Host,
                BufferAccess::Write, BufferAccess::Read};
    }
    
    static MemoryBarrier full() {
        return {BarrierScope::All, BarrierScope::All,
                BufferAccess::ReadWrite, BufferAccess::ReadWrite};
    }
};

/**
 * @brief Buffer barrier for specific buffer transitions
 */
struct BufferBarrier {
    u64 bufferHandle = 0;
    u64 offset = 0;
    u64 size = ~0ULL;  // Whole buffer if ~0
    BarrierScope srcScope = BarrierScope::None;
    BarrierScope dstScope = BarrierScope::None;
    BufferAccess srcAccess = BufferAccess::None;
    BufferAccess dstAccess = BufferAccess::None;
};

/**
 * @brief Image barrier for texture transitions
 */
struct ImageBarrier {
    u64 imageHandle = 0;
    u32 baseMipLevel = 0;
    u32 mipLevelCount = 1;
    u32 baseArrayLayer = 0;
    u32 arrayLayerCount = 1;
    BarrierScope srcScope = BarrierScope::None;
    BarrierScope dstScope = BarrierScope::None;
    // Layout transitions would be specified here in full impl
};

// ============================================================================
// Compute Shader Structures
// ============================================================================

/**
 * @brief Compute shader specialization constant
 */
struct SpecializationConstant {
    u32 id = 0;
    union {
        i32 intValue;
        u32 uintValue;
        f32 floatValue;
    };
    
    enum class Type : u8 { Int, UInt, Float } type = Type::UInt;
    
    static SpecializationConstant fromInt(u32 id, i32 value) {
        SpecializationConstant sc;
        sc.id = id;
        sc.intValue = value;
        sc.type = Type::Int;
        return sc;
    }
    
    static SpecializationConstant fromUInt(u32 id, u32 value) {
        SpecializationConstant sc;
        sc.id = id;
        sc.uintValue = value;
        sc.type = Type::UInt;
        return sc;
    }
    
    static SpecializationConstant fromFloat(u32 id, f32 value) {
        SpecializationConstant sc;
        sc.id = id;
        sc.floatValue = value;
        sc.type = Type::Float;
        return sc;
    }
};

/**
 * @brief Compute shader descriptor
 */
struct ComputeShaderDesc {
    std::string name;
    std::string entryPoint = "main";
    std::vector<u8> spirvCode;  // SPIR-V bytecode
    WorkgroupSize workgroupSize;
    u32 sharedMemorySize = 0;
    std::vector<SpecializationConstant> specializationConstants;
    
    [[nodiscard]] bool isValid() const {
        return !spirvCode.empty() && workgroupSize.isValid();
    }
};

/**
 * @brief Compute pipeline descriptor
 */
struct ComputePipelineDesc {
    std::string name;
    u64 shaderHandle = 0;  // Reference to compiled shader
    std::vector<u64> bufferBindings;
    std::vector<u64> imageBindings;
    std::vector<u64> samplerBindings;
    u32 pushConstantSize = 0;
    
    [[nodiscard]] bool hasPushConstants() const {
        return pushConstantSize > 0;
    }
};

// ============================================================================
// GPU-Driven Structures
// ============================================================================

/**
 * @brief GPU-driven command for compute dispatch
 */
struct GPUDispatchCommand {
    u32 pipelineIndex;
    u32 groupCountX;
    u32 groupCountY;
    u32 groupCountZ;
    u32 firstBuffer;    // Index into buffer array
    u32 bufferCount;
    u32 pushConstantOffset;
    u32 pushConstantSize;
    
    static constexpr usize byteSize() { return sizeof(GPUDispatchCommand); }
};

static_assert(sizeof(GPUDispatchCommand) == 32, "GPUDispatchCommand must be 32 bytes");

/**
 * @brief Multi-dispatch indirect arguments
 */
struct MultiDispatchArgs {
    u32 dispatchCount;
    u32 padding[3];
    // Followed by dispatchCount * IndirectDispatchArgs
    
    static constexpr usize headerSize() { return sizeof(MultiDispatchArgs); }
};

// ============================================================================
// Profiling Structures
// ============================================================================

/**
 * @brief Compute profiling data
 */
struct ComputeProfilingData {
    std::string name;
    f64 gpuTimeMs = 0.0;
    f64 cpuTimeMs = 0.0;
    u64 invocations = 0;
    u64 memoryBandwidthBytes = 0;
    u32 occupancy = 0;  // Percentage
    
    [[nodiscard]] f64 throughputGBps() const {
        if (gpuTimeMs <= 0.0) return 0.0;
        return (memoryBandwidthBytes / (1024.0 * 1024.0 * 1024.0)) / (gpuTimeMs / 1000.0);
    }
};

/**
 * @brief Compute statistics
 */
struct ComputeStats {
    u64 totalDispatches = 0;
    u64 totalInvocations = 0;
    u64 indirectDispatches = 0;
    u64 barriers = 0;
    u64 bufferUploads = 0;
    u64 bufferDownloads = 0;
    f64 totalGpuTimeMs = 0.0;
    
    void reset() {
        totalDispatches = 0;
        totalInvocations = 0;
        indirectDispatches = 0;
        barriers = 0;
        bufferUploads = 0;
        bufferDownloads = 0;
        totalGpuTimeMs = 0.0;
    }
};

// ============================================================================
// Compute Context (Command Recording)
// ============================================================================

/**
 * @brief Compute command context for recording dispatches
 */
class ComputeContext {
public:
    ComputeContext() = default;
    ~ComputeContext() = default;
    
    /**
     * @brief Bind a compute pipeline
     */
    void bindPipeline(u64 pipelineHandle) {
        m_currentPipeline = pipelineHandle;
    }
    
    /**
     * @brief Bind a buffer to a slot
     */
    void bindBuffer(u32 slot, u64 bufferHandle, u64 offset = 0, u64 size = ~0ULL) {
        if (slot < m_boundBuffers.size()) {
            m_boundBuffers[slot] = {bufferHandle, offset, size};
        }
    }
    
    /**
     * @brief Bind an image to a slot
     */
    void bindImage(u32 slot, u64 imageHandle, u32 mipLevel = 0) {
        if (slot < m_boundImages.size()) {
            m_boundImages[slot] = {imageHandle, mipLevel};
        }
    }
    
    /**
     * @brief Set push constants
     */
    template<typename T>
    void setPushConstants(const T& data) {
        static_assert(sizeof(T) <= ComputeConfig::MAX_PUSH_CONSTANT_SIZE, 
                      "Push constant size exceeds limit");
        m_pushConstantData.resize(sizeof(T));
        std::memcpy(m_pushConstantData.data(), &data, sizeof(T));
    }
    
    /**
     * @brief Dispatch compute work
     */
    void dispatch(const DispatchArgs& args) {
        ComputeCommand cmd;
        cmd.type = CommandType::Dispatch;
        cmd.dispatch.args = args;
        cmd.dispatch.pipeline = m_currentPipeline;
        m_commands.push_back(cmd);
        m_stats.totalDispatches++;
    }
    
    /**
     * @brief Dispatch compute work with 1D size
     */
    void dispatch1D(u32 elementCount, u32 workgroupSizeX = 64) {
        dispatch(DispatchArgs::for1D(elementCount, workgroupSizeX));
    }
    
    /**
     * @brief Dispatch compute work with 2D size
     */
    void dispatch2D(u32 width, u32 height, const WorkgroupSize& wgSize = WorkgroupSize::tile8x8()) {
        dispatch(DispatchArgs::for2D(width, height, wgSize));
    }
    
    /**
     * @brief Dispatch compute work with 3D size
     */
    void dispatch3D(u32 width, u32 height, u32 depth, 
                    const WorkgroupSize& wgSize = WorkgroupSize::volume4x4x4()) {
        dispatch(DispatchArgs::for3D(width, height, depth, wgSize));
    }
    
    /**
     * @brief Indirect dispatch from buffer
     */
    void dispatchIndirect(u64 argsBuffer, u64 offset = 0) {
        ComputeCommand cmd;
        cmd.type = CommandType::DispatchIndirect;
        cmd.indirect.argsBuffer = argsBuffer;
        cmd.indirect.offset = offset;
        cmd.indirect.pipeline = m_currentPipeline;
        m_commands.push_back(cmd);
        m_stats.indirectDispatches++;
    }
    
    /**
     * @brief Insert a memory barrier
     */
    void barrier(const MemoryBarrier& barrierIn) {
        ComputeCommand cmd;
        cmd.type = CommandType::Barrier;
        cmd.barrier.srcScope = barrierIn.srcScope;
        cmd.barrier.dstScope = barrierIn.dstScope;
        cmd.barrier.srcAccess = barrierIn.srcAccess;
        cmd.barrier.dstAccess = barrierIn.dstAccess;
        cmd.barrier.bufferHandle = 0;  // Global barrier
        m_commands.push_back(cmd);
        m_stats.barriers++;
    }
    
    /**
     * @brief Insert a buffer barrier
     */
    void bufferBarrier(const BufferBarrier& barrierIn) {
        ComputeCommand cmd;
        cmd.type = CommandType::BufferBarrier;
        cmd.barrier.srcScope = barrierIn.srcScope;
        cmd.barrier.dstScope = barrierIn.dstScope;
        cmd.barrier.srcAccess = barrierIn.srcAccess;
        cmd.barrier.dstAccess = barrierIn.dstAccess;
        cmd.barrier.bufferHandle = barrierIn.bufferHandle;
        cmd.barrier.bufferOffset = barrierIn.offset;
        cmd.barrier.bufferSize = barrierIn.size;
        m_commands.push_back(cmd);
        m_stats.barriers++;
    }
    
    /**
     * @brief Copy buffer data
     */
    void copyBuffer(u64 srcBuffer, u64 dstBuffer, u64 srcOffset, u64 dstOffset, u64 size) {
        ComputeCommand cmd;
        cmd.type = CommandType::CopyBuffer;
        cmd.copy.srcBuffer = srcBuffer;
        cmd.copy.dstBuffer = dstBuffer;
        cmd.copy.srcOffset = srcOffset;
        cmd.copy.dstOffset = dstOffset;
        cmd.copy.size = size;
        m_commands.push_back(cmd);
    }
    
    /**
     * @brief Fill buffer with value
     */
    void fillBuffer(u64 buffer, u32 value, u64 offset = 0, u64 size = ~0ULL) {
        ComputeCommand cmd;
        cmd.type = CommandType::FillBuffer;
        cmd.fill.buffer = buffer;
        cmd.fill.value = value;
        cmd.fill.offset = offset;
        cmd.fill.size = size;
        m_commands.push_back(cmd);
    }
    
    /**
     * @brief Reset counter buffer to zero
     */
    void resetCounter(u64 counterBuffer, u32 counterIndex = 0) {
        fillBuffer(counterBuffer, 0, counterIndex * sizeof(u32), sizeof(u32));
    }
    
    /**
     * @brief Begin timestamp query
     */
    void beginTimestamp(u32 queryIndex) {
        ComputeCommand cmd;
        cmd.type = CommandType::BeginTimestamp;
        cmd.timestamp.queryIndex = queryIndex;
        m_commands.push_back(cmd);
    }
    
    /**
     * @brief End timestamp query
     */
    void endTimestamp(u32 queryIndex) {
        ComputeCommand cmd;
        cmd.type = CommandType::EndTimestamp;
        cmd.timestamp.queryIndex = queryIndex;
        m_commands.push_back(cmd);
    }
    
    /**
     * @brief Get recorded command count
     */
    [[nodiscard]] usize commandCount() const { return m_commands.size(); }
    
    /**
     * @brief Get statistics
     */
    [[nodiscard]] const ComputeStats& stats() const { return m_stats; }
    
    /**
     * @brief Reset context for reuse
     */
    void reset() {
        m_commands.clear();
        m_pushConstantData.clear();
        m_currentPipeline = 0;
        m_stats.reset();
    }

private:
    enum class CommandType : u8 {
        Dispatch,
        DispatchIndirect,
        Barrier,
        BufferBarrier,
        CopyBuffer,
        FillBuffer,
        BeginTimestamp,
        EndTimestamp
    };
    
    struct ComputeCommand {
        CommandType type = CommandType::Dispatch;
        
        // Use a simple struct-based union approach
        struct DispatchData {
            DispatchArgs args;
            u64 pipeline = 0;
        };
        
        struct IndirectData {
            u64 argsBuffer = 0;
            u64 offset = 0;
            u64 pipeline = 0;
        };
        
        struct BarrierData {
            BarrierScope srcScope = BarrierScope::None;
            BarrierScope dstScope = BarrierScope::None;
            BufferAccess srcAccess = BufferAccess::None;
            BufferAccess dstAccess = BufferAccess::None;
            u64 bufferHandle = 0;  // 0 for global barrier
            u64 bufferOffset = 0;
            u64 bufferSize = ~0ULL;
        };
        
        struct CopyData {
            u64 srcBuffer = 0;
            u64 dstBuffer = 0;
            u64 srcOffset = 0;
            u64 dstOffset = 0;
            u64 size = 0;
        };
        
        struct FillData {
            u64 buffer = 0;
            u32 value = 0;
            u64 offset = 0;
            u64 size = ~0ULL;
        };
        
        struct TimestampData {
            u32 queryIndex = 0;
        };
        
        // Data storage - use largest needed
        DispatchData dispatch;
        IndirectData indirect;
        BarrierData barrier;
        CopyData copy;
        FillData fill;
        TimestampData timestamp;
    };
    
    struct BufferBinding {
        u64 handle = 0;
        u64 offset = 0;
        u64 size = ~0ULL;
    };
    
    struct ImageBinding {
        u64 handle = 0;
        u32 mipLevel = 0;
    };
    
    std::vector<ComputeCommand> m_commands;
    std::vector<u8> m_pushConstantData;
    std::array<BufferBinding, ComputeConfig::MAX_COMPUTE_BUFFERS> m_boundBuffers{};
    std::array<ImageBinding, ComputeConfig::MAX_COMPUTE_TEXTURES> m_boundImages{};
    u64 m_currentPipeline = 0;
    ComputeStats m_stats;
};

// ============================================================================
// Compute Manager Singleton
// ============================================================================

/**
 * @brief GPU capabilities for compute
 */
struct ComputeCapabilities {
    bool supportsAsyncCompute = false;
    bool supportsIndirectDispatch = false;
    bool supportsAtomics = false;
    bool supportsSubgroups = false;
    bool supportsSharedMemoryAtomics = false;
    u32 maxWorkgroupSizeX = 1024;
    u32 maxWorkgroupSizeY = 1024;
    u32 maxWorkgroupSizeZ = 64;
    u32 maxWorkgroupInvocations = 1024;
    u32 maxSharedMemorySize = 49152;
    u32 subgroupSize = 32;
    u32 asyncComputeQueues = 0;
    
    [[nodiscard]] bool supportsWorkgroupSize(const WorkgroupSize& size) const {
        return size.x <= maxWorkgroupSizeX &&
               size.y <= maxWorkgroupSizeY &&
               size.z <= maxWorkgroupSizeZ &&
               size.totalInvocations() <= maxWorkgroupInvocations;
    }
};

/**
 * @brief Compute manager singleton for GPU compute operations
 */
class ComputeManager {
public:
    /**
     * @brief Get singleton instance
     */
    static ComputeManager& get() {
        static ComputeManager instance;
        return instance;
    }
    
    /**
     * @brief Initialize compute system
     */
    bool initialize() {
        // Detect GPU capabilities
        m_capabilities.supportsAsyncCompute = true;
        m_capabilities.supportsIndirectDispatch = true;
        m_capabilities.supportsAtomics = true;
        m_capabilities.supportsSubgroups = true;
        m_capabilities.supportsSharedMemoryAtomics = true;
        m_capabilities.asyncComputeQueues = 2;
        m_initialized = true;
        return true;
    }
    
    /**
     * @brief Shutdown compute system
     */
    void shutdown() {
        m_shaders.clear();
        m_pipelines.clear();
        m_buffers.clear();
        m_initialized = false;
    }
    
    /**
     * @brief Check if initialized
     */
    [[nodiscard]] bool isInitialized() const { return m_initialized; }
    
    /**
     * @brief Get compute capabilities
     */
    [[nodiscard]] const ComputeCapabilities& capabilities() const { return m_capabilities; }
    
    /**
     * @brief Create a compute shader
     */
    u64 createShader(const ComputeShaderDesc& desc) {
        u64 handle = m_nextHandle++;
        m_shaders[handle] = desc;
        return handle;
    }
    
    /**
     * @brief Destroy a compute shader
     */
    void destroyShader(u64 handle) {
        m_shaders.erase(handle);
    }
    
    /**
     * @brief Create a compute pipeline
     */
    u64 createPipeline(const ComputePipelineDesc& desc) {
        u64 handle = m_nextHandle++;
        m_pipelines[handle] = desc;
        return handle;
    }
    
    /**
     * @brief Destroy a compute pipeline
     */
    void destroyPipeline(u64 handle) {
        m_pipelines.erase(handle);
    }
    
    /**
     * @brief Create a compute buffer
     */
    u64 createBuffer(const ComputeBufferDesc& desc) {
        u64 handle = m_nextHandle++;
        m_buffers[handle] = desc;
        return handle;
    }
    
    /**
     * @brief Destroy a compute buffer
     */
    void destroyBuffer(u64 handle) {
        m_buffers.erase(handle);
    }
    
    /**
     * @brief Get buffer description
     */
    [[nodiscard]] const ComputeBufferDesc* getBufferDesc(u64 handle) const {
        auto it = m_buffers.find(handle);
        return it != m_buffers.end() ? &it->second : nullptr;
    }
    
    /**
     * @brief Submit compute commands
     */
    void submit(const ComputeContext& context, ComputeQueueType queue = ComputeQueueType::AsyncCompute) {
        m_totalStats.totalDispatches += context.stats().totalDispatches;
        m_totalStats.indirectDispatches += context.stats().indirectDispatches;
        m_totalStats.barriers += context.stats().barriers;
        // In a real implementation, this would submit to the GPU
    }
    
    /**
     * @brief Wait for all compute work to complete
     */
    void waitIdle() {
        // In a real implementation, this would wait for GPU
    }
    
    /**
     * @brief Get total statistics
     */
    [[nodiscard]] const ComputeStats& totalStats() const { return m_totalStats; }
    
    /**
     * @brief Reset statistics
     */
    void resetStats() { m_totalStats.reset(); }

private:
    ComputeManager() = default;
    ~ComputeManager() = default;
    ComputeManager(const ComputeManager&) = delete;
    ComputeManager& operator=(const ComputeManager&) = delete;
    
    bool m_initialized = false;
    ComputeCapabilities m_capabilities;
    std::unordered_map<u64, ComputeShaderDesc> m_shaders;
    std::unordered_map<u64, ComputePipelineDesc> m_pipelines;
    std::unordered_map<u64, ComputeBufferDesc> m_buffers;
    std::atomic<u64> m_nextHandle{1};
    ComputeStats m_totalStats;
};

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Calculate optimal workgroup size for a 1D problem
 */
inline WorkgroupSize calculateOptimalWorkgroupSize1D(u32 elementCount, u32 preferredSize = 256) {
    // Use power of 2 workgroup sizes
    u32 size = preferredSize;
    while (size > elementCount && size > 32) {
        size /= 2;
    }
    return WorkgroupSize{size, 1, 1};
}

/**
 * @brief Calculate optimal workgroup size for a 2D problem
 */
inline WorkgroupSize calculateOptimalWorkgroupSize2D(u32 width, u32 height, 
                                                      u32 maxInvocations = 256) {
    // Start with square tiles
    u32 tileSize = 1;
    while (tileSize * tileSize * 4 <= maxInvocations) {
        tileSize *= 2;
    }
    
    // Adjust for aspect ratio
    if (width > height * 2) {
        return WorkgroupSize{tileSize * 2, tileSize / 2, 1};
    } else if (height > width * 2) {
        return WorkgroupSize{tileSize / 2, tileSize * 2, 1};
    }
    
    return WorkgroupSize{tileSize, tileSize, 1};
}

/**
 * @brief Round up to multiple
 */
inline constexpr u32 roundUpToMultiple(u32 value, u32 multiple) {
    return ((value + multiple - 1) / multiple) * multiple;
}

/**
 * @brief Calculate aligned buffer size
 */
inline constexpr u64 alignBufferSize(u64 size, u64 alignment = 256) {
    return ((size + alignment - 1) / alignment) * alignment;
}

/**
 * @brief Calculate dispatch groups needed
 */
inline constexpr u32 dispatchGroupsNeeded(u32 elementCount, u32 workgroupSize) {
    return (elementCount + workgroupSize - 1) / workgroupSize;
}

} // namespace nova
