/**
 * @file vulkan_buffer.hpp
 * @brief Nova GraphicsCore™ - Vulkan Buffer and Memory Management
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Implements Vulkan buffer creation and GPU memory allocation.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "vulkan_types.hpp"
#include "nova/core/types/result.hpp"
#include "nova/core/render/buffer.hpp"
#include <memory>
#include <vector>

namespace nova::render::vulkan {

// Forward declarations
class VulkanDevice;

// ============================================================================
// Memory Types
// ============================================================================

/**
 * @brief Memory allocation preference
 */
enum class MemoryUsage : u8 {
    /// Device local (GPU VRAM) - fastest for GPU access
    GpuOnly,
    
    /// CPU visible, cached (for staging buffers)
    CpuOnly,
    
    /// CPU visible, write combined (for dynamic updates)
    CpuToGpu,
    
    /// GPU to CPU readable (for readback)
    GpuToCpu,
    
    /// Automatic selection based on buffer usage
    Auto
};

/**
 * @brief Allocated memory block info
 */
struct MemoryAllocation {
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    VkDeviceSize size = 0;
    void* mappedPtr = nullptr;
    u32 memoryTypeIndex = UINT32_MAX;
    MemoryUsage usage = MemoryUsage::Auto;
};

// ============================================================================
// Vulkan Buffer
// ============================================================================

/**
 * @brief Vulkan buffer creation descriptor
 */
struct VulkanBufferDesc {
    VkDeviceSize size = 0;                      ///< Buffer size in bytes
    BufferUsage usage = BufferUsage::Vertex;    ///< Buffer usage flags
    MemoryUsage memoryUsage = MemoryUsage::Auto; ///< Memory allocation preference
    std::string name;                           ///< Debug name
    
    /// Factory: Vertex buffer
    static VulkanBufferDesc vertex(VkDeviceSize size, const std::string& name = "VertexBuffer") {
        VulkanBufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Vertex;
        desc.memoryUsage = MemoryUsage::GpuOnly;
        desc.name = name;
        return desc;
    }
    
    /// Factory: Index buffer
    static VulkanBufferDesc index(VkDeviceSize size, const std::string& name = "IndexBuffer") {
        VulkanBufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Index;
        desc.memoryUsage = MemoryUsage::GpuOnly;
        desc.name = name;
        return desc;
    }
    
    /// Factory: Uniform buffer (CPU to GPU, updated per frame)
    static VulkanBufferDesc uniform(VkDeviceSize size, const std::string& name = "UniformBuffer") {
        VulkanBufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Uniform;
        desc.memoryUsage = MemoryUsage::CpuToGpu;
        desc.name = name;
        return desc;
    }
    
    /// Factory: Storage buffer (GPU compute)
    static VulkanBufferDesc storage(VkDeviceSize size, const std::string& name = "StorageBuffer") {
        VulkanBufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::Storage;
        desc.memoryUsage = MemoryUsage::GpuOnly;
        desc.name = name;
        return desc;
    }
    
    /// Factory: Staging buffer (CPU visible, for upload)
    static VulkanBufferDesc staging(VkDeviceSize size, const std::string& name = "StagingBuffer") {
        VulkanBufferDesc desc;
        desc.size = size;
        desc.usage = BufferUsage::TransferSrc;
        desc.memoryUsage = MemoryUsage::CpuOnly;
        desc.name = name;
        return desc;
    }
};

/**
 * @brief Vulkan buffer wrapper
 * 
 * Manages a Vulkan buffer and its associated memory allocation.
 */
class VulkanBuffer {
public:
    ~VulkanBuffer();
    
    // Non-copyable
    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;
    
    // Moveable
    VulkanBuffer(VulkanBuffer&& other) noexcept;
    VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;
    
    /**
     * @brief Create a Vulkan buffer
     * @param device Vulkan device
     * @param desc Buffer descriptor
     * @return Buffer or error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanBuffer>, Error>
    create(VulkanDevice& device, const VulkanBufferDesc& desc);
    
    /**
     * @brief Map buffer memory for CPU access
     * @return Mapped pointer or nullptr if not mappable
     */
    [[nodiscard]] void* map();
    
    /**
     * @brief Unmap buffer memory
     */
    void unmap();
    
