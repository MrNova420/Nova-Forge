// =============================================================================
// NovaForge Platform - Lighting System Tests
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// Comprehensive tests for the advanced lighting system including:
// - Light types (directional, point, spot, area)
// - Attenuation calculations
// - Shadow configuration
// - Clustered lighting data structures
// - Environment probes and ambient lighting
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <nova/core/render/lighting.hpp>

using namespace nova;
using namespace nova::render;
using Catch::Approx;

// =============================================================================
// Lighting Configuration Tests
// =============================================================================

TEST_CASE("LightingConfig - Constants are valid", "[render][lighting]") {
    SECTION("Light count limits are reasonable") {
        REQUIRE(LightingConfig::MAX_DIRECTIONAL_LIGHTS == 4);
        REQUIRE(LightingConfig::MAX_POINT_LIGHTS == 4096);
        REQUIRE(LightingConfig::MAX_SPOT_LIGHTS == 2048);
        REQUIRE(LightingConfig::MAX_AREA_LIGHTS == 512);
        REQUIRE(LightingConfig::MAX_ENVIRONMENT_PROBES == 128);
    }
    
    SECTION("Cluster grid dimensions are valid") {
        REQUIRE(LightingConfig::CLUSTER_GRID_X == 16);
        REQUIRE(LightingConfig::CLUSTER_GRID_Y == 9);
        REQUIRE(LightingConfig::CLUSTER_GRID_Z == 24);
        REQUIRE(LightingConfig::TOTAL_CLUSTERS == 16 * 9 * 24);
        REQUIRE(LightingConfig::MAX_LIGHTS_PER_CLUSTER == 256);
    }
    
    SECTION("Shadow map resolutions are power of two") {
        REQUIRE(LightingConfig::DIRECTIONAL_SHADOW_RESOLUTION == 4096);
        REQUIRE(LightingConfig::POINT_SHADOW_RESOLUTION == 1024);
        REQUIRE(LightingConfig::SPOT_SHADOW_RESOLUTION == 2048);
        REQUIRE(LightingConfig::SHADOW_CASCADE_COUNT == 4);
    }
    
    SECTION("Other constants are reasonable") {
        REQUIRE(LightingConfig::PCF_KERNEL_SIZE == 5);
        REQUIRE(LightingConfig::MIN_LIGHT_INTENSITY == Approx(0.001f));
        REQUIRE(LightingConfig::DEFAULT_AMBIENT_INTENSITY == Approx(0.03f));
    }
}

// =============================================================================
// Light Type Tests
// =============================================================================

TEST_CASE("LightType - Enum values and names", "[render][lighting]") {
    SECTION("All light types are defined") {
        REQUIRE(static_cast<u8>(LightType::Directional) == 0);
        REQUIRE(static_cast<u8>(LightType::Point) == 1);
        REQUIRE(static_cast<u8>(LightType::Spot) == 2);
        REQUIRE(static_cast<u8>(LightType::AreaSphere) == 3);
        REQUIRE(static_cast<u8>(LightType::AreaRect) == 4);
        REQUIRE(static_cast<u8>(LightType::AreaDisc) == 5);
        REQUIRE(static_cast<u8>(LightType::AreaTube) == 6);
    }
    
    SECTION("Light type names are correct") {
        REQUIRE(std::string(getLightTypeName(LightType::Directional)) == "Directional");
        REQUIRE(std::string(getLightTypeName(LightType::Point)) == "Point");
        REQUIRE(std::string(getLightTypeName(LightType::Spot)) == "Spot");
        REQUIRE(std::string(getLightTypeName(LightType::AreaSphere)) == "AreaSphere");
        REQUIRE(std::string(getLightTypeName(LightType::AreaRect)) == "AreaRect");
        REQUIRE(std::string(getLightTypeName(LightType::AreaDisc)) == "AreaDisc");
        REQUIRE(std::string(getLightTypeName(LightType::AreaTube)) == "AreaTube");
    }
}

