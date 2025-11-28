/**
 * @file test_global_illumination.cpp
 * @brief Comprehensive tests for the global illumination system
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/global_illumination.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// GIConfig Constants Tests
// =============================================================================

TEST_CASE("GIConfig constants are valid", "[gi][config]") {
    SECTION("Probe grid limits") {
        REQUIRE(GIConfig::MAX_PROBE_GRID_DIM == 64);
        REQUIRE(GIConfig::MAX_PROBES == 64 * 64 * 64);
    }
    
    SECTION("Spherical harmonics") {
        REQUIRE(GIConfig::SH_COEFFICIENT_COUNT == 9);
    }
    
    SECTION("Reflection probe settings") {
        REQUIRE(GIConfig::MAX_REFLECTION_PROBE_RES == 1024);
        REQUIRE(GIConfig::DEFAULT_REFLECTION_PROBE_RES == 256);
    }
    
    SECTION("Ray tracing settings") {
        REQUIRE(GIConfig::MAX_RAY_BOUNCES == 8);
        REQUIRE(GIConfig::DEFAULT_RAY_BOUNCES == 2);
    }
    
    SECTION("SSGI and voxel settings") {
        REQUIRE(GIConfig::SSGI_MAX_STEPS == 32);
        REQUIRE(GIConfig::VOXEL_GRID_DEFAULT_RES == 128);
        REQUIRE(GIConfig::LPV_CASCADE_COUNT == 3);
    }
    
    SECTION("Probe texture sizes") {
        REQUIRE(GIConfig::IRRADIANCE_PROBE_SIZE == 8);
        REQUIRE(GIConfig::DISTANCE_PROBE_SIZE == 16);
    }
}

// =============================================================================
// GITechnique Enum Tests
// =============================================================================

TEST_CASE("GITechnique enum values", "[gi][technique]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(GITechnique::None) == 0);
        REQUIRE(static_cast<u32>(GITechnique::SSGI) == 1);
        REQUIRE(static_cast<u32>(GITechnique::LPV) == 2);
        REQUIRE(static_cast<u32>(GITechnique::VXGI) == 3);
        REQUIRE(static_cast<u32>(GITechnique::RTGI) == 4);
        REQUIRE(static_cast<u32>(GITechnique::ProbeGrid) == 5);
        REQUIRE(static_cast<u32>(GITechnique::Hybrid) == 6);
        REQUIRE(static_cast<u32>(GITechnique::COUNT) == 7);
    }
    
    SECTION("Technique names") {
        REQUIRE(std::string(getGITechniqueName(GITechnique::None)) == "None");
        REQUIRE(std::string(getGITechniqueName(GITechnique::SSGI)) == "SSGI");
        REQUIRE(std::string(getGITechniqueName(GITechnique::LPV)) == "LPV");
        REQUIRE(std::string(getGITechniqueName(GITechnique::VXGI)) == "VXGI");
        REQUIRE(std::string(getGITechniqueName(GITechnique::RTGI)) == "RTGI");
        REQUIRE(std::string(getGITechniqueName(GITechnique::ProbeGrid)) == "Probe Grid");
        REQUIRE(std::string(getGITechniqueName(GITechnique::Hybrid)) == "Hybrid");
    }
}

// =============================================================================
// GIQuality Enum Tests
// =============================================================================

TEST_CASE("GIQuality enum values", "[gi][quality]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(GIQuality::Low) == 0);
        REQUIRE(static_cast<u32>(GIQuality::Medium) == 1);
        REQUIRE(static_cast<u32>(GIQuality::High) == 2);
        REQUIRE(static_cast<u32>(GIQuality::Ultra) == 3);
        REQUIRE(static_cast<u32>(GIQuality::COUNT) == 4);
    }
    
    SECTION("Quality names") {
        REQUIRE(std::string(getGIQualityName(GIQuality::Low)) == "Low");
        REQUIRE(std::string(getGIQualityName(GIQuality::Medium)) == "Medium");
        REQUIRE(std::string(getGIQualityName(GIQuality::High)) == "High");
        REQUIRE(std::string(getGIQualityName(GIQuality::Ultra)) == "Ultra");
    }
}

// =============================================================================
// ProbeState Enum Tests
// =============================================================================

TEST_CASE("ProbeState enum values", "[gi][probe]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(ProbeState::Invalid) == 0);
        REQUIRE(static_cast<u32>(ProbeState::Updating) == 1);
        REQUIRE(static_cast<u32>(ProbeState::Valid) == 2);
        REQUIRE(static_cast<u32>(ProbeState::Disabled) == 3);
    }
    
    SECTION("State names") {
        REQUIRE(std::string(getProbeStateName(ProbeState::Invalid)) == "Invalid");
        REQUIRE(std::string(getProbeStateName(ProbeState::Updating)) == "Updating");
        REQUIRE(std::string(getProbeStateName(ProbeState::Valid)) == "Valid");
        REQUIRE(std::string(getProbeStateName(ProbeState::Disabled)) == "Disabled");
    }
}

// =============================================================================
// SphericalHarmonics Tests
// =============================================================================

TEST_CASE("SphericalHarmonics functionality", "[gi][sh]") {
    SECTION("Default constructor creates zero SH") {
        SphericalHarmonics sh;
        for (const auto& c : sh.coefficients) {
            REQUIRE(c.x == Approx(0.0f));
            REQUIRE(c.y == Approx(0.0f));
            REQUIRE(c.z == Approx(0.0f));
        }
    }
    
    SECTION("Add sample") {
        SphericalHarmonics sh;
        Vec3 dir{0.0f, 1.0f, 0.0f}; // Up direction
        Vec3 color{1.0f, 0.5f, 0.25f};
        
        sh.addSample(dir, color, 1.0f);
        
        // L0 should have some contribution
        REQUIRE(sh.coefficients[0].x > 0.0f);
        REQUIRE(sh.coefficients[0].y > 0.0f);
        REQUIRE(sh.coefficients[0].z > 0.0f);
    }
    
    SECTION("Evaluate returns reasonable values") {
        SphericalHarmonics sh;
        Vec3 dir{0.0f, 1.0f, 0.0f};
        Vec3 color{1.0f, 1.0f, 1.0f};
        
        sh.addSample(dir, color, 4.0f * 3.14159f);
        
        Vec3 result = sh.evaluate(dir);
        REQUIRE(result.x > 0.0f);
    }
    
    SECTION("Scale coefficients") {
        SphericalHarmonics sh;
        sh.coefficients[0] = Vec3{1.0f, 2.0f, 3.0f};
        
        sh.scale(2.0f);
        
        REQUIRE(sh.coefficients[0].x == Approx(2.0f));
        REQUIRE(sh.coefficients[0].y == Approx(4.0f));
        REQUIRE(sh.coefficients[0].z == Approx(6.0f));
    }
    
    SECTION("Add two SH") {
        SphericalHarmonics sh1, sh2;
        sh1.coefficients[0] = Vec3{1.0f, 2.0f, 3.0f};
        sh2.coefficients[0] = Vec3{0.5f, 0.5f, 0.5f};
        
        sh1.add(sh2);
        
        REQUIRE(sh1.coefficients[0].x == Approx(1.5f));
        REQUIRE(sh1.coefficients[0].y == Approx(2.5f));
        REQUIRE(sh1.coefficients[0].z == Approx(3.5f));
    }
    
    SECTION("Lerp between SH") {
        SphericalHarmonics sh1, sh2;
        sh1.coefficients[0] = Vec3{0.0f, 0.0f, 0.0f};
        sh2.coefficients[0] = Vec3{1.0f, 1.0f, 1.0f};
        
        auto result = SphericalHarmonics::lerp(sh1, sh2, 0.5f);
        
        REQUIRE(result.coefficients[0].x == Approx(0.5f));
        REQUIRE(result.coefficients[0].y == Approx(0.5f));
        REQUIRE(result.coefficients[0].z == Approx(0.5f));
    }
    
    SECTION("Get dominant direction") {
        SphericalHarmonics sh;
        // Add strong up direction
        sh.coefficients[1] = Vec3{10.0f, 10.0f, 10.0f}; // Y component
        sh.coefficients[3] = Vec3{0.0f, 0.0f, 0.0f};    // X component
        sh.coefficients[2] = Vec3{0.0f, 0.0f, 0.0f};    // Z component
        
        Vec3 dominant = sh.getDominantDirection();
        
        // Should point roughly up
        REQUIRE(dominant.y > 0.9f);
    }
}

// =============================================================================
// IrradianceProbe Tests
// =============================================================================

TEST_CASE("IrradianceProbe functionality", "[gi][probe]") {
    SECTION("Default values") {
        IrradianceProbe probe;
        REQUIRE(probe.state == ProbeState::Invalid);
        REQUIRE(probe.influenceRadius == Approx(10.0f));
        REQUIRE(probe.lastUpdateFrame == 0);
    }
    
    SECTION("Needs update check") {
        IrradianceProbe probe;
        probe.state = ProbeState::Invalid;
        REQUIRE(probe.needsUpdate(100) == true);
        
        probe.state = ProbeState::Valid;
        probe.lastUpdateFrame = 100;
        REQUIRE(probe.needsUpdate(150, 60) == false);
        REQUIRE(probe.needsUpdate(200, 60) == true);
        
        probe.state = ProbeState::Disabled;
        REQUIRE(probe.needsUpdate(1000) == false);
    }
    
    SECTION("Calculate influence") {
        IrradianceProbe probe;
        probe.position = Vec3{0.0f, 0.0f, 0.0f};
        probe.influenceRadius = 10.0f;
        
        // At center
        REQUIRE(probe.calculateInfluence(Vec3{0.0f, 0.0f, 0.0f}) == Approx(1.0f));
        
        // Outside radius
        REQUIRE(probe.calculateInfluence(Vec3{15.0f, 0.0f, 0.0f}) == Approx(0.0f));
        
        // At edge (should be close to 0)
        f32 edgeInfluence = probe.calculateInfluence(Vec3{10.0f, 0.0f, 0.0f});
        REQUIRE(edgeInfluence >= 0.0f);
        REQUIRE(edgeInfluence <= 0.1f);
    }
}

// =============================================================================
// GPUIrradianceProbe Tests
// =============================================================================

TEST_CASE("GPUIrradianceProbe creation", "[gi][probe][gpu]") {
    SECTION("Create from IrradianceProbe") {
        IrradianceProbe probe;
        probe.position = Vec3{10.0f, 20.0f, 30.0f};
        probe.influenceRadius = 15.0f;
        probe.state = ProbeState::Valid;
        
        auto gpu = GPUIrradianceProbe::fromProbe(probe);
        
        REQUIRE(gpu.positionRadius.x == Approx(10.0f));
        REQUIRE(gpu.positionRadius.y == Approx(20.0f));
        REQUIRE(gpu.positionRadius.z == Approx(30.0f));
        REQUIRE(gpu.positionRadius.w == Approx(15.0f));
        REQUIRE(gpu.state == static_cast<u32>(ProbeState::Valid));
    }
}

// =============================================================================
// ProbeGridConfig Tests
// =============================================================================

TEST_CASE("ProbeGridConfig functionality", "[gi][grid]") {
    SECTION("Default values") {
        ProbeGridConfig config;
        REQUIRE(config.gridSizeX == 8);
        REQUIRE(config.gridSizeY == 4);
        REQUIRE(config.gridSizeZ == 8);
    }
    
    SECTION("Get total probes") {
        ProbeGridConfig config;
        config.gridSizeX = 4;
        config.gridSizeY = 4;
        config.gridSizeZ = 4;
        
        REQUIRE(config.getTotalProbes() == 64);
    }
    
    SECTION("Get probe spacing") {
        ProbeGridConfig config;
        config.gridSizeX = 5;
        config.gridSizeY = 5;
        config.gridSizeZ = 5;
        config.boundsMin = Vec3{0.0f, 0.0f, 0.0f};
        config.boundsMax = Vec3{40.0f, 40.0f, 40.0f};
        
        Vec3 spacing = config.getProbeSpacing();
        
        REQUIRE(spacing.x == Approx(10.0f));
        REQUIRE(spacing.y == Approx(10.0f));
        REQUIRE(spacing.z == Approx(10.0f));
    }
    
    SECTION("Get probe position") {
        ProbeGridConfig config;
        config.gridSizeX = 3;
        config.gridSizeY = 3;
        config.gridSizeZ = 3;
        config.boundsMin = Vec3{0.0f, 0.0f, 0.0f};
        config.boundsMax = Vec3{20.0f, 20.0f, 20.0f};
        
        Vec3 pos = config.getProbePosition(1, 1, 1);
        
        REQUIRE(pos.x == Approx(10.0f));
        REQUIRE(pos.y == Approx(10.0f));
        REQUIRE(pos.z == Approx(10.0f));
    }
    
    SECTION("Get probe index") {
        ProbeGridConfig config;
        config.gridSizeX = 4;
        config.gridSizeY = 4;
        config.gridSizeZ = 4;
        
        REQUIRE(config.getProbeIndex(0, 0, 0) == 0);
        REQUIRE(config.getProbeIndex(1, 0, 0) == 1);
        REQUIRE(config.getProbeIndex(0, 1, 0) == 4);
        REQUIRE(config.getProbeIndex(0, 0, 1) == 16);
    }
    
    SECTION("Get grid coords from position") {
        ProbeGridConfig config;
        config.gridSizeX = 4;
        config.gridSizeY = 4;
        config.gridSizeZ = 4;
        config.boundsMin = Vec3{0.0f, 0.0f, 0.0f};
        config.boundsMax = Vec3{30.0f, 30.0f, 30.0f};
        
        u32 x, y, z;
        bool valid = config.getGridCoords(Vec3{15.0f, 15.0f, 15.0f}, x, y, z);
        
        REQUIRE(valid == true);
        // Position at center should give middle indices
    }
    
    SECTION("Grid coords outside bounds") {
        ProbeGridConfig config;
        config.boundsMin = Vec3{0.0f, 0.0f, 0.0f};
        config.boundsMax = Vec3{10.0f, 10.0f, 10.0f};
        
        u32 x, y, z;
        bool valid = config.getGridCoords(Vec3{-5.0f, 0.0f, 0.0f}, x, y, z);
        
        REQUIRE(valid == false);
    }
}

// =============================================================================
// ReflectionProbe Tests
// =============================================================================

TEST_CASE("ReflectionProbe functionality", "[gi][reflection]") {
    SECTION("Default values") {
        ReflectionProbe probe;
        REQUIRE(probe.type == ReflectionProbeType::Box);
        REQUIRE(probe.resolution == GIConfig::DEFAULT_REFLECTION_PROBE_RES);
        REQUIRE(probe.state == ProbeState::Invalid);
        REQUIRE(probe.intensity == Approx(1.0f));
    }
    
    SECTION("Sphere influence calculation") {
        ReflectionProbe probe;
        probe.type = ReflectionProbeType::Sphere;
        probe.position = Vec3{0.0f, 0.0f, 0.0f};
        probe.sphereRadius = 10.0f;
        probe.blendDistance = 2.0f;
        
        // At center
        REQUIRE(probe.calculateInfluence(Vec3{0.0f, 0.0f, 0.0f}) == Approx(1.0f));
        
        // Well inside
        REQUIRE(probe.calculateInfluence(Vec3{5.0f, 0.0f, 0.0f}) == Approx(1.0f));
        
        // Outside
        REQUIRE(probe.calculateInfluence(Vec3{15.0f, 0.0f, 0.0f}) == Approx(0.0f));
    }
    
    SECTION("Box containment") {
        ReflectionProbe probe;
        probe.type = ReflectionProbeType::Box;
        probe.position = Vec3{0.0f, 0.0f, 0.0f};
        probe.boxExtents = Vec3{5.0f, 5.0f, 5.0f};
        
        REQUIRE(probe.containsPoint(Vec3{0.0f, 0.0f, 0.0f}) == true);
        REQUIRE(probe.containsPoint(Vec3{4.0f, 4.0f, 4.0f}) == true);
        REQUIRE(probe.containsPoint(Vec3{6.0f, 0.0f, 0.0f}) == false);
    }
    
    SECTION("Sphere containment") {
        ReflectionProbe probe;
        probe.type = ReflectionProbeType::Sphere;
        probe.position = Vec3{0.0f, 0.0f, 0.0f};
        probe.sphereRadius = 10.0f;
        
        REQUIRE(probe.containsPoint(Vec3{0.0f, 0.0f, 0.0f}) == true);
        REQUIRE(probe.containsPoint(Vec3{7.0f, 0.0f, 0.0f}) == true);
        REQUIRE(probe.containsPoint(Vec3{11.0f, 0.0f, 0.0f}) == false);
    }
}

// =============================================================================
// SSGIConfig Tests
// =============================================================================

TEST_CASE("SSGIConfig functionality", "[gi][ssgi]") {
    SECTION("Default values") {
        SSGIConfig config;
        REQUIRE(config.enabled == true);
        REQUIRE(config.quality == GIQuality::Medium);
        REQUIRE(config.temporalFilter == true);
    }
    
    SECTION("Effective sample count by quality") {
        SSGIConfig config;
        
        config.quality = GIQuality::Low;
        REQUIRE(config.getEffectiveSampleCount() == 4);
        
        config.quality = GIQuality::Medium;
        REQUIRE(config.getEffectiveSampleCount() == 8);
        
        config.quality = GIQuality::High;
        REQUIRE(config.getEffectiveSampleCount() == 16);
        
        config.quality = GIQuality::Ultra;
        REQUIRE(config.getEffectiveSampleCount() == 32);
    }
}

// =============================================================================
// RTGIConfig Tests
// =============================================================================

TEST_CASE("RTGIConfig functionality", "[gi][rtgi]") {
    SECTION("Default values") {
        RTGIConfig config;
        REQUIRE(config.enabled == true);
        REQUIRE(config.quality == GIQuality::High);
        REQUIRE(config.maxBounces == GIConfig::DEFAULT_RAY_BOUNCES);
        REQUIRE(config.denoise == true);
    }
    
    SECTION("Effective rays per pixel by quality") {
        RTGIConfig config;
        
        config.quality = GIQuality::Low;
        REQUIRE(config.getEffectiveRaysPerPixel() == 1);
        
        config.quality = GIQuality::Medium;
        REQUIRE(config.getEffectiveRaysPerPixel() == 1);
        
        config.quality = GIQuality::High;
        REQUIRE(config.getEffectiveRaysPerPixel() == 2);
        
        config.quality = GIQuality::Ultra;
        REQUIRE(config.getEffectiveRaysPerPixel() == 4);
    }
}

// =============================================================================
// GIStats Tests
// =============================================================================

TEST_CASE("GIStats functionality", "[gi][stats]") {
    SECTION("Reset frame stats") {
        GIStats stats;
        stats.probesUpdatedThisFrame = 10;
        stats.reflectionProbesUpdatedThisFrame = 2;
        stats.giTimeMs = 5.0f;
        stats.raysThisFrame = 1000000;
        
        stats.resetFrameStats();
        
        REQUIRE(stats.probesUpdatedThisFrame == 0);
        REQUIRE(stats.reflectionProbesUpdatedThisFrame == 0);
        REQUIRE(stats.giTimeMs == Approx(0.0f));
        REQUIRE(stats.raysThisFrame == 0);
    }
}

// =============================================================================
// GIManager Tests
// =============================================================================

TEST_CASE("GIManager construction", "[gi][manager]") {
    SECTION("Default construction") {
        GIManager manager;
        REQUIRE(manager.getTechnique() == GITechnique::None);
        REQUIRE(manager.getQuality() == GIQuality::Medium);
    }
}

TEST_CASE("GIManager configuration", "[gi][manager]") {
    GIManager manager;
    
    SECTION("Set technique") {
        manager.setTechnique(GITechnique::SSGI);
        REQUIRE(manager.getTechnique() == GITechnique::SSGI);
    }
    
    SECTION("Set quality") {
        manager.setQuality(GIQuality::Ultra);
        REQUIRE(manager.getQuality() == GIQuality::Ultra);
    }
    
    SECTION("Set SSGI config") {
        SSGIConfig config;
        config.intensity = 1.5f;
        manager.setSSGIConfig(config);
        
        REQUIRE(manager.getSSGIConfig().intensity == Approx(1.5f));
    }
    
    SECTION("Set RTGI config") {
        RTGIConfig config;
        config.maxBounces = 4;
        manager.setRTGIConfig(config);
        
        REQUIRE(manager.getRTGIConfig().maxBounces == 4);
    }
}

TEST_CASE("GIManager probe grid", "[gi][manager]") {
    GIManager manager;
    
    SECTION("Initialize probe grid") {
        ProbeGridConfig config;
        config.gridSizeX = 4;
        config.gridSizeY = 2;
        config.gridSizeZ = 4;
        
        manager.initializeProbeGrid(config);
        
        REQUIRE(manager.getIrradianceProbes().size() == 32);
        
        const auto& stats = manager.getStats();
        REQUIRE(stats.totalProbes == 32);
    }
    
    SECTION("Probe positions are correct") {
        ProbeGridConfig config;
        config.gridSizeX = 2;
        config.gridSizeY = 2;
        config.gridSizeZ = 2;
        config.boundsMin = Vec3{0.0f, 0.0f, 0.0f};
        config.boundsMax = Vec3{10.0f, 10.0f, 10.0f};
        
        manager.initializeProbeGrid(config);
        
        const auto& probes = manager.getIrradianceProbes();
        REQUIRE(probes.size() == 8);
        
        // First probe at min bounds
        REQUIRE(probes[0].position.x == Approx(0.0f));
        REQUIRE(probes[0].position.y == Approx(0.0f));
        REQUIRE(probes[0].position.z == Approx(0.0f));
    }
}

TEST_CASE("GIManager reflection probes", "[gi][manager]") {
    GIManager manager;
    
    SECTION("Add reflection probe") {
        ReflectionProbe probe;
        probe.name = "TestProbe";
        probe.position = Vec3{10.0f, 5.0f, 10.0f};
        
        u32 id = manager.addReflectionProbe(probe);
        REQUIRE(id > 0);
        
        auto* retrieved = manager.getReflectionProbe(id);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->name == "TestProbe");
    }
    
    SECTION("Remove reflection probe") {
        ReflectionProbe probe;
        u32 id = manager.addReflectionProbe(probe);
        
        manager.removeReflectionProbe(id);
        
        REQUIRE(manager.getReflectionProbe(id) == nullptr);
    }
    
    SECTION("Find relevant probes") {
        ReflectionProbe probe1;
        probe1.type = ReflectionProbeType::Sphere;
        probe1.position = Vec3{0.0f, 0.0f, 0.0f};
        probe1.sphereRadius = 10.0f;
        probe1.blendDistance = 2.0f;
        
        ReflectionProbe probe2;
        probe2.type = ReflectionProbeType::Sphere;
        probe2.position = Vec3{100.0f, 0.0f, 0.0f};
        probe2.sphereRadius = 10.0f;
        probe2.blendDistance = 2.0f;
        
        u32 id1 = manager.addReflectionProbe(probe1);
        manager.addReflectionProbe(probe2);
        
        auto relevant = manager.findRelevantReflectionProbes(Vec3{0.0f, 0.0f, 0.0f});
        
        REQUIRE(relevant.size() >= 1);
        REQUIRE(relevant[0] == id1);
    }
}

TEST_CASE("GIManager frame lifecycle", "[gi][manager]") {
    GIManager manager;
    
    ProbeGridConfig config;
    config.gridSizeX = 2;
    config.gridSizeY = 2;
    config.gridSizeZ = 2;
    manager.initializeProbeGrid(config);
    
    SECTION("Begin/end frame updates probes") {
        manager.beginFrame(0);
        manager.updateProbes(8);
        manager.endFrame();
        
        const auto& stats = manager.getStats();
        REQUIRE(stats.probesUpdatedThisFrame > 0);
    }
    
    SECTION("Multiple frames update all probes") {
        for (u32 i = 0; i < 10; ++i) {
            manager.beginFrame(i);
            manager.updateProbes(4);
            manager.endFrame();
        }
        
        const auto& stats = manager.getStats();
        REQUIRE(stats.validProbes > 0);
    }
}

// =============================================================================
// Utility Function Tests
// =============================================================================

TEST_CASE("createAmbientSH utility", "[gi][utility]") {
    SECTION("Creates valid SH") {
        Vec3 color{0.5f, 0.5f, 0.5f};
        auto sh = createAmbientSH(color);
        
        // L0 should have contribution
        REQUIRE(sh.coefficients[0].x > 0.0f);
        REQUIRE(sh.coefficients[0].y > 0.0f);
        REQUIRE(sh.coefficients[0].z > 0.0f);
    }
}

TEST_CASE("createDirectionalSH utility", "[gi][utility]") {
    SECTION("Creates valid SH for up direction") {
        Vec3 direction{0.0f, 1.0f, 0.0f};
        Vec3 color{1.0f, 1.0f, 1.0f};
        
        auto sh = createDirectionalSH(direction, color);
        
        // Should have some contribution
        REQUIRE(sh.coefficients[0].x > 0.0f);
    }
}

TEST_CASE("convolveDiffuse utility", "[gi][utility]") {
    SECTION("Convolves SH") {
        SphericalHarmonics sh;
        sh.coefficients[0] = Vec3{1.0f, 1.0f, 1.0f};
        sh.coefficients[1] = Vec3{1.0f, 1.0f, 1.0f};
        
        auto convolved = convolveDiffuse(sh);
        
        // L0 should be unchanged (A0 = 1.0)
        REQUIRE(convolved.coefficients[0].x == Approx(1.0f));
        
        // L1 should be scaled by 2/3
        REQUIRE(convolved.coefficients[1].x == Approx(2.0f / 3.0f));
    }
}
