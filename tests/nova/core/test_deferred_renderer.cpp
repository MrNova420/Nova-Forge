/**
 * @file test_deferred_renderer.cpp
 * @brief Comprehensive tests for the deferred rendering system
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/deferred_renderer.hpp>

using namespace nova;
using Catch::Approx;

// =============================================================================
// DeferredConfig Constants Tests
// =============================================================================

TEST_CASE("DeferredConfig constants are valid", "[deferred][config]") {
    SECTION("G-Buffer limits") {
        REQUIRE(DeferredConfig::MAX_GBUFFER_TARGETS == 8);
        REQUIRE(DeferredConfig::MAX_GBUFFER_TARGETS >= static_cast<usize>(GBufferTarget::COUNT));
    }
    
    SECTION("Decal and light volume limits") {
        REQUIRE(DeferredConfig::MAX_DECALS == 4096);
        REQUIRE(DeferredConfig::MAX_LIGHT_VOLUMES == 2048);
    }
    
    SECTION("Resolution scale") {
        REQUIRE(DeferredConfig::DEFAULT_RESOLUTION_SCALE == 1.0f);
    }
    
    SECTION("SSAO settings") {
        REQUIRE(DeferredConfig::SSAO_KERNEL_SIZE_HIGH == 64);
        REQUIRE(DeferredConfig::SSAO_KERNEL_SIZE_MEDIUM == 32);
        REQUIRE(DeferredConfig::SSAO_KERNEL_SIZE_LOW == 16);
        REQUIRE(DeferredConfig::SSAO_NOISE_SIZE == 4);
    }
    
    SECTION("SSR settings") {
        REQUIRE(DeferredConfig::SSR_MAX_STEPS == 64);
        REQUIRE(DeferredConfig::SSR_REFINEMENT_STEPS == 8);
    }
    
    SECTION("Hi-Z limits") {
        REQUIRE(DeferredConfig::MAX_HIZ_MIPS == 12);
    }
}

// =============================================================================
// GBufferTarget Enum Tests
// =============================================================================

TEST_CASE("GBufferTarget enum values", "[deferred][gbuffer]") {
    SECTION("Enum values are sequential") {
        REQUIRE(static_cast<u32>(GBufferTarget::AlbedoMetallic) == 0);
        REQUIRE(static_cast<u32>(GBufferTarget::NormalRoughnessAO) == 1);
        REQUIRE(static_cast<u32>(GBufferTarget::MotionEmissive) == 2);
        REQUIRE(static_cast<u32>(GBufferTarget::VelocityMaterialID) == 3);
        REQUIRE(static_cast<u32>(GBufferTarget::Depth) == 4);
        REQUIRE(static_cast<u32>(GBufferTarget::Stencil) == 5);
        REQUIRE(static_cast<u32>(GBufferTarget::ShadingFlags) == 6);
        REQUIRE(static_cast<u32>(GBufferTarget::Custom) == 7);
    }
    
    SECTION("COUNT is correct") {
        REQUIRE(static_cast<u32>(GBufferTarget::COUNT) == 8);
    }
    
    SECTION("Target names are correct") {
        REQUIRE(std::string(getGBufferTargetName(GBufferTarget::AlbedoMetallic)) == "AlbedoMetallic");
        REQUIRE(std::string(getGBufferTargetName(GBufferTarget::NormalRoughnessAO)) == "NormalRoughnessAO");
        REQUIRE(std::string(getGBufferTargetName(GBufferTarget::MotionEmissive)) == "MotionEmissive");
        REQUIRE(std::string(getGBufferTargetName(GBufferTarget::Depth)) == "Depth");
    }
}

// =============================================================================
// AOTechnique Enum Tests
// =============================================================================

TEST_CASE("AOTechnique enum values", "[deferred][ao]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(AOTechnique::None) == 0);
        REQUIRE(static_cast<u32>(AOTechnique::SSAO) == 1);
        REQUIRE(static_cast<u32>(AOTechnique::HBAOPlus) == 2);
        REQUIRE(static_cast<u32>(AOTechnique::GTAO) == 3);
        REQUIRE(static_cast<u32>(AOTechnique::RTAO) == 4);
        REQUIRE(static_cast<u32>(AOTechnique::COUNT) == 5);
    }
    
    SECTION("Technique names") {
        REQUIRE(std::string(getAOTechniqueName(AOTechnique::None)) == "None");
        REQUIRE(std::string(getAOTechniqueName(AOTechnique::SSAO)) == "SSAO");
        REQUIRE(std::string(getAOTechniqueName(AOTechnique::HBAOPlus)) == "HBAO+");
        REQUIRE(std::string(getAOTechniqueName(AOTechnique::GTAO)) == "GTAO");
        REQUIRE(std::string(getAOTechniqueName(AOTechnique::RTAO)) == "RTAO");
    }
}

// =============================================================================
// SSRQuality Enum Tests
// =============================================================================

TEST_CASE("SSRQuality enum values", "[deferred][ssr]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(SSRQuality::Off) == 0);
        REQUIRE(static_cast<u32>(SSRQuality::Low) == 1);
        REQUIRE(static_cast<u32>(SSRQuality::Medium) == 2);
        REQUIRE(static_cast<u32>(SSRQuality::High) == 3);
        REQUIRE(static_cast<u32>(SSRQuality::Ultra) == 4);
    }
    
    SECTION("Quality names") {
        REQUIRE(std::string(getSSRQualityName(SSRQuality::Off)) == "Off");
        REQUIRE(std::string(getSSRQualityName(SSRQuality::Low)) == "Low");
        REQUIRE(std::string(getSSRQualityName(SSRQuality::Medium)) == "Medium");
        REQUIRE(std::string(getSSRQualityName(SSRQuality::High)) == "High");
        REQUIRE(std::string(getSSRQualityName(SSRQuality::Ultra)) == "Ultra");
    }
}

// =============================================================================
// DecalBlendMode Enum Tests
// =============================================================================

TEST_CASE("DecalBlendMode enum values", "[deferred][decal]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u32>(DecalBlendMode::Replace) == 0);
        REQUIRE(static_cast<u32>(DecalBlendMode::AlphaBlend) == 1);
        REQUIRE(static_cast<u32>(DecalBlendMode::Multiply) == 2);
        REQUIRE(static_cast<u32>(DecalBlendMode::Additive) == 3);
        REQUIRE(static_cast<u32>(DecalBlendMode::NormalBlend) == 4);
        REQUIRE(static_cast<u32>(DecalBlendMode::PBROverlay) == 5);
    }
    
    SECTION("Blend mode names") {
        REQUIRE(std::string(getDecalBlendModeName(DecalBlendMode::Replace)) == "Replace");
        REQUIRE(std::string(getDecalBlendModeName(DecalBlendMode::AlphaBlend)) == "AlphaBlend");
        REQUIRE(std::string(getDecalBlendModeName(DecalBlendMode::Multiply)) == "Multiply");
        REQUIRE(std::string(getDecalBlendModeName(DecalBlendMode::Additive)) == "Additive");
        REQUIRE(std::string(getDecalBlendModeName(DecalBlendMode::NormalBlend)) == "NormalBlend");
        REQUIRE(std::string(getDecalBlendModeName(DecalBlendMode::PBROverlay)) == "PBROverlay");
    }
}

// =============================================================================
// DeferredPhase Enum Tests
// =============================================================================

TEST_CASE("DeferredPhase enum values", "[deferred][phase]") {
    SECTION("Phase ordering") {
        REQUIRE(static_cast<u32>(DeferredPhase::DepthPrePass) == 0);
        REQUIRE(static_cast<u32>(DeferredPhase::GBufferFill) == 1);
        REQUIRE(static_cast<u32>(DeferredPhase::DecalPass) == 2);
        REQUIRE(static_cast<u32>(DeferredPhase::AOPass) == 3);
        REQUIRE(static_cast<u32>(DeferredPhase::ScreenSpaceShadows) == 4);
        REQUIRE(static_cast<u32>(DeferredPhase::LightingPass) == 5);
        REQUIRE(static_cast<u32>(DeferredPhase::SSRPass) == 6);
        REQUIRE(static_cast<u32>(DeferredPhase::VolumetricPass) == 7);
        REQUIRE(static_cast<u32>(DeferredPhase::CompositePass) == 8);
        REQUIRE(static_cast<u32>(DeferredPhase::COUNT) == 9);
    }
}

// =============================================================================
// GBufferTextureDesc Tests
// =============================================================================

TEST_CASE("GBufferTextureDesc factory methods", "[deferred][gbuffer]") {
    SECTION("Create albedo metallic target") {
        auto desc = GBufferTextureDesc::createAlbedoMetallic();
        REQUIRE(desc.target == GBufferTarget::AlbedoMetallic);
        REQUIRE(desc.format == GBufferFormat::RGBA8_SRGB);
        REQUIRE(desc.enabled == true);
        REQUIRE(desc.name == "GBuffer_AlbedoMetallic");
    }
    
    SECTION("Create normal roughness AO target") {
        auto desc = GBufferTextureDesc::createNormalRoughnessAO();
        REQUIRE(desc.target == GBufferTarget::NormalRoughnessAO);
        REQUIRE(desc.format == GBufferFormat::RGBA16F);
        REQUIRE(desc.clearValue[3] == 1.0f); // Full AO by default
    }
    
    SECTION("Create motion emissive target") {
        auto desc = GBufferTextureDesc::createMotionEmissive();
        REQUIRE(desc.target == GBufferTarget::MotionEmissive);
        REQUIRE(desc.format == GBufferFormat::RGBA16F);
    }
    
    SECTION("Create depth target") {
        auto desc = GBufferTextureDesc::createDepth();
        REQUIRE(desc.target == GBufferTarget::Depth);
        REQUIRE(desc.format == GBufferFormat::D32F);
        REQUIRE(desc.clearValue[0] == 1.0f); // Far plane
    }
}

// =============================================================================
// GBufferConfig Tests
// =============================================================================

TEST_CASE("GBufferConfig functionality", "[deferred][gbuffer]") {
    SECTION("Default configuration") {
        auto config = GBufferConfig::createDefault(1920, 1080);
        REQUIRE(config.width == 1920);
        REQUIRE(config.height == 1080);
        REQUIRE(config.resolutionScale == 1.0f);
        REQUIRE(config.targets.size() == 4);
        REQUIRE(config.enableVelocity == true);
        REQUIRE(config.enableEmissive == true);
        REQUIRE(config.enableStencil == true);
    }
    
    SECTION("Minimal configuration") {
        auto config = GBufferConfig::createMinimal(1280, 720);
        REQUIRE(config.width == 1280);
        REQUIRE(config.height == 720);
        REQUIRE(config.targets.size() == 3);
        REQUIRE(config.enableVelocity == false);
        REQUIRE(config.enableEmissive == false);
    }
    
    SECTION("Resolution scaling") {
        GBufferConfig config;
        config.width = 1920;
        config.height = 1080;
        config.resolutionScale = 0.5f;
        
        REQUIRE(config.getScaledWidth() == 960);
        REQUIRE(config.getScaledHeight() == 540);
    }
    
    SECTION("Resolution scaling at 2x") {
        GBufferConfig config;
        config.width = 1920;
        config.height = 1080;
        config.resolutionScale = 2.0f;
        
        REQUIRE(config.getScaledWidth() == 3840);
        REQUIRE(config.getScaledHeight() == 2160);
    }
}

// =============================================================================
// SSAOConfig Tests
// =============================================================================

TEST_CASE("SSAOConfig functionality", "[deferred][ssao]") {
    SECTION("Default values") {
        SSAOConfig config;
        REQUIRE(config.technique == AOTechnique::GTAO);
        REQUIRE(config.kernelSize == DeferredConfig::SSAO_KERNEL_SIZE_MEDIUM);
        REQUIRE(config.radius == Approx(0.5f));
        REQUIRE(config.intensity == Approx(1.0f));
    }
    
    SECTION("Low quality preset") {
        auto config = SSAOConfig::createLow();
        REQUIRE(config.technique == AOTechnique::SSAO);
        REQUIRE(config.kernelSize == DeferredConfig::SSAO_KERNEL_SIZE_LOW);
        REQUIRE(config.halfResolution == true);
        REQUIRE(config.temporalFilter == false);
    }
    
    SECTION("High quality preset") {
        auto config = SSAOConfig::createHigh();
        REQUIRE(config.technique == AOTechnique::GTAO);
        REQUIRE(config.kernelSize == DeferredConfig::SSAO_KERNEL_SIZE_HIGH);
        REQUIRE(config.halfResolution == false);
        REQUIRE(config.temporalFilter == true);
    }
    
    SECTION("Validation clamping") {
        SSAOConfig config;
        config.kernelSize = 256; // Too high
        config.radius = 10.0f;   // Too high
        config.intensity = 10.0f; // Too high
        config.bias = 0.5f;      // Too high
        
        config.validate();
        
        REQUIRE(config.kernelSize == 128);
        REQUIRE(config.radius == Approx(5.0f));
        REQUIRE(config.intensity == Approx(4.0f));
        REQUIRE(config.bias == Approx(0.1f));
    }
}

// =============================================================================
// SSRConfig Tests
// =============================================================================

TEST_CASE("SSRConfig functionality", "[deferred][ssr]") {
    SECTION("Default values") {
        SSRConfig config;
        REQUIRE(config.quality == SSRQuality::Medium);
        REQUIRE(config.isEnabled() == true);
    }
    
    SECTION("Off quality is disabled") {
        SSRConfig config;
        config.quality = SSRQuality::Off;
        REQUIRE(config.isEnabled() == false);
        REQUIRE(config.getEffectiveSteps() == 0);
    }
    
    SECTION("Effective steps by quality") {
        SSRConfig config;
        
        config.quality = SSRQuality::Low;
        REQUIRE(config.getEffectiveSteps() == 16);
        
        config.quality = SSRQuality::Medium;
        REQUIRE(config.getEffectiveSteps() == 32);
        
        config.quality = SSRQuality::High;
        REQUIRE(config.getEffectiveSteps() == 64);
        
        config.quality = SSRQuality::Ultra;
        REQUIRE(config.getEffectiveSteps() == 128);
    }
}

// =============================================================================
// DecalData Tests
// =============================================================================

TEST_CASE("DecalData functionality", "[deferred][decal]") {
    SECTION("Default values") {
        DecalData decal;
        REQUIRE(decal.blendMode == DecalBlendMode::AlphaBlend);
        REQUIRE(decal.normalStrength == Approx(1.0f));
        REQUIRE(decal.angleFade == Approx(0.5f));
        REQUIRE(decal.priority == 0);
        REQUIRE(decal.depthFade == true);
    }
    
    SECTION("Inverse matrix update") {
        DecalData decal;
        decal.worldMatrix = Mat4::identity();
        decal.updateInverseMatrix();
        
        // Inverse of identity is identity
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                f32 expected = (i == j) ? 1.0f : 0.0f;
                REQUIRE(decal.invWorldMatrix[i][j] == Approx(expected).margin(0.0001f));
            }
        }
    }
}

// =============================================================================
// GPUDecalData Tests
// =============================================================================

TEST_CASE("GPUDecalData creation", "[deferred][decal][gpu]") {
    SECTION("Create from DecalData") {
        DecalData decal;
        decal.albedoTint = Vec4{1.0f, 0.5f, 0.25f, 1.0f};
        decal.normalStrength = 0.8f;
        decal.angleFade = 0.6f;
        decal.blendMode = DecalBlendMode::Multiply;
        decal.albedoTexture = 10;
        decal.normalTexture = 11;
        decal.pbrTexture = 12;
        
        auto gpu = GPUDecalData::fromDecal(decal);
        
        REQUIRE(gpu.albedoTint.x == Approx(1.0f));
        REQUIRE(gpu.albedoTint.y == Approx(0.5f));
        REQUIRE(gpu.albedoTint.z == Approx(0.25f));
        REQUIRE(gpu.albedoTint.w == Approx(1.0f));
        REQUIRE(gpu.parameters.x == Approx(0.8f)); // normalStrength
        REQUIRE(gpu.parameters.y == Approx(0.6f)); // angleFade
        REQUIRE(gpu.parameters.z == Approx(2.0f)); // blendMode (Multiply = 2)
        REQUIRE(gpu.textureIndices[0] == 10);
        REQUIRE(gpu.textureIndices[1] == 11);
        REQUIRE(gpu.textureIndices[2] == 12);
    }
}

// =============================================================================
// LightVolume Tests
// =============================================================================

TEST_CASE("LightVolume functionality", "[deferred][light]") {
    SECTION("Default values") {
        LightVolume volume;
        REQUIRE(volume.shape == LightVolumeShape::Sphere);
        REQUIRE(volume.useStencil == true);
    }
    
    SECTION("Bounding radius calculation") {
        LightVolume volume;
        volume.scale = Vec3{2.0f, 3.0f, 1.5f};
        REQUIRE(volume.getBoundingRadius() == Approx(3.0f));
    }
    
    SECTION("Visibility check") {
        LightVolume volume;
        volume.position = Vec3{0.0f, 0.0f, 0.0f};
        volume.scale = Vec3{10.0f, 10.0f, 10.0f};
        
        Vec3 cameraPos{0.0f, 0.0f, -50.0f};
        
        // Within range
        REQUIRE(volume.isVisible(cameraPos, 100.0f) == true);
        
        // Out of range
        REQUIRE(volume.isVisible(cameraPos, 30.0f) == false);
    }
}

// =============================================================================
// DeferredStats Tests
// =============================================================================

TEST_CASE("DeferredStats functionality", "[deferred][stats]") {
    SECTION("Reset clears all values") {
        DeferredStats stats;
        stats.gbufferObjects = 100;
        stats.decalsRendered = 50;
        stats.lightVolumes = 25;
        stats.pixelsLit = 1000000;
        stats.gbufferTimeMs = 5.0f;
        stats.totalTimeMs = 10.0f;
        
        stats.reset();
        
        REQUIRE(stats.gbufferObjects == 0);
        REQUIRE(stats.decalsRendered == 0);
        REQUIRE(stats.lightVolumes == 0);
        REQUIRE(stats.pixelsLit == 0);
        REQUIRE(stats.gbufferTimeMs == Approx(0.0f));
        REQUIRE(stats.totalTimeMs == Approx(0.0f));
    }
}

// =============================================================================
// DeferredRenderer Class Tests
// =============================================================================

TEST_CASE("DeferredRenderer construction", "[deferred][renderer]") {
    SECTION("Default construction") {
        DeferredRenderer renderer;
        REQUIRE(renderer.needsRebuild() == false);
    }
    
    SECTION("Construction with config") {
        auto config = GBufferConfig::createDefault(1920, 1080);
        DeferredRenderer renderer(config);
        
        const auto& storedConfig = renderer.getGBufferConfig();
        REQUIRE(storedConfig.width == 1920);
        REQUIRE(storedConfig.height == 1080);
    }
}

TEST_CASE("DeferredRenderer configuration", "[deferred][renderer]") {
    DeferredRenderer renderer;
    
    SECTION("Set G-Buffer config marks rebuild needed") {
        auto config = GBufferConfig::createDefault(1920, 1080);
        renderer.setGBufferConfig(config);
        REQUIRE(renderer.needsRebuild() == true);
    }
    
    SECTION("Set SSAO config") {
        auto ssaoConfig = SSAOConfig::createHigh();
        renderer.setSSAOConfig(ssaoConfig);
        
        const auto& stored = renderer.getSSAOConfig();
        REQUIRE(stored.technique == AOTechnique::GTAO);
        REQUIRE(stored.kernelSize == DeferredConfig::SSAO_KERNEL_SIZE_HIGH);
    }
    
    SECTION("Set SSR config") {
        SSRConfig ssrConfig;
        ssrConfig.quality = SSRQuality::Ultra;
        renderer.setSSRConfig(ssrConfig);
        
        const auto& stored = renderer.getSSRConfig();
        REQUIRE(stored.quality == SSRQuality::Ultra);
    }
}

TEST_CASE("DeferredRenderer decal management", "[deferred][renderer]") {
    DeferredRenderer renderer;
    
    SECTION("Add decals") {
        DecalData decal1, decal2;
        decal1.priority = 10;
        decal2.priority = 5;
        
        renderer.addDecal(decal1);
        renderer.addDecal(decal2);
        
        REQUIRE(renderer.getDecals().size() == 2);
    }
    
    SECTION("Sort decals by priority") {
        DecalData decal1, decal2, decal3;
        decal1.priority = 10;
        decal2.priority = 5;
        decal3.priority = 15;
        
        renderer.addDecal(decal1);
        renderer.addDecal(decal2);
        renderer.addDecal(decal3);
        renderer.sortDecals();
        
        const auto& decals = renderer.getDecals();
        REQUIRE(decals[0].priority == 5);
        REQUIRE(decals[1].priority == 10);
        REQUIRE(decals[2].priority == 15);
    }
    
    SECTION("Clear decals") {
        renderer.addDecal(DecalData{});
        renderer.addDecal(DecalData{});
        renderer.clearDecals();
        
        REQUIRE(renderer.getDecals().empty());
    }
}

TEST_CASE("DeferredRenderer light volume management", "[deferred][renderer]") {
    DeferredRenderer renderer;
    
    SECTION("Add light volumes") {
        LightVolume vol1, vol2;
        renderer.addLightVolume(vol1);
        renderer.addLightVolume(vol2);
        
        REQUIRE(renderer.getLightVolumes().size() == 2);
    }
    
    SECTION("Clear light volumes") {
        renderer.addLightVolume(LightVolume{});
        renderer.clearLightVolumes();
        
        REQUIRE(renderer.getLightVolumes().empty());
    }
}

TEST_CASE("DeferredRenderer frame lifecycle", "[deferred][renderer]") {
    DeferredRenderer renderer;
    
    SECTION("Begin frame resets state") {
        renderer.addDecal(DecalData{});
        renderer.addLightVolume(LightVolume{});
        
        renderer.beginFrame();
        
        REQUIRE(renderer.getDecals().empty());
        REQUIRE(renderer.getLightVolumes().empty());
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::DepthPrePass);
    }
    
    SECTION("Execute phases in order") {
        renderer.beginFrame();
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::DepthPrePass);
        
        renderer.executeDepthPrePass();
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::DepthPrePass);
        
        renderer.executeGBufferPass();
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::GBufferFill);
        
        renderer.executeDecalPass();
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::DecalPass);
        
        renderer.executeAOPass();
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::AOPass);
        
        renderer.executeLightingPass();
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::LightingPass);
        
        renderer.executeSSRPass();
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::SSRPass);
        
        renderer.endFrame();
        REQUIRE(renderer.getCurrentPhase() == DeferredPhase::CompositePass);
    }
}

TEST_CASE("DeferredRenderer memory calculation", "[deferred][renderer]") {
    auto config = GBufferConfig::createDefault(1920, 1080);
    DeferredRenderer renderer(config);
    
    SECTION("Memory usage is non-zero") {
        u64 memory = renderer.calculateGBufferMemory();
        REQUIRE(memory > 0);
    }
    
    SECTION("Stats track memory") {
        const auto& stats = renderer.getStats();
        REQUIRE(stats.gbufferMemory > 0);
    }
}

// =============================================================================
// Utility Function Tests
// =============================================================================

TEST_CASE("generateSSAOKernel creates valid samples", "[deferred][ssao][utility]") {
    std::vector<Vec3> samples;
    
    SECTION("Correct sample count") {
        generateSSAOKernel(samples, 32);
        REQUIRE(samples.size() == 32);
    }
    
    SECTION("Samples are in reasonable range") {
        generateSSAOKernel(samples, 64);
        
        for (const auto& sample : samples) {
            // All samples should be within unit sphere
            f32 length = std::sqrt(sample.x * sample.x + sample.y * sample.y + sample.z * sample.z);
            REQUIRE(length <= 1.5f); // Allow some margin
        }
    }
}

TEST_CASE("generateSSAONoise creates valid noise", "[deferred][ssao][utility]") {
    std::vector<Vec3> noise;
    
    SECTION("Correct noise count") {
        generateSSAONoise(noise, 4);
        REQUIRE(noise.size() == 16); // 4x4
    }
    
    SECTION("Noise vectors are normalized in XY") {
        generateSSAONoise(noise, 4);
        
        for (const auto& n : noise) {
            REQUIRE(n.z == Approx(0.0f));
            f32 length = std::sqrt(n.x * n.x + n.y * n.y);
            REQUIRE(length == Approx(1.0f).margin(0.01f));
        }
    }
}

TEST_CASE("reconstructViewPosition basic test", "[deferred][utility]") {
    SECTION("Center screen at near plane") {
        Vec2 uv{0.5f, 0.5f};
        f32 depth = 0.0f;
        Mat4 invProj = Mat4::identity();
        
        Vec3 pos = reconstructViewPosition(uv, depth, invProj);
        // Result depends on projection matrix, but should be defined
        REQUIRE(std::isfinite(pos.x));
        REQUIRE(std::isfinite(pos.y));
        REQUIRE(std::isfinite(pos.z));
    }
}