TEST_CASE("ShadowQuality - Enum values and names", "[render][lighting]") {
    SECTION("All shadow quality levels are defined") {
        REQUIRE(static_cast<u8>(ShadowQuality::Off) == 0);
        REQUIRE(static_cast<u8>(ShadowQuality::Low) == 1);
        REQUIRE(static_cast<u8>(ShadowQuality::Medium) == 2);
        REQUIRE(static_cast<u8>(ShadowQuality::High) == 3);
        REQUIRE(static_cast<u8>(ShadowQuality::Ultra) == 4);
    }
    
    SECTION("Shadow quality names are correct") {
        REQUIRE(std::string(getShadowQualityName(ShadowQuality::Off)) == "Off");
        REQUIRE(std::string(getShadowQualityName(ShadowQuality::Low)) == "Low");
        REQUIRE(std::string(getShadowQualityName(ShadowQuality::Medium)) == "Medium");
        REQUIRE(std::string(getShadowQualityName(ShadowQuality::High)) == "High");
        REQUIRE(std::string(getShadowQualityName(ShadowQuality::Ultra)) == "Ultra");
    }
}

TEST_CASE("AttenuationModel - Enum values", "[render][lighting]") {
    SECTION("All attenuation models are defined") {
        REQUIRE(static_cast<u8>(AttenuationModel::None) == 0);
        REQUIRE(static_cast<u8>(AttenuationModel::Linear) == 1);
        REQUIRE(static_cast<u8>(AttenuationModel::Quadratic) == 2);
        REQUIRE(static_cast<u8>(AttenuationModel::InverseSquare) == 3);
        REQUIRE(static_cast<u8>(AttenuationModel::Exponential) == 4);
        REQUIRE(static_cast<u8>(AttenuationModel::Custom) == 5);
    }
}

// =============================================================================
// Light Base Tests
// =============================================================================

TEST_CASE("LightBase - Default values", "[render][lighting]") {
    LightBase light;
    
    SECTION("Color defaults to white") {
        REQUIRE(light.color.x == Approx(1.0f));
        REQUIRE(light.color.y == Approx(1.0f));
        REQUIRE(light.color.z == Approx(1.0f));
    }
    
    SECTION("Intensity defaults to 1000 lumens") {
        REQUIRE(light.intensity == Approx(1000.0f));
    }
    
    SECTION("Light is enabled by default") {
        REQUIRE(light.enabled == true);
    }
    
    SECTION("Shadow casting enabled by default") {
        REQUIRE(light.castShadows == true);
    }
    
    SECTION("Shadow bias has reasonable defaults") {
        REQUIRE(light.shadowBias == Approx(0.0005f));
        REQUIRE(light.shadowNormalBias == Approx(0.02f));
        REQUIRE(light.shadowSoftness == Approx(1.0f));
    }
    
    SECTION("Light affects both diffuse and specular") {
        REQUIRE(light.affectsSpecular == true);
        REQUIRE(light.affectsDiffuse == true);
    }
    
    SECTION("Layer mask defaults to all layers") {
        REQUIRE(light.layerMask == 0xFFFFFFFF);
    }
}

// =============================================================================
// Directional Light Tests
// =============================================================================

TEST_CASE("DirectionalLight - Default values", "[render][lighting]") {
    DirectionalLight light;
    
    SECTION("Direction defaults to downward") {
        REQUIRE(light.direction.x == Approx(0.0f));
        REQUIRE(light.direction.y == Approx(-1.0f));
        REQUIRE(light.direction.z == Approx(0.0f));
    }
    
    SECTION("Angular diameter is sun-like") {
        REQUIRE(light.angularDiameter == Approx(0.0093f));
    }
    
    SECTION("Cascade splits are reasonable") {
        REQUIRE(light.cascadeSplits[0] == Approx(0.05f));
        REQUIRE(light.cascadeSplits[1] == Approx(0.15f));
        REQUIRE(light.cascadeSplits[2] == Approx(0.4f));
        REQUIRE(light.cascadeSplits[3] == Approx(1.0f));
    }
    
    SECTION("Shadow distance is reasonable") {
        REQUIRE(light.shadowDistance == Approx(200.0f));
    }
}

