/**
 * @file texture_streaming.hpp
 * @brief Advanced Texture Streaming System for NovaCore Engine
 * 
 * Provides intelligent texture streaming with:
 * - Virtual texturing support
 * - Mipmap streaming based on screen coverage
 * - Priority-based loading with urgency calculation
 * - Memory budget management
 * - Asynchronous disk I/O
 * - Tile-based sparse textures
 * - Feedback buffer analysis
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"

#include <array>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <mutex>

namespace nova {

// =============================================================================
// Configuration Constants
// =============================================================================

/**
 * @brief Texture streaming system configuration constants
 */
struct TextureStreamingConfig {
    /// Default memory budget (MB)
    static constexpr u64 DEFAULT_MEMORY_BUDGET_MB = 512;
    
    /// Minimum memory budget (MB)
    static constexpr u64 MIN_MEMORY_BUDGET_MB = 128;
    
    /// Maximum memory budget (MB)
    static constexpr u64 MAX_MEMORY_BUDGET_MB = 8192;
    
    /// Default tile size for virtual textures
    static constexpr u32 DEFAULT_TILE_SIZE = 128;
    
    /// Maximum tile size
    static constexpr u32 MAX_TILE_SIZE = 512;
    
    /// Maximum mip levels
    static constexpr u32 MAX_MIP_LEVELS = 16;
    
    /// Maximum pending requests
    static constexpr u32 MAX_PENDING_REQUESTS = 256;
    
    /// Feedback buffer resolution divisor
    static constexpr u32 FEEDBACK_RESOLUTION_DIVISOR = 4;
    
    /// Maximum textures tracked
    static constexpr u32 MAX_TRACKED_TEXTURES = 8192;
    
    /// Hysteresis frames before eviction
    static constexpr u32 EVICTION_HYSTERESIS_FRAMES = 30;
    
    /// Priority boost for visible textures
    static constexpr f32 VISIBLE_PRIORITY_BOOST = 2.0f;
};

// =============================================================================
// Enumerations
// =============================================================================

/**
 * @brief Texture streaming priority level
 */
enum class StreamPriority : u32 {
    /// Background/prefetch loading
    Background = 0,
    
    /// Normal priority
    Normal = 1,
    
    /// High priority (recently visible)
    High = 2,
    
    /// Urgent (currently visible, low mip)
    Urgent = 3,
    
    /// Critical (required immediately)
    Critical = 4,
    
    COUNT
};

/**
 * @brief Get priority name
 */
[[nodiscard]] constexpr const char* getStreamPriorityName(StreamPriority priority) noexcept {
    switch (priority) {
        case StreamPriority::Background: return "Background";
        case StreamPriority::Normal: return "Normal";
        case StreamPriority::High: return "High";
        case StreamPriority::Urgent: return "Urgent";
        case StreamPriority::Critical: return "Critical";
        default: return "Unknown";
    }
}

/**
 * @brief Texture streaming state
 */
enum class StreamState : u32 {
    /// Not loaded, no request pending
    Unloaded = 0,
    
    /// Load request queued
    Queued = 1,
    
    /// Currently loading from disk
    Loading = 2,
    
    /// Currently uploading to GPU
    Uploading = 3,
    
    /// Fully resident in GPU memory
    Resident = 4,
    
    /// Eviction pending
    Evicting = 5,
    
    /// Error during load
    Error = 6,
    
    COUNT
};

/**
 * @brief Get stream state name
 */
[[nodiscard]] constexpr const char* getStreamStateName(StreamState state) noexcept {
    switch (state) {
        case StreamState::Unloaded: return "Unloaded";
        case StreamState::Queued: return "Queued";
        case StreamState::Loading: return "Loading";
        case StreamState::Uploading: return "Uploading";
        case StreamState::Resident: return "Resident";
        case StreamState::Evicting: return "Evicting";
        case StreamState::Error: return "Error";
        default: return "Unknown";
    }
}

/**
 * @brief Virtual texture mode
 */
