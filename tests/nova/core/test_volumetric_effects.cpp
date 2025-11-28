/**
 * @file test_volumetric_effects.cpp
 * @brief Comprehensive tests for the volumetric effects system
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/volumetric_effects.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// VolumetricConfig Constants Tests
// =============================================================================

TEST_CASE("VolumetricConfig constants are valid", "[volumetric][config]") {
    SECTION("Froxel settings") {
        REQUIRE(VolumetricConfig::DEFAULT_FROXEL_WIDTH == 160);
        REQUIRE(VolumetricConfig::DEFAULT_FROXEL_HEIGHT == 90);
        REQUIRE(VolumetricConfig::DEFAULT_FROXEL_DEPTH == 64);
        REQUIRE(VolumetricConfig::MAX_FROXEL_DEPTH == 256);
    }
    
    SECTION("Volume limits") {
        REQUIRE(VolumetricConfig::MAX_FOG_VOLUMES == 64);
        REQUIRE(VolumetricConfig::MAX_LIGHT_SHAFTS == 16);
    }
    
    SECTION("Default coefficients") {
        REQUIRE(VolumetricConfig::DEFAULT_SCATTERING == Approx(0.01f));
        REQUIRE(VolumetricConfig::DEFAULT_ABSORPTION == Approx(0.001f));
    }
    
    SECTION("Ray marching settings") {
        REQUIRE(VolumetricConfig::MAX_RAY_MARCH_STEPS == 128);
        REQUIRE(VolumetricConfig::TEMPORAL_BLEND == Approx(0.95f));
    }
}

// =============================================================================
// FogMode Enum Tests
// =============================================================================

TEST_CASE("FogMode enum values", "[volumetric][fog]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(FogMode::None) == 0);
        REQUIRE(static_cast<u32>(FogMode::Linear) == 1);
        REQUIRE(static_cast<u32>(FogMode::Exponential) == 2);
        REQUIRE(static_cast<u32>(FogMode::ExponentialSquared) == 3);
        REQUIRE(static_cast<u32>(FogMode::Height) == 4);
        REQUIRE(static_cast<u32>(FogMode::Volumetric) == 5);
        REQUIRE(static_cast<u32>(FogMode::COUNT) == 6);
    }
    
    SECTION("Mode names") {
        REQUIRE(std::string(getFogModeName(FogMode::None)) == "None");
        REQUIRE(std::string(getFogModeName(FogMode::Linear)) == "Linear");
        REQUIRE(std::string(getFogModeName(FogMode::Exponential)) == "Exponential");
        REQUIRE(std::string(getFogModeName(FogMode::ExponentialSquared)) == "Exponential Squared");
        REQUIRE(std::string(getFogModeName(FogMode::Height)) == "Height");
        REQUIRE(std::string(getFogModeName(FogMode::Volumetric)) == "Volumetric");
    }
}

// =============================================================================
// FogVolumeShape Enum Tests
// =============================================================================

TEST_CASE("FogVolumeShape enum values", "[volumetric][fog]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(FogVolumeShape::Global) == 0);
        REQUIRE(static_cast<u32>(FogVolumeShape::Box) == 1);
        REQUIRE(static_cast<u32>(FogVolumeShape::Sphere) == 2);
        REQUIRE(static_cast<u32>(FogVolumeShape::Cylinder) == 3);
        REQUIRE(static_cast<u32>(FogVolumeShape::Cone) == 4);
    }
    
    SECTION("Shape names") {
        REQUIRE(std::string(getFogVolumeShapeName(FogVolumeShape::Global)) == "Global");
        REQUIRE(std::string(getFogVolumeShapeName(FogVolumeShape::Box)) == "Box");
        REQUIRE(std::string(getFogVolumeShapeName(FogVolumeShape::Sphere)) == "Sphere");
    }
}

// =============================================================================
// CloudMode Enum Tests
// =============================================================================

TEST_CASE("CloudMode enum values", "[volumetric][cloud]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(CloudMode::None) == 0);
        REQUIRE(static_cast<u32>(CloudMode::Layer2D) == 1);
        REQUIRE(static_cast<u32>(CloudMode::Procedural3D) == 2);
        REQUIRE(static_cast<u32>(CloudMode::Volumetric) == 3);
    }
    
    SECTION("Mode names") {
        REQUIRE(std::string(getCloudModeName(CloudMode::None)) == "None");
        REQUIRE(std::string(getCloudModeName(CloudMode::Layer2D)) == "2D Layer");
        REQUIRE(std::string(getCloudModeName(CloudMode::Volumetric)) == "Volumetric");
    }
}

// =============================================================================
// LightShaftQuality Enum Tests
// =============================================================================

TEST_CASE("LightShaftQuality enum values", "[volumetric][lightshaft]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(LightShaftQuality::Off) == 0);
        REQUIRE(static_cast<u32>(LightShaftQuality::Low) == 1);
        REQUIRE(static_cast<u32>(LightShaftQuality::Medium) == 2);
        REQUIRE(static_cast<u32>(LightShaftQuality::High) == 3);
    }
    
    SECTION("Quality names") {
        REQUIRE(std::string(getLightShaftQualityName(LightShaftQuality::Off)) == "Off");
        REQUIRE(std::string(getLightShaftQualityName(LightShaftQuality::Low)) == "Low");
        REQUIRE(std::string(getLightShaftQualityName(LightShaftQuality::Medium)) == "Medium");
        REQUIRE(std::string(getLightShaftQualityName(LightShaftQuality::High)) == "High");
    }
}

// =============================================================================
// FogSettings Tests
// =============================================================================

TEST_CASE("FogSettings functionality", "[volumetric][fog]") {
    SECTION("Default values") {
        FogSettings fog;
        REQUIRE(fog.mode == FogMode::Exponential);
        REQUIRE(fog.density == Approx(0.02f));
        REQUIRE(fog.maxOpacity == Approx(1.0f));
    }
    
    SECTION("Linear fog calculation") {
        FogSettings fog;
        fog.mode = FogMode::Linear;
        fog.startDistance = 10.0f;
        fog.endDistance = 100.0f;
        
        REQUIRE(fog.calculateLinearFog(0.0f) == Approx(0.0f));
        REQUIRE(fog.calculateLinearFog(10.0f) == Approx(0.0f));
        REQUIRE(fog.calculateLinearFog(55.0f) == Approx(0.5f));
        REQUIRE(fog.calculateLinearFog(100.0f) == Approx(1.0f));
        REQUIRE(fog.calculateLinearFog(150.0f) == Approx(1.0f));
    }
    
    SECTION("Exponential fog calculation") {
        FogSettings fog;
        fog.mode = FogMode::Exponential;
        fog.density = 0.1f;
        
        f32 fogAt0 = fog.calculateExponentialFog(0.0f);
        f32 fogAt10 = fog.calculateExponentialFog(10.0f);
        f32 fogAt100 = fog.calculateExponentialFog(100.0f);
        
        REQUIRE(fogAt0 == Approx(0.0f));
        REQUIRE(fogAt10 > 0.0f);
        REQUIRE(fogAt100 > fogAt10);
    }
    
    SECTION("Height fog calculation") {
        FogSettings fog;
        fog.mode = FogMode::Height;
        fog.heightFloor = 0.0f;
        fog.heightFalloff = 0.1f;
        
        REQUIRE(fog.calculateHeightFog(-5.0f) == Approx(1.0f)); // Below floor
        REQUIRE(fog.calculateHeightFog(0.0f) == Approx(1.0f));   // At floor
        
        f32 above = fog.calculateHeightFog(10.0f);
        REQUIRE(above < 1.0f);
        REQUIRE(above > 0.0f);
    }
    
    SECTION("Create outdoor preset") {
        auto fog = FogSettings::createOutdoor();
        REQUIRE(fog.mode == FogMode::Height);
        REQUIRE(fog.density < 0.01f);
    }
    
    SECTION("Create dense preset") {
        auto fog = FogSettings::createDense();
        REQUIRE(fog.mode == FogMode::Exponential);
        REQUIRE(fog.density > 0.05f);
    }
}

// =============================================================================
// FogVolume Tests
// =============================================================================

TEST_CASE("FogVolume functionality", "[volumetric][fog]") {
    SECTION("Default values") {
        FogVolume volume;
        REQUIRE(volume.shape == FogVolumeShape::Box);
        REQUIRE(volume.enabled == true);
        REQUIRE(volume.densityMultiplier == Approx(1.0f));
    }
    
    SECTION("Box containment") {
        FogVolume volume;
        volume.shape = FogVolumeShape::Box;
        volume.position = Vec3{0.0f, 0.0f, 0.0f};
        volume.boxExtents = Vec3{5.0f, 5.0f, 5.0f};
        
        REQUIRE(volume.containsPoint(Vec3{0.0f, 0.0f, 0.0f}) == true);
        REQUIRE(volume.containsPoint(Vec3{4.0f, 4.0f, 4.0f}) == true);
        REQUIRE(volume.containsPoint(Vec3{6.0f, 0.0f, 0.0f}) == false);
    }
    
    SECTION("Sphere containment") {
        FogVolume volume;
        volume.shape = FogVolumeShape::Sphere;
        volume.position = Vec3{0.0f, 0.0f, 0.0f};
        volume.sphereRadius = 10.0f;
        
        REQUIRE(volume.containsPoint(Vec3{0.0f, 0.0f, 0.0f}) == true);
        REQUIRE(volume.containsPoint(Vec3{7.0f, 0.0f, 0.0f}) == true);
        REQUIRE(volume.containsPoint(Vec3{11.0f, 0.0f, 0.0f}) == false);
    }
    
    SECTION("Cylinder containment") {
        FogVolume volume;
        volume.shape = FogVolumeShape::Cylinder;
        volume.position = Vec3{0.0f, 0.0f, 0.0f};
        volume.cylinderRadius = 5.0f;
        volume.cylinderHeight = 10.0f;
        
        REQUIRE(volume.containsPoint(Vec3{0.0f, 0.0f, 0.0f}) == true);
        REQUIRE(volume.containsPoint(Vec3{4.0f, 0.0f, 0.0f}) == true);
        REQUIRE(volume.containsPoint(Vec3{0.0f, 4.0f, 0.0f}) == true);
        REQUIRE(volume.containsPoint(Vec3{6.0f, 0.0f, 0.0f}) == false);
        REQUIRE(volume.containsPoint(Vec3{0.0f, 6.0f, 0.0f}) == false);
    }
    
    SECTION("Global containment") {
        FogVolume volume;
        volume.shape = FogVolumeShape::Global;
        
        REQUIRE(volume.containsPoint(Vec3{1000.0f, 1000.0f, 1000.0f}) == true);
        REQUIRE(volume.containsPoint(Vec3{-1000.0f, -1000.0f, -1000.0f}) == true);
    }
    
    SECTION("Disabled volume") {
        FogVolume volume;
        volume.enabled = false;
        
        REQUIRE(volume.containsPoint(Vec3{0.0f, 0.0f, 0.0f}) == false);
        REQUIRE(volume.getDensityAtPoint(Vec3{0.0f, 0.0f, 0.0f}) == Approx(0.0f));
    }
    
    SECTION("Density with blend distance") {
        FogVolume volume;
        volume.shape = FogVolumeShape::Box;
        volume.boxExtents = Vec3{10.0f, 10.0f, 10.0f};
        volume.blendDistance = 2.0f;
        volume.densityMultiplier = 1.0f;
        
        // Center should have full density
        REQUIRE(volume.getDensityAtPoint(Vec3{0.0f, 0.0f, 0.0f}) == Approx(1.0f));
        
        // Near edge should have reduced density
        f32 nearEdge = volume.getDensityAtPoint(Vec3{9.0f, 0.0f, 0.0f});
        REQUIRE(nearEdge < 1.0f);
        REQUIRE(nearEdge > 0.0f);
        
        // Outside should be zero
        REQUIRE(volume.getDensityAtPoint(Vec3{15.0f, 0.0f, 0.0f}) == Approx(0.0f));
    }
}

// =============================================================================
// FroxelGridConfig Tests
// =============================================================================

TEST_CASE("FroxelGridConfig functionality", "[volumetric][froxel]") {
    SECTION("Default values") {
        FroxelGridConfig config;
        REQUIRE(config.width == VolumetricConfig::DEFAULT_FROXEL_WIDTH);
        REQUIRE(config.height == VolumetricConfig::DEFAULT_FROXEL_HEIGHT);
        REQUIRE(config.depth == VolumetricConfig::DEFAULT_FROXEL_DEPTH);
    }
    
    SECTION("Total froxel count") {
        FroxelGridConfig config;
        config.width = 10;
        config.height = 10;
        config.depth = 10;
        
        REQUIRE(config.getTotalFroxels() == 1000);
    }
    
    SECTION("Depth slice linear") {
        FroxelGridConfig config;
        config.nearPlane = 1.0f;
        config.farPlane = 100.0f;
        config.depth = 100;
        config.logarithmicDepth = false;
        
        REQUIRE(config.getDepthSlice(1.0f) == 0);
        REQUIRE(config.getDepthSlice(100.0f) == 99);
        // Middle point: (50.5 - 1) / (100 - 1) * 99 ≈ 49
        REQUIRE(config.getDepthSlice(50.5f) == 49);
    }
    
    SECTION("Slice depth roundtrip") {
        FroxelGridConfig config;
        config.nearPlane = 0.1f;
        config.farPlane = 100.0f;
        config.depth = 64;
        
        // First slice should be near plane
        f32 depth0 = config.getSliceDepth(0);
        REQUIRE(depth0 == Approx(config.nearPlane));
        
        // Last slice should be far plane
        f32 depth63 = config.getSliceDepth(63);
        REQUIRE(depth63 == Approx(config.farPlane));
    }
}

// =============================================================================
// CloudSettings Tests
// =============================================================================

TEST_CASE("CloudSettings functionality", "[volumetric][cloud]") {
    SECTION("Default values") {
        CloudSettings clouds;
        REQUIRE(clouds.mode == CloudMode::None);
        REQUIRE(clouds.altitude == Approx(2000.0f));
        REQUIRE(clouds.coverage == Approx(0.5f));
    }
    
    SECTION("Is enabled check") {
        CloudSettings clouds;
        REQUIRE(clouds.isEnabled() == false);
        
        clouds.mode = CloudMode::Volumetric;
        REQUIRE(clouds.isEnabled() == true);
    }
    
    SECTION("Cirrus preset") {
        auto clouds = CloudSettings::createCirrus();
        REQUIRE(clouds.mode == CloudMode::Layer2D);
        REQUIRE(clouds.altitude > 5000.0f); // High altitude
        REQUIRE(clouds.coverage < 0.5f);    // Sparse
    }
    
    SECTION("Cumulus preset") {
        auto clouds = CloudSettings::createCumulus();
        REQUIRE(clouds.mode == CloudMode::Volumetric);
        REQUIRE(clouds.thickness > 500.0f);
    }
    
    SECTION("Overcast preset") {
        auto clouds = CloudSettings::createOvercast();
        REQUIRE(clouds.mode == CloudMode::Volumetric);
        REQUIRE(clouds.coverage > 0.8f);
    }
}

// =============================================================================
// LightShaftSettings Tests
// =============================================================================

TEST_CASE("LightShaftSettings functionality", "[volumetric][lightshaft]") {
    SECTION("Default values") {
        LightShaftSettings settings;
        REQUIRE(settings.enabled == true);
        REQUIRE(settings.quality == LightShaftQuality::Medium);
        REQUIRE(settings.intensity == Approx(1.0f));
    }
    
    SECTION("Effective sample count") {
        LightShaftSettings settings;
        
        settings.quality = LightShaftQuality::Off;
        REQUIRE(settings.getEffectiveSampleCount() == 0);
        
        settings.quality = LightShaftQuality::Low;
        REQUIRE(settings.getEffectiveSampleCount() == 32);
        
        settings.quality = LightShaftQuality::Medium;
        REQUIRE(settings.getEffectiveSampleCount() == 64);
        
        settings.quality = LightShaftQuality::High;
        REQUIRE(settings.getEffectiveSampleCount() == 128);
    }
    
    SECTION("Is enabled check") {
        LightShaftSettings settings;
        REQUIRE(settings.isEnabled() == true);
        
        settings.enabled = false;
        REQUIRE(settings.isEnabled() == false);
        
        settings.enabled = true;
        settings.quality = LightShaftQuality::Off;
        REQUIRE(settings.isEnabled() == false);
    }
}

// =============================================================================
// ParticipatingMedia Tests
// =============================================================================

TEST_CASE("ParticipatingMedia functionality", "[volumetric][media]") {
    SECTION("Default values") {
        ParticipatingMedia media;
        REQUIRE(media.enabled == true);
        REQUIRE(media.density == Approx(1.0f));
    }
    
    SECTION("Get extinction") {
        ParticipatingMedia media;
        media.scattering = Vec3{0.1f, 0.2f, 0.3f};
        media.absorption = Vec3{0.01f, 0.02f, 0.03f};
        
        Vec3 ext = media.getExtinction();
        
        REQUIRE(ext.x == Approx(0.11f));
        REQUIRE(ext.y == Approx(0.22f));
        REQUIRE(ext.z == Approx(0.33f));
    }
    
    SECTION("Get albedo") {
        ParticipatingMedia media;
        media.scattering = Vec3{0.9f, 0.9f, 0.9f};
        media.absorption = Vec3{0.1f, 0.1f, 0.1f};
        
        Vec3 albedo = media.getAlbedo();
        
        // albedo = scattering / extinction = 0.9 / 1.0 = 0.9
        REQUIRE(albedo.x == Approx(0.9f));
        REQUIRE(albedo.y == Approx(0.9f));
        REQUIRE(albedo.z == Approx(0.9f));
    }
    
    SECTION("Smoke preset") {
        auto media = ParticipatingMedia::createSmoke();
        REQUIRE(media.name == "Smoke");
        REQUIRE(media.phaseG > 0.0f); // Forward scattering
    }
    
    SECTION("Dust preset") {
        auto media = ParticipatingMedia::createDust();
        REQUIRE(media.name == "Dust");
        REQUIRE(media.phaseG > media.absorption.x); // Higher forward scattering
    }
    
    SECTION("Steam preset") {
        auto media = ParticipatingMedia::createSteam();
        REQUIRE(media.name == "Steam");
        REQUIRE(media.scattering.x > 0.5f); // High scattering (white)
    }
}

// =============================================================================
// VolumetricStats Tests
// =============================================================================

TEST_CASE("VolumetricStats functionality", "[volumetric][stats]") {
    SECTION("Reset frame stats") {
        VolumetricStats stats;
        stats.fogTimeMs = 5.0f;
        stats.cloudTimeMs = 10.0f;
        stats.lightShaftTimeMs = 2.0f;
        stats.froxelsComputed = 100000;
        
        stats.resetFrameStats();
        
        REQUIRE(stats.fogTimeMs == Approx(0.0f));
        REQUIRE(stats.cloudTimeMs == Approx(0.0f));
        REQUIRE(stats.lightShaftTimeMs == Approx(0.0f));
        REQUIRE(stats.froxelsComputed == 0);
    }
    
    SECTION("Calculate total") {
        VolumetricStats stats;
        stats.fogTimeMs = 2.0f;
        stats.cloudTimeMs = 5.0f;
        stats.lightShaftTimeMs = 1.0f;
        
        stats.calculateTotal();
        
        REQUIRE(stats.totalTimeMs == Approx(8.0f));
    }
}

// =============================================================================
// VolumetricManager Tests
// =============================================================================

TEST_CASE("VolumetricManager construction", "[volumetric][manager]") {
    SECTION("Default construction") {
        VolumetricManager manager;
        REQUIRE(manager.getFogSettings().mode == FogMode::Exponential);
        REQUIRE(manager.getCloudSettings().mode == CloudMode::None);
    }
}

TEST_CASE("VolumetricManager fog configuration", "[volumetric][manager]") {
    VolumetricManager manager;
    
    SECTION("Set fog settings") {
        FogSettings fog;
        fog.mode = FogMode::Height;
        fog.density = 0.05f;
        
        manager.setFogSettings(fog);
        
        REQUIRE(manager.getFogSettings().mode == FogMode::Height);
        REQUIRE(manager.getFogSettings().density == Approx(0.05f));
    }
    
    SECTION("Set froxel config") {
        FroxelGridConfig config;
        config.width = 80;
        config.height = 45;
        config.depth = 32;
        
        manager.setFroxelConfig(config);
        
        REQUIRE(manager.getFroxelConfig().width == 80);
        REQUIRE(manager.getFroxelConfig().depth == 32);
    }
}

TEST_CASE("VolumetricManager fog volumes", "[volumetric][manager]") {
    VolumetricManager manager;
    
    SECTION("Add fog volume") {
        FogVolume volume;
        volume.name = "TestFog";
        volume.shape = FogVolumeShape::Sphere;
        
        u32 id = manager.addFogVolume(volume);
        REQUIRE(id > 0);
        
        auto* retrieved = manager.getFogVolume(id);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->name == "TestFog");
    }
    
    SECTION("Remove fog volume") {
        FogVolume volume;
        u32 id = manager.addFogVolume(volume);
        
        manager.removeFogVolume(id);
        
        REQUIRE(manager.getFogVolume(id) == nullptr);
    }
    
    SECTION("Sample fog at point") {
        FogSettings fog;
        fog.mode = FogMode::Exponential;
        fog.density = 0.1f;
        fog.color = Vec3{1.0f, 1.0f, 1.0f};
        manager.setFogSettings(fog);
        
        Vec4 sample = manager.sampleFogAtPoint(Vec3{0.0f, 0.0f, 0.0f}, 10.0f);
        
        // Should have some fog contribution
        REQUIRE(sample.w > 0.0f);
    }
}

TEST_CASE("VolumetricManager cloud configuration", "[volumetric][manager]") {
    VolumetricManager manager;
    
    SECTION("Set cloud settings") {
        CloudSettings clouds;
        clouds.mode = CloudMode::Volumetric;
        clouds.coverage = 0.8f;
        
        manager.setCloudSettings(clouds);
        
        REQUIRE(manager.getCloudSettings().mode == CloudMode::Volumetric);
        REQUIRE(manager.getCloudSettings().coverage == Approx(0.8f));
    }
}

TEST_CASE("VolumetricManager light shafts", "[volumetric][manager]") {
    VolumetricManager manager;
    
    SECTION("Set light shaft settings") {
        LightShaftSettings settings;
        settings.quality = LightShaftQuality::High;
        settings.intensity = 1.5f;
        
        manager.setLightShaftSettings(settings);
        
        REQUIRE(manager.getLightShaftSettings().quality == LightShaftQuality::High);
        REQUIRE(manager.getLightShaftSettings().intensity == Approx(1.5f));
    }
}

TEST_CASE("VolumetricManager media", "[volumetric][manager]") {
    VolumetricManager manager;
    
    SECTION("Add participating media") {
        auto smoke = ParticipatingMedia::createSmoke();
        u32 id = manager.addMedia(smoke);
        
        REQUIRE(id > 0);
        
        auto* retrieved = manager.getMedia(id);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->name == "Smoke");
    }
    
    SECTION("Remove media") {
        auto dust = ParticipatingMedia::createDust();
        u32 id = manager.addMedia(dust);
        
        manager.removeMedia(id);
        
        REQUIRE(manager.getMedia(id) == nullptr);
    }
}

TEST_CASE("VolumetricManager frame lifecycle", "[volumetric][manager]") {
    VolumetricManager manager;
    
    FogVolume vol1;
    vol1.enabled = true;
    manager.addFogVolume(vol1);
    
    auto smoke = ParticipatingMedia::createSmoke();
    manager.addMedia(smoke);
    
    SECTION("Begin/end frame updates stats") {
        manager.beginFrame(0);
        manager.endFrame();
        
        const auto& stats = manager.getStats();
        REQUIRE(stats.activeFogVolumes == 1);
        REQUIRE(stats.activeMediaVolumes == 1);
    }
    
    SECTION("Current frame tracking") {
        manager.beginFrame(42);
        REQUIRE(manager.getCurrentFrame() == 42);
    }
}

// =============================================================================
// Utility Function Tests
// =============================================================================

TEST_CASE("henyeyGreenstein phase function", "[volumetric][utility]") {
    SECTION("Isotropic (g=0)") {
        f32 result = henyeyGreenstein(0.0f, 0.0f);
        // For g=0, should be 1/(4*pi) ≈ 0.0796
        REQUIRE(result == Approx(1.0f / (4.0f * 3.14159265359f)).margin(0.01f));
    }
    
    SECTION("Forward scattering") {
        f32 forward = henyeyGreenstein(1.0f, 0.8f);  // cos=1 (forward), g=0.8
        f32 backward = henyeyGreenstein(-1.0f, 0.8f); // cos=-1 (backward)
        
        REQUIRE(forward > backward);
    }
}

TEST_CASE("schlickPhase approximation", "[volumetric][utility]") {
    SECTION("Basic evaluation") {
        f32 result = schlickPhase(0.0f, 0.0f);
        // For k=0, should be 1/(4*pi)
        REQUIRE(result == Approx(1.0f / (4.0f * 3.14159265359f)).margin(0.01f));
    }
}

TEST_CASE("beerLambert transmittance", "[volumetric][utility]") {
    SECTION("Zero distance") {
        f32 t = beerLambert(0.1f, 0.0f);
        REQUIRE(t == Approx(1.0f));
    }
    
    SECTION("Increasing distance decreases transmittance") {
        f32 t1 = beerLambert(0.1f, 5.0f);
        f32 t2 = beerLambert(0.1f, 10.0f);
        
        REQUIRE(t1 > t2);
        REQUIRE(t1 < 1.0f);
        REQUIRE(t2 > 0.0f);
    }
    
    SECTION("Higher extinction decreases transmittance") {
        f32 t1 = beerLambert(0.1f, 10.0f);
        f32 t2 = beerLambert(0.5f, 10.0f);
        
        REQUIRE(t1 > t2);
    }
}

TEST_CASE("beerLambertRGB", "[volumetric][utility]") {
    SECTION("RGB transmittance") {
        Vec3 extinction{0.1f, 0.2f, 0.3f};
        Vec3 t = beerLambertRGB(extinction, 10.0f);
        
        // Red should transmit more than green, which transmits more than blue
        REQUIRE(t.x > t.y);
        REQUIRE(t.y > t.z);
    }
}
