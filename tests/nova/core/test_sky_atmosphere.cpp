// =============================================================================
// NovaForge Platform - Sky and Atmosphere System Tests
// =============================================================================
// Platform: NovaForge | Engine: NovaCore | Company: WeNova Interactive
// (operating as Kayden Shawn Massengill)
//
// Comprehensive tests for the sky and atmosphere system including:
// - Atmospheric scattering (Rayleigh, Mie, Ozone)
// - Celestial body rendering
// - Cloud layers and volumetric clouds
// - Time of day management
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <nova/core/render/sky_atmosphere.hpp>

using namespace nova;
using namespace nova::render;
using Catch::Approx;

// =============================================================================
// Sky Configuration Tests
// =============================================================================

TEST_CASE("SkyConfig - Constants are valid", "[render][sky]") {
    SECTION("Planet and atmosphere defaults") {
        REQUIRE(SkyConfig::DEFAULT_PLANET_RADIUS > 0.0f);
        REQUIRE(SkyConfig::DEFAULT_ATMOSPHERE_HEIGHT > 0.0f);
        REQUIRE(SkyConfig::DEFAULT_RAYLEIGH_SCALE_HEIGHT > 0.0f);
        REQUIRE(SkyConfig::DEFAULT_MIE_SCALE_HEIGHT > 0.0f);
    }
    
    SECTION("Ozone layer defaults") {
        REQUIRE(SkyConfig::DEFAULT_OZONE_CENTER_HEIGHT > 0.0f);
        REQUIRE(SkyConfig::DEFAULT_OZONE_WIDTH > 0.0f);
    }
    
    SECTION("LUT texture sizes are valid") {
        REQUIRE(SkyConfig::TRANSMITTANCE_LUT_WIDTH > 0);
        REQUIRE(SkyConfig::TRANSMITTANCE_LUT_HEIGHT > 0);
        REQUIRE(SkyConfig::MULTISCATTER_LUT_SIZE > 0);
        REQUIRE(SkyConfig::SKYVIEW_LUT_WIDTH > 0);
        REQUIRE(SkyConfig::SKYVIEW_LUT_HEIGHT > 0);
    }
    
    SECTION("Limits are reasonable") {
        REQUIRE(SkyConfig::MAX_CELESTIAL_BODIES > 0);
        REQUIRE(SkyConfig::MAX_CLOUD_NOISE_OCTAVES > 0);
        REQUIRE(SkyConfig::MAX_CLOUD_LAYERS > 0);
    }
}

// =============================================================================
// Sky Technique Tests
// =============================================================================

TEST_CASE("SkyTechnique - Enum values", "[render][sky]") {
    SECTION("All techniques are defined") {
        REQUIRE(static_cast<int>(SkyTechnique::None) == 0);
        REQUIRE(static_cast<int>(SkyTechnique::SolidColor) == 1);
        REQUIRE(static_cast<int>(SkyTechnique::Gradient) == 2);
        REQUIRE(static_cast<int>(SkyTechnique::Skybox) == 3);
        REQUIRE(static_cast<int>(SkyTechnique::HDRI) == 4);
        REQUIRE(static_cast<int>(SkyTechnique::Procedural) == 5);
        REQUIRE(static_cast<int>(SkyTechnique::PhysicalAtmosphere) == 6);
        REQUIRE(static_cast<int>(SkyTechnique::Bruneton) == 7);
    }
}

TEST_CASE("ProceduralSkyModel - Enum values", "[render][sky]") {
    REQUIRE(static_cast<int>(ProceduralSkyModel::Preetham) == 0);
    REQUIRE(static_cast<int>(ProceduralSkyModel::HosekWilkie) == 1);
    REQUIRE(static_cast<int>(ProceduralSkyModel::Hillaire) == 2);
    REQUIRE(static_cast<int>(ProceduralSkyModel::Custom) == 3);
}

TEST_CASE("CloudTechnique - Enum values", "[render][sky]") {
    REQUIRE(static_cast<int>(CloudTechnique::None) == 0);
    REQUIRE(static_cast<int>(CloudTechnique::Texture2D) == 1);
    REQUIRE(static_cast<int>(CloudTechnique::Billboards) == 2);
    REQUIRE(static_cast<int>(CloudTechnique::Volumetric) == 3);
    REQUIRE(static_cast<int>(CloudTechnique::VolumetricLit) == 4);
}