TEST_CASE("DirectionalLight - Temperature to RGB conversion", "[render][lighting]") {
    SECTION("Warm temperatures produce reddish colors") {
        Vec3 warm = DirectionalLight::temperatureToRGB(2700.0f); // Incandescent
        REQUIRE(warm.x > warm.y);  // More red than green
        REQUIRE(warm.x > warm.z);  // More red than blue
    }
    
    SECTION("Daylight temperature is balanced") {
        Vec3 daylight = DirectionalLight::temperatureToRGB(6500.0f); // Daylight
        // Daylight should be roughly balanced
        REQUIRE(daylight.x > 0.8f);
        REQUIRE(daylight.y > 0.8f);
        REQUIRE(daylight.z > 0.8f);
    }
    
    SECTION("Cool temperatures produce bluish colors") {
        Vec3 cool = DirectionalLight::temperatureToRGB(10000.0f); // Cool
        REQUIRE(cool.z >= cool.x); // More blue than red
    }
    
    SECTION("Temperature is clamped to valid range") {
        Vec3 tooLow = DirectionalLight::temperatureToRGB(500.0f);
        Vec3 tooHigh = DirectionalLight::temperatureToRGB(50000.0f);
        
        // Values should be valid (0-1)
        REQUIRE(tooLow.x >= 0.0f);
        REQUIRE(tooLow.x <= 1.0f);
        REQUIRE(tooHigh.x >= 0.0f);
        REQUIRE(tooHigh.x <= 1.0f);
    }
}

TEST_CASE("DirectionalLight - Effective color", "[render][lighting]") {
    DirectionalLight light;
    
    SECTION("With temperature = 0, uses direct color") {
        light.temperature = 0.0f;
        light.color = Vec3{1.0f, 0.5f, 0.25f};
        
        Vec3 effective = light.getEffectiveColor();
        REQUIRE(effective.x == Approx(1.0f));
        REQUIRE(effective.y == Approx(0.5f));
        REQUIRE(effective.z == Approx(0.25f));
    }
    
    SECTION("With temperature > 0, modulates by temperature") {
        light.temperature = 6500.0f;
        light.color = Vec3{1.0f, 1.0f, 1.0f};
        
        Vec3 effective = light.getEffectiveColor();
        // Should be similar to daylight
        REQUIRE(effective.x > 0.0f);
        REQUIRE(effective.y > 0.0f);
        REQUIRE(effective.z > 0.0f);
    }
}

// =============================================================================
// Point Light Tests
// =============================================================================

TEST_CASE("PointLight - Default values", "[render][lighting]") {
    PointLight light;
    
    SECTION("Position defaults to origin") {
        REQUIRE(light.position.x == Approx(0.0f));
        REQUIRE(light.position.y == Approx(0.0f));
        REQUIRE(light.position.z == Approx(0.0f));
    }
    
    SECTION("Radius and range have defaults") {
        REQUIRE(light.radius == Approx(0.1f));
        REQUIRE(light.range == Approx(10.0f));
    }
    
    SECTION("Attenuation defaults to quadratic") {
        REQUIRE(light.attenuation == AttenuationModel::Quadratic);
    }
    
    SECTION("No IES profile by default") {
        REQUIRE(light.iesProfileIndex == -1);
    }
}

