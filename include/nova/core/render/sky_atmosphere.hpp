/**
 * @file sky_atmosphere.hpp
 * @brief NovaCore Sky and Atmosphere System™ - Physically-based atmospheric rendering
 * 
 * Production-grade sky and atmosphere system for the NovaCore engine.
 * Features physically-based atmospheric scattering, procedural clouds, 
 * dynamic time-of-day, and celestial body rendering.
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <functional>

namespace nova::render {

// Use types from nova namespace
using nova::u8;
using nova::u16;
using nova::u32;
using nova::u64;
using nova::i32;
using nova::f32;
using nova::f64;
using nova::usize;
using nova::Handle;

// Use types from nova::math namespace
using nova::math::Vec2;
using nova::math::Vec3;
using nova::math::Vec4;
using nova::math::Mat4;

/// Color type (RGBA, 0-1 range)
using Color = Vec4;

// ============================================================================
// Configuration Constants
// ============================================================================

/**
 * @brief Sky system configuration constants
 */
namespace SkyConfig {
    /// Default planet radius in meters (Earth)
    constexpr f32 DEFAULT_PLANET_RADIUS = 6371000.0f;
    
    /// Default atmosphere height in meters
    constexpr f32 DEFAULT_ATMOSPHERE_HEIGHT = 100000.0f;
    
    /// Default Rayleigh scale height
    constexpr f32 DEFAULT_RAYLEIGH_SCALE_HEIGHT = 8500.0f;
    
    /// Default Mie scale height
    constexpr f32 DEFAULT_MIE_SCALE_HEIGHT = 1200.0f;
    
    /// Default ozone layer center height
    constexpr f32 DEFAULT_OZONE_CENTER_HEIGHT = 25000.0f;
    
    /// Default ozone layer width
    constexpr f32 DEFAULT_OZONE_WIDTH = 15000.0f;
    
    /// LUT texture sizes
    constexpr u32 TRANSMITTANCE_LUT_WIDTH = 256;
    constexpr u32 TRANSMITTANCE_LUT_HEIGHT = 64;
    constexpr u32 MULTISCATTER_LUT_SIZE = 32;
    constexpr u32 SKYVIEW_LUT_WIDTH = 200;
    constexpr u32 SKYVIEW_LUT_HEIGHT = 100;
    constexpr u32 AERIAL_PERSPECTIVE_SLICES = 32;
    
    /// Maximum number of celestial bodies
    constexpr u32 MAX_CELESTIAL_BODIES = 8;
    
    /// Cloud noise octaves
    constexpr u32 MAX_CLOUD_NOISE_OCTAVES = 8;
    
    /// Maximum cloud layers
    constexpr u32 MAX_CLOUD_LAYERS = 4;
}

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Sky rendering technique
 */
enum class SkyTechnique : u8 {
    None,               ///< No sky rendering
    SolidColor,         ///< Simple solid color
    Gradient,           ///< Two-color gradient
    Skybox,             ///< Cubemap skybox
    HDRI,               ///< HDRI environment map
    Procedural,         ///< Procedural sky (Preetham/Hosek-Wilkie)
    PhysicalAtmosphere, ///< Full physical atmosphere simulation
    Bruneton            ///< Bruneton atmosphere model
};

/**
 * @brief Procedural sky model type
 */
enum class ProceduralSkyModel : u8 {
    Preetham,           ///< Preetham et al. analytical sky model
    HosekWilkie,        ///< Hosek-Wilkie sky model
    Hillaire,           ///< Sébastien Hillaire's model (UE4/5 style)
    Custom              ///< Custom implementation
};

/**
 * @brief Cloud rendering technique
 */
enum class CloudTechnique : u8 {
    None,               ///< No clouds
    Texture2D,          ///< Simple 2D cloud texture
    Billboards,         ///< Cloud billboards
    Volumetric,         ///< Ray-marched volumetric clouds
    VolumetricLit       ///< Volumetric with full lighting
};

/**
 * @brief Cloud layer type
 */
enum class CloudLayerType : u8 {
    Cirrus,             ///< High altitude, thin wispy clouds
    Cirrocumulus,       ///< High altitude, small puffy patches
    Cirrostratus,       ///< High altitude, thin sheet
    Altostratus,        ///< Mid altitude, gray sheet
    Altocumulus,        ///< Mid altitude, white/gray patches
    Stratocumulus,      ///< Low altitude, lumpy layer
    Stratus,            ///< Low altitude, uniform gray layer
    Cumulus,            ///< Fair weather puffy clouds
    Cumulonimbus        ///< Storm clouds, tall towers
};