// =============================================================================
// Moon Phase Tests
// =============================================================================

TEST_CASE("MoonPhase - Enum values and names", "[render][sky]") {
    SECTION("All phases are defined") {
        REQUIRE(static_cast<int>(MoonPhase::NewMoon) == 0);
        REQUIRE(static_cast<int>(MoonPhase::WaxingCrescent) == 1);
        REQUIRE(static_cast<int>(MoonPhase::FirstQuarter) == 2);
        REQUIRE(static_cast<int>(MoonPhase::WaxingGibbous) == 3);
        REQUIRE(static_cast<int>(MoonPhase::FullMoon) == 4);
        REQUIRE(static_cast<int>(MoonPhase::WaningGibbous) == 5);
        REQUIRE(static_cast<int>(MoonPhase::LastQuarter) == 6);
        REQUIRE(static_cast<int>(MoonPhase::WaningCrescent) == 7);
    }
    
    SECTION("Phase names are correct") {
        REQUIRE(std::string(getMoonPhaseName(MoonPhase::NewMoon)) == "New Moon");
        REQUIRE(std::string(getMoonPhaseName(MoonPhase::FullMoon)) == "Full Moon");
        REQUIRE(std::string(getMoonPhaseName(MoonPhase::FirstQuarter)) == "First Quarter");
        REQUIRE(std::string(getMoonPhaseName(MoonPhase::LastQuarter)) == "Last Quarter");
    }
}

// =============================================================================
// Rayleigh Scattering Tests
// =============================================================================

TEST_CASE("RayleighScattering - Calculations", "[render][sky]") {
    RayleighScattering rayleigh;
    
    SECTION("Default values are Earth-like") {
        REQUIRE(rayleigh.scaleHeight == Approx(SkyConfig::DEFAULT_RAYLEIGH_SCALE_HEIGHT));
        REQUIRE(rayleigh.densityMultiplier == Approx(1.0f));
        REQUIRE(rayleigh.scatteringCoefficients.x > 0.0f);
        REQUIRE(rayleigh.scatteringCoefficients.y > 0.0f);
        REQUIRE(rayleigh.scatteringCoefficients.z > 0.0f);
    }
    
    SECTION("getDensity decreases with height") {
        f32 densityAtGround = rayleigh.getDensity(0.0f);
        f32 densityAt1km = rayleigh.getDensity(1000.0f);
        f32 densityAt10km = rayleigh.getDensity(10000.0f);
        
        REQUIRE(densityAtGround > densityAt1km);
        REQUIRE(densityAt1km > densityAt10km);
        REQUIRE(densityAtGround == Approx(1.0f));
    }
    
    SECTION("getScattering scales with density") {
        Vec3 scatteringAtGround = rayleigh.getScattering(0.0f);
        Vec3 scatteringAt10km = rayleigh.getScattering(10000.0f);
        
        REQUIRE(scatteringAtGround.x > scatteringAt10km.x);
        REQUIRE(scatteringAtGround.y > scatteringAt10km.y);
        REQUIRE(scatteringAtGround.z > scatteringAt10km.z);
    }
}

// =============================================================================
// Mie Scattering Tests
// =============================================================================

TEST_CASE("MieScattering - Calculations", "[render][sky]") {
    MieScattering mie;
    
    SECTION("Default values") {
        REQUIRE(mie.scaleHeight == Approx(SkyConfig::DEFAULT_MIE_SCALE_HEIGHT));
        REQUIRE(mie.phaseG >= -1.0f);
        REQUIRE(mie.phaseG <= 1.0f);
    }
    
    SECTION("Phase function calculations") {
        mie.phaseG = 0.8f;
        
        f32 forwardPhase = mie.phaseFunction(1.0f);
        f32 sidePhase = mie.phaseFunction(0.0f);
        f32 backPhase = mie.phaseFunction(-1.0f);
        
        REQUIRE(forwardPhase > sidePhase);
        REQUIRE(sidePhase > backPhase);
    }
}

// =============================================================================
// Atmosphere Parameters Tests
// =============================================================================

