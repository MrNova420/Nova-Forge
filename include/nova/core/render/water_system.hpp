/**
 * @file water_system.hpp
 * @brief Advanced water and ocean rendering system for NovaCore engine
 * @author NovaForge Team
 * @copyright Copyright (c) 2024-2025 NovaForge - All Rights Reserved
 * 
 * Provides comprehensive water rendering including:
 * - FFT-based ocean simulation
 * - Screen-space reflections and refractions
 * - Subsurface scattering
 * - Foam and spray simulation
 * - Caustics rendering
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <array>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <cmath>
#include <algorithm>
#include <complex>

namespace nova {

// =============================================================================
// WATER CONFIGURATION
// =============================================================================

/**
 * @brief Water system configuration constants
 */
struct WaterConfig {
    /// Default FFT resolution
    static constexpr u32 DEFAULT_FFT_RESOLUTION = 512;
    
    /// Maximum FFT resolution
    static constexpr u32 MAX_FFT_RESOLUTION = 2048;
    
    /// Maximum wave cascades
    static constexpr u32 MAX_CASCADES = 4;
    
    /// Default water depth
    static constexpr f32 DEFAULT_DEPTH = 100.0f;
    
    /// Default tile size (meters)
    static constexpr f32 DEFAULT_TILE_SIZE = 100.0f;
    
    /// Gravity constant (m/s^2)
    static constexpr f32 GRAVITY = 9.81f;
    
    /// Default wind speed (m/s)
    static constexpr f32 DEFAULT_WIND_SPEED = 10.0f;
    
    /// Maximum foam particles
    static constexpr u32 MAX_FOAM_PARTICLES = 100000;
    
    /// Water absorption coefficient base
    static constexpr f32 ABSORPTION_COEFF = 0.45f;
    
    /// Fresnel F0 for water
    static constexpr f32 WATER_F0 = 0.02f;
};

// =============================================================================
// WATER ENUMERATIONS
// =============================================================================

/**
 * @brief Water simulation technique
 */
enum class WaterTechnique : u8 {
    Simple,         ///< Simple sine wave sum
    Gerstner,       ///< Gerstner waves
    FFT,            ///< FFT ocean simulation
    Tessellation    ///< GPU tessellation with displacement
};

/**
 * @brief Get string name for water technique
 */
[[nodiscard]] constexpr const char* getWaterTechniqueName(WaterTechnique tech) noexcept {
    switch (tech) {
        case WaterTechnique::Simple: return "Simple";
        case WaterTechnique::Gerstner: return "Gerstner";
        case WaterTechnique::FFT: return "FFT";
        case WaterTechnique::Tessellation: return "Tessellation";
        default: return "Unknown";
    }
}

/**
 * @brief Ocean spectrum type
 */
enum class OceanSpectrum : u8 {
    Phillips,       ///< Phillips spectrum
    JONSWAP,        ///< JONSWAP spectrum
    PiersonMoskowitz, ///< Pierson-Moskowitz spectrum
    TMA,            ///< TMA shallow water spectrum
    Unified         ///< Unified ocean spectrum
};

/**
 * @brief Water quality preset
 */
enum class WaterQuality : u8 {
    Low,            ///< Mobile/low-end
    Medium,         ///< Standard quality
    High,           ///< High quality
    Ultra,          ///< Ultra quality with all effects
    Cinematic       ///< Maximum quality for cinematics
};

/**
 * @brief Foam generation mode
 */
enum class FoamMode : u8 {
    None,           ///< No foam
    TextureBased,   ///< Foam from texture
    Jacobian,       ///< Foam from wave Jacobian
    Particle,       ///< Particle-based foam
    Combined        ///< Combined approaches
};

/**
 * @brief Water body type
 */
enum class WaterBodyType : u8 {
    Ocean,          ///< Deep ocean
    Sea,            ///< Shallow sea
    Lake,           ///< Lake/pond
    River,          ///< Flowing river
    Pool,           ///< Swimming pool
    Custom          ///< Custom parameters
};

/**
 * @brief Reflection mode
 */