/**
 * @brief Celestial body type
 */
enum class CelestialBodyType : u8 {
    Sun,                ///< Star (primary light source)
    Moon,               ///< Moon with phases
    Planet,             ///< Distant planet
    Star,               ///< Background star
    Custom              ///< Custom celestial body
};

/**
 * @brief Moon phase
 */
enum class MoonPhase : u8 {
    NewMoon,
    WaxingCrescent,
    FirstQuarter,
    WaxingGibbous,
    FullMoon,
    WaningGibbous,
    LastQuarter,
    WaningCrescent
};

/**
 * @brief Get moon phase name
 */
inline const char* getMoonPhaseName(MoonPhase phase) {
    switch (phase) {
        case MoonPhase::NewMoon: return "New Moon";
        case MoonPhase::WaxingCrescent: return "Waxing Crescent";
        case MoonPhase::FirstQuarter: return "First Quarter";
        case MoonPhase::WaxingGibbous: return "Waxing Gibbous";
        case MoonPhase::FullMoon: return "Full Moon";
        case MoonPhase::WaningGibbous: return "Waning Gibbous";
        case MoonPhase::LastQuarter: return "Last Quarter";
        case MoonPhase::WaningCrescent: return "Waning Crescent";
        default: return "Unknown";
    }
}

// ============================================================================
// Atmosphere Parameters
// ============================================================================

/**
 * @brief Rayleigh scattering parameters
 */
struct RayleighScattering {
    Vec3 scatteringCoefficients{5.802e-6f, 13.558e-6f, 33.1e-6f}; ///< RGB coefficients
    f32 scaleHeight = SkyConfig::DEFAULT_RAYLEIGH_SCALE_HEIGHT;    ///< Scale height in meters
    f32 densityMultiplier = 1.0f;                                   ///< Density scaling
    
    /// Calculate density at height
    f32 getDensity(f32 height) const {
        return densityMultiplier * std::exp(-height / scaleHeight);
    }
    
    /// Get scattering at height
    Vec3 getScattering(f32 height) const {
        return scatteringCoefficients * getDensity(height);
    }
};

/**
 * @brief Mie scattering parameters
 */
struct MieScattering {
    Vec3 scatteringCoefficient{3.996e-6f, 3.996e-6f, 3.996e-6f}; ///< Scattering coefficient
    Vec3 extinctionCoefficient{4.440e-6f, 4.440e-6f, 4.440e-6f}; ///< Extinction (absorption + scattering)
    f32 scaleHeight = SkyConfig::DEFAULT_MIE_SCALE_HEIGHT;        ///< Scale height in meters
    f32 phaseG = 0.8f;                                            ///< Phase function anisotropy (-1 to 1)
    f32 densityMultiplier = 1.0f;                                 ///< Density scaling
    
    /// Calculate density at height
    f32 getDensity(f32 height) const {
        return densityMultiplier * std::exp(-height / scaleHeight);
    }
    
    /// Henyey-Greenstein phase function
    f32 phaseFunction(f32 cosTheta) const {
        f32 g2 = phaseG * phaseG;
        f32 denom = 1.0f + g2 - 2.0f * phaseG * cosTheta;
        return (1.0f - g2) / (4.0f * 3.14159265f * denom * std::sqrt(denom));
    }
};

/**
 * @brief Ozone absorption parameters
 */
struct OzoneAbsorption {
    Vec3 absorptionCoefficients{0.650e-6f, 1.881e-6f, 0.085e-6f}; ///< RGB absorption
    f32 centerHeight = SkyConfig::DEFAULT_OZONE_CENTER_HEIGHT;     ///< Layer center height
    f32 layerWidth = SkyConfig::DEFAULT_OZONE_WIDTH;               ///< Layer width
    f32 densityMultiplier = 1.0f;                                  ///< Density scaling
    bool enabled = true;                                           ///< Enable ozone layer
    
    /// Calculate density at height (tent function)
    f32 getDensity(f32 height) const {
        if (!enabled) return 0.0f;
        f32 dist = std::abs(height - centerHeight);
        return densityMultiplier * std::max(0.0f, 1.0f - dist / layerWidth);
    }
};

/**
 * @brief Complete atmosphere parameters
 */
struct AtmosphereParameters {
    f32 planetRadius = SkyConfig::DEFAULT_PLANET_RADIUS;
    f32 atmosphereHeight = SkyConfig::DEFAULT_ATMOSPHERE_HEIGHT;
    
    RayleighScattering rayleigh;
    MieScattering mie;
    OzoneAbsorption ozone;
    
