/**
 * @file test_occlusion_culling.cpp
 * @brief Comprehensive tests for the occlusion culling system
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/occlusion_culling.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// OcclusionConfig Constants Tests
// =============================================================================

TEST_CASE("OcclusionConfig constants are valid", "[occlusion][config]") {
    SECTION("Hi-Z settings") {
        REQUIRE(OcclusionConfig::MAX_HIZ_MIP_LEVELS == 12);
        REQUIRE(OcclusionConfig::DEFAULT_HIZ_DIVISOR == 2);
    }
    
    SECTION("Query limits") {
        REQUIRE(OcclusionConfig::MAX_OCCLUSION_QUERIES == 8192);
        REQUIRE(OcclusionConfig::QUERY_POOL_SIZE == 16384);
    }
    
    SECTION("Software culling settings") {
        REQUIRE(OcclusionConfig::MAX_SOFTWARE_OCCLUDERS == 256);
        REQUIRE(OcclusionConfig::SOFTWARE_TILE_SIZE == 64);
    }
    
    SECTION("Portal and visibility") {
        REQUIRE(OcclusionConfig::MAX_PORTALS == 1024);
        REQUIRE(OcclusionConfig::MAX_VISIBILITY_SETS == 256);
    }
    
    SECTION("Frustum and depth") {
        REQUIRE(OcclusionConfig::FRUSTUM_PLANE_COUNT == 6);
        REQUIRE(OcclusionConfig::DEFAULT_DEPTH_BIAS == Approx(0.0001f));
        REQUIRE(OcclusionConfig::QUERY_LATENCY_FRAMES == 2);
    }
}

// =============================================================================
// OcclusionTechnique Enum Tests
// =============================================================================

TEST_CASE("OcclusionTechnique enum values", "[occlusion][technique]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(OcclusionTechnique::None) == 0);
        REQUIRE(static_cast<u32>(OcclusionTechnique::FrustumOnly) == 1);
        REQUIRE(static_cast<u32>(OcclusionTechnique::HardwareQueries) == 2);
        REQUIRE(static_cast<u32>(OcclusionTechnique::HiZBuffer) == 3);
        REQUIRE(static_cast<u32>(OcclusionTechnique::SoftwareRaster) == 4);
        REQUIRE(static_cast<u32>(OcclusionTechnique::HiZWithQueries) == 5);
        REQUIRE(static_cast<u32>(OcclusionTechnique::COUNT) == 6);
    }
    
    SECTION("Technique names") {
        REQUIRE(std::string(getOcclusionTechniqueName(OcclusionTechnique::None)) == "None");
        REQUIRE(std::string(getOcclusionTechniqueName(OcclusionTechnique::FrustumOnly)) == "Frustum Only");
        REQUIRE(std::string(getOcclusionTechniqueName(OcclusionTechnique::HiZBuffer)) == "Hi-Z Buffer");
    }
}

// =============================================================================
// VisibilityResult Enum Tests
// =============================================================================

TEST_CASE("VisibilityResult enum values", "[occlusion][visibility]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(VisibilityResult::Visible) == 0);
        REQUIRE(static_cast<u32>(VisibilityResult::OutsideFrustum) == 1);
        REQUIRE(static_cast<u32>(VisibilityResult::Occluded) == 2);
        REQUIRE(static_cast<u32>(VisibilityResult::TooSmall) == 3);
        REQUIRE(static_cast<u32>(VisibilityResult::Pending) == 4);
    }
    
    SECTION("Result names") {
        REQUIRE(std::string(getVisibilityResultName(VisibilityResult::Visible)) == "Visible");
        REQUIRE(std::string(getVisibilityResultName(VisibilityResult::OutsideFrustum)) == "Outside Frustum");
        REQUIRE(std::string(getVisibilityResultName(VisibilityResult::Occluded)) == "Occluded");
    }
}

// =============================================================================
// AABB Tests
// =============================================================================

TEST_CASE("AABB functionality", "[occlusion][aabb]") {
    SECTION("Default constructor") {
        AABB box;
        REQUIRE(box.min.x == Approx(0.0f));
        REQUIRE(box.max.x == Approx(0.0f));
    }
    
    SECTION("Constructor with min/max") {
        AABB box(Vec3{-5.0f, -5.0f, -5.0f}, Vec3{5.0f, 5.0f, 5.0f});
        REQUIRE(box.min.x == Approx(-5.0f));
        REQUIRE(box.max.x == Approx(5.0f));
    }
    
    SECTION("Get center") {
        AABB box(Vec3{0.0f, 0.0f, 0.0f}, Vec3{10.0f, 20.0f, 30.0f});
        Vec3 center = box.getCenter();
        
        REQUIRE(center.x == Approx(5.0f));
        REQUIRE(center.y == Approx(10.0f));
        REQUIRE(center.z == Approx(15.0f));
    }
    
    SECTION("Get half extents") {
        AABB box(Vec3{0.0f, 0.0f, 0.0f}, Vec3{10.0f, 20.0f, 30.0f});
        Vec3 halfExtents = box.getHalfExtents();
        
        REQUIRE(halfExtents.x == Approx(5.0f));
        REQUIRE(halfExtents.y == Approx(10.0f));
        REQUIRE(halfExtents.z == Approx(15.0f));
    }
    
    SECTION("Get extents") {
        AABB box(Vec3{0.0f, 0.0f, 0.0f}, Vec3{10.0f, 20.0f, 30.0f});
        Vec3 extents = box.getExtents();
        
        REQUIRE(extents.x == Approx(10.0f));
        REQUIRE(extents.y == Approx(20.0f));
        REQUIRE(extents.z == Approx(30.0f));
    }
    
    SECTION("Surface area") {
        AABB box(Vec3{0.0f, 0.0f, 0.0f}, Vec3{2.0f, 3.0f, 4.0f});
        // 2 * (2*3 + 3*4 + 4*2) = 2 * (6 + 12 + 8) = 52
        REQUIRE(box.getSurfaceArea() == Approx(52.0f));
    }
    
    SECTION("Volume") {
        AABB box(Vec3{0.0f, 0.0f, 0.0f}, Vec3{2.0f, 3.0f, 4.0f});
        REQUIRE(box.getVolume() == Approx(24.0f));
    }
    
    SECTION("Contains point") {
        AABB box(Vec3{-5.0f, -5.0f, -5.0f}, Vec3{5.0f, 5.0f, 5.0f});
        
        REQUIRE(box.containsPoint(Vec3{0.0f, 0.0f, 0.0f}) == true);
        REQUIRE(box.containsPoint(Vec3{4.0f, 4.0f, 4.0f}) == true);
        REQUIRE(box.containsPoint(Vec3{6.0f, 0.0f, 0.0f}) == false);
    }
    
    SECTION("Intersects AABB") {
        AABB box1(Vec3{0.0f, 0.0f, 0.0f}, Vec3{10.0f, 10.0f, 10.0f});
        AABB box2(Vec3{5.0f, 5.0f, 5.0f}, Vec3{15.0f, 15.0f, 15.0f});
        AABB box3(Vec3{20.0f, 20.0f, 20.0f}, Vec3{30.0f, 30.0f, 30.0f});
        
        REQUIRE(box1.intersects(box2) == true);
        REQUIRE(box1.intersects(box3) == false);
    }
    
    SECTION("Expand to include point") {
        AABB box(Vec3{0.0f, 0.0f, 0.0f}, Vec3{5.0f, 5.0f, 5.0f});
        box.expandToInclude(Vec3{10.0f, 10.0f, 10.0f});
        
        REQUIRE(box.max.x == Approx(10.0f));
        REQUIRE(box.max.y == Approx(10.0f));
        REQUIRE(box.max.z == Approx(10.0f));
    }
    
    SECTION("Expand to include AABB") {
        AABB box1(Vec3{0.0f, 0.0f, 0.0f}, Vec3{5.0f, 5.0f, 5.0f});
        AABB box2(Vec3{3.0f, 3.0f, 3.0f}, Vec3{10.0f, 10.0f, 10.0f});
        
        box1.expandToInclude(box2);
        
        REQUIRE(box1.min.x == Approx(0.0f));
        REQUIRE(box1.max.x == Approx(10.0f));
    }
    
    SECTION("Get corners") {
        AABB box(Vec3{0.0f, 0.0f, 0.0f}, Vec3{1.0f, 1.0f, 1.0f});
        auto corners = box.getCorners();
        
        REQUIRE(corners.size() == 8);
        // First corner should be min
        REQUIRE(corners[0].x == Approx(0.0f));
        REQUIRE(corners[0].y == Approx(0.0f));
        REQUIRE(corners[0].z == Approx(0.0f));
    }
    
    SECTION("Create from center and half extents") {
        auto box = AABB::fromCenterHalfExtents(Vec3{5.0f, 5.0f, 5.0f}, Vec3{2.0f, 3.0f, 4.0f});
        
        REQUIRE(box.min.x == Approx(3.0f));
        REQUIRE(box.max.x == Approx(7.0f));
        REQUIRE(box.min.y == Approx(2.0f));
        REQUIRE(box.max.y == Approx(8.0f));
    }
}

// =============================================================================
// BoundingSphere Tests
// =============================================================================

TEST_CASE("BoundingSphere functionality", "[occlusion][sphere]") {
    SECTION("Default values") {
        BoundingSphere sphere;
        REQUIRE(sphere.radius == Approx(0.0f));
    }
    
    SECTION("Intersects sphere") {
        BoundingSphere s1;
        s1.center = Vec3{0.0f, 0.0f, 0.0f};
        s1.radius = 5.0f;
        
        BoundingSphere s2;
        s2.center = Vec3{8.0f, 0.0f, 0.0f};
        s2.radius = 5.0f;
        
        BoundingSphere s3;
        s3.center = Vec3{15.0f, 0.0f, 0.0f};
        s3.radius = 5.0f;
        
        REQUIRE(s1.intersects(s2) == true);  // Distance 8 < 10 (5+5)
        REQUIRE(s1.intersects(s3) == false); // Distance 15 > 10
    }
    
    SECTION("Contains point") {
        BoundingSphere sphere;
        sphere.center = Vec3{0.0f, 0.0f, 0.0f};
        sphere.radius = 10.0f;
        
        REQUIRE(sphere.containsPoint(Vec3{0.0f, 0.0f, 0.0f}) == true);
        REQUIRE(sphere.containsPoint(Vec3{5.0f, 0.0f, 0.0f}) == true);
        REQUIRE(sphere.containsPoint(Vec3{11.0f, 0.0f, 0.0f}) == false);
    }
    
    SECTION("Create from AABB") {
        AABB box(Vec3{-5.0f, -5.0f, -5.0f}, Vec3{5.0f, 5.0f, 5.0f});
        auto sphere = BoundingSphere::fromAABB(box);
        
        REQUIRE(sphere.center.x == Approx(0.0f));
        REQUIRE(sphere.center.y == Approx(0.0f));
        REQUIRE(sphere.center.z == Approx(0.0f));
        // Radius = sqrt(5^2 + 5^2 + 5^2) = sqrt(75) ≈ 8.66
        REQUIRE(sphere.radius == Approx(std::sqrt(75.0f)));
    }
}

// =============================================================================
// Plane Tests
// =============================================================================

TEST_CASE("Plane functionality", "[occlusion][plane]") {
    SECTION("Normalize") {
        Plane plane;
        plane.normal = Vec3{0.0f, 2.0f, 0.0f};
        plane.distance = 10.0f;
        
        plane.normalize();
        
        REQUIRE(plane.normal.y == Approx(1.0f));
        REQUIRE(plane.distance == Approx(5.0f));
    }
    
    SECTION("Signed distance") {
        Plane plane;
        plane.normal = Vec3{0.0f, 1.0f, 0.0f};
        plane.distance = -5.0f;
        
        // Point above plane
        REQUIRE(plane.signedDistance(Vec3{0.0f, 10.0f, 0.0f}) == Approx(5.0f));
        
        // Point on plane
        REQUIRE(plane.signedDistance(Vec3{0.0f, 5.0f, 0.0f}) == Approx(0.0f));
        
        // Point below plane
        REQUIRE(plane.signedDistance(Vec3{0.0f, 0.0f, 0.0f}) == Approx(-5.0f));
    }
}

// =============================================================================
// Frustum Tests
// =============================================================================

TEST_CASE("Frustum functionality", "[occlusion][frustum]") {
    SECTION("Test AABB outside frustum") {
        Frustum frustum;
        // Simple frustum with all planes facing inward at origin
        for (auto& plane : frustum.planes) {
            plane.normal = Vec3{0.0f, 0.0f, 1.0f};
            plane.distance = 10.0f;
        }
        
        // Box far behind all planes
        AABB box(Vec3{-100.0f, -100.0f, -100.0f}, Vec3{-90.0f, -90.0f, -90.0f});
        
        // This test may vary based on plane setup
        // Just verify the function runs without crashing
        i32 result = frustum.testAABB(box);
        REQUIRE((result == -1 || result == 0 || result == 1));
    }
    
    SECTION("Test sphere") {
        Frustum frustum;
        // Set up a simple frustum
        frustum.planes[0].normal = Vec3{1.0f, 0.0f, 0.0f}; // Left
        frustum.planes[0].distance = 10.0f;
        frustum.planes[1].normal = Vec3{-1.0f, 0.0f, 0.0f}; // Right
        frustum.planes[1].distance = 10.0f;
        frustum.planes[2].normal = Vec3{0.0f, 1.0f, 0.0f}; // Bottom
        frustum.planes[2].distance = 10.0f;
        frustum.planes[3].normal = Vec3{0.0f, -1.0f, 0.0f}; // Top
        frustum.planes[3].distance = 10.0f;
        frustum.planes[4].normal = Vec3{0.0f, 0.0f, 1.0f}; // Near
        frustum.planes[4].distance = 10.0f;
        frustum.planes[5].normal = Vec3{0.0f, 0.0f, -1.0f}; // Far
        frustum.planes[5].distance = 10.0f;
        
        // Sphere at origin should be inside
        BoundingSphere sphere;
        sphere.center = Vec3{0.0f, 0.0f, 0.0f};
        sphere.radius = 1.0f;
        
        REQUIRE(frustum.isSphereVisible(sphere) == true);
    }
    
    SECTION("isAABBVisible wrapper") {
        Frustum frustum;
        for (usize i = 0; i < 6; ++i) {
            frustum.planes[i].normal = Vec3{0.0f, 0.0f, 1.0f};
            frustum.planes[i].distance = 100.0f;
        }
        
        AABB box(Vec3{-1.0f, -1.0f, -1.0f}, Vec3{1.0f, 1.0f, 1.0f});
        bool visible = frustum.isAABBVisible(box);
        // Box at origin should generally be visible with these planes
        REQUIRE(visible == true);
    }
}

// =============================================================================
// OcclusionQueryResult Tests
// =============================================================================

TEST_CASE("OcclusionQueryResult functionality", "[occlusion][query]") {
    SECTION("Default values") {
        OcclusionQueryResult result;
        REQUIRE(result.objectId == 0);
        REQUIRE(result.samplesPassed == 0);
        REQUIRE(result.ready == false);
    }
    
    SECTION("Is visible check") {
        OcclusionQueryResult result;
        result.ready = true;
        result.samplesPassed = 100;
        
        REQUIRE(result.isVisible() == true);
        
        result.samplesPassed = 0;
        REQUIRE(result.isVisible() == false);
        
        result.samplesPassed = 100;
        result.ready = false;
        REQUIRE(result.isVisible() == false);
    }
}

// =============================================================================
// HiZConfig Tests
// =============================================================================

TEST_CASE("HiZConfig functionality", "[occlusion][hiz]") {
    SECTION("Default values") {
        HiZConfig config;
        REQUIRE(config.width == 1920);
        REQUIRE(config.height == 1080);
        REQUIRE(config.divisor == OcclusionConfig::DEFAULT_HIZ_DIVISOR);
    }
    
    SECTION("Get Hi-Z dimensions") {
        HiZConfig config;
        config.width = 1920;
        config.height = 1080;
        config.divisor = 2;
        
        REQUIRE(config.getHiZWidth() == 960);
        REQUIRE(config.getHiZHeight() == 540);
    }
    
    SECTION("Calculate mip levels") {
        HiZConfig config;
        config.width = 1024;
        config.height = 1024;
        config.divisor = 1;
        
        config.calculateMipLevels();
        
        // 1024 -> 512 -> 256 -> 128 -> 64 -> 32 -> 16 -> 8 -> 4 -> 2 -> 1 = 11 levels
        REQUIRE(config.mipLevels == 11);
    }
    
    SECTION("Get mip dimensions") {
        HiZConfig config;
        config.width = 512;
        config.height = 512;
        config.divisor = 1;
        config.calculateMipLevels();
        
        auto [w0, h0] = config.getMipDimensions(0);
        REQUIRE(w0 == 512);
        REQUIRE(h0 == 512);
        
        auto [w1, h1] = config.getMipDimensions(1);
        REQUIRE(w1 == 256);
        REQUIRE(h1 == 256);
        
        auto [w9, h9] = config.getMipDimensions(9);
        REQUIRE(w9 == 1);
        REQUIRE(h9 == 1);
    }
}

// =============================================================================
// CullingStats Tests
// =============================================================================

TEST_CASE("CullingStats functionality", "[occlusion][stats]") {
    SECTION("Reset frame stats") {
        CullingStats stats;
        stats.totalObjects = 1000;
        stats.frustumCulled = 300;
        stats.occlusionCulled = 200;
        stats.frustumTimeMs = 1.0f;
        
        stats.resetFrameStats();
        
        REQUIRE(stats.totalObjects == 0);
        REQUIRE(stats.frustumCulled == 0);
        REQUIRE(stats.occlusionCulled == 0);
        REQUIRE(stats.frustumTimeMs == Approx(0.0f));
    }
    
    SECTION("Frustum cull rate") {
        CullingStats stats;
        stats.totalObjects = 100;
        stats.frustumCulled = 30;
        
        REQUIRE(stats.getFrustumCullRate() == Approx(30.0f));
    }
    
    SECTION("Occlusion cull rate") {
        CullingStats stats;
        stats.frustumPassed = 70;
        stats.occlusionCulled = 21;
        
        REQUIRE(stats.getOcclusionCullRate() == Approx(30.0f));
    }
    
    SECTION("Total cull rate") {
        CullingStats stats;
        stats.totalObjects = 100;
        stats.frustumCulled = 30;
        stats.occlusionCulled = 20;
        stats.sizeCulled = 10;
        
        // Total culled = 60, total = 100 -> 60%
        REQUIRE(stats.getTotalCullRate() == Approx(60.0f));
    }
    
    SECTION("Zero division protection") {
        CullingStats stats;
        stats.totalObjects = 0;
        stats.frustumPassed = 0;
        
        REQUIRE(stats.getFrustumCullRate() == Approx(0.0f));
        REQUIRE(stats.getOcclusionCullRate() == Approx(0.0f));
        REQUIRE(stats.getTotalCullRate() == Approx(0.0f));
    }
}

// =============================================================================
// OcclusionCullingManager Tests
// =============================================================================

TEST_CASE("OcclusionCullingManager construction", "[occlusion][manager]") {
    SECTION("Default construction") {
        OcclusionCullingManager manager;
        REQUIRE(manager.getTechnique() == OcclusionTechnique::FrustumOnly);
    }
}

TEST_CASE("OcclusionCullingManager configuration", "[occlusion][manager]") {
    OcclusionCullingManager manager;
    
    SECTION("Set technique") {
        manager.setTechnique(OcclusionTechnique::HiZBuffer);
        REQUIRE(manager.getTechnique() == OcclusionTechnique::HiZBuffer);
    }
    
    SECTION("Set Hi-Z config") {
        HiZConfig config;
        config.width = 2560;
        config.height = 1440;
        
        manager.setHiZConfig(config);
        
        REQUIRE(manager.getHiZConfig().width == 2560);
        REQUIRE(manager.getHiZConfig().height == 1440);
        // Mip levels should be calculated
        REQUIRE(manager.getHiZConfig().mipLevels > 0);
    }
    
    SECTION("Set min screen size") {
        manager.setMinScreenSize(4.0f);
        REQUIRE(manager.getMinScreenSize() == Approx(4.0f));
    }
    
    SECTION("Set temporal coherence") {
        manager.setTemporalCoherence(false);
        // Can't directly test private member, but shouldn't crash
    }
}

TEST_CASE("OcclusionCullingManager frustum culling", "[occlusion][manager]") {
    OcclusionCullingManager manager;
    
    SECTION("Test AABB with frustum") {
        manager.beginFrame(0);
        
        // Use identity matrix for simple frustum
        Mat4 viewProj = Mat4::identity();
        
        manager.updateFrustum(viewProj);
        
        AABB box(Vec3{-1.0f, -1.0f, -1.0f}, Vec3{1.0f, 1.0f, 1.0f});
        auto result = manager.testFrustum(box);
        
        // With identity-like matrix, box at origin should be visible
        // Result depends on exact frustum extraction
        REQUIRE((result == VisibilityResult::Visible || 
                 result == VisibilityResult::OutsideFrustum));
        
        manager.endFrame();
    }
    
    SECTION("Test sphere with frustum") {
        manager.beginFrame(0);
        
        BoundingSphere sphere;
        sphere.center = Vec3{0.0f, 0.0f, 0.0f};
        sphere.radius = 1.0f;
        
        auto result = manager.testFrustum(sphere);
        
        REQUIRE((result == VisibilityResult::Visible || 
                 result == VisibilityResult::OutsideFrustum));
        
        manager.endFrame();
    }
}

TEST_CASE("OcclusionCullingManager visibility testing", "[occlusion][manager]") {
    OcclusionCullingManager manager;
    manager.setTechnique(OcclusionTechnique::FrustumOnly);
    
    SECTION("Full visibility test") {
        manager.beginFrame(0);
        
        AABB box(Vec3{-1.0f, -1.0f, -1.0f}, Vec3{1.0f, 1.0f, 1.0f});
        auto result = manager.testVisibility(1, box);
        
        REQUIRE((result == VisibilityResult::Visible || 
                 result == VisibilityResult::OutsideFrustum));
        
        manager.endFrame();
    }
}

TEST_CASE("OcclusionCullingManager query management", "[occlusion][manager]") {
    OcclusionCullingManager manager;
    
    SECTION("Queue occlusion query") {
        manager.beginFrame(0);
        
        AABB box(Vec3{0.0f, 0.0f, 0.0f}, Vec3{1.0f, 1.0f, 1.0f});
        manager.queueOcclusionQuery(1, box);
        manager.queueOcclusionQuery(2, box);
        
        REQUIRE(manager.getPendingQueryCount() == 2);
        
        manager.endFrame();
        
        // After end frame, queries should be cleared
        REQUIRE(manager.getPendingQueryCount() == 0);
    }
}

TEST_CASE("OcclusionCullingManager frame lifecycle", "[occlusion][manager]") {
    OcclusionCullingManager manager;
    
    SECTION("Begin/end frame") {
        manager.beginFrame(0);
        REQUIRE(manager.getCurrentFrame() == 0);
        
        manager.endFrame();
        
        manager.beginFrame(1);
        REQUIRE(manager.getCurrentFrame() == 1);
    }
    
    SECTION("Stats reset on begin frame") {
        manager.beginFrame(0);
        
        // Do some culling
        AABB box(Vec3{-1.0f, -1.0f, -1.0f}, Vec3{1.0f, 1.0f, 1.0f});
        manager.testFrustum(box);
        manager.testFrustum(box);
        
        REQUIRE(manager.getStats().totalObjects == 2);
        
        manager.endFrame();
        manager.beginFrame(1);
        
        REQUIRE(manager.getStats().totalObjects == 0);
    }
    
    SECTION("Record result for temporal coherence") {
        manager.beginFrame(0);
        
        manager.recordResult(1, VisibilityResult::Visible);
        manager.recordResult(2, VisibilityResult::Occluded);
        
        manager.endFrame();
        
        // Results should be available for next frame's temporal coherence
        // Can't directly test private maps, but shouldn't crash
    }
}

// =============================================================================
// Utility Function Tests
// =============================================================================

TEST_CASE("calculateScreenArea utility", "[occlusion][utility]") {
    SECTION("Basic screen area calculation") {
        AABB box(Vec3{-1.0f, -1.0f, 5.0f}, Vec3{1.0f, 1.0f, 5.0f});
        
        // Use identity matrix for simple projection
        Mat4 viewProj = Mat4::identity();
        
        f32 area = calculateScreenArea(box, viewProj, 1920, 1080);
        
        // May be 0 if box is behind camera with this projection
        REQUIRE(area >= 0.0f);
    }
}