enum class WaterReflectionMode : u8 {
    None,           ///< No reflections
    Cubemap,        ///< Environment cubemap
    Planar,         ///< Planar reflection
    SSR,            ///< Screen-space reflections
    Raytraced,      ///< Ray traced reflections
    Hybrid          ///< Combined SSR + cubemap fallback
};

// =============================================================================
// WATER STRUCTURES
// =============================================================================

/**
 * @brief Gerstner wave parameters
 */
struct GerstnerWave {
    Vec2 direction{1.0f, 0.0f};          ///< Normalized wave direction
    f32 wavelength{10.0f};               ///< Wavelength (meters)
    f32 amplitude{0.5f};                 ///< Wave amplitude (meters)
    f32 steepness{0.5f};                 ///< Wave steepness (0-1)
    f32 speed{1.0f};                     ///< Wave speed multiplier
    f32 phase{0.0f};                     ///< Phase offset
    
    /// Calculate angular frequency
    [[nodiscard]] f32 getAngularFrequency() const noexcept {
        return std::sqrt(WaterConfig::GRAVITY * 2.0f * 3.14159265f / wavelength);
    }
    
    /// Calculate wave number
    [[nodiscard]] f32 getWaveNumber() const noexcept {
        return 2.0f * 3.14159265f / wavelength;
    }
    
    /// Calculate displacement at position and time
    [[nodiscard]] Vec3 calculateDisplacement(const Vec2& pos, f32 time) const noexcept {
        f32 k = getWaveNumber();
        f32 omega = getAngularFrequency();
        f32 theta = k * (direction.x * pos.x + direction.y * pos.y) - omega * time * speed + phase;
        
        f32 sinTheta = std::sin(theta);
        f32 cosTheta = std::cos(theta);
        
        Vec3 displacement;
        displacement.x = steepness * amplitude * direction.x * cosTheta;
        displacement.y = amplitude * sinTheta;
        displacement.z = steepness * amplitude * direction.y * cosTheta;
        
        return displacement;
    }
    
    /// Factory for small waves
    static GerstnerWave small(const Vec2& dir) noexcept {
        GerstnerWave wave;
        wave.direction = dir.normalized();
        wave.wavelength = 5.0f;
        wave.amplitude = 0.1f;
        wave.steepness = 0.3f;
        return wave;
    }
    
    /// Factory for medium waves
    static GerstnerWave medium(const Vec2& dir) noexcept {
        GerstnerWave wave;
        wave.direction = dir.normalized();
        wave.wavelength = 20.0f;
        wave.amplitude = 0.5f;
        wave.steepness = 0.5f;
        return wave;
    }
    
    /// Factory for large waves
    static GerstnerWave large(const Vec2& dir) noexcept {
        GerstnerWave wave;
        wave.direction = dir.normalized();
        wave.wavelength = 50.0f;
        wave.amplitude = 1.5f;
        wave.steepness = 0.7f;
        return wave;
    }
};

/**
 * @brief FFT ocean cascade parameters
 */
struct OceanCascade {
    u32 resolution{512};                 ///< FFT resolution (power of 2)
    f32 tileSize{100.0f};                ///< World space tile size (meters)
    f32 windSpeed{10.0f};                ///< Wind speed (m/s)
    Vec2 windDirection{1.0f, 0.0f};      ///< Normalized wind direction
    f32 waveDamping{0.001f};             ///< Small wave suppression
    f32 amplitude{1.0f};                 ///< Overall amplitude scale
    f32 depth{100.0f};                   ///< Water depth (meters)
    OceanSpectrum spectrum{OceanSpectrum::Phillips};
    
    /// Get wave number at frequency
    [[nodiscard]] f32 getWaveNumber(f32 omega) const noexcept {
        // Deep water: k = omega^2 / g
        // Shallow water correction using dispersion relation
        f32 k = omega * omega / WaterConfig::GRAVITY;
        
        // Iterate for better accuracy (shallow water)
        if (depth < 50.0f) {
            for (int i = 0; i < 3; ++i) {
                f32 tanh_kd = std::tanh(k * depth);
                k = omega * omega / (WaterConfig::GRAVITY * tanh_kd);
            }
        }
        
        return k;
    }
    