enum class VirtualTextureMode : u32 {
    /// Disabled (traditional streaming)
    Disabled = 0,
    
    /// Software virtual texturing
    Software = 1,
    
    /// Hardware sparse textures (Vulkan/DX12)
    HardwareSparse = 2,
    
    /// Bindless virtual texturing
    Bindless = 3,
    
    COUNT
};

/**
 * @brief Get virtual texture mode name
 */
[[nodiscard]] constexpr const char* getVirtualTextureModeName(VirtualTextureMode mode) noexcept {
    switch (mode) {
        case VirtualTextureMode::Disabled: return "Disabled";
        case VirtualTextureMode::Software: return "Software";
        case VirtualTextureMode::HardwareSparse: return "Hardware Sparse";
        case VirtualTextureMode::Bindless: return "Bindless";
        default: return "Unknown";
    }
}

/**
 * @brief Texture compression format
 */
enum class StreamingFormat : u32 {
    /// Uncompressed RGBA
    RGBA8 = 0,
    
    /// BC1/DXT1 (RGB, 1-bit alpha)
    BC1 = 1,
    
    /// BC3/DXT5 (RGBA with alpha)
    BC3 = 2,
    
    /// BC4 (single channel)
    BC4 = 3,
    
    /// BC5 (two channels, normal maps)
    BC5 = 4,
    
    /// BC6H (HDR)
    BC6H = 5,
    
    /// BC7 (high quality RGBA)
    BC7 = 6,
    
    /// ASTC 4x4 (mobile)
    ASTC_4x4 = 7,
    
    /// ASTC 6x6 (mobile, higher compression)
    ASTC_6x6 = 8,
    
    /// ASTC 8x8 (mobile, max compression)
    ASTC_8x8 = 9,
    
    /// ETC2 (mobile fallback)
    ETC2 = 10,
    
    COUNT
};

/**
 * @brief Get streaming format name
 */
[[nodiscard]] constexpr const char* getStreamingFormatName(StreamingFormat format) noexcept {
    switch (format) {
        case StreamingFormat::RGBA8: return "RGBA8";
        case StreamingFormat::BC1: return "BC1";
        case StreamingFormat::BC3: return "BC3";
        case StreamingFormat::BC4: return "BC4";
        case StreamingFormat::BC5: return "BC5";
        case StreamingFormat::BC6H: return "BC6H";
        case StreamingFormat::BC7: return "BC7";
        case StreamingFormat::ASTC_4x4: return "ASTC 4x4";
        case StreamingFormat::ASTC_6x6: return "ASTC 6x6";
        case StreamingFormat::ASTC_8x8: return "ASTC 8x8";
        case StreamingFormat::ETC2: return "ETC2";
        default: return "Unknown";
    }
}

/**
 * @brief Get bytes per block for format
 */
[[nodiscard]] constexpr u32 getStreamingFormatBytesPerBlock(StreamingFormat format) noexcept {
    switch (format) {
        case StreamingFormat::RGBA8: return 4;
        case StreamingFormat::BC1: return 8;
        case StreamingFormat::BC3: return 16;
        case StreamingFormat::BC4: return 8;
        case StreamingFormat::BC5: return 16;
        case StreamingFormat::BC6H: return 16;
        case StreamingFormat::BC7: return 16;
        case StreamingFormat::ASTC_4x4: return 16;
        case StreamingFormat::ASTC_6x6: return 16;
        case StreamingFormat::ASTC_8x8: return 16;
        case StreamingFormat::ETC2: return 8;
        default: return 4;
    }
}

/**
 * @brief Get block size for format
 */
[[nodiscard]] constexpr u32 getStreamingFormatBlockSize(StreamingFormat format) noexcept {
    switch (format) {
        case StreamingFormat::RGBA8: return 1;
        case StreamingFormat::BC1:
        case StreamingFormat::BC3:
        case StreamingFormat::BC4:
        case StreamingFormat::BC5:
        case StreamingFormat::BC6H:
        case StreamingFormat::BC7:
        case StreamingFormat::ETC2:
            return 4;
        case StreamingFormat::ASTC_4x4: return 4;
        case StreamingFormat::ASTC_6x6: return 6;
        case StreamingFormat::ASTC_8x8: return 8;
        default: return 4;
    }
}