TEST_CASE("PointLight - Attenuation calculation", "[render][lighting]") {
    PointLight light;
    light.range = 10.0f;
    
    SECTION("Attenuation is 0 beyond range") {
        f32 atten = light.calculateAttenuation(15.0f);
        REQUIRE(atten == Approx(0.0f));
    }
    
    SECTION("Attenuation at range boundary is 0") {
        f32 atten = light.calculateAttenuation(10.0f);
        REQUIRE(atten == Approx(0.0f));
    }
    
    SECTION("Attenuation is higher closer to light") {
        f32 attenNear = light.calculateAttenuation(1.0f);
        f32 attenFar = light.calculateAttenuation(5.0f);
        REQUIRE(attenNear > attenFar);
    }
    
    SECTION("No attenuation model returns 1") {
        light.attenuation = AttenuationModel::None;
        f32 atten = light.calculateAttenuation(5.0f);
        REQUIRE(atten == Approx(1.0f));
    }
    
    SECTION("Linear attenuation works") {
        light.attenuation = AttenuationModel::Linear;
        f32 atten1 = light.calculateAttenuation(1.0f);
        f32 atten2 = light.calculateAttenuation(2.0f);
        // Linear: 1/d, so 2x distance should be roughly 0.5x attenuation
        REQUIRE(atten1 > atten2);
    }
    
    SECTION("Quadratic attenuation works") {
        light.attenuation = AttenuationModel::Quadratic;
        f32 atten1 = light.calculateAttenuation(1.0f);
        f32 atten2 = light.calculateAttenuation(2.0f);
        // Quadratic: 1/d², so 2x distance should be roughly 0.25x attenuation
        REQUIRE(atten1 > atten2);
    }
}

// =============================================================================
// Spot Light Tests
// =============================================================================

TEST_CASE("SpotLight - Default values", "[render][lighting]") {
    SpotLight light;
    
    SECTION("Position defaults to origin") {
        REQUIRE(light.position.x == Approx(0.0f));
        REQUIRE(light.position.y == Approx(0.0f));
        REQUIRE(light.position.z == Approx(0.0f));
    }
    
    SECTION("Direction defaults to downward") {
        REQUIRE(light.direction.x == Approx(0.0f));
        REQUIRE(light.direction.y == Approx(-1.0f));
        REQUIRE(light.direction.z == Approx(0.0f));
    }
    
    SECTION("Cone angles are reasonable") {
        // Inner should be less than outer
        REQUIRE(light.innerConeAngle < light.outerConeAngle);
        // Both should be positive
        REQUIRE(light.innerConeAngle > 0.0f);
        REQUIRE(light.outerConeAngle > 0.0f);
    }
}

TEST_CASE("SpotLight - Cone attenuation", "[render][lighting]") {
    SpotLight light;
    light.direction = Vec3{0.0f, -1.0f, 0.0f}; // Pointing down
    light.innerConeAngle = nova::math::radians(20.0f);
    light.outerConeAngle = nova::math::radians(40.0f);
    
    SECTION("Full intensity along light direction") {
        // Light points down, so lightToFrag pointing down = in light cone
        Vec3 lightToFrag = Vec3{0.0f, -1.0f, 0.0f}; // From light toward fragment (same as light dir)
        f32 atten = light.calculateConeAttenuation(lightToFrag);
        REQUIRE(atten == Approx(1.0f).margin(0.01f));
    }
    
    SECTION("Zero intensity perpendicular to light") {
        Vec3 lightToFrag = Vec3{1.0f, 0.0f, 0.0f}; // Perpendicular
        f32 atten = light.calculateConeAttenuation(lightToFrag);
        REQUIRE(atten == Approx(0.0f).margin(0.01f));
    }
    
    SECTION("Attenuation decreases toward cone edge") {
        // Direction slightly off-axis (mostly down, slight X offset)
        // Inner cone = 20°, outer cone = 40°
        // Use angles that are clearly in the falloff region
        f32 angle1 = nova::math::radians(25.0f); // Between inner (20) and outer (40)
        f32 angle2 = nova::math::radians(35.0f); // More toward outer edge
        
        Vec3 lightToFrag1 = Vec3{std::sin(angle1), -std::cos(angle1), 0.0f};
        Vec3 lightToFrag2 = Vec3{std::sin(angle2), -std::cos(angle2), 0.0f};
        
        f32 atten1 = light.calculateConeAttenuation(lightToFrag1);
        f32 atten2 = light.calculateConeAttenuation(lightToFrag2);
        
        REQUIRE(atten1 > atten2); // Closer to center = higher attenuation
    }
}