    /// Calculate Phillips spectrum at wave vector
    [[nodiscard]] f32 calculatePhillipsSpectrum(const Vec2& k) const noexcept {
        f32 kLength = std::sqrt(k.x * k.x + k.y * k.y);
        if (kLength < 0.0001f) return 0.0f;
        
        Vec2 kNorm = k * (1.0f / kLength);
        f32 L = windSpeed * windSpeed / WaterConfig::GRAVITY;
        f32 kL = kLength * L;
        
        f32 philips = amplitude * std::exp(-1.0f / (kL * kL)) / (kLength * kLength * kLength * kLength);
        
        // Directional spreading
        f32 kDotW = kNorm.x * windDirection.x + kNorm.y * windDirection.y;
        philips *= kDotW * kDotW;
        
        // Suppress small waves
        f32 damping = std::exp(-kLength * kLength * waveDamping * waveDamping);
        
        return philips * damping;
    }
    
    /// Factory for calm conditions
    static OceanCascade calm() noexcept {
        OceanCascade cascade;
        cascade.windSpeed = 3.0f;
        cascade.amplitude = 0.3f;
        return cascade;
    }
    
    /// Factory for moderate conditions
    static OceanCascade moderate() noexcept {
        OceanCascade cascade;
        cascade.windSpeed = 10.0f;
        cascade.amplitude = 1.0f;
        return cascade;
    }
    
    /// Factory for stormy conditions
    static OceanCascade stormy() noexcept {
        OceanCascade cascade;
        cascade.windSpeed = 25.0f;
        cascade.amplitude = 3.0f;
        cascade.waveDamping = 0.0001f;
        return cascade;
    }
};

/**
 * @brief Water optical properties
 */
struct WaterOpticalProperties {
    Vec3 scatteringColor{0.0f, 0.05f, 0.1f};    ///< Scattering color (deep)
    Vec3 absorptionColor{0.1f, 0.3f, 0.4f};     ///< Absorption color (shallow)
    f32 turbidity{0.1f};                         ///< Water clarity (0=clear, 1=murky)
    f32 refractionIndex{1.333f};                 ///< Index of refraction
    f32 scatteringCoeff{0.05f};                  ///< Scattering coefficient
    f32 absorptionCoeff{0.45f};                  ///< Absorption coefficient
    f32 visibility{20.0f};                       ///< Underwater visibility (meters)
    f32 subsurfaceIntensity{1.0f};               ///< SSS intensity
    f32 sunlightPenetration{3.0f};               ///< Depth of sunlight penetration
    
    /// Calculate extinction at depth
    [[nodiscard]] Vec3 calculateExtinction(f32 depth) const noexcept {
        f32 extinction = std::exp(-absorptionCoeff * depth);
        return absorptionColor * extinction + scatteringColor * (1.0f - extinction);
    }
    
    /// Calculate Fresnel at angle
    [[nodiscard]] f32 calculateFresnel(f32 cosTheta) const noexcept {
        f32 f0 = WaterConfig::WATER_F0;
        return f0 + (1.0f - f0) * std::pow(1.0f - cosTheta, 5.0f);
    }
    
    /// Factory for clear ocean
    static WaterOpticalProperties clearOcean() noexcept {
        WaterOpticalProperties props;
        props.scatteringColor = Vec3{0.0f, 0.1f, 0.2f};
        props.absorptionColor = Vec3{0.1f, 0.4f, 0.6f};
        props.turbidity = 0.05f;
        props.visibility = 40.0f;
        return props;
    }
    
    /// Factory for tropical water
    static WaterOpticalProperties tropical() noexcept {
        WaterOpticalProperties props;
        props.scatteringColor = Vec3{0.1f, 0.3f, 0.4f};
        props.absorptionColor = Vec3{0.2f, 0.6f, 0.7f};
        props.turbidity = 0.02f;
        props.visibility = 60.0f;
        return props;
    }
    
    /// Factory for murky water
    static WaterOpticalProperties murky() noexcept {
        WaterOpticalProperties props;
        props.scatteringColor = Vec3{0.15f, 0.12f, 0.08f};
        props.absorptionColor = Vec3{0.3f, 0.25f, 0.2f};
        props.turbidity = 0.8f;
        props.visibility = 3.0f;
        return props;
    }
    