// =============================================================================
// Data Structures
// =============================================================================

/**
 * @brief Texture mip level information
 */
struct MipLevelInfo {
    /// Width at this mip level
    u32 width = 0;
    
    /// Height at this mip level
    u32 height = 0;
    
    /// Size in bytes
    u64 sizeBytes = 0;
    
    /// Offset in file
    u64 fileOffset = 0;
    
    /// Current stream state
    StreamState state = StreamState::Unloaded;
    
    /// Last frame this mip was requested
    u32 lastRequestFrame = 0;
    
    /// Last frame this mip was used
    u32 lastUsedFrame = 0;
    
    /**
     * @brief Calculate size for given format
     */
    void calculateSize(StreamingFormat format) noexcept {
        u32 blockSize = getStreamingFormatBlockSize(format);
        u32 bytesPerBlock = getStreamingFormatBytesPerBlock(format);
        
        u32 blocksX = (width + blockSize - 1) / blockSize;
        u32 blocksY = (height + blockSize - 1) / blockSize;
        
        sizeBytes = static_cast<u64>(blocksX) * blocksY * bytesPerBlock;
    }
};

/**
 * @brief Streaming texture descriptor
 */
struct StreamingTextureDesc {
    /// Texture handle/ID
    u32 handle = 0;
    
    /// Full texture path
    std::string path;
    
    /// Base width (mip 0)
    u32 width = 0;
    
    /// Base height (mip 0)
    u32 height = 0;
    
    /// Number of mip levels
    u32 mipLevels = 0;
    
    /// Compression format
    StreamingFormat format = StreamingFormat::BC7;
    
    /// Total size on disk (all mips)
    u64 totalSizeBytes = 0;
    
    /// Mip level information
    std::vector<MipLevelInfo> mips;
    
    /// Minimum required mip (always resident)
    u32 minResidentMip = 0;
    
    /// Current highest loaded mip (0 = highest quality)
    u32 currentMip = 0;
    
    /// Desired mip based on screen coverage
    u32 desiredMip = 0;
    
    /// Priority score for streaming
    f32 priorityScore = 0.0f;
    
    /// Is texture currently visible
    bool isVisible = false;
    
    /// Screen coverage (0-1)
    f32 screenCoverage = 0.0f;
    
    /**
     * @brief Calculate total memory for loaded mips
     */
    [[nodiscard]] u64 getLoadedMemory() const noexcept {
        u64 total = 0;
        for (const auto& mip : mips) {
            if (mip.state == StreamState::Resident) {
                total += mip.sizeBytes;
            }
        }
        return total;
    }
    
    /**
     * @brief Calculate memory needed for desired mip
     */
    [[nodiscard]] u64 getDesiredMemory() const noexcept {
        u64 total = 0;
        for (u32 i = desiredMip; i < mipLevels; ++i) {
            total += mips[i].sizeBytes;
        }
        return total;
    }
    
    /**
     * @brief Check if fully loaded to desired quality
     */
    [[nodiscard]] bool isFullyLoaded() const noexcept {
        return currentMip <= desiredMip;
    }
};

/**
 * @brief Stream request for a texture mip
 */
struct StreamRequest {
    /// Texture handle
    u32 textureHandle = 0;
    
    /// Mip level to load
    u32 mipLevel = 0;
    
    /// Priority
    StreamPriority priority = StreamPriority::Normal;
    
    /// Frame when requested
    u32 requestFrame = 0;
    
    /// Urgency score (for sorting)
    f32 urgency = 0.0f;
    
    /**
     * @brief Compare for priority queue (higher urgency first)
     */
    [[nodiscard]] bool operator<(const StreamRequest& other) const noexcept {
        if (priority != other.priority) {
            return static_cast<u32>(priority) < static_cast<u32>(other.priority);
        }
        return urgency < other.urgency;
    }
};