    Vec3 groundAlbedo{0.3f, 0.3f, 0.3f}; ///< Ground reflectance
    f32 multiScatteringFactor = 1.0f;    ///< Multi-scattering approximation
    
    /// Get top of atmosphere radius
    f32 getTopRadius() const { return planetRadius + atmosphereHeight; }
    
    /// Get atmosphere thickness at point
    f32 getAtmosphereThickness(f32 altitude) const {
        return std::max(0.0f, atmosphereHeight - altitude);
    }
    
    /// Create Earth-like atmosphere
    static AtmosphereParameters earth() {
        return AtmosphereParameters{}; // Defaults are Earth-like
    }
    
    /// Create Mars-like atmosphere (thin, red)
    static AtmosphereParameters mars() {
        AtmosphereParameters params;
        params.planetRadius = 3389500.0f;
        params.atmosphereHeight = 50000.0f;
        params.rayleigh.scatteringCoefficients = Vec3{19.918e-6f, 13.57e-6f, 5.75e-6f};
        params.rayleigh.scaleHeight = 11000.0f;
        params.mie.phaseG = 0.65f;
        params.ozone.enabled = false;
        return params;
    }
    
    /// Create alien atmosphere (purple sky)
    static AtmosphereParameters alien() {
        AtmosphereParameters params;
        params.rayleigh.scatteringCoefficients = Vec3{33.1e-6f, 5.802e-6f, 13.558e-6f};
        return params;
    }
};

// ============================================================================
// Celestial Bodies
// ============================================================================

/**
 * @brief Celestial body description
 */
struct CelestialBody {
    std::string name;
    CelestialBodyType type = CelestialBodyType::Sun;
    
    Vec3 direction{0.0f, 1.0f, 0.0f};    ///< Direction to body (normalized)
    f32 angularDiameter = 0.5f;           ///< Angular diameter in degrees
    f32 intensity = 1.0f;                 ///< Light intensity multiplier
    Color color{1.0f, 1.0f, 1.0f, 1.0f}; ///< Body color/tint
    
    // For sun
    f32 diskSharpness = 100.0f;          ///< Sun disk edge sharpness
    f32 limbDarkening = 0.6f;            ///< Limb darkening coefficient
    
    // For moon
    MoonPhase moonPhase = MoonPhase::FullMoon;
    f32 earthshine = 0.1f;               ///< Earthshine intensity on dark side
    
    // Texture handles (for custom bodies)
    u32 textureHandle = 0;
    
    /// Calculate apparent size in screen space
    f32 getApparentSize(f32 fovDegrees, f32 screenHeight) const {
        return (angularDiameter / fovDegrees) * screenHeight;
    }
    
    /// Calculate disk intensity at angle from center
    f32 getDiskIntensity(f32 angleFromCenter) const {
        f32 halfAngle = angularDiameter * 0.5f * (3.14159265f / 180.0f);
        f32 t = angleFromCenter / halfAngle;
        if (t >= 1.0f) return 0.0f;
        
        // Apply limb darkening
        f32 mu = std::sqrt(1.0f - t * t);
        return 1.0f - limbDarkening * (1.0f - mu);
    }
    
    /// Create sun
    static CelestialBody sun() {
        CelestialBody body;
        body.name = "Sun";
        body.type = CelestialBodyType::Sun;
        body.angularDiameter = 0.533f;
        body.color = Color{1.0f, 0.95f, 0.9f, 1.0f};
        return body;
    }
    
    /// Create moon
    static CelestialBody moon() {
        CelestialBody body;
        body.name = "Moon";
        body.type = CelestialBodyType::Moon;
        body.angularDiameter = 0.517f;
        body.intensity = 0.001f;
        body.color = Color{0.9f, 0.9f, 1.0f, 1.0f};
        return body;
    }
};

// ============================================================================
// Stars
// ============================================================================

/**
 * @brief Star field parameters
 */
struct StarFieldSettings {
    bool enabled = true;
    f32 brightness = 1.0f;              ///< Overall brightness
    f32 twinkleFactor = 0.3f;           ///< Twinkle animation amount
    f32 twinkleSpeed = 1.0f;            ///< Twinkle animation speed
    f32 sizeMin = 0.5f;                 ///< Minimum star size in pixels
    f32 sizeMax = 3.0f;                 ///< Maximum star size in pixels
    f32 magnitudeLimit = 6.5f;          ///< Apparent magnitude visibility limit
    bool colorVariation = true;          ///< Enable star color temperature variation
    f32 extinctionFactor = 0.5f;        ///< Atmospheric extinction near horizon
    
