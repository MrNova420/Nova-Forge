/**
 * @file test_ray_tracing.cpp
 * @brief Comprehensive tests for the ray tracing system
 * @author NovaForge Team
 * @copyright Copyright (c) 2024-2025 NovaForge - All Rights Reserved
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/ray_tracing.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// RT CONFIGURATION TESTS
// =============================================================================

TEST_CASE("RTConfig constants are valid", "[ray_tracing][config]") {
    SECTION("Maximum values are reasonable") {
        REQUIRE(RTConfig::MAX_RECURSION_DEPTH >= 1);
        REQUIRE(RTConfig::MAX_RECURSION_DEPTH <= 64);
        REQUIRE(RTConfig::MAX_MISS_SHADERS > 0);
        REQUIRE(RTConfig::MAX_HIT_SHADERS > 0);
        REQUIRE(RTConfig::MAX_CALLABLE_SHADERS > 0);
    }
    
    SECTION("Geometry limits are reasonable") {
        REQUIRE(RTConfig::MAX_GEOMETRY_PER_BLAS > 0);
        REQUIRE(RTConfig::MAX_INSTANCES_PER_TLAS > 0);
    }
    
    SECTION("Default values are sensible") {
        REQUIRE(RTConfig::DEFAULT_RT_SCALE > 0.0f);
        REQUIRE(RTConfig::DEFAULT_RT_SCALE <= 1.0f);
        REQUIRE(RTConfig::DEFAULT_SPP >= 1);
        REQUIRE(RTConfig::MAX_GI_BOUNCES > 0);
    }
    
    SECTION("SBT alignment is power of 2") {
        REQUIRE((RTConfig::SBT_ALIGNMENT & (RTConfig::SBT_ALIGNMENT - 1)) == 0);
    }
}

// =============================================================================
// RT TECHNIQUE TESTS
// =============================================================================

TEST_CASE("RTTechnique enumeration", "[ray_tracing][enums]") {
    SECTION("All techniques have valid names") {
        REQUIRE(std::string(getRTTechniqueName(RTTechnique::None)) == "None");
        REQUIRE(std::string(getRTTechniqueName(RTTechnique::Shadows)) == "Shadows");
        REQUIRE(std::string(getRTTechniqueName(RTTechnique::AO)) == "Ambient Occlusion");
        REQUIRE(std::string(getRTTechniqueName(RTTechnique::Reflections)) == "Reflections");
        REQUIRE(std::string(getRTTechniqueName(RTTechnique::GI)) == "Global Illumination");
        REQUIRE(std::string(getRTTechniqueName(RTTechnique::PathTracing)) == "Path Tracing");
        REQUIRE(std::string(getRTTechniqueName(RTTechnique::Hybrid)) == "Hybrid");
    }
}

TEST_CASE("ASBuildFlags bitwise operations", "[ray_tracing][flags]") {
    SECTION("Flags can be combined") {
        ASBuildFlags combined = ASBuildFlags::AllowUpdate | ASBuildFlags::PreferFastTrace;
        REQUIRE(hasFlag(combined, ASBuildFlags::AllowUpdate));
        REQUIRE(hasFlag(combined, ASBuildFlags::PreferFastTrace));
        REQUIRE_FALSE(hasFlag(combined, ASBuildFlags::AllowCompaction));
    }
    
    SECTION("Flags can be tested individually") {
        REQUIRE_FALSE(hasFlag(ASBuildFlags::None, ASBuildFlags::AllowUpdate));
        REQUIRE(hasFlag(ASBuildFlags::AllowUpdate, ASBuildFlags::AllowUpdate));
    }
}

TEST_CASE("RayFlags bitwise operations", "[ray_tracing][flags]") {
    RayFlags combined = RayFlags::Opaque | RayFlags::TerminateOnFirstHit;
    REQUIRE((static_cast<u32>(combined) & static_cast<u32>(RayFlags::Opaque)) != 0);
    REQUIRE((static_cast<u32>(combined) & static_cast<u32>(RayFlags::TerminateOnFirstHit)) != 0);
}

// =============================================================================
// RAY TESTS
// =============================================================================

TEST_CASE("Ray structure operations", "[ray_tracing][ray]") {
    SECTION("Ray creation") {
        Ray ray = Ray::create(Vec3{0, 0, 0}, Vec3{0, 0, -1});
        REQUIRE(ray.origin.x == Approx(0.0f));
        REQUIRE(ray.origin.y == Approx(0.0f));
        REQUIRE(ray.origin.z == Approx(0.0f));
        REQUIRE(ray.direction.z == Approx(-1.0f));
        REQUIRE(ray.tMin == Approx(0.001f));
        REQUIRE(ray.tMax == Approx(10000.0f));
    }
    
    SECTION("Ray point calculation") {
        Ray ray = Ray::create(Vec3{1, 2, 3}, Vec3{1, 0, 0});
        Vec3 point = ray.at(5.0f);
        REQUIRE(point.x == Approx(6.0f));
        REQUIRE(point.y == Approx(2.0f));
        REQUIRE(point.z == Approx(3.0f));
    }
    
    SECTION("Ray direction is normalized") {
        Ray ray = Ray::create(Vec3{0, 0, 0}, Vec3{3, 4, 0});
        f32 length = std::sqrt(ray.direction.x * ray.direction.x +
                              ray.direction.y * ray.direction.y +
                              ray.direction.z * ray.direction.z);
        REQUIRE(length == Approx(1.0f).margin(0.001f));
    }
}

// =============================================================================
// RAY HIT TESTS
// =============================================================================

TEST_CASE("RayHit structure", "[ray_tracing][hit]") {
    SECTION("Default hit is invalid") {
        RayHit hit;
        REQUIRE_FALSE(hit.isValid());
    }
    
    SECTION("Valid hit has positive t") {
        RayHit hit;
        hit.t = 5.0f;
        REQUIRE(hit.isValid());
    }
}

// =============================================================================
// TRIANGLE GEOMETRY TESTS
// =============================================================================

TEST_CASE("RTTriangleGeometry creation", "[ray_tracing][geometry]") {
    RTTriangleGeometry geom = RTTriangleGeometry::create(1000, 2000, 32, 100, 300);
    
    REQUIRE(geom.vertexBufferAddress == 1000);
    REQUIRE(geom.indexBufferAddress == 2000);
    REQUIRE(geom.vertexStride == 32);
    REQUIRE(geom.vertexCount == 100);
    REQUIRE(geom.indexCount == 300);
    REQUIRE(geom.triangleCount == 100);
}

// =============================================================================
// BLAS DESCRIPTION TESTS
// =============================================================================

TEST_CASE("BLASDesc operations", "[ray_tracing][blas]") {
    BLASDesc desc;
    desc.name = "TestBLAS";
    
    SECTION("Empty BLAS is invalid") {
        REQUIRE_FALSE(desc.isValid());
        REQUIRE(desc.getGeometryCount() == 0);
    }
    
    SECTION("BLAS with triangles is valid") {
        RTTriangleGeometry geom = RTTriangleGeometry::create(1000, 2000, 32, 100, 300);
        desc.addTriangles(geom);
        
        REQUIRE(desc.isValid());
        REQUIRE(desc.getGeometryCount() == 1);
    }
    
    SECTION("BLAS can have multiple geometries") {
        desc.addTriangles(RTTriangleGeometry::create(1000, 2000, 32, 100, 300));
        desc.addTriangles(RTTriangleGeometry::create(3000, 4000, 32, 200, 600));
        
        REQUIRE(desc.getGeometryCount() == 2);
    }
}

// =============================================================================
// RT INSTANCE TESTS
// =============================================================================

TEST_CASE("RTInstance creation", "[ray_tracing][instance]") {
    RTInstance inst = RTInstance::create(Mat4::identity(), 42, 12345);
    
    REQUIRE(inst.instanceID == 42);
    REQUIRE(inst.blasAddress == 12345);
    REQUIRE(inst.mask == 0xFF);
    
    SECTION("Custom index is masked to 24 bits") {
        inst.setCustomIndex(0xFFFFFFFF);
        REQUIRE(inst.instanceID == 0x00FFFFFF);
    }
}

// =============================================================================
// TLAS DESCRIPTION TESTS
// =============================================================================

TEST_CASE("TLASDesc operations", "[ray_tracing][tlas]") {
    TLASDesc desc;
    desc.name = "TestTLAS";
    
    SECTION("Empty TLAS is invalid") {
        REQUIRE_FALSE(desc.isValid());
        REQUIRE(desc.getInstanceCount() == 0);
    }
    
    SECTION("TLAS with instances is valid") {
        RTInstance inst = RTInstance::create(Mat4::identity(), 0, 1000);
        desc.addInstance(inst);
        
        REQUIRE(desc.isValid());
        REQUIRE(desc.getInstanceCount() == 1);
    }
    
    SECTION("Instance transform can be updated") {
        desc.addInstance(RTInstance::create(Mat4::identity(), 0, 1000));
        
        Mat4 newTransform = Mat4::identity();
        desc.updateInstanceTransform(0, newTransform);
        // Should not crash
    }
}

// =============================================================================
// ACCELERATION STRUCTURE TESTS
// =============================================================================

TEST_CASE("AccelerationStructure validity", "[ray_tracing][as]") {
    AccelerationStructure as;
    
    SECTION("Default is invalid") {
        REQUIRE_FALSE(as.isValid());
    }
    
    SECTION("With handle and address is valid") {
        as.handle = 1;
        as.deviceAddress = 1;
        REQUIRE(as.isValid());
    }
}

// =============================================================================
// SHADER BINDING TABLE TESTS
// =============================================================================

TEST_CASE("SBTEntry operations", "[ray_tracing][sbt]") {
    SBTEntry entry;
    entry.shaderHandle = 12345;
    
    SECTION("Local data can be set") {
        struct TestData { u32 value; f32 scale; };
        TestData data{42, 1.5f};
        entry.setLocalData(data);
        
        REQUIRE(entry.localData.size() == sizeof(TestData));
    }
    
    SECTION("Aligned size calculation") {
        u32 size = entry.getAlignedSize(32, 64);
        REQUIRE(size >= 32);
        REQUIRE((size % 64) == 0);
    }
}

TEST_CASE("ShaderBindingTable operations", "[ray_tracing][sbt]") {
    ShaderBindingTable sbt;
    
    SBTEntry rayGen;
    rayGen.shaderHandle = 1;
    sbt.addRayGen(rayGen);
    
    SBTEntry miss;
    miss.shaderHandle = 2;
    sbt.addMiss(miss);
    
    SBTEntry hitGroup;
    hitGroup.shaderHandle = 3;
    sbt.addHitGroup(hitGroup);
    
    SECTION("Entries are added correctly") {
        REQUIRE(sbt.rayGenEntries.size() == 1);
        REQUIRE(sbt.missEntries.size() == 1);
        REQUIRE(sbt.hitGroupEntries.size() == 1);
    }
    
    SECTION("Buffer size is calculated") {
        u64 size = sbt.calculateBufferSize(32, 64);
        REQUIRE(size > 0);
    }
}

// =============================================================================
// RT SETTINGS TESTS
// =============================================================================

TEST_CASE("RTShadowSettings factories", "[ray_tracing][settings]") {
    SECTION("Hard shadows") {
        auto settings = RTShadowSettings::hardShadows();
        REQUIRE(settings.samplesPerPixel == 1);
        REQUIRE(settings.softShadowRadius == Approx(0.0f));
    }
    
    SECTION("Soft shadows") {
        auto settings = RTShadowSettings::softShadows(4, 0.1f);
        REQUIRE(settings.samplesPerPixel == 4);
        REQUIRE(settings.softShadowRadius == Approx(0.1f));
    }
}

TEST_CASE("RTAOSettings factories", "[ray_tracing][settings]") {
    SECTION("Low quality") {
        auto settings = RTAOSettings::low();
        REQUIRE(settings.samplesPerPixel == 1);
    }
    
    SECTION("Medium quality") {
        auto settings = RTAOSettings::medium();
        REQUIRE(settings.samplesPerPixel == 4);
    }
    
    SECTION("High quality") {
        auto settings = RTAOSettings::high();
        REQUIRE(settings.samplesPerPixel == 8);
    }
}

TEST_CASE("RTReflectionSettings factories", "[ray_tracing][settings]") {
    SECTION("Performance mode") {
        auto settings = RTReflectionSettings::performance();
        REQUIRE(settings.maxBounces == 1);
        REQUIRE(settings.resolutionScale == Approx(0.5f));
    }
    
    SECTION("Quality mode") {
        auto settings = RTReflectionSettings::quality();
        REQUIRE(settings.maxBounces == 3);
        REQUIRE(settings.resolutionScale == Approx(0.75f));
    }
}

TEST_CASE("RTGISettings factories", "[ray_tracing][settings]") {
    SECTION("Low quality") {
        auto settings = RTGISettings::low();
        REQUIRE(settings.maxBounces == 1);
        REQUIRE(settings.resolutionScale == Approx(0.25f));
    }
    
    SECTION("Medium quality") {
        auto settings = RTGISettings::medium();
        REQUIRE(settings.maxBounces == 2);
        REQUIRE(settings.resolutionScale == Approx(0.5f));
    }
    
    SECTION("High quality") {
        auto settings = RTGISettings::high();
        REQUIRE(settings.maxBounces == 3);
        REQUIRE(settings.resolutionScale == Approx(0.75f));
    }
}

TEST_CASE("PathTracingSettings factories", "[ray_tracing][settings]") {
    SECTION("Realtime mode") {
        auto settings = PathTracingSettings::realtime();
        REQUIRE(settings.samplesPerPixel == 1);
        REQUIRE(settings.maxBounces == 4);
        REQUIRE(settings.denoise == true);
    }
    
    SECTION("Progressive mode") {
        auto settings = PathTracingSettings::progressive();
        REQUIRE(settings.maxAccumulatedSamples == 4096);
        REQUIRE(settings.maxBounces == 8);
    }
}

TEST_CASE("DenoiserSettings factories", "[ray_tracing][settings]") {
    SECTION("SVGF denoiser") {
        auto settings = DenoiserSettings::svgf();
        REQUIRE(settings.type == RTDenoiser::SVGF);
    }
    
    SECTION("Temporal denoiser") {
        auto settings = DenoiserSettings::temporal();
        REQUIRE(settings.type == RTDenoiser::Temporal);
        REQUIRE(settings.historyLength == 16);
    }
}

// =============================================================================
// RT MANAGER TESTS
// =============================================================================

TEST_CASE("RayTracingManager initialization", "[ray_tracing][manager]") {
    auto& manager = RayTracingManager::instance();
    
    SECTION("Can initialize") {
        REQUIRE(manager.initialize());
        REQUIRE(manager.isInitialized());
    }
    
    SECTION("Capabilities are available") {
        manager.initialize();
        const auto& caps = manager.getCapabilities();
        REQUIRE(caps.supported);
        REQUIRE(caps.canUseRayTracing());
    }
}

TEST_CASE("RayTracingManager BLAS creation", "[ray_tracing][manager]") {
    auto& manager = RayTracingManager::instance();
    manager.initialize();
    
    BLASDesc desc;
    desc.name = "TestBLAS";
    desc.addTriangles(RTTriangleGeometry::create(1000, 2000, 32, 100, 300));
    
    AccelerationStructure blas = manager.createBLAS(desc);
    
    SECTION("BLAS is created successfully") {
        REQUIRE(blas.isValid());
        REQUIRE(blas.type == AccelerationStructureType::BottomLevel);
        REQUIRE(blas.bufferSize > 0);
    }
    
    SECTION("BLAS can be retrieved by name") {
        const AccelerationStructure* cached = manager.getBLAS("TestBLAS");
        REQUIRE(cached != nullptr);
        REQUIRE(cached->handle == blas.handle);
    }
}

TEST_CASE("RayTracingManager TLAS creation", "[ray_tracing][manager]") {
    auto& manager = RayTracingManager::instance();
    manager.initialize();
    
    TLASDesc desc;
    desc.name = "TestTLAS";
    desc.addInstance(RTInstance::create(Mat4::identity(), 0, 1000));
    
    AccelerationStructure tlas = manager.createTLAS(desc);
    
    REQUIRE(tlas.isValid());
    REQUIRE(tlas.type == AccelerationStructureType::TopLevel);
    REQUIRE(tlas.bufferSize > 0);
}

TEST_CASE("RayTracingManager settings", "[ray_tracing][manager]") {
    auto& manager = RayTracingManager::instance();
    manager.initialize();
    
    SECTION("Set and get technique") {
        manager.setTechnique(RTTechnique::PathTracing);
        REQUIRE(manager.getTechnique() == RTTechnique::PathTracing);
    }
    
    SECTION("Set and get shadow settings") {
        auto settings = RTShadowSettings::softShadows();
        manager.setShadowSettings(settings);
        REQUIRE(manager.getShadowSettings().samplesPerPixel == settings.samplesPerPixel);
    }
    
    SECTION("Set and get AO settings") {
        auto settings = RTAOSettings::high();
        manager.setAOSettings(settings);
        REQUIRE(manager.getAOSettings().samplesPerPixel == settings.samplesPerPixel);
    }
}

TEST_CASE("RayTracingManager technique support", "[ray_tracing][manager]") {
    auto& manager = RayTracingManager::instance();
    manager.initialize();
    
    SECTION("Basic techniques are supported") {
        REQUIRE(manager.isTechniqueSupported(RTTechnique::None));
        REQUIRE(manager.isTechniqueSupported(RTTechnique::Shadows));
        REQUIRE(manager.isTechniqueSupported(RTTechnique::AO));
        REQUIRE(manager.isTechniqueSupported(RTTechnique::Reflections));
    }
}

// =============================================================================
// UTILITY FUNCTION TESTS
// =============================================================================

TEST_CASE("Fresnel calculations", "[ray_tracing][utility]") {
    SECTION("Fresnel at perpendicular is F0") {
        f32 f = fresnelSchlick(1.0f, 0.04f);
        REQUIRE(f == Approx(0.04f));
    }
    
    SECTION("Fresnel at grazing angle approaches 1") {
        f32 f = fresnelSchlick(0.0f, 0.04f);
        REQUIRE(f == Approx(1.0f));
    }
}

TEST_CASE("GGX distribution", "[ray_tracing][utility]") {
    Vec3 n{0.0f, 1.0f, 0.0f};
    Vec3 h{0.0f, 1.0f, 0.0f};
    
    SECTION("Maximum at aligned normal") {
        f32 d = distributionGGX(n, h, 0.5f);
        REQUIRE(d > 0.0f);
    }
}

TEST_CASE("Geometry Smith function", "[ray_tracing][utility]") {
    Vec3 n{0.0f, 1.0f, 0.0f};
    Vec3 v{0.0f, 1.0f, 0.0f};
    Vec3 l{0.0f, 1.0f, 0.0f};
    
    f32 g = geometrySmith(n, v, l, 0.5f);
    REQUIRE(g > 0.0f);
    REQUIRE(g <= 1.0f);
}

TEST_CASE("Hemisphere sampling produces valid directions", "[ray_tracing][utility]") {
    Vec3 normal{0.0f, 1.0f, 0.0f};
    
    for (int i = 0; i < 10; ++i) {
        Vec2 u{static_cast<f32>(i) / 10.0f, static_cast<f32>(i) / 10.0f};
        Vec3 dir = cosineWeightedHemisphere(u, normal);
        
        // Direction should be normalized
        f32 length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        REQUIRE(length == Approx(1.0f).margin(0.01f));
        
        // Direction should be in hemisphere (dot with normal >= 0)
        f32 dot = dir.x * normal.x + dir.y * normal.y + dir.z * normal.z;
        REQUIRE(dot >= -0.01f);
    }
}

TEST_CASE("Sphere sampling produces valid directions", "[ray_tracing][utility]") {
    for (int i = 0; i < 10; ++i) {
        Vec2 u{static_cast<f32>(i) / 10.0f, static_cast<f32>(i) / 10.0f};
        Vec3 dir = uniformSphere(u);
        
        // Direction should be normalized
        f32 length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        REQUIRE(length == Approx(1.0f).margin(0.01f));
    }
}