/**
 * @brief Virtual texture tile
 */
struct VirtualTextureTile {
    /// Tile X coordinate
    u32 tileX = 0;
    
    /// Tile Y coordinate
    u32 tileY = 0;
    
    /// Mip level
    u32 mipLevel = 0;
    
    /// Page table index
    u32 pageIndex = 0;
    
    /// Physical memory offset
    u64 physicalOffset = 0;
    
    /// Tile state
    StreamState state = StreamState::Unloaded;
    
    /// Last used frame
    u32 lastUsedFrame = 0;
    
    /**
     * @brief Get tile hash for map lookup
     */
    [[nodiscard]] u64 getHash() const noexcept {
        return (static_cast<u64>(mipLevel) << 48) | 
               (static_cast<u64>(tileY) << 24) | 
               static_cast<u64>(tileX);
    }
};

/**
 * @brief Feedback buffer pixel (from GPU)
 */
struct FeedbackPixel {
    /// Texture ID
    u16 textureId;
    
    /// Desired mip level
    u8 mipLevel;
    
    /// Flags (visible, etc.)
    u8 flags;
};

/**
 * @brief Streaming budget allocation
 */
struct StreamingBudget {
    /// Total memory budget (bytes)
    u64 totalBudget = TextureStreamingConfig::DEFAULT_MEMORY_BUDGET_MB * 1024 * 1024;
    
    /// Currently used memory (bytes)
    u64 usedMemory = 0;
    
    /// Reserved memory for loading (bytes)
    u64 reservedMemory = 0;
    
    /// High watermark for eviction trigger
    f32 evictionWatermark = 0.9f;
    
    /// Low watermark for eviction stop
    f32 targetWatermark = 0.8f;
    
    /**
     * @brief Get available memory
     */
    [[nodiscard]] u64 getAvailable() const noexcept {
        return (usedMemory + reservedMemory < totalBudget) 
             ? totalBudget - usedMemory - reservedMemory 
             : 0;
    }
    
    /**
     * @brief Get usage percentage
     */
    [[nodiscard]] f32 getUsagePercent() const noexcept {
        if (totalBudget == 0) return 1.0f;
        return static_cast<f32>(usedMemory) / static_cast<f32>(totalBudget);
    }
    
    /**
     * @brief Check if eviction needed
     */
    [[nodiscard]] bool needsEviction() const noexcept {
        return getUsagePercent() >= evictionWatermark;
    }
    
    /**
     * @brief Check if eviction can stop
     */
    [[nodiscard]] bool evictionComplete() const noexcept {
        return getUsagePercent() <= targetWatermark;
    }
    
    /**
     * @brief Can allocate given size
     */
    [[nodiscard]] bool canAllocate(u64 size) const noexcept {
        return usedMemory + reservedMemory + size <= totalBudget;
    }
};

/**
 * @brief Streaming statistics
 */
struct StreamingStats {
    /// Total textures being tracked
    u32 totalTextures = 0;
    
    /// Textures fully loaded
    u32 fullyLoadedTextures = 0;
    
    /// Textures with pending loads
    u32 pendingTextures = 0;
    
    /// Current memory usage (bytes)
    u64 memoryUsage = 0;
    
    /// Memory budget (bytes)
    u64 memoryBudget = 0;
    
    /// Requests this frame
    u32 requestsThisFrame = 0;
    
    /// Loads completed this frame
    u32 loadsThisFrame = 0;
    
    /// Evictions this frame
    u32 evictionsThisFrame = 0;
    
    /// Bytes loaded this frame
    u64 bytesLoadedThisFrame = 0;
    
    /// Average load time (ms)
    f32 avgLoadTimeMs = 0.0f;
    
    /// IO bandwidth utilization (0-1)
    f32 ioBandwidthUtilization = 0.0f;
    
    /// Visible texture pop-ins
    u32 popIns = 0;
    