    /// Factory for pool water
    static WaterOpticalProperties pool() noexcept {
        WaterOpticalProperties props;
        props.scatteringColor = Vec3{0.1f, 0.2f, 0.4f};
        props.absorptionColor = Vec3{0.15f, 0.5f, 0.8f};
        props.turbidity = 0.0f;
        props.visibility = 100.0f;
        return props;
    }
};

/**
 * @brief Foam settings
 */
struct FoamSettings {
    FoamMode mode{FoamMode::Jacobian};
    f32 threshold{0.35f};                ///< Jacobian threshold for foam generation
    f32 intensity{1.0f};                 ///< Foam intensity multiplier
    f32 coverage{0.5f};                  ///< Base foam coverage
    f32 persistence{0.95f};              ///< Foam decay rate per frame
    f32 shoreFoamDistance{10.0f};        ///< Distance from shore for foam
    f32 shoreFoamIntensity{1.5f};        ///< Shore foam intensity
    Vec3 foamColor{1.0f, 1.0f, 1.0f};    ///< Foam tint color
    f32 foamRoughness{0.8f};             ///< Foam surface roughness
    
    /// Factory for calm water
    static FoamSettings calm() noexcept {
        FoamSettings settings;
        settings.threshold = 0.5f;
        settings.coverage = 0.1f;
        return settings;
    }
    
    /// Factory for rough water
    static FoamSettings rough() noexcept {
        FoamSettings settings;
        settings.threshold = 0.25f;
        settings.coverage = 0.7f;
        settings.intensity = 1.5f;
        return settings;
    }
};

/**
 * @brief Caustics settings
 */
struct CausticsSettings {
    bool enabled{true};
    f32 intensity{1.0f};                 ///< Caustics brightness
    f32 scale{2.0f};                     ///< Pattern scale
    f32 speed{0.5f};                     ///< Animation speed
    f32 maxDepth{10.0f};                 ///< Maximum depth for caustics
    f32 focusDepth{2.0f};                ///< Depth of maximum intensity
    Vec3 tintColor{1.0f, 1.0f, 1.0f};    ///< Caustics tint
    bool useRayTracing{false};           ///< Use ray traced caustics
    
    /// Calculate intensity at depth
    [[nodiscard]] f32 getIntensityAtDepth(f32 depth) const noexcept {
        if (depth > maxDepth || depth < 0.0f) return 0.0f;
        
        f32 falloff;
        if (depth < focusDepth) {
            falloff = depth / focusDepth;
        } else {
            falloff = 1.0f - (depth - focusDepth) / (maxDepth - focusDepth);
        }
        
        return intensity * falloff;
    }
};

/**
 * @brief Water body description
 */
struct WaterBodyDesc {
    std::string name;
    WaterBodyType type{WaterBodyType::Ocean};
    Vec3 position{0.0f, 0.0f, 0.0f};     ///< World position
    Vec2 size{1000.0f, 1000.0f};         ///< Surface size (meters)
    f32 elevation{0.0f};                 ///< Water surface elevation
    f32 depth{100.0f};                   ///< Water depth
    Vec2 flowDirection{0.0f, 0.0f};      ///< Flow direction (for rivers)
    f32 flowSpeed{0.0f};                 ///< Flow speed (m/s)
    bool infinite{true};                 ///< Infinite ocean vs bounded
    
    /// Factory for ocean
    static WaterBodyDesc ocean(f32 elevation = 0.0f) noexcept {
        WaterBodyDesc desc;
        desc.name = "Ocean";
        desc.type = WaterBodyType::Ocean;
        desc.elevation = elevation;
        desc.depth = WaterConfig::DEFAULT_DEPTH;
        desc.infinite = true;
        return desc;
    }
    
    /// Factory for lake
    static WaterBodyDesc lake(const Vec3& pos, const Vec2& sz, f32 dp) noexcept {
        WaterBodyDesc desc;
        desc.name = "Lake";
        desc.type = WaterBodyType::Lake;
        desc.position = pos;
        desc.size = sz;
        desc.elevation = pos.y;
        desc.depth = dp;
        desc.infinite = false;
        return desc;
    }
    
