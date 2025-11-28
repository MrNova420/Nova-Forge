/**
 * @file test_shadow_system.cpp
 * @brief Comprehensive tests for the Shadow Mapping System
 * 
 * Tests all shadow functionality including:
 * - Shadow configuration constants
 * - Shadow filter techniques
 * - Cascade shadow maps
 * - Shadow bias settings
 * - PCF/PCSS/VSM settings
 * - Shadow manager
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "nova/core/render/shadow_system.hpp"
#include <cmath>

using namespace nova;
using namespace nova::render;
using Catch::Approx;

// ============================================================================
// Shadow Configuration Tests
// ============================================================================

TEST_CASE("ShadowSystemConfig constants are valid", "[render][shadow]") {
    SECTION("Cascade settings") {
        CHECK(ShadowSystemConfig::MAX_SHADOW_CASCADES >= 4);
        CHECK(ShadowSystemConfig::MAX_SHADOW_CASCADES <= 16);
    }
    
    SECTION("Resolution settings") {
        CHECK(ShadowSystemConfig::DEFAULT_SHADOW_RESOLUTION >= 1024);
        CHECK(ShadowSystemConfig::MIN_SHADOW_RESOLUTION >= 128);
        CHECK(ShadowSystemConfig::MAX_SHADOW_RESOLUTION <= 16384);
        CHECK(ShadowSystemConfig::MIN_SHADOW_RESOLUTION < ShadowSystemConfig::DEFAULT_SHADOW_RESOLUTION);
        CHECK(ShadowSystemConfig::DEFAULT_SHADOW_RESOLUTION < ShadowSystemConfig::MAX_SHADOW_RESOLUTION);
    }
    
    SECTION("Bias settings") {
        CHECK(ShadowSystemConfig::DEFAULT_SHADOW_BIAS > 0.0f);
        CHECK(ShadowSystemConfig::DEFAULT_SHADOW_BIAS < 0.1f);
        CHECK(ShadowSystemConfig::DEFAULT_NORMAL_BIAS > 0.0f);
    }
    
    SECTION("Virtual shadow map settings") {
        CHECK(ShadowSystemConfig::VIRTUAL_SHADOW_PAGE_SIZE >= 64);
        CHECK(ShadowSystemConfig::VIRTUAL_SHADOW_ATLAS_SIZE >= 8192);
    }
    
    SECTION("Split lambda default") {
        CHECK(ShadowSystemConfig::DEFAULT_CASCADE_SPLIT_LAMBDA >= 0.0f);
        CHECK(ShadowSystemConfig::DEFAULT_CASCADE_SPLIT_LAMBDA <= 1.0f);
    }
}

// ============================================================================
// Shadow Enumeration Tests
// ============================================================================

TEST_CASE("ShadowFilterTechnique enum values", "[render][shadow]") {
    CHECK(static_cast<u8>(ShadowFilterTechnique::None) == 0);
    CHECK(static_cast<u8>(ShadowFilterTechnique::PCF) == 1);
    CHECK(static_cast<u8>(ShadowFilterTechnique::PCSS) == 2);
    CHECK(static_cast<u8>(ShadowFilterTechnique::VSM) == 3);
    CHECK(static_cast<u8>(ShadowFilterTechnique::ESM) == 4);
    CHECK(static_cast<u8>(ShadowFilterTechnique::EVSM) == 5);
    CHECK(static_cast<u8>(ShadowFilterTechnique::MSM) == 6);
}

TEST_CASE("ShadowMapType enum values", "[render][shadow]") {
    CHECK(static_cast<u8>(ShadowMapType::Standard2D) == 0);
    CHECK(static_cast<u8>(ShadowMapType::CubeMap) == 1);
    CHECK(static_cast<u8>(ShadowMapType::CascadedArray) == 2);
    CHECK(static_cast<u8>(ShadowMapType::DualParaboloid) == 3);
    CHECK(static_cast<u8>(ShadowMapType::Virtual) == 4);
}

TEST_CASE("ShadowCasterType enum values", "[render][shadow]") {
    CHECK(static_cast<u8>(ShadowCasterType::Static) == 0);
    CHECK(static_cast<u8>(ShadowCasterType::Dynamic) == 1);
    CHECK(static_cast<u8>(ShadowCasterType::Skinned) == 2);
    CHECK(static_cast<u8>(ShadowCasterType::Particle) == 3);
}

TEST_CASE("ShadowQualityPreset enum values", "[render][shadow]") {
    CHECK(static_cast<u8>(ShadowQualityPreset::Off) == 0);
    CHECK(static_cast<u8>(ShadowQualityPreset::Low) == 1);
    CHECK(static_cast<u8>(ShadowQualityPreset::Medium) == 2);
    CHECK(static_cast<u8>(ShadowQualityPreset::High) == 3);
    CHECK(static_cast<u8>(ShadowQualityPreset::Ultra) == 4);
    CHECK(static_cast<u8>(ShadowQualityPreset::Cinematic) == 5);
}

// ============================================================================
// Shadow Map Descriptor Tests
// ============================================================================

TEST_CASE("ShadowMapDesc construction and factory methods", "[render][shadow]") {
    SECTION("Default values") {
        ShadowMapDesc desc;
        CHECK(desc.type == ShadowMapType::Standard2D);
        CHECK(desc.resolution == ShadowSystemConfig::DEFAULT_SHADOW_RESOLUTION);
        CHECK(desc.format == TextureFormat::D32_FLOAT);
        CHECK(desc.arrayLayers == 1);
        CHECK(desc.generateMoments == false);
        CHECK(desc.blur == false);
    }
    
    SECTION("Directional CSM factory") {
        auto desc = ShadowMapDesc::directionalCSM(4, 2048);
        CHECK(desc.type == ShadowMapType::CascadedArray);
        CHECK(desc.resolution == 2048);
        CHECK(desc.arrayLayers == 4);
        CHECK(desc.format == TextureFormat::D32_FLOAT);
    }
    
    SECTION("Point light factory") {
        auto desc = ShadowMapDesc::pointLight(1024);
        CHECK(desc.type == ShadowMapType::CubeMap);
        CHECK(desc.resolution == 1024);
        CHECK(desc.arrayLayers == 6);
    }
    
    SECTION("Spot light factory") {
        auto desc = ShadowMapDesc::spotLight(1024);
        CHECK(desc.type == ShadowMapType::Standard2D);
        CHECK(desc.resolution == 1024);
        CHECK(desc.arrayLayers == 1);
    }
    
    SECTION("VSM factory") {
        auto desc = ShadowMapDesc::varianceShadowMap(1024);
        CHECK(desc.type == ShadowMapType::Standard2D);
        CHECK(desc.format == TextureFormat::RG32_FLOAT);
        CHECK(desc.generateMoments == true);
        CHECK(desc.blur == true);
    }
    
    SECTION("Memory calculation") {
        ShadowMapDesc desc;
        desc.resolution = 1024;
        desc.format = TextureFormat::D32_FLOAT;  // 4 bytes per pixel
        
        // 2D texture: 1024 * 1024 * 4 = 4,194,304 bytes
        CHECK(desc.calculateMemoryUsage() == 1024 * 1024 * 4);
        
        // Cube map: 6 faces
        desc.type = ShadowMapType::CubeMap;
        CHECK(desc.calculateMemoryUsage() == 1024 * 1024 * 4 * 6);
    }
}

// ============================================================================
// Shadow Cascade Tests
// ============================================================================

TEST_CASE("ShadowCascade structure", "[render][shadow]") {
    ShadowCascadeData cascade;
    
    SECTION("Default values") {
        CHECK(cascade.splitNear == 0.0f);
        CHECK(cascade.splitFar == 100.0f);
        CHECK(cascade.texelSize == 0.0f);
        CHECK(cascade.resolution == 2048);
    }
    
    SECTION("Texel size calculation") {
        cascade.projMatrix = math::Mat4::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
        cascade.resolution = 1024;
        cascade.calculateTexelSize();
        
        // Width = 20, resolution = 1024, texelSize = 20/1024
        CHECK(cascade.texelSize == Approx(20.0f / 1024.0f));
    }
}

TEST_CASE("GPUCascadeData structure", "[render][shadow]") {
    CHECK(sizeof(GPUCascadeData) >= 80);
    CHECK(alignof(GPUCascadeData) >= 16);  // May be 64 due to Mat4's alignment
    
    ShadowCascadeData cascade;
    cascade.splitNear = 0.1f;
    cascade.splitFar = 50.0f;
    cascade.texelSize = 0.01f;
    
    GPUCascadeData gpuData(cascade);
    CHECK(gpuData.splitDepths.x == 0.1f);
    CHECK(gpuData.splitDepths.y == 50.0f);
    CHECK(gpuData.splitDepths.z == 0.01f);
}

// ============================================================================
// Shadow Bias Settings Tests
// ============================================================================

TEST_CASE("ShadowBiasSettings", "[render][shadow]") {
    SECTION("Default settings") {
        ShadowBiasSettings bias;
        CHECK(bias.constantBias == ShadowSystemConfig::DEFAULT_SHADOW_BIAS);
        CHECK(bias.slopeBias == 0.0f);
        CHECK(bias.normalBias == ShadowSystemConfig::DEFAULT_NORMAL_BIAS);
        CHECK(bias.autoAdjust == true);
    }
    
    SECTION("Factory methods") {
        auto def = ShadowBiasSettings::defaults();
        CHECK(def.autoAdjust == true);
        
        auto min = ShadowBiasSettings::minimal();
        CHECK(min.constantBias < def.constantBias);
        CHECK(min.normalBias < def.normalBias);
    }
    
    SECTION("Adjusted bias for cascades") {
        ShadowBiasSettings bias;
        bias.constantBias = 0.005f;
        bias.autoAdjust = true;
        
        f32 bias0 = bias.getAdjustedBias(0);
        f32 bias1 = bias.getAdjustedBias(1);
        f32 bias2 = bias.getAdjustedBias(2);
        
        // Farther cascades should have more bias
        CHECK(bias1 > bias0);
        CHECK(bias2 > bias1);
        
        // With auto-adjust off
        bias.autoAdjust = false;
        CHECK(bias.getAdjustedBias(3) == bias.constantBias);
    }
    
    SECTION("Normal offset bias") {
        ShadowBiasSettings bias;
        bias.normalBias = 0.01f;
        
        // Perpendicular surface (cos=1) should have minimal offset
        f32 perpOffset = bias.getNormalOffset(1.0f);
        CHECK(perpOffset == Approx(0.0f).margin(0.001f));
        
        // Grazing angle (cos=0) should have maximum offset
        f32 grazingOffset = bias.getNormalOffset(0.0f);
        CHECK(grazingOffset == Approx(bias.normalBias));
        
        // 45 degree angle
        f32 angle45Offset = bias.getNormalOffset(0.707f);
        CHECK(angle45Offset > 0.0f);
        CHECK(angle45Offset < bias.normalBias);
    }
}

// ============================================================================
// PCF Settings Tests
// ============================================================================

TEST_CASE("PCFSettings", "[render][shadow]") {
    SECTION("Default settings") {
        PCFSettings pcf;
        CHECK(pcf.sampleCount == 16);
        CHECK(pcf.filterRadius == 1.5f);
        CHECK(pcf.usePoisson == true);
        CHECK(pcf.rotatePerPixel == true);
    }
    
    SECTION("Kernel size string") {
        PCFSettings pcf;
        
        pcf.sampleCount = 4;
        CHECK(std::string(pcf.getKernelSizeString()) == "PCF_2X2");
        
        pcf.sampleCount = 9;
        CHECK(std::string(pcf.getKernelSizeString()) == "PCF_3X3");
        
        pcf.sampleCount = 25;
        CHECK(std::string(pcf.getKernelSizeString()) == "PCF_5X5");
        
        pcf.sampleCount = 49;
        CHECK(std::string(pcf.getKernelSizeString()) == "PCF_7X7");
    }
    
    SECTION("Factory methods") {
        auto def = PCFSettings::defaults();
        auto hq = PCFSettings::highQuality();
        
        CHECK(hq.sampleCount > def.sampleCount);
        CHECK(hq.filterRadius > def.filterRadius);
    }
}

// ============================================================================
// PCSS Settings Tests
// ============================================================================

TEST_CASE("PCSSSettings", "[render][shadow]") {
    SECTION("Default settings") {
        PCSSSettings pcss;
        CHECK(pcss.lightSize == 1.0f);
        CHECK(pcss.blockerSearchSamples == 16);
        CHECK(pcss.pcfSamples == 32);
        CHECK(pcss.minFilterRadius == 0.5f);
        CHECK(pcss.maxFilterRadius == 10.0f);
    }
    
    SECTION("Penumbra width calculation") {
        PCSSSettings pcss;
        pcss.lightSize = 2.0f;
        pcss.minFilterRadius = 0.5f;
        pcss.maxFilterRadius = 10.0f;
        
        // No blocker = minimum penumbra
        f32 noPenumbra = pcss.calculatePenumbraWidth(10.0f, 0.0f);
        CHECK(noPenumbra == pcss.minFilterRadius);
        
        // Blocker at half distance
        f32 halfPenumbra = pcss.calculatePenumbraWidth(10.0f, 5.0f);
        CHECK(halfPenumbra > pcss.minFilterRadius);
        
        // Penumbra should be clamped to max
        f32 largePenumbra = pcss.calculatePenumbraWidth(100.0f, 1.0f);
        CHECK(largePenumbra == pcss.maxFilterRadius);
    }
    
    SECTION("Factory methods") {
        auto def = PCSSSettings::defaults();
        auto area = PCSSSettings::areaLight(5.0f);
        
        CHECK(area.lightSize == 5.0f);
        CHECK(area.blockerSearchSamples > def.blockerSearchSamples);
        CHECK(area.pcfSamples > def.pcfSamples);
    }
}

// ============================================================================
// VSM Settings Tests
// ============================================================================

TEST_CASE("VSMSettings", "[render][shadow]") {
    SECTION("Default settings") {
        VSMSettings vsm;
        CHECK(vsm.minVariance > 0.0f);
        CHECK(vsm.lightBleedReduction >= 0.0f);
        CHECK(vsm.lightBleedReduction <= 1.0f);
        CHECK(vsm.blurPasses >= 1);
        CHECK(vsm.useExponential == false);
    }
    
    SECTION("Chebyshev upper bound") {
        VSMSettings vsm;
        vsm.minVariance = 0.0001f;
        vsm.lightBleedReduction = 0.2f;
        
        // In shadow (depth > mean)
        f32 inShadow = vsm.chebyshevUpperBound(0.8f, 0.5f, 0.3f);
        CHECK(inShadow >= 0.0f);
        CHECK(inShadow <= 1.0f);
        
        // Fully lit (depth <= mean)
        f32 lit = vsm.chebyshevUpperBound(0.4f, 0.5f, 0.3f);
        CHECK(lit == 1.0f);
        
        // At surface (depth == mean)
        f32 atSurface = vsm.chebyshevUpperBound(0.5f, 0.5f, 0.3f);
        CHECK(atSurface == 1.0f);
    }
    
    SECTION("Factory methods") {
        auto def = VSMSettings::defaults();
        auto evsm = VSMSettings::exponential();
        
        CHECK(def.useExponential == false);
        CHECK(evsm.useExponential == true);
        CHECK(evsm.exponentialConstant > 0.0f);
    }
}

// ============================================================================
// Shadow Light Data Tests
// ============================================================================

TEST_CASE("ShadowLightData", "[render][shadow]") {
    ShadowLightData data;
    
    SECTION("Default values") {
        CHECK(data.lightIndex == 0);
        CHECK(data.lightType == LightType::Directional);
        CHECK(data.filterTechnique == ShadowFilterTechnique::PCF);
        CHECK(data.maxShadowDistance == 100.0f);
        CHECK(data.fadeRange == 10.0f);
        CHECK(data.enabled == true);
        CHECK(data.cacheable == false);
        CHECK(data.dirty == true);
    }
    
    SECTION("Fade factor calculation") {
        data.maxShadowDistance = 100.0f;
        data.fadeRange = 10.0f;
        
        // Before fade starts
        CHECK(data.calculateFadeFactor(80.0f) == 1.0f);
        
        // At fade start
        CHECK(data.calculateFadeFactor(90.0f) == 1.0f);
        
        // Mid-fade
        CHECK(data.calculateFadeFactor(95.0f) == Approx(0.5f));
        
        // At max distance
        CHECK(data.calculateFadeFactor(100.0f) == 0.0f);
        
        // Beyond max distance
        CHECK(data.calculateFadeFactor(110.0f) == 0.0f);
    }
    
    SECTION("Cascade setup") {
        data.mapDesc = ShadowMapDesc::directionalCSM(4, 2048);
        data.setupCascades(4, 0.1f, 100.0f, 0.75f);
        
        CHECK(data.cascades.size() == 4);
        CHECK(data.cascades[0].splitNear == 0.1f);
        
        // Each cascade should extend farther
        for (usize i = 1; i < data.cascades.size(); ++i) {
            CHECK(data.cascades[i].splitNear == data.cascades[i - 1].splitFar);
            CHECK(data.cascades[i].splitFar > data.cascades[i - 1].splitFar);
        }
        
        // Last cascade should reach far plane
        CHECK(data.cascades.back().splitFar == Approx(100.0f));
    }
    
    SECTION("Get cascade index for depth") {
        data.setupCascades(4, 0.1f, 100.0f, 0.75f);
        
        // Very close = cascade 0
        CHECK(data.getCascadeIndex(0.5f) == 0);
        
        // Beyond all cascades = last cascade
        CHECK(data.getCascadeIndex(150.0f) == 3);
    }
}

// ============================================================================
// GPU Shadow Data Tests
// ============================================================================

TEST_CASE("GPUShadowData structure", "[render][shadow]") {
    CHECK(sizeof(GPUShadowData) >= 112);
    CHECK(alignof(GPUShadowData) >= 16);  // May be 64 due to Mat4's alignment
    
    ShadowLightData lightData;
    lightData.bias.constantBias = 0.005f;
    lightData.bias.normalBias = 0.01f;
    lightData.pcfSettings.filterRadius = 2.0f;
    lightData.pcssSettings.lightSize = 3.0f;
    lightData.maxShadowDistance = 150.0f;
    lightData.fadeRange = 20.0f;
    lightData.filterTechnique = ShadowFilterTechnique::PCSS;
    lightData.enabled = true;
    
    lightData.setupCascades(4, 0.1f, 100.0f);
    
    GPUShadowData gpuData(lightData, 5);
    
    CHECK(gpuData.shadowMapIndex == 5);
    CHECK(gpuData.filterTechnique == static_cast<u32>(ShadowFilterTechnique::PCSS));
    CHECK(gpuData.cascadeCount == 4);
    CHECK(gpuData.flags == 1);  // Enabled
    CHECK(gpuData.shadowParams.x == Approx(0.005f));
    CHECK(gpuData.shadowParams.y == Approx(0.01f));
    CHECK(gpuData.shadowParams.w == Approx(2.0f));
    CHECK(gpuData.shadowParams2.z == Approx(3.0f));
}

// ============================================================================
// Shadow Manager Tests
// ============================================================================

TEST_CASE("ShadowManager singleton and settings", "[render][shadow]") {
    auto& manager = ShadowManager::getInstance();
    
    SECTION("Quality preset") {
        manager.setQualityPreset(ShadowQualityPreset::Ultra);
        CHECK(manager.getQualityPreset() == ShadowQualityPreset::Ultra);
        manager.setQualityPreset(ShadowQualityPreset::High);  // Reset
    }
    
    SECTION("Shadow distance") {
        manager.setMaxShadowDistance(200.0f);
        CHECK(manager.getMaxShadowDistance() == 200.0f);
        manager.setMaxShadowDistance(100.0f);  // Reset
    }
    
    SECTION("Filter technique") {
        manager.setDefaultFilterTechnique(ShadowFilterTechnique::VSM);
        CHECK(manager.getDefaultFilterTechnique() == ShadowFilterTechnique::VSM);
        manager.setDefaultFilterTechnique(ShadowFilterTechnique::PCF);  // Reset
    }
    
    SECTION("Cascade split lambda") {
        manager.setCascadeSplitLambda(0.5f);
        CHECK(manager.getCascadeSplitLambda() == 0.5f);
        
        // Should clamp to valid range
        manager.setCascadeSplitLambda(-0.5f);
        CHECK(manager.getCascadeSplitLambda() == 0.0f);
        
        manager.setCascadeSplitLambda(1.5f);
        CHECK(manager.getCascadeSplitLambda() == 1.0f);
        
        manager.setCascadeSplitLambda(ShadowSystemConfig::DEFAULT_CASCADE_SPLIT_LAMBDA);  // Reset
    }
    
    SECTION("Enable/disable") {
        manager.setEnabled(false);
        CHECK(manager.isEnabled() == false);
        manager.setEnabled(true);
        CHECK(manager.isEnabled() == true);
    }
    
    SECTION("Statistics") {
        manager.resetStatistics();
        auto& stats = manager.getStatistics();
        CHECK(stats.totalShadowMaps == 0);
        CHECK(stats.trianglesRendered == 0);
        CHECK(stats.shadowPassTimeMs == 0.0f);
    }
}

// ============================================================================
// Shadow Utility Function Tests
// ============================================================================

TEST_CASE("Shadow utility functions", "[render][shadow]") {
    SECTION("Cascade split calculation") {
        f32 splits[4];
        calculateCascadeSplits(splits, 4, 0.1f, 100.0f, 0.75f);
        
        // Splits should be monotonically increasing
        CHECK(splits[0] > 0.1f);
        for (u32 i = 1; i < 4; ++i) {
            CHECK(splits[i] > splits[i - 1]);
        }
        CHECK(splits[3] == Approx(100.0f));
        
        // Pure logarithmic (lambda=1)
        calculateCascadeSplits(splits, 4, 0.1f, 100.0f, 1.0f);
        CHECK(splits[0] < splits[1]);
        
        // Pure linear (lambda=0)
        calculateCascadeSplits(splits, 4, 0.1f, 100.0f, 0.0f);
        f32 expectedLinear = 0.1f + (100.0f - 0.1f) * 0.25f;
        CHECK(splits[0] == Approx(expectedLinear));
    }
    
    SECTION("Shadow texel size calculation") {
        f32 texelSize = calculateShadowTexelSize(100.0f, 1024);
        CHECK(texelSize == Approx(100.0f / 1024.0f));
        
        // Higher resolution = smaller texels
        f32 hiResTexel = calculateShadowTexelSize(100.0f, 2048);
        CHECK(hiResTexel < texelSize);
    }
}