    /**
     * @brief Reset per-frame stats
     */
    void resetFrameStats() noexcept {
        requestsThisFrame = 0;
        loadsThisFrame = 0;
        evictionsThisFrame = 0;
        bytesLoadedThisFrame = 0;
        popIns = 0;
    }
    
    /**
     * @brief Get memory usage percentage
     */
    [[nodiscard]] f32 getMemoryUsagePercent() const noexcept {
        if (memoryBudget == 0) return 0.0f;
        return static_cast<f32>(memoryUsage) / static_cast<f32>(memoryBudget) * 100.0f;
    }
};

// =============================================================================
// Texture Streaming Manager
// =============================================================================

/**
 * @brief Texture streaming manager
 * 
 * Manages texture streaming, virtual texturing, and memory budget.
 */
class TextureStreamingManager {
public:
    /**
     * @brief Default constructor
     */
    TextureStreamingManager() = default;
    
    /**
     * @brief Initialize with memory budget
     */
    explicit TextureStreamingManager(u64 budgetMB) {
        m_budget.totalBudget = std::clamp(budgetMB, 
            TextureStreamingConfig::MIN_MEMORY_BUDGET_MB,
            TextureStreamingConfig::MAX_MEMORY_BUDGET_MB) * 1024 * 1024;
    }
    
    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------
    
    /**
     * @brief Set memory budget
     */
    void setMemoryBudget(u64 budgetMB) noexcept {
        m_budget.totalBudget = std::clamp(budgetMB,
            TextureStreamingConfig::MIN_MEMORY_BUDGET_MB,
            TextureStreamingConfig::MAX_MEMORY_BUDGET_MB) * 1024 * 1024;
    }
    
    /**
     * @brief Get memory budget
     */
    [[nodiscard]] u64 getMemoryBudgetMB() const noexcept {
        return m_budget.totalBudget / (1024 * 1024);
    }
    
    /**
     * @brief Set virtual texture mode
     */
    void setVirtualTextureMode(VirtualTextureMode mode) noexcept {
        m_virtualTextureMode = mode;
    }
    
    /**
     * @brief Get virtual texture mode
     */
    [[nodiscard]] VirtualTextureMode getVirtualTextureMode() const noexcept {
        return m_virtualTextureMode;
    }
    
    /**
     * @brief Set tile size for virtual texturing
     */
    void setTileSize(u32 size) noexcept {
        m_tileSize = std::clamp(size, 64u, TextureStreamingConfig::MAX_TILE_SIZE);
    }
    
    /**
     * @brief Get tile size
     */
    [[nodiscard]] u32 getTileSize() const noexcept {
        return m_tileSize;
    }
    
    // -------------------------------------------------------------------------
    // Texture Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Register a texture for streaming
     */
    u32 registerTexture(const StreamingTextureDesc& desc) noexcept {
        u32 handle = m_nextHandle++;
        
        StreamingTextureDesc texDesc = desc;
        texDesc.handle = handle;
        
        // Calculate mip sizes if not provided
        if (texDesc.mips.empty()) {
            calculateMipChain(texDesc);
        }
        
        m_textures[handle] = texDesc;
        m_stats.totalTextures++;
        
        return handle;
    }
    
    /**
     * @brief Unregister a texture
     */
    void unregisterTexture(u32 handle) noexcept {
        auto it = m_textures.find(handle);
        if (it != m_textures.end()) {
            // Free memory
            m_budget.usedMemory -= it->second.getLoadedMemory();
            m_textures.erase(it);
            m_stats.totalTextures--;
        }
    }
    
    /**
     * @brief Get texture descriptor
     */
    [[nodiscard]] StreamingTextureDesc* getTexture(u32 handle) noexcept {
        auto it = m_textures.find(handle);
        return it != m_textures.end() ? &it->second : nullptr;
    }
    
    /**
     * @brief Get texture descriptor (const)
     */
    [[nodiscard]] const StreamingTextureDesc* getTexture(u32 handle) const noexcept {
        auto it = m_textures.find(handle);
        return it != m_textures.end() ? &it->second : nullptr;
    }
    