TEST_CASE("AtmosphereParameters - Configuration", "[render][sky]") {
    SECTION("Default Earth-like parameters") {
        AtmosphereParameters atm;
        REQUIRE(atm.planetRadius == Approx(SkyConfig::DEFAULT_PLANET_RADIUS));
        REQUIRE(atm.atmosphereHeight == Approx(SkyConfig::DEFAULT_ATMOSPHERE_HEIGHT));
    }
    
    SECTION("getTopRadius calculation") {
        AtmosphereParameters atm;
        REQUIRE(atm.getTopRadius() == Approx(atm.planetRadius + atm.atmosphereHeight));
    }
    
    SECTION("Presets have distinct parameters") {
        auto earth = AtmosphereParameters::earth();
        auto mars = AtmosphereParameters::mars();
        auto alien = AtmosphereParameters::alien();
        
        REQUIRE(mars.planetRadius < earth.planetRadius);
        REQUIRE(mars.ozone.enabled == false);
        REQUIRE(alien.rayleigh.scatteringCoefficients.x > alien.rayleigh.scatteringCoefficients.y);
    }
}

// =============================================================================
// Celestial Body Tests
// =============================================================================

TEST_CASE("CelestialBody - Creation and properties", "[render][sky]") {
    SECTION("Sun factory") {
        auto sun = CelestialBody::sun();
        REQUIRE(sun.type == CelestialBodyType::Sun);
        REQUIRE(sun.angularDiameter > 0.0f);
        REQUIRE(sun.intensity > 0.0f);
    }
    
    SECTION("Moon factory") {
        auto moon = CelestialBody::moon();
        REQUIRE(moon.type == CelestialBodyType::Moon);
        REQUIRE(moon.angularDiameter > 0.0f);
        REQUIRE(moon.intensity < 1.0f);
    }
    
    SECTION("Apparent size calculation") {
        CelestialBody body;
        body.angularDiameter = 0.5f;
        
        f32 size = body.getApparentSize(90.0f, 1080.0f);
        REQUIRE(size > 0.0f);
        REQUIRE(size == Approx((0.5f / 90.0f) * 1080.0f));
    }
}

// =============================================================================
// Star Field Tests
// =============================================================================

TEST_CASE("StarFieldSettings - Visibility", "[render][sky]") {
    StarFieldSettings stars;
    
    SECTION("Default settings") {
        REQUIRE(stars.enabled == true);
        REQUIRE(stars.brightness > 0.0f);
        REQUIRE(stars.magnitudeLimit > 0.0f);
    }
    
    SECTION("Visibility decreases during day") {
        f32 nightVisibility = stars.getVisibility(-10.0f);
        f32 dayVisibility = stars.getVisibility(30.0f);
        
        REQUIRE(nightVisibility == Approx(1.0f));
        REQUIRE(dayVisibility < nightVisibility);
    }
}

// =============================================================================
// Cloud Layer Tests
// =============================================================================

TEST_CASE("CloudLayer - Configuration", "[render][sky]") {
    CloudLayer layer;
    layer.altitude = 2000.0f;
    layer.thickness = 1000.0f;
    
    SECTION("Default values are valid") {
        CloudLayer defaultLayer;
        REQUIRE(defaultLayer.enabled == true);
        REQUIRE(defaultLayer.altitude > 0.0f);
        REQUIRE(defaultLayer.thickness > 0.0f);
        REQUIRE(defaultLayer.coverage >= 0.0f);
        REQUIRE(defaultLayer.coverage <= 1.0f);
    }
    
    SECTION("getBounds calculation") {
        f32 bottom, top;
        layer.getBounds(bottom, top);
        
        REQUIRE(bottom == Approx(2000.0f));
        REQUIRE(top == Approx(3000.0f));
    }
    
    SECTION("containsHeight tests") {
        REQUIRE(layer.containsHeight(2500.0f) == true);
        REQUIRE(layer.containsHeight(1000.0f) == false);
        REQUIRE(layer.containsHeight(4000.0f) == false);
    }
    
    SECTION("getNormalizedHeight calculation") {
        REQUIRE(layer.getNormalizedHeight(2000.0f) == Approx(0.0f));
        REQUIRE(layer.getNormalizedHeight(2500.0f) == Approx(0.5f));
        REQUIRE(layer.getNormalizedHeight(3000.0f) == Approx(1.0f));
    }
    
    SECTION("Cloud presets have correct types") {
        auto cirrus = CloudLayer::cirrus();
        auto cumulus = CloudLayer::cumulus();
        auto stratus = CloudLayer::stratus();
        
        REQUIRE(cirrus.type == CloudLayerType::Cirrus);
        REQUIRE(cumulus.type == CloudLayerType::Cumulus);
        REQUIRE(stratus.type == CloudLayerType::Stratus);
        REQUIRE(stratus.altitude < cirrus.altitude);
    }
}