    /// Calculate star visibility based on sun altitude
    f32 getVisibility(f32 sunAltitudeDegrees) const {
        // Stars fade out as sun rises
        if (sunAltitudeDegrees > 0.0f) {
            return std::max(0.0f, 1.0f - sunAltitudeDegrees / 18.0f); // Civil twilight
        }
        return 1.0f;
    }
};

/**
 * @brief Individual star data (for procedural/catalog stars)
 */
struct StarData {
    Vec3 direction{};           ///< Direction to star (normalized)
    f32 magnitude = 0.0f;       ///< Apparent magnitude (lower = brighter)
    f32 colorTemperature = 5778.0f; ///< Color temperature in Kelvin
    
    /// Calculate brightness from magnitude
    f32 getBrightness(f32 magnitudeLimit = 6.5f) const {
        return std::pow(2.512f, magnitudeLimit - magnitude);
    }
    
    /// Get star color from temperature
    Color getColor() const {
        // Approximate blackbody color
        f32 temp = colorTemperature / 100.0f;
        f32 r, g, b;
        
        if (temp <= 66.0f) {
            r = 1.0f;
            g = 0.39f * std::log(temp) - 0.634f;
        } else {
            r = 1.292f * std::pow(temp - 60.0f, -0.1332f);
            g = 1.129f * std::pow(temp - 60.0f, -0.0755f);
        }
        
        if (temp <= 19.0f) {
            b = 0.0f;
        } else if (temp <= 66.0f) {
            b = 0.543f * std::log(temp - 10.0f) - 1.196f;
        } else {
            b = 1.0f;
        }
        
        return Color{
            std::clamp(r, 0.0f, 1.0f),
            std::clamp(g, 0.0f, 1.0f),
            std::clamp(b, 0.0f, 1.0f),
            1.0f
        };
    }
};

// ============================================================================
// Cloud System
// ============================================================================

/**
 * @brief Cloud layer settings
 */
struct CloudLayer {
    bool enabled = true;
    CloudLayerType type = CloudLayerType::Cumulus;
    
    f32 altitude = 2000.0f;             ///< Layer altitude in meters
    f32 thickness = 1000.0f;            ///< Layer thickness
    f32 coverage = 0.5f;                ///< Cloud coverage (0-1)
    f32 density = 1.0f;                 ///< Cloud density
    
    Vec2 windDirection{1.0f, 0.0f};     ///< Wind direction (horizontal)
    f32 windSpeed = 10.0f;              ///< Wind speed in m/s
    
    f32 detailScale = 0.001f;           ///< Detail noise scale
    f32 detailStrength = 0.3f;          ///< Detail noise strength
    
    Color tint{1.0f, 1.0f, 1.0f, 1.0f}; ///< Cloud tint
    
    /// Get layer bounds
    void getBounds(f32& outBottom, f32& outTop) const {
        outBottom = altitude;
        outTop = altitude + thickness;
    }
    
    /// Check if height is within layer
    bool containsHeight(f32 height) const {
        return height >= altitude && height <= altitude + thickness;
    }
    
    /// Get normalized height within layer
    f32 getNormalizedHeight(f32 height) const {
        if (!containsHeight(height)) return -1.0f;
        return (height - altitude) / thickness;
    }
    
    /// Create presets
    static CloudLayer cirrus() {
        CloudLayer layer;
        layer.type = CloudLayerType::Cirrus;
        layer.altitude = 8000.0f;
        layer.thickness = 2000.0f;
        layer.coverage = 0.3f;
        layer.density = 0.2f;
        return layer;
    }
    
    static CloudLayer cumulus() {
        CloudLayer layer;
        layer.type = CloudLayerType::Cumulus;
        layer.altitude = 1500.0f;
        layer.thickness = 2000.0f;
        layer.coverage = 0.5f;
        layer.density = 0.8f;
        return layer;
    }
    
    static CloudLayer stratus() {
        CloudLayer layer;
        layer.type = CloudLayerType::Stratus;
        layer.altitude = 500.0f;
        layer.thickness = 500.0f;
        layer.coverage = 0.9f;
        layer.density = 0.6f;
        return layer;
    }
};

/**
 * @brief Volumetric cloud settings
 */
struct VolumetricCloudSettings {
    bool enabled = true;
    CloudTechnique technique = CloudTechnique::Volumetric;
    
    // Ray marching
    u32 primarySteps = 64;              ///< Primary ray march steps
    u32 lightSteps = 6;                 ///< Light ray march steps
    f32 maxDistance = 50000.0f;         ///< Maximum ray march distance
    
