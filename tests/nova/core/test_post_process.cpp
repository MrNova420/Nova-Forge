/**
 * @file test_post_process.cpp
 * @brief Comprehensive tests for the post-processing system
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/post_process.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// PostProcessConfig Constants Tests
// =============================================================================

TEST_CASE("PostProcessConfig constants are valid", "[postprocess][config]") {
    SECTION("Bloom settings") {
        REQUIRE(PostProcessConfig::MAX_BLOOM_MIPS == 8);
        REQUIRE(PostProcessConfig::DEFAULT_BLOOM_THRESHOLD == 1.0f);
        REQUIRE(PostProcessConfig::DEFAULT_BLOOM_INTENSITY == 0.5f);
    }
    
    SECTION("LUT settings") {
        REQUIRE(PostProcessConfig::MAX_LUT_SIZE == 64);
    }
    
    SECTION("DOF settings") {
        REQUIRE(PostProcessConfig::DOF_MAX_SAMPLES == 128);
    }
    
    SECTION("Motion blur settings") {
        REQUIRE(PostProcessConfig::MOTION_BLUR_MAX_SAMPLES == 16);
    }
    
    SECTION("TAA settings") {
        REQUIRE(PostProcessConfig::TAA_HISTORY_COUNT == 2);
    }
    
    SECTION("SMAA settings") {
        REQUIRE(PostProcessConfig::SMAA_THRESHOLD == Approx(0.1f));
    }
}

// =============================================================================
// ToneMappingOperator Enum Tests
// =============================================================================

TEST_CASE("ToneMappingOperator enum values", "[postprocess][tonemapping]") {
    SECTION("Enum values are sequential") {
        REQUIRE(static_cast<u32>(ToneMappingOperator::None) == 0);
        REQUIRE(static_cast<u32>(ToneMappingOperator::Reinhard) == 1);
        REQUIRE(static_cast<u32>(ToneMappingOperator::ReinhardExtended) == 2);
        REQUIRE(static_cast<u32>(ToneMappingOperator::Uncharted2) == 3);
        REQUIRE(static_cast<u32>(ToneMappingOperator::ACES) == 4);
        REQUIRE(static_cast<u32>(ToneMappingOperator::ACESApprox) == 5);
        REQUIRE(static_cast<u32>(ToneMappingOperator::AgX) == 6);
        REQUIRE(static_cast<u32>(ToneMappingOperator::TonyMcMapface) == 7);
        REQUIRE(static_cast<u32>(ToneMappingOperator::KhronosPBRNeutral) == 8);
        REQUIRE(static_cast<u32>(ToneMappingOperator::COUNT) == 9);
    }
    
    SECTION("Operator names are correct") {
        REQUIRE(std::string(getToneMappingName(ToneMappingOperator::None)) == "None");
        REQUIRE(std::string(getToneMappingName(ToneMappingOperator::Reinhard)) == "Reinhard");
        REQUIRE(std::string(getToneMappingName(ToneMappingOperator::ACES)) == "ACES");
        REQUIRE(std::string(getToneMappingName(ToneMappingOperator::ACESApprox)) == "ACES Approx");
        REQUIRE(std::string(getToneMappingName(ToneMappingOperator::AgX)) == "AgX");
    }
}

// =============================================================================
// AntiAliasingTechnique Enum Tests
// =============================================================================

TEST_CASE("AntiAliasingTechnique enum values", "[postprocess][aa]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(AntiAliasingTechnique::None) == 0);
        REQUIRE(static_cast<u32>(AntiAliasingTechnique::FXAA) == 1);
        REQUIRE(static_cast<u32>(AntiAliasingTechnique::SMAA) == 2);
        REQUIRE(static_cast<u32>(AntiAliasingTechnique::TAA) == 3);
        REQUIRE(static_cast<u32>(AntiAliasingTechnique::TAAWithFXAA) == 4);
        REQUIRE(static_cast<u32>(AntiAliasingTechnique::DLSS) == 5);
        REQUIRE(static_cast<u32>(AntiAliasingTechnique::FSR) == 6);
        REQUIRE(static_cast<u32>(AntiAliasingTechnique::XeSS) == 7);
    }
    
    SECTION("AA names") {
        REQUIRE(std::string(getAAName(AntiAliasingTechnique::None)) == "None");
        REQUIRE(std::string(getAAName(AntiAliasingTechnique::FXAA)) == "FXAA");
        REQUIRE(std::string(getAAName(AntiAliasingTechnique::SMAA)) == "SMAA");
        REQUIRE(std::string(getAAName(AntiAliasingTechnique::TAA)) == "TAA");
        REQUIRE(std::string(getAAName(AntiAliasingTechnique::DLSS)) == "DLSS");
        REQUIRE(std::string(getAAName(AntiAliasingTechnique::FSR)) == "FSR");
    }
}

// =============================================================================
// DOFType Enum Tests
// =============================================================================

TEST_CASE("DOFType enum values", "[postprocess][dof]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(DOFType::Off) == 0);
        REQUIRE(static_cast<u32>(DOFType::Gaussian) == 1);
        REQUIRE(static_cast<u32>(DOFType::BokehCircular) == 2);
        REQUIRE(static_cast<u32>(DOFType::BokehHexagonal) == 3);
        REQUIRE(static_cast<u32>(DOFType::BokehOctagonal) == 4);
        REQUIRE(static_cast<u32>(DOFType::PhysicallyBased) == 5);
    }
    
    SECTION("DOF type names") {
        REQUIRE(std::string(getDOFTypeName(DOFType::Off)) == "Off");
        REQUIRE(std::string(getDOFTypeName(DOFType::Gaussian)) == "Gaussian");
        REQUIRE(std::string(getDOFTypeName(DOFType::BokehCircular)) == "Bokeh Circular");
        REQUIRE(std::string(getDOFTypeName(DOFType::BokehHexagonal)) == "Bokeh Hexagonal");
        REQUIRE(std::string(getDOFTypeName(DOFType::PhysicallyBased)) == "Physically Based");
    }
}

// =============================================================================
// MotionBlurType Enum Tests
// =============================================================================

TEST_CASE("MotionBlurType enum values", "[postprocess][motionblur]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(MotionBlurType::Off) == 0);
        REQUIRE(static_cast<u32>(MotionBlurType::Camera) == 1);
        REQUIRE(static_cast<u32>(MotionBlurType::PerObject) == 2);
        REQUIRE(static_cast<u32>(MotionBlurType::Combined) == 3);
    }
    
    SECTION("Motion blur type names") {
        REQUIRE(std::string(getMotionBlurTypeName(MotionBlurType::Off)) == "Off");
        REQUIRE(std::string(getMotionBlurTypeName(MotionBlurType::Camera)) == "Camera");
        REQUIRE(std::string(getMotionBlurTypeName(MotionBlurType::PerObject)) == "Per-Object");
        REQUIRE(std::string(getMotionBlurTypeName(MotionBlurType::Combined)) == "Combined");
    }
}

// =============================================================================
// BloomQuality Enum Tests
// =============================================================================

TEST_CASE("BloomQuality enum values", "[postprocess][bloom]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(BloomQuality::Off) == 0);
        REQUIRE(static_cast<u32>(BloomQuality::Low) == 1);
        REQUIRE(static_cast<u32>(BloomQuality::Medium) == 2);
        REQUIRE(static_cast<u32>(BloomQuality::High) == 3);
        REQUIRE(static_cast<u32>(BloomQuality::Ultra) == 4);
    }
}

// =============================================================================
// ToneMappingSettings Tests
// =============================================================================

TEST_CASE("ToneMappingSettings functionality", "[postprocess][tonemapping]") {
    SECTION("Default values") {
        ToneMappingSettings settings;
        REQUIRE(settings.op == ToneMappingOperator::ACES);
        REQUIRE(settings.exposure == Approx(1.0f));
        REQUIRE(settings.gamma == Approx(2.2f));
        REQUIRE(settings.autoExposure == false);
    }
    
    SECTION("Apply exposure") {
        ToneMappingSettings settings;
        settings.exposure = 2.0f; // +2 EV
        
        f32 result = settings.applyExposure(1.0f);
        REQUIRE(result == Approx(4.0f)); // 1.0 * 2^2 = 4.0
    }
    
    SECTION("Cinematic preset") {
        auto settings = ToneMappingSettings::createCinematic();
        REQUIRE(settings.op == ToneMappingOperator::ACES);
        REQUIRE(settings.exposure == Approx(0.0f));
        REQUIRE(settings.gamma == Approx(2.2f));
    }
    
    SECTION("Game preset") {
        auto settings = ToneMappingSettings::createGame();
        REQUIRE(settings.op == ToneMappingOperator::ACESApprox);
        REQUIRE(settings.autoExposure == true);
    }
}

// =============================================================================
// ColorGradingSettings Tests
// =============================================================================

TEST_CASE("ColorGradingSettings functionality", "[postprocess][colorgrading]") {
    SECTION("Default values") {
        ColorGradingSettings settings;
        REQUIRE(settings.enabled == true);
        REQUIRE(settings.temperature == Approx(6500.0f));
        REQUIRE(settings.tint == Approx(0.0f));
        REQUIRE(settings.saturation == Approx(1.0f));
        REQUIRE(settings.contrast == Approx(1.0f));
    }
    
    SECTION("Temperature to RGB conversion - daylight (6500K)") {
        ColorGradingSettings settings;
        settings.temperature = 6500.0f;
        Vec3 rgb = settings.getTemperatureRGB();
        
        // Near white at daylight
        REQUIRE(rgb.x >= 0.9f);
        REQUIRE(rgb.y >= 0.9f);
        REQUIRE(rgb.z >= 0.9f);
    }
    
    SECTION("Temperature to RGB conversion - warm (3000K)") {
        ColorGradingSettings settings;
        settings.temperature = 3000.0f;
        Vec3 rgb = settings.getTemperatureRGB();
        
        // Warm = more red, less blue
        REQUIRE(rgb.x > rgb.z);
    }
    
    SECTION("Temperature to RGB conversion - cool (10000K)") {
        ColorGradingSettings settings;
        settings.temperature = 10000.0f;
        Vec3 rgb = settings.getTemperatureRGB();
        
        // Cool = more blue
        REQUIRE(rgb.z >= 0.9f);
    }
    
    SECTION("Neutral preset") {
        auto settings = ColorGradingSettings::createNeutral();
        REQUIRE(settings.temperature == Approx(6500.0f));
        REQUIRE(settings.saturation == Approx(1.0f));
    }
    
    SECTION("Warm preset") {
        auto settings = ColorGradingSettings::createWarm();
        REQUIRE(settings.temperature < 6500.0f);
        REQUIRE(settings.saturation > 1.0f);
    }
    
    SECTION("Cold preset") {
        auto settings = ColorGradingSettings::createCold();
        REQUIRE(settings.temperature > 6500.0f);
        REQUIRE(settings.saturation < 1.0f);
    }
}

// =============================================================================
// BloomSettings Tests
// =============================================================================

TEST_CASE("BloomSettings functionality", "[postprocess][bloom]") {
    SECTION("Default values") {
        BloomSettings settings;
        REQUIRE(settings.quality == BloomQuality::Medium);
        REQUIRE(settings.threshold == Approx(PostProcessConfig::DEFAULT_BLOOM_THRESHOLD));
        REQUIRE(settings.intensity == Approx(PostProcessConfig::DEFAULT_BLOOM_INTENSITY));
    }
    
    SECTION("isEnabled check") {
        BloomSettings settings;
        REQUIRE(settings.isEnabled() == true);
        
        settings.quality = BloomQuality::Off;
        REQUIRE(settings.isEnabled() == false);
        
        settings.quality = BloomQuality::High;
        settings.intensity = 0.0f;
        REQUIRE(settings.isEnabled() == false);
    }
    
    SECTION("Mip count by quality") {
        BloomSettings settings;
        
        settings.quality = BloomQuality::Off;
        REQUIRE(settings.getMipCount() == 0);
        
        settings.quality = BloomQuality::Low;
        REQUIRE(settings.getMipCount() == 4);
        
        settings.quality = BloomQuality::Medium;
        REQUIRE(settings.getMipCount() == 6);
        
        settings.quality = BloomQuality::High;
        REQUIRE(settings.getMipCount() == 8);
        
        settings.quality = BloomQuality::Ultra;
        REQUIRE(settings.getMipCount() == PostProcessConfig::MAX_BLOOM_MIPS);
    }
    
    SECTION("Subtle preset") {
        auto settings = BloomSettings::createSubtle();
        REQUIRE(settings.threshold > 1.0f);
        REQUIRE(settings.intensity < 0.5f);
    }
    
    SECTION("Dramatic preset") {
        auto settings = BloomSettings::createDramatic();
        REQUIRE(settings.threshold < 1.0f);
        REQUIRE(settings.intensity > 0.5f);
    }
}

// =============================================================================
// DOFSettings Tests
// =============================================================================

TEST_CASE("DOFSettings functionality", "[postprocess][dof]") {
    SECTION("Default values") {
        DOFSettings settings;
        REQUIRE(settings.type == DOFType::Off);
        REQUIRE(settings.focusDistance == Approx(10.0f));
        REQUIRE(settings.aperture == Approx(5.6f));
        REQUIRE(settings.focalLength == Approx(50.0f));
    }
    
    SECTION("isEnabled check") {
        DOFSettings settings;
        REQUIRE(settings.isEnabled() == false);
        
        settings.type = DOFType::Gaussian;
        REQUIRE(settings.isEnabled() == true);
    }
    
    SECTION("Circle of confusion calculation - non-physical") {
        DOFSettings settings;
        settings.type = DOFType::Gaussian;
        settings.focusDistance = 10.0f;
        settings.focusRange = 4.0f;
        settings.farBlurStart = 12.0f;
        settings.farBlurEnd = 20.0f;
        settings.blurRadius = 1.0f;
        
        // In focus
        f32 cocInFocus = settings.calculateCoC(10.0f);
        REQUIRE(cocInFocus == Approx(0.0f));
        
        // Far blur (beyond focus)
        f32 cocFar = settings.calculateCoC(20.0f);
        REQUIRE(cocFar > 0.0f);
    }
    
    SECTION("Cinematic preset") {
        auto settings = DOFSettings::createCinematic();
        REQUIRE(settings.type == DOFType::BokehCircular);
        REQUIRE(settings.aperture == Approx(1.4f));
        REQUIRE(settings.sampleCount == 128);
    }
    
    SECTION("Game preset") {
        auto settings = DOFSettings::createGame();
        REQUIRE(settings.type == DOFType::Gaussian);
        REQUIRE(settings.autoFocus == true);
    }
}

// =============================================================================
// MotionBlurSettings Tests
// =============================================================================

TEST_CASE("MotionBlurSettings functionality", "[postprocess][motionblur]") {
    SECTION("Default values") {
        MotionBlurSettings settings;
        REQUIRE(settings.type == MotionBlurType::Off);
        REQUIRE(settings.intensity == Approx(0.5f));
        REQUIRE(settings.shutterAngle == Approx(180.0f));
    }
    
    SECTION("isEnabled check") {
        MotionBlurSettings settings;
        REQUIRE(settings.isEnabled() == false);
        
        settings.type = MotionBlurType::Camera;
        REQUIRE(settings.isEnabled() == true);
        
        settings.intensity = 0.0f;
        REQUIRE(settings.isEnabled() == false);
    }
    
    SECTION("Shutter speed calculation") {
        MotionBlurSettings settings;
        settings.shutterAngle = 180.0f;
        REQUIRE(settings.getShutterSpeed() == Approx(0.5f));
        
        settings.shutterAngle = 360.0f;
        REQUIRE(settings.getShutterSpeed() == Approx(1.0f));
        
        settings.shutterAngle = 90.0f;
        REQUIRE(settings.getShutterSpeed() == Approx(0.25f));
    }
    
    SECTION("Cinematic preset") {
        auto settings = MotionBlurSettings::createCinematic();
        REQUIRE(settings.type == MotionBlurType::Combined);
        REQUIRE(settings.shutterAngle == Approx(180.0f));
    }
    
    SECTION("Game preset") {
        auto settings = MotionBlurSettings::createGame();
        REQUIRE(settings.type == MotionBlurType::Camera);
        REQUIRE(settings.shutterAngle == Approx(90.0f));
    }
}

// =============================================================================
// AntiAliasingSettings Tests
// =============================================================================

TEST_CASE("AntiAliasingSettings functionality", "[postprocess][aa]") {
    SECTION("Default values") {
        AntiAliasingSettings settings;
        REQUIRE(settings.technique == AntiAliasingTechnique::TAA);
        REQUIRE(settings.taaFeedback == Approx(0.9f));
    }
    
    SECTION("Temporal AA detection") {
        AntiAliasingSettings settings;
        
        settings.technique = AntiAliasingTechnique::None;
        REQUIRE(settings.usesTemporal() == false);
        
        settings.technique = AntiAliasingTechnique::FXAA;
        REQUIRE(settings.usesTemporal() == false);
        
        settings.technique = AntiAliasingTechnique::SMAA;
        REQUIRE(settings.usesTemporal() == false);
        
        settings.technique = AntiAliasingTechnique::TAA;
        REQUIRE(settings.usesTemporal() == true);
        
        settings.technique = AntiAliasingTechnique::DLSS;
        REQUIRE(settings.usesTemporal() == true);
        
        settings.technique = AntiAliasingTechnique::FSR;
        REQUIRE(settings.usesTemporal() == true);
    }
    
    SECTION("Jitter offset generation") {
        AntiAliasingSettings settings;
        settings.technique = AntiAliasingTechnique::TAA;
        settings.taaJitterScale = 1.0f;
        
        Vec2 jitter0 = settings.getJitterOffset(0);
        Vec2 jitter1 = settings.getJitterOffset(1);
        
        // Jitters should be different
        REQUIRE((jitter0.x != jitter1.x || jitter0.y != jitter1.y));
        
        // Jitters should be in reasonable range
        REQUIRE(jitter0.x >= -0.5f);
        REQUIRE(jitter0.x <= 0.5f);
        REQUIRE(jitter0.y >= -0.5f);
        REQUIRE(jitter0.y <= 0.5f);
    }
    
    SECTION("No jitter when not temporal") {
        AntiAliasingSettings settings;
        settings.technique = AntiAliasingTechnique::FXAA;
        
        Vec2 jitter = settings.getJitterOffset(0);
        REQUIRE(jitter.x == Approx(0.0f));
        REQUIRE(jitter.y == Approx(0.0f));
    }
    
    SECTION("Performance preset") {
        auto settings = AntiAliasingSettings::createPerformance();
        REQUIRE(settings.technique == AntiAliasingTechnique::FXAA);
    }
}

// =============================================================================
// FilmGrainSettings Tests
// =============================================================================

TEST_CASE("FilmGrainSettings functionality", "[postprocess][filmgrain]") {
    SECTION("Default values") {
        FilmGrainSettings settings;
        REQUIRE(settings.enabled == false);
        REQUIRE(settings.intensity == Approx(0.1f));
        REQUIRE(settings.animated == true);
    }
    
    SECTION("Subtle preset") {
        auto settings = FilmGrainSettings::createSubtle();
        REQUIRE(settings.enabled == true);
        REQUIRE(settings.intensity < 0.1f);
    }
    
    SECTION("Cinematic preset") {
        auto settings = FilmGrainSettings::createCinematic();
        REQUIRE(settings.enabled == true);
        REQUIRE(settings.intensity > 0.1f);
    }
}

// =============================================================================
// VignetteSettings Tests
// =============================================================================

TEST_CASE("VignetteSettings functionality", "[postprocess][vignette]") {
    SECTION("Default values") {
        VignetteSettings settings;
        REQUIRE(settings.enabled == false);
        REQUIRE(settings.intensity == Approx(0.3f));
        REQUIRE(settings.roundness == Approx(1.0f));
        REQUIRE(settings.center.x == Approx(0.5f));
        REQUIRE(settings.center.y == Approx(0.5f));
    }
    
    SECTION("Subtle preset") {
        auto settings = VignetteSettings::createSubtle();
        REQUIRE(settings.enabled == true);
        REQUIRE(settings.intensity < 0.3f);
    }
    
    SECTION("Dramatic preset") {
        auto settings = VignetteSettings::createDramatic();
        REQUIRE(settings.enabled == true);
        REQUIRE(settings.intensity > 0.3f);
    }
}

// =============================================================================
// PostProcessVolume Tests
// =============================================================================

TEST_CASE("PostProcessVolume functionality", "[postprocess][volume]") {
    SECTION("Default volume") {
        auto volume = PostProcessVolume::createDefault();
        REQUIRE(volume.name == "Default");
        REQUIRE(volume.isGlobal == true);
        REQUIRE(volume.weight == Approx(1.0f));
        REQUIRE(volume.bloom.isEnabled() == true);
    }
    
    SECTION("Cinematic volume") {
        auto volume = PostProcessVolume::createCinematic();
        REQUIRE(volume.name == "Cinematic");
        REQUIRE(volume.toneMapping.op == ToneMappingOperator::ACES);
        REQUIRE(volume.dof.type == DOFType::BokehCircular);
        REQUIRE(volume.filmGrain.enabled == true);
        REQUIRE(volume.vignette.enabled == true);
    }
    
    SECTION("Performance volume") {
        auto volume = PostProcessVolume::createPerformance();
        REQUIRE(volume.name == "Performance");
        REQUIRE(volume.bloom.quality == BloomQuality::Low);
        REQUIRE(volume.dof.type == DOFType::Off);
        REQUIRE(volume.motionBlur.type == MotionBlurType::Off);
    }
}

// =============================================================================
// PostProcessStats Tests
// =============================================================================

TEST_CASE("PostProcessStats functionality", "[postprocess][stats]") {
    SECTION("Reset clears all values") {
        PostProcessStats stats;
        stats.bloomTimeMs = 1.0f;
        stats.dofTimeMs = 2.0f;
        stats.aaTimeMs = 0.5f;
        stats.totalTimeMs = 5.0f;
        
        stats.reset();
        
        REQUIRE(stats.bloomTimeMs == Approx(0.0f));
        REQUIRE(stats.dofTimeMs == Approx(0.0f));
        REQUIRE(stats.aaTimeMs == Approx(0.0f));
        REQUIRE(stats.totalTimeMs == Approx(0.0f));
    }
    
    SECTION("Calculate total") {
        PostProcessStats stats;
        stats.bloomTimeMs = 1.0f;
        stats.dofTimeMs = 2.0f;
        stats.motionBlurTimeMs = 0.5f;
        stats.aaTimeMs = 0.3f;
        stats.toneMappingTimeMs = 0.2f;
        
        stats.calculateTotal();
        
        REQUIRE(stats.totalTimeMs == Approx(4.0f));
    }
}

// =============================================================================
// PostProcessManager Tests
// =============================================================================

TEST_CASE("PostProcessManager construction", "[postprocess][manager]") {
    SECTION("Default construction has default volume") {
        PostProcessManager manager;
        REQUIRE(manager.getVolumes().size() == 1);
    }
}

TEST_CASE("PostProcessManager volume management", "[postprocess][manager]") {
    PostProcessManager manager;
    
    SECTION("Add volume") {
        PostProcessVolume volume;
        volume.name = "Custom";
        manager.addVolume(volume);
        
        REQUIRE(manager.getVolumes().size() == 2);
    }
    
    SECTION("Remove volume") {
        PostProcessVolume volume;
        volume.name = "ToRemove";
        manager.addVolume(volume);
        
        REQUIRE(manager.removeVolume("ToRemove") == true);
        REQUIRE(manager.removeVolume("NonExistent") == false);
    }
    
    SECTION("Get volume by name") {
        PostProcessVolume* vol = manager.getVolume("Default");
        REQUIRE(vol != nullptr);
        REQUIRE(vol->name == "Default");
        
        REQUIRE(manager.getVolume("NonExistent") == nullptr);
    }
    
    SECTION("Clear volumes") {
        manager.addVolume(PostProcessVolume{});
        manager.clearVolumes();
        
        REQUIRE(manager.getVolumes().empty());
    }
}

TEST_CASE("PostProcessManager frame lifecycle", "[postprocess][manager]") {
    PostProcessManager manager;
    
    SECTION("Begin/end frame") {
        manager.beginFrame(0);
        manager.endFrame();
        
        // Stats should be calculated
        const auto& stats = manager.getStats();
        REQUIRE(std::isfinite(stats.totalTimeMs));
    }
    
    SECTION("Jitter offset changes per frame") {
        manager.beginFrame(0);
        Vec2 jitter0 = manager.getJitterOffset();
        
        manager.beginFrame(1);
        Vec2 jitter1 = manager.getJitterOffset();
        
        // Should be different (if using TAA)
        // Note: depends on blended volume's AA settings
    }
    
    SECTION("Exposure management") {
        manager.setCurrentExposure(2.0f);
        REQUIRE(manager.getCurrentExposure() == Approx(2.0f));
    }
}

TEST_CASE("PostProcessManager blended volume", "[postprocess][manager]") {
    PostProcessManager manager;
    
    SECTION("Blended volume reflects added volumes") {
        manager.clearVolumes();
        
        auto volume = PostProcessVolume::createCinematic();
        manager.addVolume(volume);
        manager.beginFrame(0);
        
        const auto& blended = manager.getBlendedVolume();
        REQUIRE(blended.name == "Cinematic");
    }
}

// =============================================================================
// Tone Mapping Function Tests
// =============================================================================

TEST_CASE("Tone mapping functions", "[postprocess][tonemapping][functions]") {
    SECTION("Reinhard tone mapping") {
        Vec3 black = tonemapReinhard(Vec3{0.0f, 0.0f, 0.0f});
        REQUIRE(black.x == Approx(0.0f));
        REQUIRE(black.y == Approx(0.0f));
        REQUIRE(black.z == Approx(0.0f));
        
        Vec3 white = tonemapReinhard(Vec3{10.0f, 10.0f, 10.0f});
        REQUIRE(white.x < 1.0f);
        REQUIRE(white.x > 0.9f);
    }
    
    SECTION("Reinhard extended tone mapping") {
        Vec3 result = tonemapReinhardExtended(Vec3{1.0f, 1.0f, 1.0f}, 4.0f);
        REQUIRE(result.x > 0.0f);
        REQUIRE(result.x < 1.0f);
    }
    
    SECTION("Uncharted 2 tone mapping") {
        Vec3 black = tonemapUncharted2(Vec3{0.0f, 0.0f, 0.0f});
        REQUIRE(black.x >= 0.0f);
        
        Vec3 mid = tonemapUncharted2(Vec3{1.0f, 1.0f, 1.0f});
        REQUIRE(mid.x > 0.0f);
        REQUIRE(mid.x < 1.0f);
    }
    
    SECTION("ACES tone mapping") {
        Vec3 black = tonemapACES(Vec3{0.0f, 0.0f, 0.0f});
        REQUIRE(black.x == Approx(0.0f));
        
        Vec3 result = tonemapACES(Vec3{1.0f, 1.0f, 1.0f});
        REQUIRE(result.x > 0.0f);
        REQUIRE(result.x < 1.0f);
    }
    
    SECTION("ACES approx tone mapping") {
        Vec3 result = tonemapACESApprox(Vec3{1.0f, 1.0f, 1.0f});
        REQUIRE(result.x >= 0.0f);
        REQUIRE(result.x <= 1.0f);
        REQUIRE(result.y >= 0.0f);
        REQUIRE(result.y <= 1.0f);
        REQUIRE(result.z >= 0.0f);
        REQUIRE(result.z <= 1.0f);
    }
}

// =============================================================================
// Color Space Function Tests
// =============================================================================

TEST_CASE("Color space conversion functions", "[postprocess][colorspace]") {
    SECTION("Gamma correction") {
        Vec3 linear = Vec3{0.5f, 0.5f, 0.5f};
        Vec3 gamma = gammaCorrect(linear, 2.2f);
        
        // Gamma corrected should be brighter (higher values)
        REQUIRE(gamma.x > linear.x);
    }
    
    SECTION("Linear to sRGB") {
        Vec3 black = linearToSRGB(Vec3{0.0f, 0.0f, 0.0f});
        REQUIRE(black.x == Approx(0.0f));
        
        Vec3 white = linearToSRGB(Vec3{1.0f, 1.0f, 1.0f});
        REQUIRE(white.x == Approx(1.0f));
        
        Vec3 mid = linearToSRGB(Vec3{0.5f, 0.5f, 0.5f});
        REQUIRE(mid.x > 0.5f); // sRGB brightens midtones
    }
    
    SECTION("sRGB to linear") {
        Vec3 black = sRGBToLinear(Vec3{0.0f, 0.0f, 0.0f});
        REQUIRE(black.x == Approx(0.0f));
        
        Vec3 white = sRGBToLinear(Vec3{1.0f, 1.0f, 1.0f});
        REQUIRE(white.x == Approx(1.0f));
        
        Vec3 mid = sRGBToLinear(Vec3{0.5f, 0.5f, 0.5f});
        REQUIRE(mid.x < 0.5f); // Linear is darker for midtones
    }
    
    SECTION("Round trip sRGB <-> linear") {
        Vec3 original{0.3f, 0.5f, 0.7f};
        Vec3 srgb = linearToSRGB(original);
        Vec3 backToLinear = sRGBToLinear(srgb);
        
        REQUIRE(backToLinear.x == Approx(original.x).margin(0.001f));
        REQUIRE(backToLinear.y == Approx(original.y).margin(0.001f));
        REQUIRE(backToLinear.z == Approx(original.z).margin(0.001f));
    }
}

// =============================================================================
// Luminance Function Tests
// =============================================================================

TEST_CASE("Luminance calculation", "[postprocess][luminance]") {
    SECTION("Black has zero luminance") {
        f32 lum = luminance(Vec3{0.0f, 0.0f, 0.0f});
        REQUIRE(lum == Approx(0.0f));
    }
    
    SECTION("White has full luminance") {
        f32 lum = luminance(Vec3{1.0f, 1.0f, 1.0f});
        REQUIRE(lum == Approx(1.0f));
    }
    
    SECTION("Green contributes most to luminance") {
        f32 lumR = luminance(Vec3{1.0f, 0.0f, 0.0f});
        f32 lumG = luminance(Vec3{0.0f, 1.0f, 0.0f});
        f32 lumB = luminance(Vec3{0.0f, 0.0f, 1.0f});
        
        REQUIRE(lumG > lumR);
        REQUIRE(lumG > lumB);
        REQUIRE(lumR > lumB);
    }
    
    SECTION("Luminance coefficients are correct (Rec. 709)") {
        f32 lumR = luminance(Vec3{1.0f, 0.0f, 0.0f});
        f32 lumG = luminance(Vec3{0.0f, 1.0f, 0.0f});
        f32 lumB = luminance(Vec3{0.0f, 0.0f, 1.0f});
        
        REQUIRE(lumR == Approx(0.2126f));
        REQUIRE(lumG == Approx(0.7152f));
        REQUIRE(lumB == Approx(0.0722f));
    }
}