    /// Factory for river
    static WaterBodyDesc river(const Vec3& pos, f32 width, f32 dp, const Vec2& flowDir, f32 flowSpd) noexcept {
        WaterBodyDesc desc;
        desc.name = "River";
        desc.type = WaterBodyType::River;
        desc.position = pos;
        desc.size = Vec2{width, 1000.0f};
        desc.elevation = pos.y;
        desc.depth = dp;
        desc.flowDirection = flowDir.normalized();
        desc.flowSpeed = flowSpd;
        desc.infinite = false;
        return desc;
    }
};

/**
 * @brief GPU water data for shader consumption
 */
struct alignas(16) GPUWaterData {
    Mat4 worldTransform;
    Vec4 waterParams;                    ///< x=elevation, y=depth, z=time, w=windSpeed
    Vec4 opticalParams;                  ///< x=turbidity, y=refractionIndex, z=visibility, w=sssIntensity
    Vec4 scatterColor;                   ///< xyz=color, w=scatterCoeff
    Vec4 absorbColor;                    ///< xyz=color, w=absorbCoeff
    Vec4 foamParams;                     ///< x=threshold, y=intensity, z=coverage, w=persistence
    Vec4 flowParams;                     ///< xy=direction, z=speed, w=unused
    Vec4 cascadeScales[WaterConfig::MAX_CASCADES]; ///< Per-cascade tile sizes
    
    static GPUWaterData create(f32 elevation, f32 depth, const WaterOpticalProperties& optical) noexcept {
        GPUWaterData data;
        data.worldTransform = Mat4::identity();
        data.waterParams = Vec4{elevation, depth, 0.0f, WaterConfig::DEFAULT_WIND_SPEED};
        data.opticalParams = Vec4{optical.turbidity, optical.refractionIndex, optical.visibility, optical.subsurfaceIntensity};
        data.scatterColor = Vec4{optical.scatteringColor.x, optical.scatteringColor.y, optical.scatteringColor.z, optical.scatteringCoeff};
        data.absorbColor = Vec4{optical.absorptionColor.x, optical.absorptionColor.y, optical.absorptionColor.z, optical.absorptionCoeff};
        data.foamParams = Vec4{0.35f, 1.0f, 0.5f, 0.95f};
        data.flowParams = Vec4{0.0f, 0.0f, 0.0f, 0.0f};
        return data;
    }
};

/**
 * @brief Water settings
 */
struct WaterSettings {
    WaterTechnique technique{WaterTechnique::FFT};
    WaterQuality quality{WaterQuality::High};
    WaterReflectionMode reflectionMode{WaterReflectionMode::Hybrid};
    u32 fftResolution{512};              ///< FFT grid resolution
    u32 cascadeCount{3};                 ///< Number of detail cascades
    f32 tessellationFactor{64.0f};       ///< GPU tessellation factor
    f32 tessellationMinDistance{5.0f};   ///< Distance for max tessellation
    f32 tessellationMaxDistance{200.0f}; ///< Distance for min tessellation
    bool enableReflections{true};
    bool enableRefractions{true};
    bool enableFoam{true};
    bool enableCaustics{true};
    bool enableSubsurface{true};
    bool enableDepthFog{true};
    bool enableFlowmap{false};           ///< Flowmap-based animation
    bool castShadows{false};
    bool receiveShadows{true};
    
    /// Factory for low quality
    static WaterSettings low() noexcept {
        WaterSettings settings;
        settings.quality = WaterQuality::Low;
        settings.technique = WaterTechnique::Gerstner;
        settings.fftResolution = 128;
        settings.cascadeCount = 1;
        settings.tessellationFactor = 16.0f;
        settings.reflectionMode = WaterReflectionMode::Cubemap;
        settings.enableCaustics = false;
        settings.enableSubsurface = false;
        return settings;
    }
    
    /// Factory for medium quality
    static WaterSettings medium() noexcept {
        WaterSettings settings;
        settings.quality = WaterQuality::Medium;
        settings.fftResolution = 256;
        settings.cascadeCount = 2;
        settings.tessellationFactor = 32.0f;
        settings.reflectionMode = WaterReflectionMode::SSR;
        return settings;
    }
    