    // Noise
    f32 baseScale = 0.00003f;           ///< Base noise scale
    f32 detailScale = 0.0003f;          ///< Detail noise scale
    u32 noiseOctaves = 4;               ///< Noise octaves
    
    // Shaping
    f32 coverageModifier = 0.0f;        ///< Coverage bias
    f32 typeModifier = 0.5f;            ///< Cloud type (affects height distribution)
    f32 anvil = 0.0f;                   ///< Anvil shape strength for storm clouds
    
    // Lighting
    f32 absorptionCoefficient = 0.04f;  ///< Light absorption
    f32 scatteringCoefficient = 0.04f;  ///< Light scattering
    f32 ambientLightScale = 0.4f;       ///< Ambient light contribution
    f32 sunLightScale = 1.0f;           ///< Sun light contribution
    f32 silverIntensity = 1.0f;         ///< Silver lining intensity
    f32 silverSpread = 0.1f;            ///< Silver lining spread
    
    // Animation
    f32 animationSpeed = 1.0f;          ///< Overall animation speed
    
    // Layers
    std::vector<CloudLayer> layers;
    
    /// Get total cloud layers
    usize getLayerCount() const { return layers.size(); }
    
    /// Add default cumulus layer
    void addDefaultLayer() {
        layers.push_back(CloudLayer::cumulus());
    }
};

// ============================================================================
// Sky Settings
// ============================================================================

/**
 * @brief Complete sky configuration
 */
struct SkySettings {
    SkyTechnique technique = SkyTechnique::PhysicalAtmosphere;
    ProceduralSkyModel proceduralModel = ProceduralSkyModel::Hillaire;
    
    AtmosphereParameters atmosphere;
    VolumetricCloudSettings clouds;
    StarFieldSettings stars;
    
    // Celestial bodies
    std::vector<CelestialBody> celestialBodies;
    
    // Simple sky settings
    Color solidColor{0.5f, 0.7f, 1.0f, 1.0f};
    Color zenithColor{0.1f, 0.3f, 0.8f, 1.0f};
    Color horizonColor{0.8f, 0.9f, 1.0f, 1.0f};
    Color groundColor{0.4f, 0.35f, 0.3f, 1.0f};
    
    // Exposure and color grading
    f32 exposure = 1.0f;                ///< Sky exposure
    f32 saturation = 1.0f;              ///< Color saturation
    Vec3 tint{1.0f, 1.0f, 1.0f};        ///< Overall tint
    
    // Fog integration
    bool enableAerialPerspective = true;
    f32 aerialPerspectiveDistance = 10000.0f;
    f32 aerialPerspectiveScale = 1.0f;
    
    /// Get primary sun direction
    Vec3 getSunDirection() const {
        for (const auto& body : celestialBodies) {
            if (body.type == CelestialBodyType::Sun) {
                return body.direction;
            }
        }
        return Vec3{0.0f, 1.0f, 0.0f};
    }
    
    /// Calculate sun altitude in degrees
    f32 getSunAltitude() const {
        Vec3 sunDir = getSunDirection();
        return std::asin(sunDir.y) * (180.0f / 3.14159265f);
    }
    
    /// Is it nighttime?
    bool isNight() const {
        return getSunAltitude() < -6.0f; // Civil twilight
    }
    
    /// Create default day sky
    static SkySettings defaultDay() {
        SkySettings settings;
        settings.celestialBodies.push_back(CelestialBody::sun());
        settings.celestialBodies[0].direction = Vec3{0.3f, 0.8f, 0.5f}.normalized();
        return settings;
    }
    
    /// Create sunset sky
    static SkySettings sunset() {
        SkySettings settings;
        settings.celestialBodies.push_back(CelestialBody::sun());
        settings.celestialBodies[0].direction = Vec3{0.9f, 0.1f, 0.4f}.normalized();
        settings.celestialBodies[0].color = Color{1.0f, 0.6f, 0.3f, 1.0f};
        settings.saturation = 1.2f;
        return settings;
    }
    
    /// Create night sky
    static SkySettings nightSky() {
        SkySettings settings;
        settings.celestialBodies.push_back(CelestialBody::sun());
        settings.celestialBodies[0].direction = Vec3{0.0f, -0.5f, 0.866f}.normalized();
        settings.celestialBodies.push_back(CelestialBody::moon());
        settings.celestialBodies[1].direction = Vec3{0.3f, 0.6f, 0.742f}.normalized();
        settings.stars.enabled = true;
        settings.stars.brightness = 1.5f;
        return settings;
    }
};

// ============================================================================
// Time of Day
// ============================================================================

/**
 * @brief Time of day controller
 */
