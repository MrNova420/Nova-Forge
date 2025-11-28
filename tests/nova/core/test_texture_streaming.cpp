/**
 * @file test_texture_streaming.cpp
 * @brief Comprehensive tests for the texture streaming system
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/texture_streaming.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// TextureStreamingConfig Constants Tests
// =============================================================================

TEST_CASE("TextureStreamingConfig constants are valid", "[streaming][config]") {
    SECTION("Memory budget limits") {
        REQUIRE(TextureStreamingConfig::DEFAULT_MEMORY_BUDGET_MB == 512);
        REQUIRE(TextureStreamingConfig::MIN_MEMORY_BUDGET_MB == 128);
        REQUIRE(TextureStreamingConfig::MAX_MEMORY_BUDGET_MB == 8192);
        REQUIRE(TextureStreamingConfig::MIN_MEMORY_BUDGET_MB < TextureStreamingConfig::DEFAULT_MEMORY_BUDGET_MB);
        REQUIRE(TextureStreamingConfig::DEFAULT_MEMORY_BUDGET_MB < TextureStreamingConfig::MAX_MEMORY_BUDGET_MB);
    }
    
    SECTION("Tile settings") {
        REQUIRE(TextureStreamingConfig::DEFAULT_TILE_SIZE == 128);
        REQUIRE(TextureStreamingConfig::MAX_TILE_SIZE == 512);
        REQUIRE(TextureStreamingConfig::DEFAULT_TILE_SIZE <= TextureStreamingConfig::MAX_TILE_SIZE);
    }
    
    SECTION("Mip and request limits") {
        REQUIRE(TextureStreamingConfig::MAX_MIP_LEVELS == 16);
        REQUIRE(TextureStreamingConfig::MAX_PENDING_REQUESTS == 256);
        REQUIRE(TextureStreamingConfig::MAX_TRACKED_TEXTURES == 8192);
    }
    
    SECTION("Feedback and eviction settings") {
        REQUIRE(TextureStreamingConfig::FEEDBACK_RESOLUTION_DIVISOR == 4);
        REQUIRE(TextureStreamingConfig::EVICTION_HYSTERESIS_FRAMES == 30);
        REQUIRE(TextureStreamingConfig::VISIBLE_PRIORITY_BOOST == Approx(2.0f));
    }
}

// =============================================================================
// StreamPriority Enum Tests
// =============================================================================

TEST_CASE("StreamPriority enum values", "[streaming][priority]") {
    SECTION("Enum values are ordered") {
        REQUIRE(static_cast<u32>(StreamPriority::Background) == 0);
        REQUIRE(static_cast<u32>(StreamPriority::Normal) == 1);
        REQUIRE(static_cast<u32>(StreamPriority::High) == 2);
        REQUIRE(static_cast<u32>(StreamPriority::Urgent) == 3);
        REQUIRE(static_cast<u32>(StreamPriority::Critical) == 4);
        REQUIRE(static_cast<u32>(StreamPriority::COUNT) == 5);
    }
    
    SECTION("Priority names are correct") {
        REQUIRE(std::string(getStreamPriorityName(StreamPriority::Background)) == "Background");
        REQUIRE(std::string(getStreamPriorityName(StreamPriority::Normal)) == "Normal");
        REQUIRE(std::string(getStreamPriorityName(StreamPriority::High)) == "High");
        REQUIRE(std::string(getStreamPriorityName(StreamPriority::Urgent)) == "Urgent");
        REQUIRE(std::string(getStreamPriorityName(StreamPriority::Critical)) == "Critical");
    }
}

// =============================================================================
// StreamState Enum Tests
// =============================================================================

TEST_CASE("StreamState enum values", "[streaming][state]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(StreamState::Unloaded) == 0);
        REQUIRE(static_cast<u32>(StreamState::Queued) == 1);
        REQUIRE(static_cast<u32>(StreamState::Loading) == 2);
        REQUIRE(static_cast<u32>(StreamState::Uploading) == 3);
        REQUIRE(static_cast<u32>(StreamState::Resident) == 4);
        REQUIRE(static_cast<u32>(StreamState::Evicting) == 5);
        REQUIRE(static_cast<u32>(StreamState::Error) == 6);
        REQUIRE(static_cast<u32>(StreamState::COUNT) == 7);
    }
    
    SECTION("State names are correct") {
        REQUIRE(std::string(getStreamStateName(StreamState::Unloaded)) == "Unloaded");
        REQUIRE(std::string(getStreamStateName(StreamState::Queued)) == "Queued");
        REQUIRE(std::string(getStreamStateName(StreamState::Loading)) == "Loading");
        REQUIRE(std::string(getStreamStateName(StreamState::Uploading)) == "Uploading");
        REQUIRE(std::string(getStreamStateName(StreamState::Resident)) == "Resident");
        REQUIRE(std::string(getStreamStateName(StreamState::Evicting)) == "Evicting");
        REQUIRE(std::string(getStreamStateName(StreamState::Error)) == "Error");
    }
}

// =============================================================================
// VirtualTextureMode Enum Tests
// =============================================================================

TEST_CASE("VirtualTextureMode enum values", "[streaming][virtual]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(VirtualTextureMode::Disabled) == 0);
        REQUIRE(static_cast<u32>(VirtualTextureMode::Software) == 1);
        REQUIRE(static_cast<u32>(VirtualTextureMode::HardwareSparse) == 2);
        REQUIRE(static_cast<u32>(VirtualTextureMode::Bindless) == 3);
        REQUIRE(static_cast<u32>(VirtualTextureMode::COUNT) == 4);
    }
    
    SECTION("Mode names are correct") {
        REQUIRE(std::string(getVirtualTextureModeName(VirtualTextureMode::Disabled)) == "Disabled");
        REQUIRE(std::string(getVirtualTextureModeName(VirtualTextureMode::Software)) == "Software");
        REQUIRE(std::string(getVirtualTextureModeName(VirtualTextureMode::HardwareSparse)) == "Hardware Sparse");
        REQUIRE(std::string(getVirtualTextureModeName(VirtualTextureMode::Bindless)) == "Bindless");
    }
}

// =============================================================================
// StreamingFormat Enum Tests
// =============================================================================

TEST_CASE("StreamingFormat enum values", "[streaming][format]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(StreamingFormat::RGBA8) == 0);
        REQUIRE(static_cast<u32>(StreamingFormat::BC1) == 1);
        REQUIRE(static_cast<u32>(StreamingFormat::BC3) == 2);
        REQUIRE(static_cast<u32>(StreamingFormat::BC4) == 3);
        REQUIRE(static_cast<u32>(StreamingFormat::BC5) == 4);
        REQUIRE(static_cast<u32>(StreamingFormat::BC6H) == 5);
        REQUIRE(static_cast<u32>(StreamingFormat::BC7) == 6);
        REQUIRE(static_cast<u32>(StreamingFormat::ASTC_4x4) == 7);
        REQUIRE(static_cast<u32>(StreamingFormat::ASTC_6x6) == 8);
        REQUIRE(static_cast<u32>(StreamingFormat::ASTC_8x8) == 9);
        REQUIRE(static_cast<u32>(StreamingFormat::ETC2) == 10);
        REQUIRE(static_cast<u32>(StreamingFormat::COUNT) == 11);
    }
    
    SECTION("Format names are correct") {
        REQUIRE(std::string(getStreamingFormatName(StreamingFormat::RGBA8)) == "RGBA8");
        REQUIRE(std::string(getStreamingFormatName(StreamingFormat::BC1)) == "BC1");
        REQUIRE(std::string(getStreamingFormatName(StreamingFormat::BC7)) == "BC7");
        REQUIRE(std::string(getStreamingFormatName(StreamingFormat::ASTC_4x4)) == "ASTC 4x4");
    }
    
    SECTION("Bytes per block are correct") {
        REQUIRE(getStreamingFormatBytesPerBlock(StreamingFormat::RGBA8) == 4);
        REQUIRE(getStreamingFormatBytesPerBlock(StreamingFormat::BC1) == 8);
        REQUIRE(getStreamingFormatBytesPerBlock(StreamingFormat::BC3) == 16);
        REQUIRE(getStreamingFormatBytesPerBlock(StreamingFormat::BC4) == 8);
        REQUIRE(getStreamingFormatBytesPerBlock(StreamingFormat::BC5) == 16);
        REQUIRE(getStreamingFormatBytesPerBlock(StreamingFormat::BC7) == 16);
        REQUIRE(getStreamingFormatBytesPerBlock(StreamingFormat::ASTC_4x4) == 16);
    }
    
    SECTION("Block sizes are correct") {
        REQUIRE(getStreamingFormatBlockSize(StreamingFormat::RGBA8) == 1);
        REQUIRE(getStreamingFormatBlockSize(StreamingFormat::BC1) == 4);
        REQUIRE(getStreamingFormatBlockSize(StreamingFormat::BC7) == 4);
        REQUIRE(getStreamingFormatBlockSize(StreamingFormat::ASTC_4x4) == 4);
        REQUIRE(getStreamingFormatBlockSize(StreamingFormat::ASTC_6x6) == 6);
        REQUIRE(getStreamingFormatBlockSize(StreamingFormat::ASTC_8x8) == 8);
    }
}

// =============================================================================
// MipLevelInfo Tests
// =============================================================================

TEST_CASE("MipLevelInfo functionality", "[streaming][mip]") {
    SECTION("Default values") {
        MipLevelInfo mip;
        REQUIRE(mip.width == 0);
        REQUIRE(mip.height == 0);
        REQUIRE(mip.sizeBytes == 0);
        REQUIRE(mip.state == StreamState::Unloaded);
    }
    
    SECTION("Calculate size for RGBA8") {
        MipLevelInfo mip;
        mip.width = 256;
        mip.height = 256;
        mip.calculateSize(StreamingFormat::RGBA8);
        
        // 256 * 256 * 4 bytes = 262144
        REQUIRE(mip.sizeBytes == 262144);
    }
    
    SECTION("Calculate size for BC7") {
        MipLevelInfo mip;
        mip.width = 256;
        mip.height = 256;
        mip.calculateSize(StreamingFormat::BC7);
        
        // 64 blocks * 64 blocks * 16 bytes = 65536
        REQUIRE(mip.sizeBytes == 65536);
    }
    
    SECTION("Calculate size for non-power-of-two") {
        MipLevelInfo mip;
        mip.width = 100;
        mip.height = 100;
        mip.calculateSize(StreamingFormat::BC7);
        
        // ceil(100/4) * ceil(100/4) * 16 = 25 * 25 * 16 = 10000
        REQUIRE(mip.sizeBytes == 10000);
    }
}

// =============================================================================
// StreamingTextureDesc Tests
// =============================================================================

TEST_CASE("StreamingTextureDesc functionality", "[streaming][texture]") {
    SECTION("Default values") {
        StreamingTextureDesc desc;
        REQUIRE(desc.handle == 0);
        REQUIRE(desc.width == 0);
        REQUIRE(desc.height == 0);
        REQUIRE(desc.mipLevels == 0);
        REQUIRE(desc.format == StreamingFormat::BC7);
        REQUIRE(desc.isVisible == false);
    }
    
    SECTION("Get loaded memory") {
        StreamingTextureDesc desc;
        desc.mipLevels = 3;
        desc.mips.resize(3);
        
        desc.mips[0].sizeBytes = 1000;
        desc.mips[0].state = StreamState::Resident;
        
        desc.mips[1].sizeBytes = 500;
        desc.mips[1].state = StreamState::Resident;
        
        desc.mips[2].sizeBytes = 250;
        desc.mips[2].state = StreamState::Unloaded;
        
        REQUIRE(desc.getLoadedMemory() == 1500);
    }
    
    SECTION("Get desired memory") {
        StreamingTextureDesc desc;
        desc.mipLevels = 4;
        desc.desiredMip = 1;
        desc.mips.resize(4);
        
        desc.mips[0].sizeBytes = 1000;
        desc.mips[1].sizeBytes = 500;
        desc.mips[2].sizeBytes = 250;
        desc.mips[3].sizeBytes = 125;
        
        // From mip 1 onwards: 500 + 250 + 125 = 875
        REQUIRE(desc.getDesiredMemory() == 875);
    }
    
    SECTION("Is fully loaded check") {
        StreamingTextureDesc desc;
        desc.currentMip = 2;
        desc.desiredMip = 2;
        REQUIRE(desc.isFullyLoaded() == true);
        
        desc.desiredMip = 1;
        REQUIRE(desc.isFullyLoaded() == false);
        
        desc.currentMip = 0;
        REQUIRE(desc.isFullyLoaded() == true);
    }
}

// =============================================================================
// StreamRequest Tests
// =============================================================================

TEST_CASE("StreamRequest comparison", "[streaming][request]") {
    SECTION("Priority comparison") {
        StreamRequest lowPriority;
        lowPriority.priority = StreamPriority::Normal;
        lowPriority.urgency = 10.0f;
        
        StreamRequest highPriority;
        highPriority.priority = StreamPriority::Urgent;
        highPriority.urgency = 5.0f;
        
        // Higher priority should be "greater" (for max-heap)
        REQUIRE(lowPriority < highPriority);
    }
    
    SECTION("Urgency comparison within same priority") {
        StreamRequest lowUrgency;
        lowUrgency.priority = StreamPriority::Normal;
        lowUrgency.urgency = 5.0f;
        
        StreamRequest highUrgency;
        highUrgency.priority = StreamPriority::Normal;
        highUrgency.urgency = 10.0f;
        
        REQUIRE(lowUrgency < highUrgency);
    }
}

// =============================================================================
// VirtualTextureTile Tests
// =============================================================================

TEST_CASE("VirtualTextureTile functionality", "[streaming][tile]") {
    SECTION("Default values") {
        VirtualTextureTile tile;
        REQUIRE(tile.tileX == 0);
        REQUIRE(tile.tileY == 0);
        REQUIRE(tile.mipLevel == 0);
        REQUIRE(tile.state == StreamState::Unloaded);
    }
    
    SECTION("Hash calculation") {
        VirtualTextureTile tile1;
        tile1.tileX = 5;
        tile1.tileY = 10;
        tile1.mipLevel = 2;
        
        VirtualTextureTile tile2;
        tile2.tileX = 5;
        tile2.tileY = 10;
        tile2.mipLevel = 2;
        
        REQUIRE(tile1.getHash() == tile2.getHash());
        
        tile2.mipLevel = 3;
        REQUIRE(tile1.getHash() != tile2.getHash());
    }
}

// =============================================================================
// StreamingBudget Tests
// =============================================================================

TEST_CASE("StreamingBudget functionality", "[streaming][budget]") {
    SECTION("Default values") {
        StreamingBudget budget;
        REQUIRE(budget.totalBudget == TextureStreamingConfig::DEFAULT_MEMORY_BUDGET_MB * 1024 * 1024);
        REQUIRE(budget.usedMemory == 0);
        REQUIRE(budget.reservedMemory == 0);
        REQUIRE(budget.evictionWatermark == Approx(0.9f));
        REQUIRE(budget.targetWatermark == Approx(0.8f));
    }
    
    SECTION("Get available") {
        StreamingBudget budget;
        budget.totalBudget = 1000;
        budget.usedMemory = 300;
        budget.reservedMemory = 200;
        
        REQUIRE(budget.getAvailable() == 500);
    }
    
    SECTION("Get available when over budget") {
        StreamingBudget budget;
        budget.totalBudget = 1000;
        budget.usedMemory = 800;
        budget.reservedMemory = 300;
        
        REQUIRE(budget.getAvailable() == 0);
    }
    
    SECTION("Get usage percent") {
        StreamingBudget budget;
        budget.totalBudget = 1000;
        budget.usedMemory = 500;
        
        REQUIRE(budget.getUsagePercent() == Approx(0.5f));
    }
    
    SECTION("Needs eviction check") {
        StreamingBudget budget;
        budget.totalBudget = 1000;
        budget.evictionWatermark = 0.9f;
        
        budget.usedMemory = 899;
        REQUIRE(budget.needsEviction() == false);
        
        budget.usedMemory = 900;
        REQUIRE(budget.needsEviction() == true);
    }
    
    SECTION("Eviction complete check") {
        StreamingBudget budget;
        budget.totalBudget = 1000;
        budget.targetWatermark = 0.8f;
        
        budget.usedMemory = 850;
        REQUIRE(budget.evictionComplete() == false);
        
        budget.usedMemory = 800;
        REQUIRE(budget.evictionComplete() == true);
    }
    
    SECTION("Can allocate check") {
        StreamingBudget budget;
        budget.totalBudget = 1000;
        budget.usedMemory = 500;
        budget.reservedMemory = 300;
        
        REQUIRE(budget.canAllocate(200) == true);
        REQUIRE(budget.canAllocate(201) == false);
    }
}

// =============================================================================
// StreamingStats Tests
// =============================================================================

TEST_CASE("StreamingStats functionality", "[streaming][stats]") {
    SECTION("Reset frame stats") {
        StreamingStats stats;
        stats.requestsThisFrame = 10;
        stats.loadsThisFrame = 5;
        stats.evictionsThisFrame = 2;
        stats.bytesLoadedThisFrame = 1000000;
        stats.popIns = 3;
        
        stats.resetFrameStats();
        
        REQUIRE(stats.requestsThisFrame == 0);
        REQUIRE(stats.loadsThisFrame == 0);
        REQUIRE(stats.evictionsThisFrame == 0);
        REQUIRE(stats.bytesLoadedThisFrame == 0);
        REQUIRE(stats.popIns == 0);
    }
    
    SECTION("Get memory usage percent") {
        StreamingStats stats;
        stats.memoryUsage = 256 * 1024 * 1024; // 256 MB
        stats.memoryBudget = 512 * 1024 * 1024; // 512 MB
        
        REQUIRE(stats.getMemoryUsagePercent() == Approx(50.0f));
    }
}

// =============================================================================
// TextureStreamingManager Tests
// =============================================================================

TEST_CASE("TextureStreamingManager construction", "[streaming][manager]") {
    SECTION("Default construction") {
        TextureStreamingManager manager;
        REQUIRE(manager.getMemoryBudgetMB() == TextureStreamingConfig::DEFAULT_MEMORY_BUDGET_MB);
        REQUIRE(manager.getTextureCount() == 0);
    }
    
    SECTION("Construction with budget") {
        TextureStreamingManager manager(256);
        REQUIRE(manager.getMemoryBudgetMB() == 256);
    }
    
    SECTION("Budget clamping") {
        TextureStreamingManager lowBudget(64); // Below minimum
        REQUIRE(lowBudget.getMemoryBudgetMB() == TextureStreamingConfig::MIN_MEMORY_BUDGET_MB);
        
        TextureStreamingManager highBudget(16384); // Above maximum
        REQUIRE(highBudget.getMemoryBudgetMB() == TextureStreamingConfig::MAX_MEMORY_BUDGET_MB);
    }
}

TEST_CASE("TextureStreamingManager configuration", "[streaming][manager]") {
    TextureStreamingManager manager;
    
    SECTION("Set memory budget") {
        manager.setMemoryBudget(1024);
        REQUIRE(manager.getMemoryBudgetMB() == 1024);
    }
    
    SECTION("Set virtual texture mode") {
        manager.setVirtualTextureMode(VirtualTextureMode::HardwareSparse);
        REQUIRE(manager.getVirtualTextureMode() == VirtualTextureMode::HardwareSparse);
    }
    
    SECTION("Set tile size") {
        manager.setTileSize(256);
        REQUIRE(manager.getTileSize() == 256);
    }
    
    SECTION("Tile size clamping") {
        manager.setTileSize(32); // Below minimum
        REQUIRE(manager.getTileSize() == 64);
        
        manager.setTileSize(1024); // Above maximum
        REQUIRE(manager.getTileSize() == TextureStreamingConfig::MAX_TILE_SIZE);
    }
}

TEST_CASE("TextureStreamingManager texture registration", "[streaming][manager]") {
    TextureStreamingManager manager;
    
    SECTION("Register texture") {
        StreamingTextureDesc desc;
        desc.path = "textures/test.dds";
        desc.width = 1024;
        desc.height = 1024;
        desc.format = StreamingFormat::BC7;
        
        u32 handle = manager.registerTexture(desc);
        REQUIRE(handle > 0);
        REQUIRE(manager.getTextureCount() == 1);
        
        auto* tex = manager.getTexture(handle);
        REQUIRE(tex != nullptr);
        REQUIRE(tex->path == "textures/test.dds");
        REQUIRE(tex->width == 1024);
        REQUIRE(tex->height == 1024);
        REQUIRE(tex->mipLevels > 0); // Auto-calculated
    }
    
    SECTION("Register multiple textures") {
        for (int i = 0; i < 5; ++i) {
            StreamingTextureDesc desc;
            desc.width = 512;
            desc.height = 512;
            manager.registerTexture(desc);
        }
        
        REQUIRE(manager.getTextureCount() == 5);
    }
    
    SECTION("Unregister texture") {
        StreamingTextureDesc desc;
        desc.width = 256;
        desc.height = 256;
        
        u32 handle = manager.registerTexture(desc);
        REQUIRE(manager.getTextureCount() == 1);
        
        manager.unregisterTexture(handle);
        REQUIRE(manager.getTextureCount() == 0);
        REQUIRE(manager.getTexture(handle) == nullptr);
    }
}

TEST_CASE("TextureStreamingManager mip calculation", "[streaming][manager]") {
    TextureStreamingManager manager;
    
    SECTION("Auto mip chain generation") {
        StreamingTextureDesc desc;
        desc.width = 1024;
        desc.height = 1024;
        desc.format = StreamingFormat::BC7;
        
        u32 handle = manager.registerTexture(desc);
        auto* tex = manager.getTexture(handle);
        
        REQUIRE(tex != nullptr);
        REQUIRE(tex->mipLevels == 11); // log2(1024) + 1
        REQUIRE(tex->mips.size() == 11);
        
        // Check mip sizes
        REQUIRE(tex->mips[0].width == 1024);
        REQUIRE(tex->mips[0].height == 1024);
        REQUIRE(tex->mips[1].width == 512);
        REQUIRE(tex->mips[1].height == 512);
        REQUIRE(tex->mips[10].width == 1);
        REQUIRE(tex->mips[10].height == 1);
    }
    
    SECTION("Mip memory calculation") {
        StreamingTextureDesc desc;
        desc.width = 256;
        desc.height = 256;
        desc.format = StreamingFormat::BC7;
        
        u32 handle = manager.registerTexture(desc);
        auto* tex = manager.getTexture(handle);
        
        REQUIRE(tex != nullptr);
        REQUIRE(tex->totalSizeBytes > 0);
        
        // BC7: 256x256 = 64x64 blocks * 16 bytes = 65536
        REQUIRE(tex->mips[0].sizeBytes == 65536);
    }
}

TEST_CASE("TextureStreamingManager streaming operations", "[streaming][manager]") {
    TextureStreamingManager manager(256);
    
    StreamingTextureDesc desc;
    desc.width = 256;
    desc.height = 256;
    desc.format = StreamingFormat::BC7;
    
    u32 handle = manager.registerTexture(desc);
    
    SECTION("Request mip load") {
        manager.beginFrame(0);
        manager.requestMipLoad(handle, 0, StreamPriority::Normal);
        
        const auto& stats = manager.getStats();
        REQUIRE(stats.requestsThisFrame == 1);
    }
    
    SECTION("Process requests loads mips") {
        manager.beginFrame(0);
        manager.requestMipLoad(handle, 4, StreamPriority::Normal);
        manager.processRequests(1);
        
        auto* tex = manager.getTexture(handle);
        REQUIRE(tex != nullptr);
        REQUIRE(tex->mips[4].state == StreamState::Resident);
        
        const auto& stats = manager.getStats();
        REQUIRE(stats.loadsThisFrame == 1);
    }
    
    SECTION("Frame lifecycle") {
        manager.beginFrame(0);
        manager.requestMipLoad(handle, 5);
        manager.processRequests();
        manager.endFrame();
        
        const auto& stats = manager.getStats();
        REQUIRE(stats.memoryUsage > 0);
    }
}

TEST_CASE("TextureStreamingManager budget tracking", "[streaming][manager]") {
    TextureStreamingManager manager(1); // 1 MB budget
    
    SECTION("Memory usage increases on load") {
        StreamingTextureDesc desc;
        desc.width = 256;
        desc.height = 256;
        desc.format = StreamingFormat::RGBA8;
        
        u32 handle = manager.registerTexture(desc);
        
        manager.beginFrame(0);
        manager.requestMipLoad(handle, 0);
        manager.processRequests();
        manager.endFrame();
        
        const auto& budget = manager.getBudget();
        REQUIRE(budget.usedMemory > 0);
    }
}

// =============================================================================
// Utility Function Tests
// =============================================================================

TEST_CASE("calculateDesiredMip utility", "[streaming][utility]") {
    SECTION("Full resolution for high coverage") {
        u32 mip = calculateDesiredMip(1024, 1024, 1024.0f * 1024.0f);
        REQUIRE(mip == 0);
    }
    
    SECTION("Lower mip for low coverage") {
        // 256x256 pixels on screen for 1024x1024 texture
        u32 mip = calculateDesiredMip(1024, 1024, 256.0f * 256.0f);
        // ratio = (1024*1024)/(256*256) = 16, log2(16) = 4
        REQUIRE(mip == 4);
    }
    
    SECTION("Maximum mip for zero coverage") {
        u32 mip = calculateDesiredMip(1024, 1024, 0.0f);
        REQUIRE(mip == TextureStreamingConfig::MAX_MIP_LEVELS - 1);
    }
}

TEST_CASE("calculateFullMipChainSize utility", "[streaming][utility]") {
    SECTION("RGBA8 full chain") {
        u64 size = calculateFullMipChainSize(256, 256, StreamingFormat::RGBA8);
        
        // 256x256 + 128x128 + 64x64 + 32x32 + 16x16 + 8x8 + 4x4 + 2x2 + 1x1
        // = 65536 + 16384 + 4096 + 1024 + 256 + 64 + 16 + 4 + 1 = 87381 * 4 bytes
        // Actually: sum of w*h*4 for each mip
        REQUIRE(size > 0);
    }
    
    SECTION("BC7 full chain") {
        u64 size = calculateFullMipChainSize(256, 256, StreamingFormat::BC7);
        
        // BC7 is 4:1 compression ratio vs RGBA8 (approximately)
        u64 rgbaSize = calculateFullMipChainSize(256, 256, StreamingFormat::RGBA8);
        REQUIRE(size < rgbaSize);
    }
    
    SECTION("Larger texture has larger chain") {
        u64 small = calculateFullMipChainSize(256, 256, StreamingFormat::BC7);
        u64 large = calculateFullMipChainSize(1024, 1024, StreamingFormat::BC7);
        
        REQUIRE(large > small);
    }
}