    // -------------------------------------------------------------------------
    // Streaming Operations
    // -------------------------------------------------------------------------
    
    /**
     * @brief Update texture usage from feedback buffer
     */
    void updateFromFeedback(const std::vector<FeedbackPixel>& feedback, u32 currentFrame) noexcept {
        // Reset visibility
        for (auto& [handle, tex] : m_textures) {
            tex.isVisible = false;
            tex.screenCoverage = 0.0f;
        }
        
        // Process feedback
        std::unordered_map<u32, u32> mipRequests;
        std::unordered_map<u32, u32> coverageCounts;
        
        for (const auto& pixel : feedback) {
            auto it = m_textures.find(pixel.textureId);
            if (it != m_textures.end()) {
                it->second.isVisible = true;
                
                // Track minimum desired mip
                auto& minMip = mipRequests[pixel.textureId];
                minMip = std::min(minMip, static_cast<u32>(pixel.mipLevel));
                
                coverageCounts[pixel.textureId]++;
            }
        }
        
        // Update desired mips and coverage
        f32 totalPixels = static_cast<f32>(feedback.size());
        for (auto& [handle, count] : coverageCounts) {
            auto& tex = m_textures[handle];
            tex.desiredMip = mipRequests[handle];
            tex.screenCoverage = static_cast<f32>(count) / totalPixels;
            
            // Update mip last used frames
            for (u32 i = tex.desiredMip; i < tex.mipLevels; ++i) {
                tex.mips[i].lastUsedFrame = currentFrame;
            }
        }
    }
    
    /**
     * @brief Request texture mip load
     */
    void requestMipLoad(u32 textureHandle, u32 mipLevel, StreamPriority priority = StreamPriority::Normal) noexcept {
        auto* tex = getTexture(textureHandle);
        if (!tex || mipLevel >= tex->mipLevels) {
            return;
        }
        
        auto& mip = tex->mips[mipLevel];
        if (mip.state != StreamState::Unloaded) {
            return;
        }
        
        // Check budget
        if (!m_budget.canAllocate(mip.sizeBytes)) {
            return;
        }
        
        StreamRequest request;
        request.textureHandle = textureHandle;
        request.mipLevel = mipLevel;
        request.priority = priority;
        request.requestFrame = m_currentFrame;
        request.urgency = calculateUrgency(*tex, mipLevel);
        
        m_pendingRequests.push(request);
        m_budget.reservedMemory += mip.sizeBytes;
        mip.state = StreamState::Queued;
        mip.lastRequestFrame = m_currentFrame;
        
        m_stats.requestsThisFrame++;
    }
    
    /**
     * @brief Process pending stream requests
     */
    void processRequests(u32 maxRequests = 4) noexcept {
        u32 processed = 0;
        
        while (!m_pendingRequests.empty() && processed < maxRequests) {
            StreamRequest request = m_pendingRequests.top();
            m_pendingRequests.pop();
            
            auto* tex = getTexture(request.textureHandle);
            if (!tex) continue;
            
            auto& mip = tex->mips[request.mipLevel];
            if (mip.state != StreamState::Queued) continue;
            
            // Simulate loading (in real implementation, this would be async)
            mip.state = StreamState::Resident;
            m_budget.usedMemory += mip.sizeBytes;
            m_budget.reservedMemory -= mip.sizeBytes;
            
            // Update current mip
            if (request.mipLevel < tex->currentMip) {
                tex->currentMip = request.mipLevel;
            }
            
            m_stats.loadsThisFrame++;
            m_stats.bytesLoadedThisFrame += mip.sizeBytes;
            processed++;
        }
    }
    