struct TimeOfDaySettings {
    f32 timeOfDay = 12.0f;              ///< Current time (0-24 hours)
    f32 dayOfYear = 172.0f;             ///< Day of year (1-365)
    f32 latitude = 45.0f;               ///< Latitude in degrees (-90 to 90)
    f32 longitude = 0.0f;               ///< Longitude in degrees (-180 to 180)
    
    f32 timeSpeed = 1.0f;               ///< Time multiplier (0 = paused)
    bool autoUpdateSun = true;          ///< Automatically update sun position
    
    /// Calculate sun direction from time/location
    Vec3 calculateSunDirection() const {
        // Simplified solar position calculation
        f32 dayAngle = 2.0f * 3.14159265f * (dayOfYear - 1.0f) / 365.0f;
        
        // Declination
        f32 declination = 0.4093f * std::sin(dayAngle - 1.405f);
        
        // Hour angle
        f32 hourAngle = 3.14159265f * (timeOfDay / 12.0f - 1.0f);
        
        // Convert latitude to radians
        f32 latRad = latitude * (3.14159265f / 180.0f);
        
        // Calculate altitude
        f32 sinAlt = std::sin(latRad) * std::sin(declination) +
                     std::cos(latRad) * std::cos(declination) * std::cos(hourAngle);
        f32 altitude = std::asin(sinAlt);
        
        // Calculate azimuth
        f32 cosAz = (std::sin(declination) - std::sin(latRad) * sinAlt) /
                    (std::cos(latRad) * std::cos(altitude));
        f32 azimuth = std::acos(std::clamp(cosAz, -1.0f, 1.0f));
        if (hourAngle > 0.0f) azimuth = 2.0f * 3.14159265f - azimuth;
        
        // Convert to direction vector
        return Vec3{
            std::cos(altitude) * std::sin(azimuth),
            std::sin(altitude),
            std::cos(altitude) * std::cos(azimuth)
        };
    }
    
    /// Calculate moon direction (simplified)
    Vec3 calculateMoonDirection() const {
        // Simplified: Moon roughly opposite to sun with 29.5 day cycle offset
        f32 moonPhaseOffset = (dayOfYear * 24.0f + timeOfDay) / (29.5f * 24.0f);
        moonPhaseOffset = moonPhaseOffset - std::floor(moonPhaseOffset);
        
        f32 moonHourOffset = 12.0f * moonPhaseOffset;
        f32 moonTime = std::fmod(timeOfDay + moonHourOffset, 24.0f);
        
        TimeOfDaySettings moonTod = *this;
        moonTod.timeOfDay = moonTime;
        return moonTod.calculateSunDirection();
    }
    
    /// Get current moon phase
    MoonPhase calculateMoonPhase() const {
        f32 phase = std::fmod((dayOfYear - 1.0f) / 29.53f, 1.0f);
        return static_cast<MoonPhase>(static_cast<int>(phase * 8.0f) % 8);
    }
    
    /// Advance time
    void update(f32 deltaTime) {
        if (timeSpeed != 0.0f) {
            timeOfDay += deltaTime * timeSpeed / 3600.0f; // Convert to hours
            while (timeOfDay >= 24.0f) {
                timeOfDay -= 24.0f;
                dayOfYear += 1.0f;
                if (dayOfYear > 365.0f) dayOfYear = 1.0f;
            }
            while (timeOfDay < 0.0f) {
                timeOfDay += 24.0f;
                dayOfYear -= 1.0f;
                if (dayOfYear < 1.0f) dayOfYear = 365.0f;
            }
        }
    }
};

// ============================================================================
// GPU Data Structures
// ============================================================================

/**
 * @brief GPU-ready atmosphere data (must match shader)
 */
struct alignas(16) GPUAtmosphereData {
    Vec4 rayleighScattering;        ///< xyz = coefficients, w = scaleHeight
    Vec4 mieScattering;             ///< xyz = coefficient, w = scaleHeight
    Vec4 mieExtinction;             ///< xyz = extinction, w = phaseG
    Vec4 ozoneAbsorption;           ///< xyz = coefficients, w = centerHeight
    Vec4 planetParams;              ///< x = planetRadius, y = atmosphereHeight, z = ozoneWidth, w = multiScatter
    Vec4 groundAlbedo;              ///< xyz = albedo, w = unused
};

/**
 * @brief GPU-ready celestial body data
 */
struct alignas(16) GPUCelestialData {
    Vec4 direction;                 ///< xyz = direction, w = angularSize
    Vec4 color;                     ///< RGB = color, A = intensity
    Vec4 params;                    ///< x = diskSharpness, y = limbDarkening, z = phase, w = type
};