// =============================================================================
// Sky Settings Tests
// =============================================================================

TEST_CASE("SkySettings - Configuration", "[render][sky]") {
    SECTION("Default settings") {
        SkySettings settings;
        REQUIRE(settings.technique == SkyTechnique::PhysicalAtmosphere);
        REQUIRE(settings.exposure > 0.0f);
    }
    
    SECTION("getSunAltitude calculation") {
        SkySettings settings;
        settings.celestialBodies.push_back(CelestialBody::sun());
        settings.celestialBodies[0].direction = Vec3{0.0f, 1.0f, 0.0f};
        
        REQUIRE(settings.getSunAltitude() == Approx(90.0f).epsilon(0.1f));
        
        settings.celestialBodies[0].direction = Vec3{1.0f, 0.0f, 0.0f};
        REQUIRE(settings.getSunAltitude() == Approx(0.0f).epsilon(0.1f));
    }
    
    SECTION("isNight detection") {
        SkySettings settings;
        settings.celestialBodies.push_back(CelestialBody::sun());
        
        settings.celestialBodies[0].direction = Vec3{0.0f, -0.5f, 0.866f};
        REQUIRE(settings.isNight() == true);
        
        settings.celestialBodies[0].direction = Vec3{0.0f, 0.5f, 0.866f};
        REQUIRE(settings.isNight() == false);
    }
    
    SECTION("Sky presets") {
        auto day = SkySettings::defaultDay();
        auto sunset = SkySettings::sunset();
        auto night = SkySettings::nightSky();
        
        REQUIRE(!day.celestialBodies.empty());
        REQUIRE(!sunset.celestialBodies.empty());
        REQUIRE(night.stars.enabled == true);
        REQUIRE(night.celestialBodies.size() >= 2);
    }
}

// =============================================================================
// Time of Day Tests
// =============================================================================

TEST_CASE("TimeOfDaySettings - Time management", "[render][sky]") {
    TimeOfDaySettings tod;
    
    SECTION("Default values") {
        REQUIRE(tod.timeOfDay >= 0.0f);
        REQUIRE(tod.timeOfDay <= 24.0f);
        REQUIRE(tod.dayOfYear >= 1.0f);
        REQUIRE(tod.dayOfYear <= 365.0f);
    }
    
    SECTION("Update advances time") {
        tod.timeOfDay = 12.0f;
        tod.timeSpeed = 3600.0f;
        
        tod.update(1.0f);
        REQUIRE(tod.timeOfDay == Approx(13.0f));
    }
    
    SECTION("Update wraps at midnight") {
        tod.timeOfDay = 23.5f;
        tod.timeSpeed = 3600.0f;
        f32 initialDay = tod.dayOfYear;
        
        tod.update(1.0f);
        REQUIRE(tod.timeOfDay < 24.0f);
        REQUIRE(tod.dayOfYear > initialDay);
    }
}

// =============================================================================
// GPU Data Tests
// =============================================================================

TEST_CASE("GPU Sky Data - Alignment", "[render][sky]") {
    REQUIRE(alignof(GPUAtmosphereData) == 16);
    REQUIRE(alignof(GPUCelestialData) == 16);
    REQUIRE(alignof(GPUSkyData) == 16);
}

// =============================================================================
// Sky Manager Tests
// =============================================================================

TEST_CASE("SkyManager - Singleton", "[render][sky]") {
    SECTION("Singleton access returns same instance") {
        auto& manager1 = SkyManager::getInstance();
        auto& manager2 = SkyManager::getInstance();
        REQUIRE(&manager1 == &manager2);
    }
    
    SECTION("Initialize and shutdown") {
        auto& manager = SkyManager::getInstance();
        manager.initialize();
        REQUIRE(manager.isInitialized() == true);
        
        manager.shutdown();
        REQUIRE(manager.isInitialized() == false);
    }
}