TEST_CASE("SpotLight - Total attenuation", "[render][lighting]") {
    SpotLight light;
    light.position = Vec3{0.0f, 10.0f, 0.0f};
    light.direction = Vec3{0.0f, -1.0f, 0.0f};
    light.range = 20.0f;
    light.innerConeAngle = nova::math::radians(20.0f);
    light.outerConeAngle = nova::math::radians(40.0f);
    
    SECTION("Attenuation considers both distance and cone") {
        Vec3 fragPos = Vec3{0.0f, 0.0f, 0.0f}; // Directly below
        f32 atten = light.calculateTotalAttenuation(fragPos);
        REQUIRE(atten > 0.0f);
    }
    
    SECTION("Zero attenuation beyond range") {
        Vec3 fragPos = Vec3{0.0f, -15.0f, 0.0f}; // 25 units away (beyond range)
        f32 atten = light.calculateTotalAttenuation(fragPos);
        REQUIRE(atten == Approx(0.0f));
    }
}

// =============================================================================
// Area Light Tests
// =============================================================================

TEST_CASE("AreaLight - Default values", "[render][lighting]") {
    AreaLight light;
    
    SECTION("Defaults to rectangular area light") {
        REQUIRE(light.type == LightType::AreaRect);
    }
    
    SECTION("Not two-sided by default") {
        REQUIRE(light.twoSided == false);
    }
}

TEST_CASE("AreaLight - Area calculation", "[render][lighting]") {
    AreaLight light;
    
    SECTION("Rectangular area") {
        light.type = LightType::AreaRect;
        light.dimensions = Vec3{2.0f, 3.0f, 0.0f};
        REQUIRE(light.getArea() == Approx(6.0f));
    }
    
    SECTION("Spherical area") {
        light.type = LightType::AreaSphere;
        light.dimensions = Vec3{1.0f, 0.0f, 0.0f}; // radius = 1
        REQUIRE(light.getArea() == Approx(4.0f * nova::math::PI_F32));
    }
    
    SECTION("Disc area") {
        light.type = LightType::AreaDisc;
        light.dimensions = Vec3{2.0f, 0.0f, 0.0f}; // radius = 2
        REQUIRE(light.getArea() == Approx(nova::math::PI_F32 * 4.0f));
    }
    
    SECTION("Tube area") {
        light.type = LightType::AreaTube;
        light.dimensions = Vec3{5.0f, 0.5f, 0.0f}; // length = 5, radius = 0.5
        REQUIRE(light.getArea() == Approx(2.0f * nova::math::PI_F32 * 0.5f * 5.0f));
    }
}

TEST_CASE("AreaLight - Luminous flux", "[render][lighting]") {
    AreaLight light;
    light.type = LightType::AreaRect;
    light.dimensions = Vec3{1.0f, 1.0f, 0.0f}; // 1m² area
    light.intensity = 1000.0f; // 1000 cd/m²
    
    SECTION("Flux is intensity * area * pi for Lambertian") {
        f32 flux = light.getLuminousFlux();
        REQUIRE(flux == Approx(1000.0f * 1.0f * nova::math::PI_F32));
    }
}

// =============================================================================
// Environment Probe Tests
// =============================================================================

TEST_CASE("EnvironmentProbe - Default values", "[render][lighting]") {
    EnvironmentProbe probe;
    
    SECTION("Position defaults to origin") {
        REQUIRE(probe.position.x == Approx(0.0f));
        REQUIRE(probe.position.y == Approx(0.0f));
        REQUIRE(probe.position.z == Approx(0.0f));
    }
    
    SECTION("Radius and blend distance are reasonable") {
        REQUIRE(probe.radius == Approx(10.0f));
        REQUIRE(probe.blendDistance == Approx(2.0f));
    }
    
    SECTION("Enabled by default") {
        REQUIRE(probe.enabled == true);
    }
    
    SECTION("Box projection disabled by default") {
        REQUIRE(probe.useBoxProjection == false);
    }
}

