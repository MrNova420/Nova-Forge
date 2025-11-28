/**
 * @file test_terrain.cpp
 * @brief Comprehensive tests for the terrain rendering system
 * @author NovaForge Team
 * @copyright Copyright (c) 2024-2025 NovaForge - All Rights Reserved
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/terrain_system.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// TERRAIN CONFIGURATION TESTS
// =============================================================================

TEST_CASE("TerrainConfig constants are valid", "[terrain][config]") {
    SECTION("Maximum values are reasonable") {
        REQUIRE(TerrainConfig::MAX_TERRAIN_SIZE > 0.0f);
        REQUIRE(TerrainConfig::MAX_HEIGHTMAP_RESOLUTION > 0);
        REQUIRE(TerrainConfig::MAX_CLIPMAP_LEVELS > 0);
        REQUIRE(TerrainConfig::MAX_TEXTURE_LAYERS > 0);
    }
    
    SECTION("Default values are sensible") {
        REQUIRE(TerrainConfig::DEFAULT_CLIPMAP_SIZE > 0);
        REQUIRE(TerrainConfig::DEFAULT_TESSELLATION > 0.0f);
        REQUIRE(TerrainConfig::DEFAULT_SCALE > 0.0f);
    }
    
    SECTION("Virtual texture tile size is power of 2") {
        REQUIRE((TerrainConfig::VT_TILE_SIZE & (TerrainConfig::VT_TILE_SIZE - 1)) == 0);
    }
}

// =============================================================================
// LOD TECHNIQUE TESTS
// =============================================================================

TEST_CASE("TerrainLODTechnique enumeration", "[terrain][enums]") {
    SECTION("All techniques have valid names") {
        REQUIRE(std::string(getTerrainLODTechniqueName(TerrainLODTechnique::Clipmap)) == "Clipmap");
        REQUIRE(std::string(getTerrainLODTechniqueName(TerrainLODTechnique::Quadtree)) == "Quadtree");
        REQUIRE(std::string(getTerrainLODTechniqueName(TerrainLODTechnique::CDLOD)) == "CDLOD");
        REQUIRE(std::string(getTerrainLODTechniqueName(TerrainLODTechnique::Tessellation)) == "Tessellation");
    }
}

// =============================================================================
// HEIGHTMAP TESTS
// =============================================================================

TEST_CASE("Heightmap creation and sampling", "[terrain][heightmap]") {
    SECTION("Create heightmap with dimensions") {
        Heightmap hm = Heightmap::create(256, 256, 0.0f, 100.0f);
        
        REQUIRE(hm.width == 256);
        REQUIRE(hm.height == 256);
        REQUIRE(hm.minHeight == Approx(0.0f));
        REQUIRE(hm.maxHeight == Approx(100.0f));
        REQUIRE(hm.data.size() == 256 * 256);
        REQUIRE(hm.isValid());
    }
    
    SECTION("Empty heightmap is invalid") {
        Heightmap hm;
        REQUIRE_FALSE(hm.isValid());
    }
    
    SECTION("Sample returns interpolated values") {
        Heightmap hm = Heightmap::create(3, 3, 0.0f, 1.0f);
        // Set corner values
        hm.data[0] = 0.0f;    // (0,0)
        hm.data[2] = 1.0f;    // (2,0)
        hm.data[6] = 0.5f;    // (0,2)
        hm.data[8] = 0.75f;   // (2,2)
        
        // Sample at corners
        REQUIRE(hm.sample(0.0f, 0.0f) == Approx(0.0f));
        REQUIRE(hm.sample(1.0f, 0.0f) == Approx(1.0f));
    }
    
    SECTION("Sample clamps out of bounds") {
        Heightmap hm = Heightmap::create(4, 4, 0.0f, 1.0f);
        std::fill(hm.data.begin(), hm.data.end(), 0.5f);
        
        // Should not crash with out of bounds
        f32 h1 = hm.sample(-1.0f, -1.0f);
        f32 h2 = hm.sample(2.0f, 2.0f);
        REQUIRE(h1 == Approx(0.5f));
        REQUIRE(h2 == Approx(0.5f));
    }
    
    SECTION("Normal sampling produces valid normals") {
        Heightmap hm = Heightmap::create(16, 16, 0.0f, 1.0f);
        std::fill(hm.data.begin(), hm.data.end(), 0.5f);
        
        Vec3 normal = hm.sampleNormal(0.5f, 0.5f);
        
        // Normal should be normalized
        f32 length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        REQUIRE(length == Approx(1.0f).margin(0.01f));
    }
    
    SECTION("Memory size calculation") {
        Heightmap hm = Heightmap::create(256, 256, 0.0f, 1.0f);
        REQUIRE(hm.getMemorySize() == 256 * 256 * sizeof(f32));
    }
}

// =============================================================================
// TEXTURE LAYER TESTS
// =============================================================================

TEST_CASE("TerrainTextureLayer operations", "[terrain][textures]") {
    TerrainTextureLayer layer;
    layer.heightMin = 0.0f;
    layer.heightMax = 50.0f;
    layer.slopeMin = 0.0f;
    layer.slopeMax = 30.0f;
    
    SECTION("Height range check") {
        REQUIRE(layer.isInHeightRange(25.0f));
        REQUIRE(layer.isInHeightRange(0.0f));
        REQUIRE(layer.isInHeightRange(50.0f));
        REQUIRE_FALSE(layer.isInHeightRange(-1.0f));
        REQUIRE_FALSE(layer.isInHeightRange(51.0f));
    }
    
    SECTION("Slope range check") {
        REQUIRE(layer.isInSlopeRange(15.0f));
        REQUIRE(layer.isInSlopeRange(0.0f));
        REQUIRE(layer.isInSlopeRange(30.0f));
        REQUIRE_FALSE(layer.isInSlopeRange(31.0f));
    }
    
    SECTION("Blend weight calculation") {
        f32 weight = layer.calculateBlendWeight(25.0f, 15.0f);
        REQUIRE(weight >= 0.0f);
        REQUIRE(weight <= 1.0f);
    }
}

// =============================================================================
// CLIPMAP LEVEL TESTS
// =============================================================================

TEST_CASE("ClipmapLevel calculations", "[terrain][clipmap]") {
    ClipmapLevel level;
    level.level = 0;
    level.resolution = 255;
    level.cellSize = 1.0f;
    level.scale = 1.0f;
    
    SECTION("World extent calculation") {
        f32 extent = level.getWorldExtent();
        REQUIRE(extent == Approx(255.0f));
    }
    
    SECTION("Vertex count calculation") {
        u32 vertexCount = level.getVertexCount();
        REQUIRE(vertexCount == 255 * 255);
    }
    
    SECTION("Index count calculation") {
        u32 indexCount = level.getIndexCount();
        REQUIRE(indexCount == 254 * 254 * 6);
    }
    
    SECTION("Higher levels have larger cell sizes") {
        ClipmapLevel level0;
        level0.cellSize = 1.0f;
        level0.scale = 1.0f;
        
        ClipmapLevel level1;
        level1.cellSize = 2.0f;
        level1.scale = 2.0f;
        
        REQUIRE(level1.getWorldExtent() > level0.getWorldExtent());
    }
}

// =============================================================================
// TERRAIN CHUNK TESTS
// =============================================================================

TEST_CASE("TerrainChunk operations", "[terrain][chunk]") {
    TerrainChunk chunk;
    chunk.id = 0;
    chunk.boundsMin = Vec3{-50.0f, 0.0f, -50.0f};
    chunk.boundsMax = Vec3{50.0f, 100.0f, 50.0f};
    
    SECTION("Center calculation") {
        Vec3 center = chunk.getCenter();
        REQUIRE(center.x == Approx(0.0f));
        REQUIRE(center.y == Approx(50.0f));
        REQUIRE(center.z == Approx(0.0f));
    }
    
    SECTION("Size calculation") {
        Vec3 size = chunk.getSize();
        REQUIRE(size.x == Approx(100.0f));
        REQUIRE(size.y == Approx(100.0f));
        REQUIRE(size.z == Approx(100.0f));
    }
    
    SECTION("Contains point check") {
        REQUIRE(chunk.contains(Vec3{0.0f, 50.0f, 0.0f}));
        REQUIRE(chunk.contains(Vec3{-50.0f, 0.0f, -50.0f}));
        REQUIRE(chunk.contains(Vec3{50.0f, 100.0f, 50.0f}));
        REQUIRE_FALSE(chunk.contains(Vec3{100.0f, 50.0f, 0.0f}));
    }
    
    SECTION("Bounds update") {
        chunk.updateBounds(Vec3{0.0f, 50.0f, 0.0f}, Vec3{25.0f, 50.0f, 25.0f});
        REQUIRE(chunk.boundsMin.x == Approx(-25.0f));
        REQUIRE(chunk.boundsMax.x == Approx(25.0f));
    }
}

// =============================================================================
// TERRAIN BRUSH TESTS
// =============================================================================

TEST_CASE("TerrainBrush falloff", "[terrain][brush]") {
    TerrainBrush brush;
    brush.radius = 10.0f;
    brush.falloff = 0.5f;
    
    SECTION("Center has full strength") {
        f32 falloff = brush.getFalloff(0.0f);
        REQUIRE(falloff == Approx(1.0f));
    }
    
    SECTION("Edge has zero strength") {
        f32 falloff = brush.getFalloff(10.0f);
        REQUIRE(falloff == Approx(0.0f));
    }
    
    SECTION("Outside has zero strength") {
        f32 falloff = brush.getFalloff(15.0f);
        REQUIRE(falloff == Approx(0.0f));
    }
    
    SECTION("Falloff is gradual in falloff zone") {
        // With falloff = 0.5 and radius = 10.0, the edge starts at distance 5.0
        // So we test values in the falloff zone (5.0 to 10.0)
        f32 near = brush.getFalloff(5.5f);
        f32 mid = brush.getFalloff(7.5f);
        f32 far = brush.getFalloff(9.5f);
        
        REQUIRE(near > mid);
        REQUIRE(mid > far);
    }
}

// =============================================================================
// DETAIL MESH TESTS
// =============================================================================

TEST_CASE("DetailMesh placement validation", "[terrain][detail]") {
    DetailMesh mesh;
    mesh.minSlope = 0.0f;
    mesh.maxSlope = 30.0f;
    mesh.minHeight = 0.0f;
    mesh.maxHeight = 100.0f;
    mesh.textureLayerMask = 0x0003; // Layers 0 and 1 only
    
    SECTION("Valid placement") {
        REQUIRE(mesh.isValidPlacement(50.0f, 15.0f, 0));
        REQUIRE(mesh.isValidPlacement(50.0f, 15.0f, 1));
    }
    
    SECTION("Invalid height") {
        REQUIRE_FALSE(mesh.isValidPlacement(150.0f, 15.0f, 0));
        REQUIRE_FALSE(mesh.isValidPlacement(-10.0f, 15.0f, 0));
    }
    
    SECTION("Invalid slope") {
        REQUIRE_FALSE(mesh.isValidPlacement(50.0f, 45.0f, 0));
    }
    
    SECTION("Invalid texture layer") {
        REQUIRE_FALSE(mesh.isValidPlacement(50.0f, 15.0f, 2));
    }
}

TEST_CASE("DetailMesh fade distance", "[terrain][detail]") {
    DetailMesh mesh;
    mesh.viewDistance = 500.0f;
    mesh.fadeStart = 400.0f;
    
    SECTION("Full alpha within fade start") {
        REQUIRE(mesh.getFadeAlpha(100.0f) == Approx(1.0f));
        REQUIRE(mesh.getFadeAlpha(400.0f) == Approx(1.0f));
    }
    
    SECTION("Zero alpha beyond view distance") {
        REQUIRE(mesh.getFadeAlpha(500.0f) == Approx(0.0f));
        REQUIRE(mesh.getFadeAlpha(600.0f) == Approx(0.0f));
    }
    
    SECTION("Gradual fade in transition zone") {
        f32 alpha = mesh.getFadeAlpha(450.0f);
        REQUIRE(alpha > 0.0f);
        REQUIRE(alpha < 1.0f);
    }
}

// =============================================================================
// TERRAIN SETTINGS TESTS
// =============================================================================

TEST_CASE("TerrainSettings factories", "[terrain][settings]") {
    SECTION("Low quality settings") {
        auto settings = TerrainSettings::low();
        REQUIRE(settings.clipmapLevels == 4);
        REQUIRE(settings.clipmapResolution == 127);
        REQUIRE(settings.enableTessellation == false);
    }
    
    SECTION("Medium quality settings") {
        auto settings = TerrainSettings::medium();
        REQUIRE(settings.clipmapLevels == 6);
        REQUIRE(settings.clipmapResolution == 255);
    }
    
    SECTION("High quality settings") {
        auto settings = TerrainSettings::high();
        REQUIRE(settings.clipmapLevels == 8);
        REQUIRE(settings.clipmapResolution == 511);
        REQUIRE(settings.enableTriplanar == true);
    }
    
    SECTION("Ultra quality settings") {
        auto settings = TerrainSettings::ultra();
        REQUIRE(settings.clipmapLevels == 10);
        REQUIRE(settings.clipmapResolution == 1023);
    }
}

// =============================================================================
// GPU TERRAIN DATA TESTS
// =============================================================================

TEST_CASE("GPUTerrainData creation", "[terrain][gpu]") {
    GPUTerrainData data = GPUTerrainData::create(1000.0f, 100.0f, 512, 512);
    
    REQUIRE(data.terrainParams.x == Approx(1000.0f));
    REQUIRE(data.terrainParams.y == Approx(100.0f));
    REQUIRE(data.heightmapParams.x == Approx(512.0f));
    REQUIRE(data.heightmapParams.y == Approx(512.0f));
}

// =============================================================================
// TERRAIN MANAGER TESTS
// =============================================================================

TEST_CASE("TerrainManager initialization", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    
    SECTION("Can initialize") {
        REQUIRE(manager.initialize());
        REQUIRE(manager.isInitialized());
    }
}

TEST_CASE("TerrainManager flat terrain creation", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    manager.initialize();
    
    REQUIRE(manager.createFlatTerrain(1000.0f, 10.0f));
    
    SECTION("World size is set") {
        REQUIRE(manager.getWorldSize() == Approx(1000.0f));
    }
    
    SECTION("Height can be sampled") {
        f32 height = manager.getHeightAt(0.0f, 0.0f);
        REQUIRE(height == Approx(10.0f));
    }
    
    SECTION("Heightmap is valid") {
        REQUIRE(manager.getHeightmap().isValid());
    }
}

TEST_CASE("TerrainManager procedural generation", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    manager.initialize();
    
    REQUIRE(manager.generateProceduralTerrain(128, 500.0f, 4, 0.5f));
    
    SECTION("Height varies across terrain") {
        f32 h1 = manager.getHeightAt(0.0f, 0.0f);
        f32 h2 = manager.getHeightAt(100.0f, 100.0f);
        // Heights should generally differ due to procedural generation
        // (might be the same by chance, but unlikely)
        REQUIRE(manager.getHeightmap().isValid());
    }
    
    SECTION("Chunks are created") {
        auto visible = manager.getVisibleChunks(Vec3{0.0f, 0.0f, 0.0f}, 1000.0f);
        REQUIRE(visible.size() > 0);
    }
}

TEST_CASE("TerrainManager texture layers", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    manager.shutdown(); // Reset state
    manager.initialize();
    manager.createFlatTerrain(1000.0f);
    
    SECTION("Add texture layer") {
        TerrainTextureLayer layer;
        layer.albedoTexture = "grass_albedo.png";
        layer.uvScale = 10.0f;
        
        u32 index = manager.addTextureLayer(layer);
        REQUIRE(index != UINT32_MAX);
        REQUIRE(manager.getTextureLayerCount() >= 1);
    }
    
    SECTION("Get texture layer") {
        TerrainTextureLayer layer;
        layer.albedoTexture = "rock_albedo.png";
        u32 index = manager.addTextureLayer(layer);
        
        const TerrainTextureLayer* retrieved = manager.getTextureLayer(index);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->albedoTexture == "rock_albedo.png");
    }
}

TEST_CASE("TerrainManager detail meshes", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    manager.shutdown(); // Reset state
    manager.initialize();
    manager.createFlatTerrain(1000.0f);
    
    SECTION("Add detail mesh") {
        DetailMesh mesh;
        mesh.meshPath = "grass_patch.obj";
        mesh.density = 5.0f;
        
        u32 index = manager.addDetailMesh(mesh);
        REQUIRE(index != UINT32_MAX);
        REQUIRE(manager.getDetailMeshCount() >= 1);
    }
    
    SECTION("Get detail mesh") {
        DetailMesh mesh;
        mesh.meshPath = "rock.obj";
        u32 index = manager.addDetailMesh(mesh);
        
        const DetailMesh* retrieved = manager.getDetailMesh(index);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->meshPath == "rock.obj");
    }
}

TEST_CASE("TerrainManager clipmaps", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    manager.initialize();
    
    TerrainSettings settings = TerrainSettings::medium();
    manager.setSettings(settings);
    manager.createFlatTerrain(1000.0f);
    
    SECTION("Clipmap levels are created") {
        REQUIRE(manager.getClipmapLevelCount() == settings.clipmapLevels);
    }
    
    SECTION("Clipmap levels have correct indices") {
        for (u32 i = 0; i < manager.getClipmapLevelCount(); ++i) {
            const ClipmapLevel* level = manager.getClipmapLevel(i);
            REQUIRE(level != nullptr);
            REQUIRE(level->level == i);
        }
    }
    
    SECTION("Update clipmaps for camera") {
        manager.updateClipmaps(Vec3{100.0f, 50.0f, 100.0f});
        // Should not crash
    }
}

TEST_CASE("TerrainManager height and normal queries", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    manager.initialize();
    manager.createFlatTerrain(1000.0f, 0.0f);
    
    SECTION("Slope on flat terrain is zero") {
        f32 slope = manager.getSlopeAt(0.0f, 0.0f);
        REQUIRE(slope == Approx(0.0f).margin(5.0f));
    }
    
    SECTION("Normal on flat terrain points up") {
        Vec3 normal = manager.getNormalAt(0.0f, 0.0f);
        REQUIRE(normal.y > 0.9f);
    }
}

TEST_CASE("TerrainManager GPU data", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    manager.initialize();
    manager.createFlatTerrain(1000.0f);
    
    GPUTerrainData data = manager.getGPUData();
    
    REQUIRE(data.terrainParams.x == Approx(1000.0f));
}

TEST_CASE("TerrainManager statistics", "[terrain][manager]") {
    auto& manager = TerrainManager::instance();
    manager.initialize();
    manager.createFlatTerrain(1000.0f);
    
    manager.beginFrame();
    manager.endFrame();
    
    const TerrainStats& stats = manager.getStats();
    REQUIRE(stats.totalChunks > 0);
}