    /// Factory for high quality
    static WaterSettings high() noexcept {
        WaterSettings settings;
        settings.quality = WaterQuality::High;
        settings.fftResolution = 512;
        settings.cascadeCount = 3;
        settings.tessellationFactor = 64.0f;
        settings.reflectionMode = WaterReflectionMode::Hybrid;
        return settings;
    }
    
    /// Factory for ultra quality
    static WaterSettings ultra() noexcept {
        WaterSettings settings;
        settings.quality = WaterQuality::Ultra;
        settings.fftResolution = 1024;
        settings.cascadeCount = 4;
        settings.tessellationFactor = 128.0f;
        settings.reflectionMode = WaterReflectionMode::Raytraced;
        return settings;
    }
};

// =============================================================================
// WATER MANAGER
// =============================================================================

/**
 * @brief Water rendering statistics
 */
struct WaterStats {
    u32 waterBodies{0};
    u32 visibleTiles{0};
    u32 triangleCount{0};
    u32 fftDispatches{0};
    u32 reflectionDrawCalls{0};
    f32 fftTimeMs{0.0f};
    f32 reflectionTimeMs{0.0f};
    f32 renderTimeMs{0.0f};
    
    void reset() noexcept {
        visibleTiles = 0;
        triangleCount = 0;
        fftDispatches = 0;
        reflectionDrawCalls = 0;
        fftTimeMs = 0.0f;
        reflectionTimeMs = 0.0f;
        renderTimeMs = 0.0f;
    }
};

/**
 * @brief Water system manager
 * 
 * Manages water bodies, ocean simulation, and rendering
 */
class WaterManager {
public:
    /// Singleton access
    static WaterManager& instance() noexcept {
        static WaterManager mgr;
        return mgr;
    }
    
    // -------------------------------------------------------------------------
    // INITIALIZATION
    // -------------------------------------------------------------------------
    
    /// Initialize water system
    bool initialize() noexcept {
        if (m_initialized) return true;
        
        // Initialize default cascade
        m_cascades.resize(m_settings.cascadeCount);
        for (u32 i = 0; i < m_settings.cascadeCount; ++i) {
            m_cascades[i] = OceanCascade::moderate();
            m_cascades[i].tileSize = WaterConfig::DEFAULT_TILE_SIZE * std::pow(4.0f, static_cast<f32>(i));
        }
        
        // Initialize Gerstner waves
        initializeGerstnerWaves();
        
        m_initialized = true;
        return true;
    }
    
    /// Shutdown water system
    void shutdown() noexcept {
        m_waterBodies.clear();
        m_cascades.clear();
        m_gerstnerWaves.clear();
        m_initialized = false;
    }
    