    /**
     * @brief Evict least recently used mips
     */
    void evictLRU(u32 maxEvictions = 8) noexcept {
        if (!m_budget.needsEviction()) {
            return;
        }
        
        // Build eviction candidates
        struct EvictionCandidate {
            u32 textureHandle;
            u32 mipLevel;
            u32 lastUsedFrame;
            u64 size;
        };
        
        std::vector<EvictionCandidate> candidates;
        
        for (auto& [handle, tex] : m_textures) {
            // Can't evict minimum resident mip
            for (u32 i = 0; i < tex.minResidentMip && i < tex.mipLevels; ++i) {
                auto& mip = tex.mips[i];
                if (mip.state == StreamState::Resident && 
                    m_currentFrame - mip.lastUsedFrame > TextureStreamingConfig::EVICTION_HYSTERESIS_FRAMES) {
                    
                    candidates.push_back({handle, i, mip.lastUsedFrame, mip.sizeBytes});
                }
            }
        }
        
        // Sort by LRU
        std::sort(candidates.begin(), candidates.end(),
            [](const EvictionCandidate& a, const EvictionCandidate& b) {
                return a.lastUsedFrame < b.lastUsedFrame;
            });
        
        // Evict until under budget
        u32 evicted = 0;
        for (const auto& candidate : candidates) {
            if (m_budget.evictionComplete() || evicted >= maxEvictions) {
                break;
            }
            
            auto* tex = getTexture(candidate.textureHandle);
            if (!tex) continue;
            
            auto& mip = tex->mips[candidate.mipLevel];
            mip.state = StreamState::Unloaded;
            m_budget.usedMemory -= mip.sizeBytes;
            
            // Update current mip if needed
            if (candidate.mipLevel == tex->currentMip) {
                // Find next loaded mip
                for (u32 i = candidate.mipLevel + 1; i < tex->mipLevels; ++i) {
                    if (tex->mips[i].state == StreamState::Resident) {
                        tex->currentMip = i;
                        break;
                    }
                }
            }
            
            m_stats.evictionsThisFrame++;
            evicted++;
        }
    }
    
    /**
     * @brief Begin frame
     */
    void beginFrame(u32 frameNumber) noexcept {
        m_currentFrame = frameNumber;
        m_stats.resetFrameStats();
    }
    
    /**
     * @brief End frame and update stats
     */
    void endFrame() noexcept {
        // Update statistics
        m_stats.memoryUsage = m_budget.usedMemory;
        m_stats.memoryBudget = m_budget.totalBudget;
        
        u32 fullyLoaded = 0;
        u32 pending = 0;
        
        for (const auto& [handle, tex] : m_textures) {
            if (tex.isFullyLoaded()) {
                fullyLoaded++;
            }
            if (tex.currentMip > tex.desiredMip) {
                pending++;
            }
        }
        
        m_stats.fullyLoadedTextures = fullyLoaded;
        m_stats.pendingTextures = pending;
        
        // Auto-evict if needed
        evictLRU();
    }
    
    // -------------------------------------------------------------------------
    // Statistics
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get streaming statistics
     */
    [[nodiscard]] const StreamingStats& getStats() const noexcept {
        return m_stats;
    }
    
    /**
     * @brief Get budget information
     */
    [[nodiscard]] const StreamingBudget& getBudget() const noexcept {
        return m_budget;
    }
    
    /**
     * @brief Get current frame number
     */
    [[nodiscard]] u32 getCurrentFrame() const noexcept {
        return m_currentFrame;
    }
    