TEST_CASE("EnvironmentProbe - Influence calculation", "[render][lighting]") {
    EnvironmentProbe probe;
    probe.position = Vec3{0.0f, 0.0f, 0.0f};
    probe.radius = 10.0f;
    probe.blendDistance = 2.0f;
    
    SECTION("Full influence at center") {
        Vec3 pos = Vec3{0.0f, 0.0f, 0.0f};
        f32 influence = probe.calculateInfluence(pos);
        REQUIRE(influence == Approx(1.0f));
    }
    
    SECTION("Full influence within blend region start") {
        Vec3 pos = Vec3{7.0f, 0.0f, 0.0f}; // 7 units, within radius - blendDist
        f32 influence = probe.calculateInfluence(pos);
        REQUIRE(influence == Approx(1.0f));
    }
    
    SECTION("Partial influence in blend region") {
        Vec3 pos = Vec3{9.0f, 0.0f, 0.0f}; // 9 units, in blend region
        f32 influence = probe.calculateInfluence(pos);
        REQUIRE(influence > 0.0f);
        REQUIRE(influence < 1.0f);
    }
    
    SECTION("Zero influence beyond radius") {
        Vec3 pos = Vec3{15.0f, 0.0f, 0.0f};
        f32 influence = probe.calculateInfluence(pos);
        REQUIRE(influence == Approx(0.0f));
    }
}

// =============================================================================
// Ambient Light Tests
// =============================================================================

TEST_CASE("AmbientLight - Default values", "[render][lighting]") {
    AmbientLight ambient;
    
    SECTION("Sky color is bluish") {
        REQUIRE(ambient.skyColor.x == Approx(0.5f));
        REQUIRE(ambient.skyColor.y == Approx(0.7f));
        REQUIRE(ambient.skyColor.z == Approx(1.0f));
    }
    
    SECTION("Ground color is brownish") {
        REQUIRE(ambient.groundColor.x == Approx(0.1f));
        REQUIRE(ambient.groundColor.y == Approx(0.08f));
        REQUIRE(ambient.groundColor.z == Approx(0.05f));
    }
    
    SECTION("Intensity matches config") {
        REQUIRE(ambient.intensity == Approx(LightingConfig::DEFAULT_AMBIENT_INTENSITY));
    }
    
    SECTION("Environment map disabled by default") {
        REQUIRE(ambient.useEnvironmentMap == false);
    }
}

TEST_CASE("AmbientLight - Sample ambient color", "[render][lighting]") {
    AmbientLight ambient;
    ambient.skyColor = Vec3{0.5f, 0.7f, 1.0f};
    ambient.groundColor = Vec3{0.1f, 0.08f, 0.05f};
    ambient.equatorColor = Vec3{0.3f, 0.3f, 0.35f};
    ambient.intensity = 1.0f;
    
    SECTION("Upward normal samples sky") {
        Vec3 normal = Vec3{0.0f, 1.0f, 0.0f};
        Vec3 color = ambient.sampleAmbient(normal);
        REQUIRE(color.x == Approx(ambient.skyColor.x));
        REQUIRE(color.y == Approx(ambient.skyColor.y));
        REQUIRE(color.z == Approx(ambient.skyColor.z));
    }
    
    SECTION("Downward normal samples ground") {
        Vec3 normal = Vec3{0.0f, -1.0f, 0.0f};
        Vec3 color = ambient.sampleAmbient(normal);
        REQUIRE(color.x == Approx(ambient.groundColor.x));
        REQUIRE(color.y == Approx(ambient.groundColor.y));
        REQUIRE(color.z == Approx(ambient.groundColor.z));
    }
    
    SECTION("Horizontal normal blends with equator") {
        Vec3 normal = Vec3{1.0f, 0.0f, 0.0f};
        Vec3 color = ambient.sampleAmbient(normal);
        // Should be equator color when y=0
        REQUIRE(color.x == Approx(ambient.equatorColor.x));
        REQUIRE(color.y == Approx(ambient.equatorColor.y));
        REQUIRE(color.z == Approx(ambient.equatorColor.z));
    }
}