    /// Check if initialized
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }
    
    // -------------------------------------------------------------------------
    // WATER BODIES
    // -------------------------------------------------------------------------
    
    /// Add water body
    u32 addWaterBody(const WaterBodyDesc& desc) noexcept {
        u32 id = static_cast<u32>(m_waterBodies.size());
        m_waterBodies.push_back(desc);
        m_stats.waterBodies = static_cast<u32>(m_waterBodies.size());
        return id;
    }
    
    /// Remove water body
    bool removeWaterBody(u32 id) noexcept {
        if (id >= m_waterBodies.size()) return false;
        m_waterBodies.erase(m_waterBodies.begin() + id);
        m_stats.waterBodies = static_cast<u32>(m_waterBodies.size());
        return true;
    }
    
    /// Get water body
    [[nodiscard]] const WaterBodyDesc* getWaterBody(u32 id) const noexcept {
        return id < m_waterBodies.size() ? &m_waterBodies[id] : nullptr;
    }
    
    /// Get water body count
    [[nodiscard]] u32 getWaterBodyCount() const noexcept {
        return static_cast<u32>(m_waterBodies.size());
    }
    
    // -------------------------------------------------------------------------
    // WAVE SIMULATION
    // -------------------------------------------------------------------------
    
    /// Update wave simulation
    void update(f32 deltaTime) noexcept {
        m_time += deltaTime;
        
        // Update Gerstner wave phases
        for (auto& wave : m_gerstnerWaves) {
            wave.phase = std::fmod(wave.phase + wave.getAngularFrequency() * wave.speed * deltaTime, 6.28318f);
        }
    }
    
    /// Get water height at world position
    [[nodiscard]] f32 getHeightAt(f32 worldX, f32 worldZ) const noexcept {
        Vec2 pos{worldX, worldZ};
        f32 height = 0.0f;
        
        // Sum Gerstner waves
        for (const auto& wave : m_gerstnerWaves) {
            Vec3 disp = wave.calculateDisplacement(pos, m_time);
            height += disp.y;
        }
        
        // Add base elevation from primary water body
        if (!m_waterBodies.empty()) {
            height += m_waterBodies[0].elevation;
        }
        
        return height;
    }
    
    /// Get water displacement at world position (full 3D)
    [[nodiscard]] Vec3 getDisplacementAt(f32 worldX, f32 worldZ) const noexcept {
        Vec2 pos{worldX, worldZ};
        Vec3 displacement{0.0f, 0.0f, 0.0f};
        
        for (const auto& wave : m_gerstnerWaves) {
            displacement = displacement + wave.calculateDisplacement(pos, m_time);
        }
        
        return displacement;
    }
    
    /// Get water normal at world position
    [[nodiscard]] Vec3 getNormalAt(f32 worldX, f32 worldZ) const noexcept {
        f32 delta = 0.1f;
        
        f32 hL = getHeightAt(worldX - delta, worldZ);
        f32 hR = getHeightAt(worldX + delta, worldZ);
        f32 hD = getHeightAt(worldX, worldZ - delta);
        f32 hU = getHeightAt(worldX, worldZ + delta);
        
        Vec3 normal;
        normal.x = hL - hR;
        normal.z = hD - hU;
        normal.y = 2.0f * delta;
        
        return normal.normalized();
    }
    
    // -------------------------------------------------------------------------
    // CASCADES
    // -------------------------------------------------------------------------
    
    /// Get cascade
    [[nodiscard]] const OceanCascade* getCascade(u32 index) const noexcept {
        return index < m_cascades.size() ? &m_cascades[index] : nullptr;
    }
    
    /// Set cascade
    void setCascade(u32 index, const OceanCascade& cascade) noexcept {
        if (index < m_cascades.size()) {
            m_cascades[index] = cascade;
        }
    }
    
    /// Get cascade count
    [[nodiscard]] u32 getCascadeCount() const noexcept {
        return static_cast<u32>(m_cascades.size());
    }
    
    // -------------------------------------------------------------------------
    // GERSTNER WAVES
    // -------------------------------------------------------------------------
    
    /// Add Gerstner wave
    void addGerstnerWave(const GerstnerWave& wave) noexcept {
        m_gerstnerWaves.push_back(wave);
    }
    
    /// Clear Gerstner waves
    void clearGerstnerWaves() noexcept {
        m_gerstnerWaves.clear();
    }
    
    /// Get Gerstner wave count
    [[nodiscard]] u32 getGerstnerWaveCount() const noexcept {
        return static_cast<u32>(m_gerstnerWaves.size());
    }
    
    /// Get Gerstner wave
    [[nodiscard]] const GerstnerWave* getGerstnerWave(u32 index) const noexcept {
        return index < m_gerstnerWaves.size() ? &m_gerstnerWaves[index] : nullptr;
    }
    
    // -------------------------------------------------------------------------
    // SETTINGS
    // -------------------------------------------------------------------------
    
    /// Set water settings
    void setSettings(const WaterSettings& settings) noexcept {
        m_settings = settings;
        
        // Resize cascades if needed
        if (m_cascades.size() != settings.cascadeCount) {
            m_cascades.resize(settings.cascadeCount);
            for (u32 i = 0; i < settings.cascadeCount; ++i) {
                m_cascades[i] = OceanCascade::moderate();
                m_cascades[i].resolution = settings.fftResolution;
                m_cascades[i].tileSize = WaterConfig::DEFAULT_TILE_SIZE * std::pow(4.0f, static_cast<f32>(i));
            }
        }
    }
    
    /// Get water settings
    [[nodiscard]] const WaterSettings& getSettings() const noexcept {
        return m_settings;
    }
    
    /// Set optical properties
    void setOpticalProperties(const WaterOpticalProperties& props) noexcept {
        m_opticalProps = props;
    }
    
    /// Get optical properties
    [[nodiscard]] const WaterOpticalProperties& getOpticalProperties() const noexcept {
        return m_opticalProps;
    }
    
    /// Set foam settings
    void setFoamSettings(const FoamSettings& settings) noexcept {
        m_foamSettings = settings;
    }
    
    /// Get foam settings
    [[nodiscard]] const FoamSettings& getFoamSettings() const noexcept {
        return m_foamSettings;
    }
    
    /// Set caustics settings
    void setCausticsSettings(const CausticsSettings& settings) noexcept {
        m_causticsSettings = settings;
    }
    
    /// Get caustics settings
    [[nodiscard]] const CausticsSettings& getCausticsSettings() const noexcept {
        return m_causticsSettings;
    }
    
    // -------------------------------------------------------------------------
    // GPU DATA
    // -------------------------------------------------------------------------
    
    /// Get GPU water data
    [[nodiscard]] GPUWaterData getGPUData() const noexcept {
        f32 elevation = !m_waterBodies.empty() ? m_waterBodies[0].elevation : 0.0f;
        f32 depth = !m_waterBodies.empty() ? m_waterBodies[0].depth : WaterConfig::DEFAULT_DEPTH;
        
        GPUWaterData data = GPUWaterData::create(elevation, depth, m_opticalProps);
        data.waterParams.z = m_time;
        data.waterParams.w = !m_cascades.empty() ? m_cascades[0].windSpeed : WaterConfig::DEFAULT_WIND_SPEED;
        
        data.foamParams = Vec4{m_foamSettings.threshold, m_foamSettings.intensity,
                               m_foamSettings.coverage, m_foamSettings.persistence};
        
        for (u32 i = 0; i < m_cascades.size() && i < WaterConfig::MAX_CASCADES; ++i) {
            data.cascadeScales[i] = Vec4{m_cascades[i].tileSize, m_cascades[i].amplitude,
                                         m_cascades[i].windSpeed, 0.0f};
        }
        
        return data;
    }
    
    /// Get current time
    [[nodiscard]] f32 getTime() const noexcept {
        return m_time;
    }
    
    // -------------------------------------------------------------------------
    // STATISTICS
    // -------------------------------------------------------------------------
    
    /// Get statistics
    [[nodiscard]] const WaterStats& getStats() const noexcept {
        return m_stats;
    }
    
    /// Begin frame
    void beginFrame() noexcept {
        m_stats.reset();
    }
    
    /// End frame
    void endFrame() noexcept {
        // Calculate frame stats
        m_stats.fftDispatches = static_cast<u32>(m_cascades.size());
    }

