/**
 * @file test_mesh_lod.cpp
 * @brief Comprehensive tests for the Mesh LOD System
 * 
 * Tests all LOD functionality including:
 * - LOD configuration constants
 * - LOD selection algorithms
 * - Mesh cluster system
 * - LOD transitions
 * - Impostor system
 * - LOD manager
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "nova/core/render/mesh_lod.hpp"
#include <cmath>

using namespace nova;
using namespace nova::render;
using Catch::Approx;

// ============================================================================
// LOD Configuration Tests
// ============================================================================

TEST_CASE("LODConfig constants are valid", "[render][mesh_lod]") {
    SECTION("Maximum values are reasonable") {
        CHECK(LODConfig::MAX_LOD_LEVELS >= 4);
        CHECK(LODConfig::MAX_LOD_LEVELS <= 16);
        CHECK(LODConfig::MAX_CLUSTERS_PER_MESH >= 256);
        CHECK(LODConfig::CLUSTER_TRIANGLE_COUNT >= 64);
        CHECK(LODConfig::CLUSTER_VERTEX_COUNT >= 32);
    }
    
    SECTION("Default values are sensible") {
        CHECK(LODConfig::DEFAULT_LOD_BIAS == 1.0f);
        CHECK(LODConfig::DEFAULT_TRANSITION_RANGE > 0.0f);
        CHECK(LODConfig::DEFAULT_TRANSITION_RANGE < 1.0f);
        CHECK(LODConfig::MIN_SCREEN_SIZE > 0.0f);
    }
    
    SECTION("Impostor settings are valid") {
        CHECK(LODConfig::IMPOSTOR_ATLAS_SIZE >= 1024);
        CHECK(LODConfig::IMPOSTOR_FRAME_COUNT >= 8);
    }
}

// ============================================================================
// LOD Selection Mode Tests
// ============================================================================

TEST_CASE("LODSelectionMode enum values", "[render][mesh_lod]") {
    CHECK(static_cast<u8>(LODSelectionMode::ScreenSize) == 0);
    CHECK(static_cast<u8>(LODSelectionMode::Distance) == 1);
    CHECK(static_cast<u8>(LODSelectionMode::ScreenError) == 2);
    CHECK(static_cast<u8>(LODSelectionMode::GPUDriven) == 3);
    CHECK(static_cast<u8>(LODSelectionMode::Manual) == 4);
}

TEST_CASE("LODTransitionMode enum values", "[render][mesh_lod]") {
    CHECK(static_cast<u8>(LODTransitionMode::Instant) == 0);
    CHECK(static_cast<u8>(LODTransitionMode::Dither) == 1);
    CHECK(static_cast<u8>(LODTransitionMode::CrossFade) == 2);
    CHECK(static_cast<u8>(LODTransitionMode::Morph) == 3);
}

TEST_CASE("LODGenerationAlgorithm enum values", "[render][mesh_lod]") {
    CHECK(static_cast<u8>(LODGenerationAlgorithm::QuadricErrorMetric) == 0);
    CHECK(static_cast<u8>(LODGenerationAlgorithm::EdgeCollapse) == 1);
    CHECK(static_cast<u8>(LODGenerationAlgorithm::VertexClustering) == 2);
    CHECK(static_cast<u8>(LODGenerationAlgorithm::NeuralSimplification) == 3);
    CHECK(static_cast<u8>(LODGenerationAlgorithm::AdaptiveSubdivision) == 4);
}

TEST_CASE("ClusterGroupType enum values", "[render][mesh_lod]") {
    CHECK(static_cast<u8>(ClusterGroupType::Standard) == 0);
    CHECK(static_cast<u8>(ClusterGroupType::Boundary) == 1);
    CHECK(static_cast<u8>(ClusterGroupType::Transition) == 2);
    CHECK(static_cast<u8>(ClusterGroupType::Impostor) == 3);
}

// ============================================================================
// LOD Level Description Tests
// ============================================================================

TEST_CASE("LODLevelDesc construction and methods", "[render][mesh_lod]") {
    LODLevelDesc desc;
    
    SECTION("Default values") {
        CHECK(desc.triangleCount == 0);
        CHECK(desc.vertexCount == 0);
        CHECK(desc.screenSizeThreshold == 0.0f);
        CHECK(desc.geometricError == 0.0f);
        CHECK(desc.simplificationRatio == 1.0f);
        CHECK(desc.hasMorphTargets == false);
    }
    
    SECTION("Memory calculation") {
        desc.triangleCount = 1000;
        desc.vertexCount = 500;
        usize vertexStride = 32; // Position + Normal + UV
        
        usize expected = (500 * 32) + (1000 * 3 * sizeof(u32));
        CHECK(desc.calculateMemoryUsage(vertexStride) == expected);
    }
    
    SECTION("Should use LOD level") {
        desc.screenSizeThreshold = 0.25f;
        
        CHECK(desc.shouldUse(0.5f) == true);   // Above threshold
        CHECK(desc.shouldUse(0.25f) == true);  // At threshold
        CHECK(desc.shouldUse(0.2f) == false);  // Below threshold
        
        // With LOD bias
        CHECK(desc.shouldUse(0.15f, 2.0f) == true);  // 0.15 * 2 = 0.3 >= 0.25
        CHECK(desc.shouldUse(0.1f, 2.0f) == false);  // 0.1 * 2 = 0.2 < 0.25
    }
}

// ============================================================================
// LOD Generation Settings Tests
// ============================================================================

TEST_CASE("LODGenerationSettings defaults", "[render][mesh_lod]") {
    LODGenerationSettings settings;
    
    CHECK(settings.algorithm == LODGenerationAlgorithm::QuadricErrorMetric);
    CHECK(settings.targetLODCount == 4);
    CHECK(settings.reductionFactor == 0.5f);
    CHECK(settings.maxGeometricError == Approx(0.001f));
    CHECK(settings.preserveBoundary == true);
    CHECK(settings.preserveUVSeams == true);
    CHECK(settings.preserveNormals == true);
    CHECK(settings.generateMorphTargets == false);
    CHECK(settings.normalWeightCoefficient == Approx(100.0f));
    CHECK(settings.uvWeightCoefficient == Approx(100.0f));
    
    // Check screen size thresholds are decreasing
    for (usize i = 1; i < LODConfig::MAX_LOD_LEVELS; ++i) {
        CHECK(settings.screenSizeThresholds[i] < settings.screenSizeThresholds[i - 1]);
    }
}

// ============================================================================
// Mesh Cluster Tests
// ============================================================================

TEST_CASE("MeshCluster construction and methods", "[render][mesh_lod]") {
    MeshCluster cluster;
    
    SECTION("Default values") {
        CHECK(cluster.triangleOffset == 0);
        CHECK(cluster.triangleCount == 0);
        CHECK(cluster.vertexOffset == 0);
        CHECK(cluster.vertexCount == 0);
        CHECK(cluster.boundsRadius == 0.0f);
        CHECK(cluster.lodError == 0.0f);
        CHECK(cluster.lodLevel == 0);
        CHECK(cluster.groupType == ClusterGroupType::Standard);
        CHECK(cluster.parentCluster == UINT32_MAX);
        CHECK(cluster.childClusterStart == UINT32_MAX);
        CHECK(cluster.childClusterCount == 0);
    }
    
    SECTION("Screen error calculation") {
        cluster.boundsCenter = math::Vec3(0.0f, 0.0f, -10.0f);
        cluster.lodError = 0.1f; // World-space error
        
        math::Vec3 cameraPos(0.0f, 0.0f, 0.0f);
        f32 screenHeight = 1080.0f;
        f32 fovY = 60.0f * (3.14159265f / 180.0f);
        
        f32 screenError = cluster.calculateScreenError(cameraPos, screenHeight, fovY);
        CHECK(screenError > 0.0f);
        
        // Error should decrease with distance
        cluster.boundsCenter.z = -20.0f;
        f32 fartherError = cluster.calculateScreenError(cameraPos, screenHeight, fovY);
        CHECK(fartherError < screenError);
    }
    
    SECTION("Should render decision") {
        f32 maxScreenError = 2.0f;
        
        CHECK(cluster.shouldRender(maxScreenError, 1.0f) == true);  // Below threshold
        CHECK(cluster.shouldRender(maxScreenError, 2.0f) == true);  // At threshold
        CHECK(cluster.shouldRender(maxScreenError, 3.0f) == false); // Above threshold
    }
}

// ============================================================================
// GPU Cluster Data Tests
// ============================================================================

TEST_CASE("GPUClusterData structure", "[render][mesh_lod]") {
    CHECK(sizeof(GPUClusterData) == 48);
    CHECK(alignof(GPUClusterData) == 16);
    
    GPUClusterData data;
    data.triangleOffset = 100;
    data.triangleCount = 128;
    data.vertexOffset = 50;
    data.lodError = 0.5f;
    data.lodLevel = 2;
    data.flags = 1;
    data.parentCluster = 5;
    
    CHECK(data.triangleOffset == 100);
    CHECK(data.triangleCount == 128);
    CHECK(data.lodLevel == 2);
}

// ============================================================================
// LOD Mesh Tests
// ============================================================================

TEST_CASE("LODMesh construction and methods", "[render][mesh_lod]") {
    LODMesh mesh("TestMesh");
    
    SECTION("Name management") {
        CHECK(mesh.getName() == "TestMesh");
        mesh.setName("NewName");
        CHECK(mesh.getName() == "NewName");
    }
    
    SECTION("LOD level management") {
        CHECK(mesh.getLODCount() == 0);
        
        LODLevelDesc lod0;
        lod0.triangleCount = 10000;
        lod0.screenSizeThreshold = 0.5f;
        mesh.addLODLevel(lod0);
        
        LODLevelDesc lod1;
        lod1.triangleCount = 5000;
        lod1.screenSizeThreshold = 0.25f;
        mesh.addLODLevel(lod1);
        
        CHECK(mesh.getLODCount() == 2);
        CHECK(mesh.getLODLevel(0).triangleCount == 10000);
        CHECK(mesh.getLODLevel(1).triangleCount == 5000);
    }
    
    SECTION("Bounds management") {
        mesh.setBoundingSphere(math::Vec4(1.0f, 2.0f, 3.0f, 5.0f));
        CHECK(mesh.getBoundingSphere().x == 1.0f);
        CHECK(mesh.getBoundingSphere().y == 2.0f);
        CHECK(mesh.getBoundingSphere().z == 3.0f);
        CHECK(mesh.getBoundingSphere().w == 5.0f);
        
        mesh.setBounds(math::Vec3(-5.0f, -5.0f, -5.0f), math::Vec3(5.0f, 5.0f, 5.0f));
        CHECK(mesh.getBoundsMin().x == -5.0f);
        CHECK(mesh.getBoundsMax().x == 5.0f);
    }
    
    SECTION("LOD selection") {
        LODLevelDesc lod0;
        lod0.screenSizeThreshold = 0.5f;
        mesh.addLODLevel(lod0);
        
        LODLevelDesc lod1;
        lod1.screenSizeThreshold = 0.25f;
        mesh.addLODLevel(lod1);
        
        LODLevelDesc lod2;
        lod2.screenSizeThreshold = 0.1f;
        mesh.addLODLevel(lod2);
        
        CHECK(mesh.selectLOD(0.6f) == 0);   // High detail
        CHECK(mesh.selectLOD(0.3f) == 1);   // Medium detail
        CHECK(mesh.selectLOD(0.15f) == 2);  // Low detail
        CHECK(mesh.selectLOD(0.05f) == 2);  // Lowest (below all thresholds)
    }
    
    SECTION("Screen size calculation") {
        f32 radius = 1.0f;
        f32 distance = 10.0f;
        f32 screenHeight = 1080.0f;
        f32 fovY = 60.0f * (3.14159265f / 180.0f);
        
        f32 screenSize = LODMesh::calculateScreenSize(radius, distance, screenHeight, fovY);
        CHECK(screenSize > 0.0f);
        CHECK(screenSize < 1.0f);
        
        // Closer = larger screen size
        f32 closerSize = LODMesh::calculateScreenSize(radius, distance / 2.0f, screenHeight, fovY);
        CHECK(closerSize > screenSize);
    }
    
    SECTION("Cluster management") {
        CHECK(mesh.hasClusterData() == false);
        
        MeshCluster cluster;
        cluster.triangleCount = 128;
        mesh.addCluster(cluster);
        
        CHECK(mesh.hasClusterData() == true);
        CHECK(mesh.getClusters().size() == 1);
        
        mesh.clearClusters();
        CHECK(mesh.hasClusterData() == false);
    }
    
    SECTION("Total triangle count") {
        LODLevelDesc lod0;
        lod0.triangleCount = 10000;
        mesh.addLODLevel(lod0);
        
        LODLevelDesc lod1;
        lod1.triangleCount = 5000;
        mesh.addLODLevel(lod1);
        
        CHECK(mesh.getTotalTriangleCount() == 15000);
    }
}

// ============================================================================
// LOD Selection State Tests
// ============================================================================

TEST_CASE("LODSelectionState transitions", "[render][mesh_lod]") {
    LODSelectionState state;
    
    SECTION("Initial state") {
        CHECK(state.currentLOD == 0);
        CHECK(state.targetLOD == 0);
        CHECK(state.transitionProgress == 1.0f);
        CHECK(state.isTransitioning == false);
    }
    
    SECTION("LOD update with hysteresis") {
        state.update(1, 0.3f, 0.1f);
        CHECK(state.targetLOD == 1);
        CHECK(state.isTransitioning == true);
        CHECK(state.transitionProgress == 0.0f);
        
        // Small change should not trigger new transition due to hysteresis
        state.update(1, 0.29f, 0.1f);
        CHECK(state.targetLOD == 1);
    }
    
    SECTION("Transition stepping") {
        state.update(1, 0.3f, 0.1f);
        CHECK(state.isTransitioning == true);
        
        state.stepTransition(0.1f, 4.0f);  // 0.1 * 4 = 0.4 progress
        CHECK(state.transitionProgress == Approx(0.4f));
        CHECK(state.currentLOD == 0);  // Still transitioning
        
        state.stepTransition(0.2f, 4.0f);  // Additional 0.8
        CHECK(state.transitionProgress == 1.0f);
        CHECK(state.currentLOD == 1);  // Transition complete
        CHECK(state.isTransitioning == false);
    }
    
    SECTION("Blend factor") {
        CHECK(state.getBlendFactor() == 1.0f);  // Not transitioning
        
        state.update(1, 0.3f, 0.1f);
        CHECK(state.getBlendFactor() == 0.0f);  // Just started
        
        state.stepTransition(0.25f, 4.0f);
        CHECK(state.getBlendFactor() == 1.0f);  // Complete
    }
}

// ============================================================================
// Impostor Data Tests
// ============================================================================

TEST_CASE("ImpostorData UV calculation", "[render][mesh_lod]") {
    ImpostorData impostor;
    
    SECTION("Default settings") {
        CHECK(impostor.frameCount == LODConfig::IMPOSTOR_FRAME_COUNT);
        CHECK(impostor.atlasWidth == LODConfig::IMPOSTOR_ATLAS_SIZE);
        CHECK(impostor.switchDistance == 100.0f);
        CHECK(impostor.useOctahedral == true);
    }
    
    SECTION("Octahedral UV mapping") {
        impostor.useOctahedral = true;
        
        // Front view (0, 0, -1) should map to center area
        math::Vec2 frontUV = impostor.calculateUV(math::Vec3(0.0f, 0.0f, -1.0f));
        CHECK(frontUV.x >= 0.0f);
        CHECK(frontUV.x <= 1.0f);
        CHECK(frontUV.y >= 0.0f);
        CHECK(frontUV.y <= 1.0f);
        
        // Right view (1, 0, 0)
        math::Vec2 rightUV = impostor.calculateUV(math::Vec3(1.0f, 0.0f, 0.0f));
        CHECK(rightUV.x >= 0.0f);
        CHECK(rightUV.x <= 1.0f);
    }
    
    SECTION("Spherical UV mapping") {
        impostor.useOctahedral = false;
        
        math::Vec2 uv = impostor.calculateUV(math::Vec3(0.0f, 0.0f, 1.0f));
        CHECK(uv.x >= 0.0f);
        CHECK(uv.x <= 1.0f);
        CHECK(uv.y >= 0.0f);
        CHECK(uv.y <= 1.0f);
    }
}

// ============================================================================
// LOD Manager Tests
// ============================================================================

TEST_CASE("LODManager singleton and settings", "[render][mesh_lod]") {
    auto& manager = LODManager::getInstance();
    
    SECTION("LOD bias") {
        manager.setLODBias(1.5f);
        CHECK(manager.getLODBias() == 1.5f);
        manager.setLODBias(1.0f);  // Reset
    }
    
    SECTION("Selection mode") {
        manager.setSelectionMode(LODSelectionMode::GPUDriven);
        CHECK(manager.getSelectionMode() == LODSelectionMode::GPUDriven);
        manager.setSelectionMode(LODSelectionMode::ScreenSize);  // Reset
    }
    
    SECTION("Transition mode") {
        manager.setTransitionMode(LODTransitionMode::CrossFade);
        CHECK(manager.getTransitionMode() == LODTransitionMode::CrossFade);
        manager.setTransitionMode(LODTransitionMode::Dither);  // Reset
    }
    
    SECTION("Max screen error") {
        manager.setMaxScreenError(2.0f);
        CHECK(manager.getMaxScreenError() == 2.0f);
        manager.setMaxScreenError(1.0f);  // Reset
    }
    
    SECTION("Statistics") {
        manager.resetStatistics();
        auto& stats = manager.getStatistics();
        CHECK(stats.totalMeshes == 0);
        CHECK(stats.trianglesRendered == 0);
        
        manager.recordTriangles(1000, 5000);
        CHECK(manager.getStatistics().trianglesRendered == 1000);
        CHECK(manager.getStatistics().trianglesPotential == 5000);
        
        manager.updateStatistics();
        CHECK(manager.getStatistics().lodReductionRatio == Approx(0.8f));
        
        manager.resetStatistics();
    }
    
    SECTION("LOD level recording") {
        manager.resetStatistics();
        manager.recordLODSelection(0);
        manager.recordLODSelection(1);
        manager.recordLODSelection(2);
        CHECK(manager.getStatistics().averageLODLevel == Approx(1.0f));
        manager.resetStatistics();
    }
}

// ============================================================================
// LOD Utility Function Tests
// ============================================================================

TEST_CASE("LOD utility functions", "[render][mesh_lod]") {
    SECTION("Screen size threshold calculation") {
        f32 lod0 = calculateScreenSizeThreshold(0, 4);
        f32 lod1 = calculateScreenSizeThreshold(1, 4);
        f32 lod2 = calculateScreenSizeThreshold(2, 4);
        
        CHECK(lod0 == 1.0f);  // LOD 0 = full detail
        CHECK(lod1 < lod0);   // Each LOD has smaller threshold
        CHECK(lod2 < lod1);
    }
    
    SECTION("Triangle budget calculation") {
        u32 baseTris = 10000;
        
        CHECK(calculateTriangleBudget(baseTris, 0) == 10000);  // LOD 0 = full
        CHECK(calculateTriangleBudget(baseTris, 1) == 5000);   // 50% of previous
        CHECK(calculateTriangleBudget(baseTris, 2) == 2500);   // 50% of previous
        
        // Custom reduction factor
        CHECK(calculateTriangleBudget(baseTris, 1, 0.25f) == 2500);  // 25% reduction
    }
}