    /**
     * @brief Get texture count
     */
    [[nodiscard]] u32 getTextureCount() const noexcept {
        return static_cast<u32>(m_textures.size());
    }

private:
    /**
     * @brief Calculate mip chain for texture
     */
    void calculateMipChain(StreamingTextureDesc& desc) noexcept {
        desc.mips.clear();
        desc.totalSizeBytes = 0;
        
        u32 w = desc.width;
        u32 h = desc.height;
        u64 offset = 0;
        
        // Calculate mip count if not specified
        if (desc.mipLevels == 0) {
            desc.mipLevels = 1 + static_cast<u32>(std::floor(std::log2(std::max(w, h))));
            desc.mipLevels = std::min(desc.mipLevels, TextureStreamingConfig::MAX_MIP_LEVELS);
        }
        
        for (u32 i = 0; i < desc.mipLevels; ++i) {
            MipLevelInfo mip;
            mip.width = std::max(1u, w);
            mip.height = std::max(1u, h);
            mip.fileOffset = offset;
            mip.calculateSize(desc.format);
            
            desc.totalSizeBytes += mip.sizeBytes;
            offset += mip.sizeBytes;
            
            desc.mips.push_back(mip);
            
            w /= 2;
            h /= 2;
        }
        
        // Set minimum resident mip (last few mips)
        desc.minResidentMip = desc.mipLevels - std::min(3u, desc.mipLevels);
        desc.currentMip = desc.mipLevels - 1; // Start with lowest quality
        desc.desiredMip = desc.mipLevels - 1;
    }
    
    /**
     * @brief Calculate urgency for a mip load
     */
    [[nodiscard]] f32 calculateUrgency(const StreamingTextureDesc& tex, u32 mipLevel) const noexcept {
        f32 urgency = 0.0f;
        
        // Higher urgency for lower mip levels (higher quality)
        urgency += static_cast<f32>(tex.mipLevels - mipLevel);
        
        // Boost for visible textures
        if (tex.isVisible) {
            urgency *= TextureStreamingConfig::VISIBLE_PRIORITY_BOOST;
        }
        
        // Boost based on screen coverage
        urgency += tex.screenCoverage * 10.0f;
        
        // Penalize if far from desired mip
        f32 mipDiff = static_cast<f32>(tex.currentMip) - static_cast<f32>(mipLevel);
        urgency += mipDiff;
        
        return urgency;
    }
    
    // Texture storage
    std::unordered_map<u32, StreamingTextureDesc> m_textures;
    u32 m_nextHandle = 1;
    
    // Request queue
    std::priority_queue<StreamRequest> m_pendingRequests;
    
    // Budget tracking
    StreamingBudget m_budget;
    
    // Configuration
    VirtualTextureMode m_virtualTextureMode = VirtualTextureMode::Disabled;
    u32 m_tileSize = TextureStreamingConfig::DEFAULT_TILE_SIZE;
    
    // Frame state
    u32 m_currentFrame = 0;
    
    // Statistics
    StreamingStats m_stats;
};

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Calculate desired mip level based on screen pixels per texel
 * @param textureWidth Texture width
 * @param textureHeight Texture height
 * @param screenPixels Number of screen pixels covered
 * @return Desired mip level (0 = highest quality)
 */
[[nodiscard]] inline u32 calculateDesiredMip(
    u32 textureWidth, u32 textureHeight, f32 screenPixels) noexcept {
    
    if (screenPixels <= 0.0f) {
        return TextureStreamingConfig::MAX_MIP_LEVELS - 1;
    }
    
    f32 texels = static_cast<f32>(textureWidth * textureHeight);
    f32 ratio = texels / screenPixels;
    
    // log2 of ratio gives us the mip level
    if (ratio <= 1.0f) {
        return 0; // Full resolution
    }
    
    return static_cast<u32>(std::floor(std::log2(ratio)));
}

/**
 * @brief Calculate memory size for a full mip chain
 * @param width Base width
 * @param height Base height
 * @param format Compression format
 * @return Total memory in bytes
 */
[[nodiscard]] inline u64 calculateFullMipChainSize(
    u32 width, u32 height, StreamingFormat format) noexcept {
    
    u64 total = 0;
    u32 w = width;
    u32 h = height;
    
    while (w > 0 && h > 0) {
        u32 blockSize = getStreamingFormatBlockSize(format);
        u32 bytesPerBlock = getStreamingFormatBytesPerBlock(format);
        
        u32 blocksX = (w + blockSize - 1) / blockSize;
        u32 blocksY = (h + blockSize - 1) / blockSize;
        
        total += static_cast<u64>(blocksX) * blocksY * bytesPerBlock;
        
        w /= 2;
        h /= 2;
    }
    
    return total;
}

} // namespace nova