private:
    WaterManager() = default;
    
    /// Initialize default Gerstner waves
    void initializeGerstnerWaves() noexcept {
        m_gerstnerWaves.clear();
        
        // Primary wave
        GerstnerWave primary = GerstnerWave::large(Vec2{1.0f, 0.3f});
        m_gerstnerWaves.push_back(primary);
        
        // Secondary waves
        m_gerstnerWaves.push_back(GerstnerWave::medium(Vec2{0.8f, 0.6f}));
        m_gerstnerWaves.push_back(GerstnerWave::medium(Vec2{-0.5f, 0.9f}));
        
        // Small detail waves
        m_gerstnerWaves.push_back(GerstnerWave::small(Vec2{0.3f, 1.0f}));
        m_gerstnerWaves.push_back(GerstnerWave::small(Vec2{-0.7f, 0.4f}));
        m_gerstnerWaves.push_back(GerstnerWave::small(Vec2{0.9f, -0.2f}));
    }
    
    bool m_initialized{false};
    f32 m_time{0.0f};
    
    WaterSettings m_settings{};
    WaterOpticalProperties m_opticalProps{WaterOpticalProperties::clearOcean()};
    FoamSettings m_foamSettings{};
    CausticsSettings m_causticsSettings{};
    
    std::vector<WaterBodyDesc> m_waterBodies;
    std::vector<OceanCascade> m_cascades;
    std::vector<GerstnerWave> m_gerstnerWaves;
    
    WaterStats m_stats{};
};

} // namespace nova