// =============================================================================
// Light Cluster Tests
// =============================================================================

TEST_CASE("LightCluster - Default values", "[render][lighting]") {
    LightCluster cluster;
    
    SECTION("All counts start at zero") {
        REQUIRE(cluster.pointLightCount == 0);
        REQUIRE(cluster.spotLightCount == 0);
        REQUIRE(cluster.areaLightCount == 0);
        REQUIRE(cluster.probeCount == 0);
    }
    
    SECTION("Light index offset starts at zero") {
        REQUIRE(cluster.lightIndexOffset == 0);
    }
}

// =============================================================================
// GPU Light Data Tests
// =============================================================================

TEST_CASE("GPULightData - From point light", "[render][lighting]") {
    PointLight light;
    light.position = Vec3{1.0f, 2.0f, 3.0f};
    light.color = Vec3{1.0f, 0.8f, 0.6f};
    light.intensity = 500.0f;
    light.range = 15.0f;
    light.radius = 0.2f;
    
    GPULightData gpuData = GPULightData::fromPointLight(light, 5);
    
    SECTION("Position is copied") {
        REQUIRE(gpuData.position.x == Approx(1.0f));
        REQUIRE(gpuData.position.y == Approx(2.0f));
        REQUIRE(gpuData.position.z == Approx(3.0f));
    }
    
    SECTION("Type is Point") {
        REQUIRE(gpuData.type == Approx(static_cast<f32>(LightType::Point)));
    }
    
    SECTION("Color and intensity are copied") {
        REQUIRE(gpuData.color.x == Approx(1.0f));
        REQUIRE(gpuData.color.y == Approx(0.8f));
        REQUIRE(gpuData.color.z == Approx(0.6f));
        REQUIRE(gpuData.intensity == Approx(500.0f));
    }
    
    SECTION("Range and radius are copied") {
        REQUIRE(gpuData.range == Approx(15.0f));
        REQUIRE(gpuData.radius == Approx(0.2f));
    }
    
    SECTION("Shadow index is set") {
        REQUIRE(gpuData.shadowIndex == Approx(5.0f));
    }
}

TEST_CASE("GPULightData - From spot light", "[render][lighting]") {
    SpotLight light;
    light.position = Vec3{5.0f, 10.0f, 0.0f};
    light.direction = Vec3{0.0f, -1.0f, 0.0f};
    light.color = Vec3{1.0f, 1.0f, 0.9f};
    light.intensity = 800.0f;
    light.range = 25.0f;
    light.innerConeAngle = nova::math::radians(25.0f);
    light.outerConeAngle = nova::math::radians(45.0f);
    
    GPULightData gpuData = GPULightData::fromSpotLight(light, -1);
    
    SECTION("Type is Spot") {
        REQUIRE(gpuData.type == Approx(static_cast<f32>(LightType::Spot)));
    }
    
    SECTION("Direction is copied") {
        REQUIRE(gpuData.direction.x == Approx(0.0f));
        REQUIRE(gpuData.direction.y == Approx(-1.0f));
        REQUIRE(gpuData.direction.z == Approx(0.0f));
    }
    
    SECTION("Cone angles are copied") {
        REQUIRE(gpuData.innerConeAngle == Approx(nova::math::radians(25.0f)));
        REQUIRE(gpuData.outerConeAngle == Approx(nova::math::radians(45.0f)));
    }
    
    SECTION("No shadow when index is -1") {
        REQUIRE(gpuData.shadowIndex == Approx(-1.0f));
    }
}

// =============================================================================
// GPU Directional Light Tests
// =============================================================================

