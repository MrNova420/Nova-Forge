/**
 * @file test_water.cpp
 * @brief Comprehensive tests for the water rendering system
 * @author NovaForge Team
 * @copyright Copyright (c) 2024-2025 NovaForge - All Rights Reserved
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/water_system.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// WATER CONFIGURATION TESTS
// =============================================================================

TEST_CASE("WaterConfig constants are valid", "[water][config]") {
    SECTION("FFT resolution limits are reasonable") {
        REQUIRE(WaterConfig::DEFAULT_FFT_RESOLUTION > 0);
        REQUIRE(WaterConfig::MAX_FFT_RESOLUTION >= WaterConfig::DEFAULT_FFT_RESOLUTION);
        REQUIRE((WaterConfig::DEFAULT_FFT_RESOLUTION & (WaterConfig::DEFAULT_FFT_RESOLUTION - 1)) == 0);
    }
    
    SECTION("Physical constants are correct") {
        REQUIRE(WaterConfig::GRAVITY == Approx(9.81f));
        REQUIRE(WaterConfig::WATER_F0 == Approx(0.02f));
    }
    
    SECTION("Default values are sensible") {
        REQUIRE(WaterConfig::DEFAULT_DEPTH > 0.0f);
        REQUIRE(WaterConfig::DEFAULT_TILE_SIZE > 0.0f);
        REQUIRE(WaterConfig::DEFAULT_WIND_SPEED > 0.0f);
        REQUIRE(WaterConfig::MAX_CASCADES > 0);
    }
}

// =============================================================================
// WATER TECHNIQUE TESTS
// =============================================================================

TEST_CASE("WaterTechnique enumeration", "[water][enums]") {
    SECTION("All techniques have valid names") {
        REQUIRE(std::string(getWaterTechniqueName(WaterTechnique::Simple)) == "Simple");
        REQUIRE(std::string(getWaterTechniqueName(WaterTechnique::Gerstner)) == "Gerstner");
        REQUIRE(std::string(getWaterTechniqueName(WaterTechnique::FFT)) == "FFT");
        REQUIRE(std::string(getWaterTechniqueName(WaterTechnique::Tessellation)) == "Tessellation");
    }
}

// =============================================================================
// GERSTNER WAVE TESTS
// =============================================================================

TEST_CASE("GerstnerWave calculations", "[water][gerstner]") {
    GerstnerWave wave;
    wave.direction = Vec2{1.0f, 0.0f};
    wave.wavelength = 10.0f;
    wave.amplitude = 0.5f;
    wave.steepness = 0.5f;
    wave.speed = 1.0f;
    wave.phase = 0.0f;
    
    SECTION("Wave number calculation") {
        f32 k = wave.getWaveNumber();
        REQUIRE(k == Approx(2.0f * 3.14159265f / 10.0f));
    }
    
    SECTION("Angular frequency calculation") {
        f32 omega = wave.getAngularFrequency();
        REQUIRE(omega > 0.0f);
    }
    
    SECTION("Displacement calculation") {
        Vec3 disp = wave.calculateDisplacement(Vec2{0.0f, 0.0f}, 0.0f);
        // Y displacement should be <= amplitude
        REQUIRE(std::abs(disp.y) <= wave.amplitude + 0.001f);
    }
}

TEST_CASE("GerstnerWave factories", "[water][gerstner]") {
    SECTION("Small wave factory") {
        auto wave = GerstnerWave::small(Vec2{1.0f, 0.0f});
        REQUIRE(wave.wavelength == Approx(5.0f));
        REQUIRE(wave.amplitude == Approx(0.1f));
    }
    
    SECTION("Medium wave factory") {
        auto wave = GerstnerWave::medium(Vec2{1.0f, 0.0f});
        REQUIRE(wave.wavelength == Approx(20.0f));
        REQUIRE(wave.amplitude == Approx(0.5f));
    }
    
    SECTION("Large wave factory") {
        auto wave = GerstnerWave::large(Vec2{1.0f, 0.0f});
        REQUIRE(wave.wavelength == Approx(50.0f));
        REQUIRE(wave.amplitude == Approx(1.5f));
    }
}

// =============================================================================
// OCEAN CASCADE TESTS
// =============================================================================

TEST_CASE("OceanCascade calculations", "[water][cascade]") {
    OceanCascade cascade;
    cascade.windSpeed = 10.0f;
    cascade.tileSize = 100.0f;
    cascade.depth = 100.0f;
    cascade.amplitude = 1.0f;
    cascade.windDirection = Vec2{1.0f, 0.0f};
    
    SECTION("Wave number calculation") {
        f32 omega = 1.0f;
        f32 k = cascade.getWaveNumber(omega);
        REQUIRE(k > 0.0f);
    }
    
    SECTION("Phillips spectrum is positive") {
        f32 spec = cascade.calculatePhillipsSpectrum(Vec2{0.1f, 0.0f});
        REQUIRE(spec >= 0.0f);
    }
    
    SECTION("Phillips spectrum is zero at origin") {
        f32 spec = cascade.calculatePhillipsSpectrum(Vec2{0.0f, 0.0f});
        REQUIRE(spec == Approx(0.0f));
    }
}

TEST_CASE("OceanCascade factories", "[water][cascade]") {
    SECTION("Calm conditions") {
        auto cascade = OceanCascade::calm();
        REQUIRE(cascade.windSpeed == Approx(3.0f));
        REQUIRE(cascade.amplitude == Approx(0.3f));
    }
    
    SECTION("Moderate conditions") {
        auto cascade = OceanCascade::moderate();
        REQUIRE(cascade.windSpeed == Approx(10.0f));
        REQUIRE(cascade.amplitude == Approx(1.0f));
    }
    
    SECTION("Stormy conditions") {
        auto cascade = OceanCascade::stormy();
        REQUIRE(cascade.windSpeed == Approx(25.0f));
        REQUIRE(cascade.amplitude == Approx(3.0f));
    }
}

// =============================================================================
// WATER OPTICAL PROPERTIES TESTS
// =============================================================================

TEST_CASE("WaterOpticalProperties calculations", "[water][optical]") {
    WaterOpticalProperties props = WaterOpticalProperties::clearOcean();
    
    SECTION("Extinction at zero depth") {
        Vec3 ext = props.calculateExtinction(0.0f);
        // At zero depth, should be mostly absorption color
        REQUIRE(ext.x >= 0.0f);
        REQUIRE(ext.y >= 0.0f);
        REQUIRE(ext.z >= 0.0f);
    }
    
    SECTION("Fresnel at perpendicular") {
        f32 f = props.calculateFresnel(1.0f);
        REQUIRE(f == Approx(WaterConfig::WATER_F0));
    }
    
    SECTION("Fresnel at grazing angle") {
        f32 f = props.calculateFresnel(0.0f);
        REQUIRE(f == Approx(1.0f));
    }
}

TEST_CASE("WaterOpticalProperties factories", "[water][optical]") {
    SECTION("Clear ocean") {
        auto props = WaterOpticalProperties::clearOcean();
        REQUIRE(props.turbidity == Approx(0.05f));
        REQUIRE(props.visibility == Approx(40.0f));
    }
    
    SECTION("Tropical water") {
        auto props = WaterOpticalProperties::tropical();
        REQUIRE(props.turbidity == Approx(0.02f));
        REQUIRE(props.visibility == Approx(60.0f));
    }
    
    SECTION("Murky water") {
        auto props = WaterOpticalProperties::murky();
        REQUIRE(props.turbidity == Approx(0.8f));
        REQUIRE(props.visibility == Approx(3.0f));
    }
    
    SECTION("Pool water") {
        auto props = WaterOpticalProperties::pool();
        REQUIRE(props.turbidity == Approx(0.0f));
        REQUIRE(props.visibility == Approx(100.0f));
    }
}

// =============================================================================
// FOAM SETTINGS TESTS
// =============================================================================

TEST_CASE("FoamSettings factories", "[water][foam]") {
    SECTION("Calm water foam") {
        auto settings = FoamSettings::calm();
        REQUIRE(settings.threshold == Approx(0.5f));
        REQUIRE(settings.coverage == Approx(0.1f));
    }
    
    SECTION("Rough water foam") {
        auto settings = FoamSettings::rough();
        REQUIRE(settings.threshold == Approx(0.25f));
        REQUIRE(settings.coverage == Approx(0.7f));
    }
}

// =============================================================================
// CAUSTICS SETTINGS TESTS
// =============================================================================

TEST_CASE("CausticsSettings depth intensity", "[water][caustics]") {
    CausticsSettings settings;
    settings.maxDepth = 10.0f;
    settings.focusDepth = 2.0f;
    settings.intensity = 1.0f;
    
    SECTION("Zero at surface") {
        f32 intensity = settings.getIntensityAtDepth(0.0f);
        REQUIRE(intensity == Approx(0.0f));
    }
    
    SECTION("Maximum at focus depth") {
        f32 intensity = settings.getIntensityAtDepth(2.0f);
        REQUIRE(intensity == Approx(1.0f));
    }
    
    SECTION("Zero beyond max depth") {
        f32 intensity = settings.getIntensityAtDepth(15.0f);
        REQUIRE(intensity == Approx(0.0f));
    }
    
    SECTION("Intermediate depth") {
        f32 intensity = settings.getIntensityAtDepth(6.0f);
        REQUIRE(intensity > 0.0f);
        REQUIRE(intensity < 1.0f);
    }
}

// =============================================================================
// WATER BODY DESCRIPTION TESTS
// =============================================================================

TEST_CASE("WaterBodyDesc factories", "[water][body]") {
    SECTION("Ocean factory") {
        auto desc = WaterBodyDesc::ocean(5.0f);
        REQUIRE(desc.type == WaterBodyType::Ocean);
        REQUIRE(desc.elevation == Approx(5.0f));
        REQUIRE(desc.infinite == true);
    }
    
    SECTION("Lake factory") {
        auto desc = WaterBodyDesc::lake(Vec3{100.0f, 50.0f, 100.0f}, Vec2{200.0f, 200.0f}, 30.0f);
        REQUIRE(desc.type == WaterBodyType::Lake);
        REQUIRE(desc.position.x == Approx(100.0f));
        REQUIRE(desc.depth == Approx(30.0f));
        REQUIRE(desc.infinite == false);
    }
    
    SECTION("River factory") {
        auto desc = WaterBodyDesc::river(Vec3{0.0f, 10.0f, 0.0f}, 20.0f, 5.0f, Vec2{1.0f, 0.0f}, 2.0f);
        REQUIRE(desc.type == WaterBodyType::River);
        REQUIRE(desc.flowSpeed == Approx(2.0f));
        REQUIRE(desc.infinite == false);
    }
}

// =============================================================================
// WATER SETTINGS TESTS
// =============================================================================

TEST_CASE("WaterSettings factories", "[water][settings]") {
    SECTION("Low quality settings") {
        auto settings = WaterSettings::low();
        REQUIRE(settings.quality == WaterQuality::Low);
        REQUIRE(settings.technique == WaterTechnique::Gerstner);
        REQUIRE(settings.fftResolution == 128);
        REQUIRE(settings.cascadeCount == 1);
        REQUIRE(settings.enableCaustics == false);
    }
    
    SECTION("Medium quality settings") {
        auto settings = WaterSettings::medium();
        REQUIRE(settings.quality == WaterQuality::Medium);
        REQUIRE(settings.fftResolution == 256);
        REQUIRE(settings.cascadeCount == 2);
    }
    
    SECTION("High quality settings") {
        auto settings = WaterSettings::high();
        REQUIRE(settings.quality == WaterQuality::High);
        REQUIRE(settings.fftResolution == 512);
        REQUIRE(settings.cascadeCount == 3);
    }
    
    SECTION("Ultra quality settings") {
        auto settings = WaterSettings::ultra();
        REQUIRE(settings.quality == WaterQuality::Ultra);
        REQUIRE(settings.fftResolution == 1024);
        REQUIRE(settings.cascadeCount == 4);
        REQUIRE(settings.reflectionMode == WaterReflectionMode::Raytraced);
    }
}

// =============================================================================
// GPU WATER DATA TESTS
// =============================================================================

TEST_CASE("GPUWaterData creation", "[water][gpu]") {
    auto optical = WaterOpticalProperties::clearOcean();
    GPUWaterData data = GPUWaterData::create(10.0f, 100.0f, optical);
    
    REQUIRE(data.waterParams.x == Approx(10.0f));  // elevation
    REQUIRE(data.waterParams.y == Approx(100.0f)); // depth
    REQUIRE(data.opticalParams.x == Approx(optical.turbidity));
    REQUIRE(data.opticalParams.y == Approx(optical.refractionIndex));
}

// =============================================================================
// WATER MANAGER TESTS
// =============================================================================

TEST_CASE("WaterManager initialization", "[water][manager]") {
    auto& manager = WaterManager::instance();
    
    SECTION("Can initialize") {
        REQUIRE(manager.initialize());
        REQUIRE(manager.isInitialized());
    }
}

TEST_CASE("WaterManager water bodies", "[water][manager]") {
    auto& manager = WaterManager::instance();
    manager.initialize();
    
    SECTION("Add water body") {
        auto desc = WaterBodyDesc::ocean();
        u32 id = manager.addWaterBody(desc);
        REQUIRE(manager.getWaterBodyCount() >= 1);
    }
    
    SECTION("Get water body") {
        auto desc = WaterBodyDesc::lake(Vec3{0.0f, 0.0f, 0.0f}, Vec2{100.0f, 100.0f}, 20.0f);
        u32 id = manager.addWaterBody(desc);
        
        const WaterBodyDesc* retrieved = manager.getWaterBody(id);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->type == WaterBodyType::Lake);
    }
}

TEST_CASE("WaterManager wave simulation", "[water][manager]") {
    auto& manager = WaterManager::instance();
    manager.initialize();
    
    // Add ocean
    manager.addWaterBody(WaterBodyDesc::ocean());
    
    SECTION("Update simulation") {
        manager.update(0.016f); // 60 FPS
        // Should not crash
    }
    
    SECTION("Get height at position") {
        f32 height = manager.getHeightAt(0.0f, 0.0f);
        // Height should be finite
        REQUIRE(std::isfinite(height));
    }
    
    SECTION("Get displacement at position") {
        Vec3 disp = manager.getDisplacementAt(0.0f, 0.0f);
        REQUIRE(std::isfinite(disp.x));
        REQUIRE(std::isfinite(disp.y));
        REQUIRE(std::isfinite(disp.z));
    }
    
    SECTION("Get normal at position") {
        Vec3 normal = manager.getNormalAt(0.0f, 0.0f);
        // Normal should be normalized
        f32 length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        REQUIRE(length == Approx(1.0f).margin(0.01f));
    }
}

TEST_CASE("WaterManager Gerstner waves", "[water][manager]") {
    auto& manager = WaterManager::instance();
    manager.initialize();
    
    SECTION("Default waves are initialized") {
        REQUIRE(manager.getGerstnerWaveCount() > 0);
    }
    
    SECTION("Add custom wave") {
        manager.clearGerstnerWaves();
        REQUIRE(manager.getGerstnerWaveCount() == 0);
        
        manager.addGerstnerWave(GerstnerWave::large(Vec2{1.0f, 0.0f}));
        REQUIRE(manager.getGerstnerWaveCount() == 1);
    }
    
    SECTION("Get wave") {
        manager.clearGerstnerWaves();
        manager.addGerstnerWave(GerstnerWave::medium(Vec2{0.5f, 0.5f}));
        
        const GerstnerWave* wave = manager.getGerstnerWave(0);
        REQUIRE(wave != nullptr);
        REQUIRE(wave->wavelength == Approx(20.0f));
    }
}

TEST_CASE("WaterManager cascades", "[water][manager]") {
    auto& manager = WaterManager::instance();
    manager.initialize();
    
    WaterSettings settings = WaterSettings::high();
    manager.setSettings(settings);
    
    SECTION("Cascades are created") {
        REQUIRE(manager.getCascadeCount() == settings.cascadeCount);
    }
    
    SECTION("Get cascade") {
        const OceanCascade* cascade = manager.getCascade(0);
        REQUIRE(cascade != nullptr);
    }
    
    SECTION("Set cascade") {
        OceanCascade stormy = OceanCascade::stormy();
        manager.setCascade(0, stormy);
        
        const OceanCascade* cascade = manager.getCascade(0);
        REQUIRE(cascade != nullptr);
        REQUIRE(cascade->windSpeed == Approx(25.0f));
    }
}

TEST_CASE("WaterManager settings", "[water][manager]") {
    auto& manager = WaterManager::instance();
    manager.initialize();
    
    SECTION("Set and get settings") {
        WaterSettings settings = WaterSettings::ultra();
        manager.setSettings(settings);
        
        REQUIRE(manager.getSettings().quality == WaterQuality::Ultra);
    }
    
    SECTION("Set and get optical properties") {
        auto props = WaterOpticalProperties::tropical();
        manager.setOpticalProperties(props);
        
        REQUIRE(manager.getOpticalProperties().visibility == Approx(60.0f));
    }
    
    SECTION("Set and get foam settings") {
        auto settings = FoamSettings::rough();
        manager.setFoamSettings(settings);
        
        REQUIRE(manager.getFoamSettings().coverage == Approx(0.7f));
    }
    
    SECTION("Set and get caustics settings") {
        CausticsSettings settings;
        settings.intensity = 2.0f;
        manager.setCausticsSettings(settings);
        
        REQUIRE(manager.getCausticsSettings().intensity == Approx(2.0f));
    }
}

TEST_CASE("WaterManager GPU data", "[water][manager]") {
    auto& manager = WaterManager::instance();
    manager.shutdown(); // Reset state
    manager.initialize();
    manager.addWaterBody(WaterBodyDesc::ocean(5.0f));
    
    GPUWaterData data = manager.getGPUData();
    
    REQUIRE(data.waterParams.x == Approx(5.0f));  // elevation
}

TEST_CASE("WaterManager time and statistics", "[water][manager]") {
    auto& manager = WaterManager::instance();
    manager.initialize();
    
    SECTION("Time advances with update") {
        f32 time1 = manager.getTime();
        manager.update(1.0f);
        f32 time2 = manager.getTime();
        
        REQUIRE(time2 > time1);
    }
    
    SECTION("Frame statistics") {
        manager.beginFrame();
        manager.endFrame();
        
        const WaterStats& stats = manager.getStats();
        // Should not crash
    }
}