    /**
     * @brief Flush mapped memory range to make writes visible to GPU
     * @param offset Offset from buffer start
     * @param size Size to flush (VK_WHOLE_SIZE for entire buffer)
     */
    void flush(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
    
    /**
     * @brief Invalidate mapped memory to make GPU writes visible to CPU
     * @param offset Offset from buffer start
     * @param size Size to invalidate (VK_WHOLE_SIZE for entire buffer)
     */
    void invalidate(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
    
    /**
     * @brief Upload data to buffer (requires mappable memory)
     * @param data Source data
     * @param size Data size
     * @param offset Offset in buffer
     */
    void upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    
    /**
     * @brief Download data from buffer (requires mappable memory)
     * @param data Destination buffer
     * @param size Data size
     * @param offset Offset in buffer
     */
    void download(void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    
    /// Get Vulkan handle
    [[nodiscard]] VkBuffer getHandle() const noexcept { return m_buffer; }
    
    /// Get buffer size
    [[nodiscard]] VkDeviceSize getSize() const noexcept { return m_size; }
    
    /// Get buffer usage
    [[nodiscard]] BufferUsage getUsage() const noexcept { return m_usage; }
    
    /// Get memory usage
    [[nodiscard]] MemoryUsage getMemoryUsage() const noexcept { return m_memoryUsage; }
    
    /// Check if buffer is currently mapped
    [[nodiscard]] bool isMapped() const noexcept { return m_mappedPtr != nullptr; }
    
    /// Get mapped pointer (nullptr if not mapped)
    [[nodiscard]] void* getMappedPtr() const noexcept { return m_mappedPtr; }
    
    /// Get device memory handle
    [[nodiscard]] VkDeviceMemory getMemory() const noexcept { return m_memory; }

private:
    VulkanBuffer(VulkanDevice& device);
    
    VulkanDevice* m_device = nullptr;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkDeviceSize m_size = 0;
    BufferUsage m_usage = BufferUsage::Vertex;
    MemoryUsage m_memoryUsage = MemoryUsage::Auto;
    void* m_mappedPtr = nullptr;
    bool m_persistentlyMapped = false;
};

// ============================================================================
// Memory Allocator
// ============================================================================

/**
 * @brief Simple Vulkan memory allocator
 * 
 * Provides basic memory allocation with type selection based on requirements.
 * For production, consider integrating VMA (Vulkan Memory Allocator).
 */
class VulkanMemoryAllocator {
public:
    explicit VulkanMemoryAllocator(VulkanDevice& device);
    ~VulkanMemoryAllocator() = default;
    
    /**
     * @brief Allocate memory for a buffer
     * @param buffer Buffer to allocate for
     * @param usage Memory usage preference
     * @return Allocation or error
     */
    [[nodiscard]] Result<MemoryAllocation, Error>
    allocateBufferMemory(VkBuffer buffer, MemoryUsage usage);
    
    /**
     * @brief Allocate memory for an image
     * @param image Image to allocate for
     * @param usage Memory usage preference
     * @return Allocation or error
     */
    [[nodiscard]] Result<MemoryAllocation, Error>
    allocateImageMemory(VkImage image, MemoryUsage usage);
    
    /**
     * @brief Free a memory allocation
     * @param allocation Allocation to free
     */
    void freeMemory(const MemoryAllocation& allocation);
    
    /**
     * @brief Find a suitable memory type
     * @param typeBits Memory type bits from VkMemoryRequirements
     * @param usage Memory usage preference
     * @return Memory type index or error
     */
    [[nodiscard]] Result<u32, Error>
    findMemoryType(u32 typeBits, MemoryUsage usage);

private:
    VulkanDevice* m_device;
    VkPhysicalDeviceMemoryProperties m_memProps{};
};

// ============================================================================
// Staging Buffer Manager
// ============================================================================

/**
 * @brief Manages staging buffers for GPU uploads
 * 
 * Provides efficient upload of vertex/index/texture data to GPU memory.
 */
class VulkanStagingManager {
public:
    explicit VulkanStagingManager(VulkanDevice& device);
    ~VulkanStagingManager();
    
    /**
     * @brief Copy data to a GPU buffer using a staging buffer
     * @param dstBuffer Destination GPU buffer
     * @param data Source CPU data
     * @param size Data size
     * @param dstOffset Offset in destination buffer
     * @return Success or error
     */
    [[nodiscard]] Result<void, Error>
    copyToBuffer(VkBuffer dstBuffer, const void* data, VkDeviceSize size, VkDeviceSize dstOffset = 0);
    
    /**
     * @brief Copy data to a GPU image using a staging buffer
     * @param dstImage Destination GPU image
     * @param data Source CPU data
     * @param size Data size
     * @param width Image width
     * @param height Image height
     * @param layerCount Number of array layers
     * @param mipLevel Mip level to copy to
     * @return Success or error
     */
    [[nodiscard]] Result<void, Error>
    copyToImage(VkImage dstImage, const void* data, VkDeviceSize size,
                u32 width, u32 height, u32 layerCount = 1, u32 mipLevel = 0);
    
    /**
     * @brief Flush all pending staging operations
     */
    void flush();

private:
    struct StagingBuffer {
        std::unique_ptr<VulkanBuffer> buffer;
        VkDeviceSize usedSize = 0;
    };
    
    [[nodiscard]] Result<VulkanBuffer*, Error> acquireStagingBuffer(VkDeviceSize size);
    void submitAndWait();
    
    VulkanDevice* m_device;
    std::vector<StagingBuffer> m_stagingBuffers;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    VkFence m_fence = VK_NULL_HANDLE;
    bool m_hasWork = false;
    
    static constexpr VkDeviceSize DEFAULT_STAGING_SIZE = 64 * 1024 * 1024; // 64MB
};

} // namespace nova::render::vulkan