TEST_CASE("GPUDirectionalLight - From directional light", "[render][lighting]") {
    DirectionalLight light;
    light.direction = Vec3{0.5f, -0.8f, 0.2f}.normalized();
    light.color = Vec3{1.0f, 0.95f, 0.9f};
    light.intensity = 120000.0f; // Bright sunlight in lux
    light.shadowDistance = 150.0f;
    
    GPUDirectionalLight gpuData = GPUDirectionalLight::fromDirectionalLight(light);
    
    SECTION("Direction is copied") {
        REQUIRE(gpuData.direction.x == Approx(light.direction.x));
        REQUIRE(gpuData.direction.y == Approx(light.direction.y));
        REQUIRE(gpuData.direction.z == Approx(light.direction.z));
    }
    
    SECTION("Intensity is copied") {
        REQUIRE(gpuData.intensity == Approx(120000.0f));
    }
    
    SECTION("Shadow distance is copied") {
        REQUIRE(gpuData.shadowDistance == Approx(150.0f));
    }
    
    SECTION("Cascade splits are copied") {
        for (u32 i = 0; i < LightingConfig::SHADOW_CASCADE_COUNT; ++i) {
            REQUIRE(gpuData.cascadeSplits[i] == Approx(light.cascadeSplits[i]));
        }
    }
}

// =============================================================================
// Shadow Configuration Tests
// =============================================================================

TEST_CASE("ShadowConfig - Default values", "[render][lighting]") {
    ShadowConfig config;
    
    SECTION("Quality defaults to High") {
        REQUIRE(config.quality == ShadowQuality::High);
    }
    
    SECTION("Resolutions match config") {
        REQUIRE(config.directionalResolution == LightingConfig::DIRECTIONAL_SHADOW_RESOLUTION);
        REQUIRE(config.pointResolution == LightingConfig::POINT_SHADOW_RESOLUTION);
        REQUIRE(config.spotResolution == LightingConfig::SPOT_SHADOW_RESOLUTION);
    }
    
    SECTION("Max distance is reasonable") {
        REQUIRE(config.maxDistance == Approx(200.0f));
    }
    
    SECTION("Contact shadows enabled by default") {
        REQUIRE(config.enableContactShadows == true);
    }
    
    SECTION("PCF filter size matches config") {
        REQUIRE(config.pcfFilterSize == LightingConfig::PCF_KERNEL_SIZE);
    }
}

// =============================================================================
// Shadow Cascade Tests
// =============================================================================

TEST_CASE("ShadowCascade - Default values", "[render][lighting]") {
    ShadowCascade cascade;
    
    SECTION("Near and far planes have defaults") {
        REQUIRE(cascade.nearPlane == Approx(0.0f));
        REQUIRE(cascade.farPlane == Approx(1.0f));
    }
    
    SECTION("Radius has default") {
        REQUIRE(cascade.radius == Approx(10.0f));
    }
    
    SECTION("Texel size has default") {
        REQUIRE(cascade.texelSize == Approx(1.0f));
    }
}

// =============================================================================
// Lighting Stats Tests
// =============================================================================

TEST_CASE("LightingStats - Default values", "[render][lighting]") {
    LightingStats stats;
    
    SECTION("All counts start at zero") {
        REQUIRE(stats.activeDirectionalLights == 0);
        REQUIRE(stats.activePointLights == 0);
        REQUIRE(stats.activeSpotLights == 0);
        REQUIRE(stats.activeAreaLights == 0);
        REQUIRE(stats.activeProbes == 0);
        REQUIRE(stats.shadowCastingLights == 0);
        REQUIRE(stats.visibleLights == 0);
    }
    
    SECTION("Timing values start at zero") {
        REQUIRE(stats.cullingTimeMs == Approx(0.0f));
        REQUIRE(stats.clusterTimeMs == Approx(0.0f));
        REQUIRE(stats.shadowTimeMs == Approx(0.0f));
    }
    
    SECTION("Cluster statistics start at zero") {
        REQUIRE(stats.avgLightsPerCluster == Approx(0.0f));
        REQUIRE(stats.maxLightsInCluster == 0);
    }
}