/**
 * @brief GPU-ready sky data
 */
struct alignas(16) GPUSkyData {
    GPUAtmosphereData atmosphere;
    GPUCelestialData sun;
    GPUCelestialData moon;
    Vec4 skyParams;                 ///< x = exposure, y = saturation, z = aerialPerspDist, w = aerialPerspScale
    Vec4 starParams;                ///< x = brightness, y = twinkle, z = twinkleSpeed, w = visibility
    u32 celestialCount;
    u32 cloudLayerCount;
    f32 time;
    f32 padding;
};

// ============================================================================
// Sky Manager
// ============================================================================

/**
 * @brief Sky system statistics
 */
struct SkyStats {
    f64 lutGenerationTimeMs = 0.0;
    f64 skyRenderTimeMs = 0.0;
    f64 cloudRenderTimeMs = 0.0;
    u32 cloudRayMarchSteps = 0;
    u32 visibleStars = 0;
};

/**
 * @brief Sky system manager singleton
 */
class SkyManager {
public:
    /// Get singleton instance
    static SkyManager& getInstance() {
        static SkyManager instance;
        return instance;
    }
    
    /// Initialize sky system
    void initialize() {
        m_initialized = true;
        m_settings = SkySettings::defaultDay();
    }
    
    /// Shutdown sky system
    void shutdown() {
        m_initialized = false;
    }
    
    /// Update sky (time of day, animations)
    void update(f32 deltaTime) {
        if (m_timeOfDay.autoUpdateSun) {
            m_timeOfDay.update(deltaTime);
            updateSunFromTimeOfDay();
        }
        
        m_time += deltaTime;
    }
    
    /// Set sky settings
    void setSettings(const SkySettings& settings) { m_settings = settings; }
    const SkySettings& getSettings() const { return m_settings; }
    SkySettings& getSettings() { return m_settings; }
    
    /// Set time of day settings
    void setTimeOfDay(const TimeOfDaySettings& tod) { m_timeOfDay = tod; }
    const TimeOfDaySettings& getTimeOfDay() const { return m_timeOfDay; }
    TimeOfDaySettings& getTimeOfDay() { return m_timeOfDay; }
    
    /// Get GPU data
    GPUSkyData getGPUData() const {
        GPUSkyData data{};
        
        // Atmosphere
        const auto& atm = m_settings.atmosphere;
        data.atmosphere.rayleighScattering = Vec4{
            atm.rayleigh.scatteringCoefficients.x,
            atm.rayleigh.scatteringCoefficients.y,
            atm.rayleigh.scatteringCoefficients.z,
            atm.rayleigh.scaleHeight
        };
        data.atmosphere.mieScattering = Vec4{
            atm.mie.scatteringCoefficient.x,
            atm.mie.scatteringCoefficient.y,
            atm.mie.scatteringCoefficient.z,
            atm.mie.scaleHeight
        };
        data.atmosphere.mieExtinction = Vec4{
            atm.mie.extinctionCoefficient.x,
            atm.mie.extinctionCoefficient.y,
            atm.mie.extinctionCoefficient.z,
            atm.mie.phaseG
        };
        data.atmosphere.ozoneAbsorption = Vec4{
            atm.ozone.absorptionCoefficients.x,
            atm.ozone.absorptionCoefficients.y,
            atm.ozone.absorptionCoefficients.z,
            atm.ozone.centerHeight
        };
        data.atmosphere.planetParams = Vec4{
            atm.planetRadius,
            atm.atmosphereHeight,
            atm.ozone.layerWidth,
            atm.multiScatteringFactor
        };
        data.atmosphere.groundAlbedo = Vec4{
            atm.groundAlbedo.x, atm.groundAlbedo.y, atm.groundAlbedo.z, 0.0f
        };
        
        // Sun
        for (const auto& body : m_settings.celestialBodies) {
            if (body.type == CelestialBodyType::Sun) {
                data.sun.direction = Vec4{body.direction.x, body.direction.y, body.direction.z, body.angularDiameter};
                data.sun.color = Vec4{body.color.x * body.intensity, body.color.y * body.intensity, body.color.z * body.intensity, body.intensity};
                data.sun.params = Vec4{body.diskSharpness, body.limbDarkening, 0.0f, 0.0f};
                break;
            }
        }
        
        // Moon
        for (const auto& body : m_settings.celestialBodies) {
            if (body.type == CelestialBodyType::Moon) {
                data.moon.direction = Vec4{body.direction.x, body.direction.y, body.direction.z, body.angularDiameter};
                data.moon.color = Vec4{body.color.x * body.intensity, body.color.y * body.intensity, body.color.z * body.intensity, body.intensity};
                data.moon.params = Vec4{body.diskSharpness, body.earthshine, static_cast<f32>(body.moonPhase), 1.0f};
                break;
            }
        }
        
        // Sky params
        data.skyParams = Vec4{
            m_settings.exposure,
            m_settings.saturation,
            m_settings.aerialPerspectiveDistance,
            m_settings.aerialPerspectiveScale
        };
        
        // Star params
        f32 starVisibility = m_settings.stars.getVisibility(m_settings.getSunAltitude());
        data.starParams = Vec4{
            m_settings.stars.brightness,
            m_settings.stars.twinkleFactor,
            m_settings.stars.twinkleSpeed,
            starVisibility
        };
        
        data.celestialCount = static_cast<u32>(m_settings.celestialBodies.size());
        data.cloudLayerCount = static_cast<u32>(m_settings.clouds.layers.size());
        data.time = m_time;
        
        return data;
    }
    
    /// Get statistics
    const SkyStats& getStats() const { return m_stats; }
    
    /// Is system initialized?
    bool isInitialized() const { return m_initialized; }
    
private:
    SkyManager() = default;
    
    void updateSunFromTimeOfDay() {
        Vec3 sunDir = m_timeOfDay.calculateSunDirection();
        for (auto& body : m_settings.celestialBodies) {
            if (body.type == CelestialBodyType::Sun) {
                body.direction = sunDir;
                break;
            }
        }
        
        // Update moon too
        Vec3 moonDir = m_timeOfDay.calculateMoonDirection();
        MoonPhase moonPhase = m_timeOfDay.calculateMoonPhase();
        for (auto& body : m_settings.celestialBodies) {
            if (body.type == CelestialBodyType::Moon) {
                body.direction = moonDir;
                body.moonPhase = moonPhase;
                break;
            }
        }
    }
    
    bool m_initialized = false;
    f32 m_time = 0.0f;
    SkySettings m_settings;
    TimeOfDaySettings m_timeOfDay;
    SkyStats m_stats;
};

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Calculate transmittance between two points in atmosphere
 */
inline Vec3 calculateTransmittance(const AtmosphereParameters& atm, 
                                   const Vec3& from, const Vec3& to,
                                   u32 samples = 32) {
    Vec3 ray = to - from;
    f32 distance = ray.length();
    Vec3 dir = ray / distance;
    f32 stepSize = distance / static_cast<f32>(samples);
    
    Vec3 opticalDepth{0.0f, 0.0f, 0.0f};
    
    for (u32 i = 0; i < samples; ++i) {
        Vec3 pos = from + dir * (static_cast<f32>(i) + 0.5f) * stepSize;
        f32 height = pos.length() - atm.planetRadius;
        
        // Rayleigh
        f32 rayleighDensity = atm.rayleigh.getDensity(height);
        opticalDepth = opticalDepth + atm.rayleigh.scatteringCoefficients * rayleighDensity * stepSize;
        
        // Mie
        f32 mieDensity = atm.mie.getDensity(height);
        opticalDepth = opticalDepth + atm.mie.extinctionCoefficient * mieDensity * stepSize;
        
        // Ozone
        f32 ozoneDensity = atm.ozone.getDensity(height);
        opticalDepth = opticalDepth + atm.ozone.absorptionCoefficients * ozoneDensity * stepSize;
    }
    
    return Vec3{
        std::exp(-opticalDepth.x),
        std::exp(-opticalDepth.y),
        std::exp(-opticalDepth.z)
    };
}

/**
 * @brief Convert solar azimuth/altitude to direction vector
 */
inline Vec3 azimuthAltitudeToDirection(f32 azimuthDegrees, f32 altitudeDegrees) {
    f32 az = azimuthDegrees * (3.14159265f / 180.0f);
    f32 alt = altitudeDegrees * (3.14159265f / 180.0f);
    
    return Vec3{
        std::cos(alt) * std::sin(az),
        std::sin(alt),
        std::cos(alt) * std::cos(az)
    };
}

/**
 * @brief Convert direction to azimuth/altitude
 */
inline void directionToAzimuthAltitude(const Vec3& dir, f32& outAzimuth, f32& outAltitude) {
    outAltitude = std::asin(dir.y) * (180.0f / 3.14159265f);
    outAzimuth = std::atan2(dir.x, dir.z) * (180.0f / 3.14159265f);
    if (outAzimuth < 0.0f) outAzimuth += 360.0f;
}

} // namespace nova::render
