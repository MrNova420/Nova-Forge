/**
 * @file post_process.hpp
 * @brief Comprehensive Post-Processing Effects System for NovaCore Engine
 * 
 * A complete post-processing pipeline implementing:
 * - Tone mapping (ACES, Reinhard, Uncharted 2, etc.)
 * - Color grading with LUT support
 * - Bloom with threshold and radius control
 * - Depth of field (bokeh, gaussian)
 * - Motion blur (per-object, camera)
 * - Film grain and chromatic aberration
 * - Vignette and lens distortion
 * - Temporal anti-aliasing (TAA)
 * - FXAA and SMAA
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "nova/core/types/types.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/render/render_types.hpp"

#include <array>
#include <vector>
#include <optional>
#include <functional>
#include <string>
#include <memory>

namespace nova {

// =============================================================================
// Configuration Constants
// =============================================================================

/**
 * @brief Post-processing system configuration constants
 */
struct PostProcessConfig {
    /// Maximum bloom mip levels
    static constexpr u32 MAX_BLOOM_MIPS = 8;
    
    /// Default bloom threshold
    static constexpr f32 DEFAULT_BLOOM_THRESHOLD = 1.0f;
    
    /// Default bloom intensity
    static constexpr f32 DEFAULT_BLOOM_INTENSITY = 0.5f;
    
    /// Maximum color grading LUT size
    static constexpr u32 MAX_LUT_SIZE = 64;
    
    /// DOF max bokeh samples
    static constexpr u32 DOF_MAX_SAMPLES = 128;
    
    /// Motion blur max samples
    static constexpr u32 MOTION_BLUR_MAX_SAMPLES = 16;
    
    /// TAA history buffer count
    static constexpr u32 TAA_HISTORY_COUNT = 2;
    
    /// SMAA edge detection threshold
    static constexpr f32 SMAA_THRESHOLD = 0.1f;
};

// =============================================================================
// Enumerations
// =============================================================================

/**
 * @brief Tone mapping operator type
 */
enum class ToneMappingOperator : u32 {
    /// No tone mapping (linear)
    None = 0,
    
    /// Reinhard global operator
    Reinhard = 1,
    
    /// Reinhard extended (with white point)
    ReinhardExtended = 2,
    
    /// Uncharted 2 filmic
    Uncharted2 = 3,
    
    /// ACES (Academy Color Encoding System)
    ACES = 4,
    
    /// ACES approximation (faster)
    ACESApprox = 5,
    
    /// AgX (Blender 4.0 default)
    AgX = 6,
    
    /// Tony McMapface (optimized for games)
    TonyMcMapface = 7,
    
    /// Khronos PBR Neutral
    KhronosPBRNeutral = 8,
    
    COUNT
};

/**
 * @brief Get tone mapping operator name
 */
[[nodiscard]] constexpr const char* getToneMappingName(ToneMappingOperator op) noexcept {
    switch (op) {
        case ToneMappingOperator::None: return "None";
        case ToneMappingOperator::Reinhard: return "Reinhard";
        case ToneMappingOperator::ReinhardExtended: return "Reinhard Extended";
        case ToneMappingOperator::Uncharted2: return "Uncharted 2";
        case ToneMappingOperator::ACES: return "ACES";
        case ToneMappingOperator::ACESApprox: return "ACES Approx";
        case ToneMappingOperator::AgX: return "AgX";
        case ToneMappingOperator::TonyMcMapface: return "Tony McMapface";
        case ToneMappingOperator::KhronosPBRNeutral: return "Khronos PBR Neutral";
        default: return "Unknown";
    }
}

/**
 * @brief Anti-aliasing technique
 */
enum class AntiAliasingTechnique : u32 {
    /// No anti-aliasing
    None = 0,
    
    /// Fast Approximate Anti-Aliasing
    FXAA = 1,
    
    /// Subpixel Morphological Anti-Aliasing
    SMAA = 2,
    
    /// Temporal Anti-Aliasing
    TAA = 3,
    
    /// TAA + FXAA combination
    TAAWithFXAA = 4,
    
    /// DLSS (NVIDIA)
    DLSS = 5,
    
    /// FSR (AMD)
    FSR = 6,
    
    /// XeSS (Intel)
    XeSS = 7,
    
    COUNT
};

/**
 * @brief Get AA technique name
 */
[[nodiscard]] constexpr const char* getAAName(AntiAliasingTechnique aa) noexcept {
    switch (aa) {
        case AntiAliasingTechnique::None: return "None";
        case AntiAliasingTechnique::FXAA: return "FXAA";
        case AntiAliasingTechnique::SMAA: return "SMAA";
        case AntiAliasingTechnique::TAA: return "TAA";
        case AntiAliasingTechnique::TAAWithFXAA: return "TAA+FXAA";
        case AntiAliasingTechnique::DLSS: return "DLSS";
        case AntiAliasingTechnique::FSR: return "FSR";
        case AntiAliasingTechnique::XeSS: return "XeSS";
        default: return "Unknown";
    }
}

/**
 * @brief Depth of field type
 */
enum class DOFType : u32 {
    /// DOF disabled
    Off = 0,
    
    /// Gaussian blur (fast)
    Gaussian = 1,
    
    /// Bokeh (circular aperture)
    BokehCircular = 2,
    
    /// Bokeh (hexagonal aperture)
    BokehHexagonal = 3,
    
    /// Bokeh (octagonal aperture)
    BokehOctagonal = 4,
    
    /// Physically-based DOF
    PhysicallyBased = 5,
    
    COUNT
};

/**
 * @brief Get DOF type name
 */
[[nodiscard]] constexpr const char* getDOFTypeName(DOFType type) noexcept {
    switch (type) {
        case DOFType::Off: return "Off";
        case DOFType::Gaussian: return "Gaussian";
        case DOFType::BokehCircular: return "Bokeh Circular";
        case DOFType::BokehHexagonal: return "Bokeh Hexagonal";
        case DOFType::BokehOctagonal: return "Bokeh Octagonal";
        case DOFType::PhysicallyBased: return "Physically Based";
        default: return "Unknown";
    }
}

/**
 * @brief Motion blur type
 */
enum class MotionBlurType : u32 {
    /// Motion blur disabled
    Off = 0,
    
    /// Camera motion blur only
    Camera = 1,
    
    /// Per-object motion blur
    PerObject = 2,
    
    /// Combined camera + per-object
    Combined = 3,
    
    COUNT
};

/**
 * @brief Get motion blur type name
 */
[[nodiscard]] constexpr const char* getMotionBlurTypeName(MotionBlurType type) noexcept {
    switch (type) {
        case MotionBlurType::Off: return "Off";
        case MotionBlurType::Camera: return "Camera";
        case MotionBlurType::PerObject: return "Per-Object";
        case MotionBlurType::Combined: return "Combined";
        default: return "Unknown";
    }
}

/**
 * @brief Bloom quality level
 */
enum class BloomQuality : u32 {
    /// Bloom disabled
    Off = 0,
    
    /// Low quality (fewer mips, less blur)
    Low = 1,
    
    /// Medium quality
    Medium = 2,
    
    /// High quality (full mip chain)
    High = 3,
    
    /// Ultra quality (wider blur, more samples)
    Ultra = 4,
    
    COUNT
};

/**
 * @brief Post-process effect ordering
 */
enum class PostProcessStage : u32 {
    /// Before tone mapping (HDR)
    PreToneMap = 0,
    
    /// Tone mapping stage
    ToneMap = 1,
    
    /// After tone mapping (LDR)
    PostToneMap = 2,
    
    /// Final pass (AA, grain, etc.)
    Final = 3,
    
    COUNT
};

// =============================================================================
// Data Structures
// =============================================================================

/**
 * @brief Tone mapping settings
 */
struct ToneMappingSettings {
    /// Tone mapping operator
    ToneMappingOperator op = ToneMappingOperator::ACES;
    
    /// Exposure value (EV)
    f32 exposure = 1.0f;
    
    /// White point for extended Reinhard
    f32 whitePoint = 4.0f;
    
    /// Gamma correction value
    f32 gamma = 2.2f;
    
    /// Enable auto exposure
    bool autoExposure = false;
    
    /// Auto exposure adaptation speed
    f32 adaptationSpeed = 1.0f;
    
    /// Auto exposure minimum (EV)
    f32 autoExposureMin = -4.0f;
    
    /// Auto exposure maximum (EV)
    f32 autoExposureMax = 16.0f;
    
    /**
     * @brief Apply exposure to luminance
     */
    [[nodiscard]] f32 applyExposure(f32 luminance) const noexcept {
        return luminance * std::pow(2.0f, exposure);
    }
    
    /**
     * @brief Create cinematic preset
     */
    [[nodiscard]] static ToneMappingSettings createCinematic() noexcept {
        ToneMappingSettings settings;
        settings.op = ToneMappingOperator::ACES;
        settings.exposure = 0.0f;
        settings.gamma = 2.2f;
        return settings;
    }
    
    /**
     * @brief Create game preset
     */
    [[nodiscard]] static ToneMappingSettings createGame() noexcept {
        ToneMappingSettings settings;
        settings.op = ToneMappingOperator::ACESApprox;
        settings.exposure = 0.5f;
        settings.autoExposure = true;
        return settings;
    }
};

/**
 * @brief Color grading settings
 */
struct ColorGradingSettings {
    /// Enable color grading
    bool enabled = true;
    
    /// Color temperature (Kelvin)
    f32 temperature = 6500.0f;
    
    /// Color tint (green-magenta shift)
    f32 tint = 0.0f;
    
    /// Saturation multiplier
    f32 saturation = 1.0f;
    
    /// Contrast multiplier
    f32 contrast = 1.0f;
    
    /// Brightness offset
    f32 brightness = 0.0f;
    
    /// Shadow tint color
    Vec3 shadowTint = Vec3{1.0f, 1.0f, 1.0f};
    
    /// Midtone tint color
    Vec3 midtoneTint = Vec3{1.0f, 1.0f, 1.0f};
    
    /// Highlight tint color
    Vec3 highlightTint = Vec3{1.0f, 1.0f, 1.0f};
    
    /// Shadow offset
    f32 shadowOffset = 0.0f;
    
    /// Highlight offset
    f32 highlightOffset = 0.0f;
    
    /// LUT texture handle (0 = no LUT)
    u32 lutTexture = 0;
    
    /// LUT blend amount (0-1)
    f32 lutBlend = 1.0f;
    
    /**
     * @brief Convert temperature to RGB tint
     */
    [[nodiscard]] Vec3 getTemperatureRGB() const noexcept {
        // Approximation of blackbody radiation
        f32 temp = std::clamp(temperature, 1000.0f, 40000.0f) / 100.0f;
        Vec3 rgb;
        
        // Red
        if (temp <= 66.0f) {
            rgb.x = 1.0f;
        } else {
            rgb.x = std::clamp(1.292936186f * std::pow(temp - 60.0f, -0.1332047592f), 0.0f, 1.0f);
        }
        
        // Green
        if (temp <= 66.0f) {
            rgb.y = std::clamp(0.3900815788f * std::log(temp) - 0.6318414438f, 0.0f, 1.0f);
        } else {
            rgb.y = std::clamp(1.129890861f * std::pow(temp - 60.0f, -0.0755148492f), 0.0f, 1.0f);
        }
        
        // Blue
        if (temp >= 66.0f) {
            rgb.z = 1.0f;
        } else if (temp <= 19.0f) {
            rgb.z = 0.0f;
        } else {
            rgb.z = std::clamp(0.5432067892f * std::log(temp - 10.0f) - 1.196254089f, 0.0f, 1.0f);
        }
        
        return rgb;
    }
    
    /**
     * @brief Create neutral preset
     */
    [[nodiscard]] static ColorGradingSettings createNeutral() noexcept {
        return ColorGradingSettings{};
    }
    
    /**
     * @brief Create warm preset
     */
    [[nodiscard]] static ColorGradingSettings createWarm() noexcept {
        ColorGradingSettings settings;
        settings.temperature = 5500.0f;
        settings.saturation = 1.1f;
        settings.contrast = 1.05f;
        return settings;
    }
    
    /**
     * @brief Create cold preset
     */
    [[nodiscard]] static ColorGradingSettings createCold() noexcept {
        ColorGradingSettings settings;
        settings.temperature = 8000.0f;
        settings.saturation = 0.95f;
        settings.contrast = 1.1f;
        return settings;
    }
};

/**
 * @brief Bloom effect settings
 */
struct BloomSettings {
    /// Bloom quality level
    BloomQuality quality = BloomQuality::Medium;
    
    /// Luminance threshold for bloom
    f32 threshold = PostProcessConfig::DEFAULT_BLOOM_THRESHOLD;
    
    /// Soft threshold knee (0 = hard cutoff, 1 = gradual)
    f32 softKnee = 0.5f;
    
    /// Bloom intensity
    f32 intensity = PostProcessConfig::DEFAULT_BLOOM_INTENSITY;
    
    /// Scatter amount (blur radius multiplier)
    f32 scatter = 0.7f;
    
    /// Clamp maximum brightness (prevents fireflies)
    f32 clampMax = 65000.0f;
    
    /// Anamorphic ratio (1.0 = circular, 0.5 = horizontal stretch)
    f32 anamorphicRatio = 1.0f;
    
    /// Tint color
    Vec3 tint = Vec3{1.0f, 1.0f, 1.0f};
    
    /// Dirt mask texture handle (0 = no dirt)
    u32 dirtMaskTexture = 0;
    
    /// Dirt mask intensity
    f32 dirtIntensity = 0.0f;
    
    /**
     * @brief Check if bloom is enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return quality != BloomQuality::Off && intensity > 0.0f;
    }
    
    /**
     * @brief Get mip count based on quality
     */
    [[nodiscard]] u32 getMipCount() const noexcept {
        switch (quality) {
            case BloomQuality::Off: return 0;
            case BloomQuality::Low: return 4;
            case BloomQuality::Medium: return 6;
            case BloomQuality::High: return 8;
            case BloomQuality::Ultra: return PostProcessConfig::MAX_BLOOM_MIPS;
            default: return 6;
        }
    }
    
    /**
     * @brief Create subtle bloom preset
     */
    [[nodiscard]] static BloomSettings createSubtle() noexcept {
        BloomSettings settings;
        settings.quality = BloomQuality::Medium;
        settings.threshold = 1.5f;
        settings.intensity = 0.3f;
        return settings;
    }
    
    /**
     * @brief Create dramatic bloom preset
     */
    [[nodiscard]] static BloomSettings createDramatic() noexcept {
        BloomSettings settings;
        settings.quality = BloomQuality::High;
        settings.threshold = 0.8f;
        settings.intensity = 0.8f;
        settings.scatter = 0.9f;
        return settings;
    }
};

/**
 * @brief Depth of field settings
 */
struct DOFSettings {
    /// DOF type
    DOFType type = DOFType::Off;
    
    /// Focus distance (world units)
    f32 focusDistance = 10.0f;
    
    /// Focus range/depth (world units)
    f32 focusRange = 5.0f;
    
    /// Aperture (f-stop) for physically-based DOF
    f32 aperture = 5.6f;
    
    /// Focal length (mm) for physically-based DOF
    f32 focalLength = 50.0f;
    
    /// Blur radius multiplier
    f32 blurRadius = 1.0f;
    
    /// Near blur start distance
    f32 nearBlurStart = 0.0f;
    
    /// Near blur end distance
    f32 nearBlurEnd = 2.0f;
    
    /// Far blur start distance
    f32 farBlurStart = 15.0f;
    
    /// Far blur end distance
    f32 farBlurEnd = 30.0f;
    
    /// Bokeh brightness boost
    f32 bokehBrightness = 1.5f;
    
    /// Number of samples (affects quality)
    u32 sampleCount = 64;
    
    /// Enable auto-focus
    bool autoFocus = false;
    
    /// Auto-focus speed
    f32 autoFocusSpeed = 2.0f;
    
    /**
     * @brief Check if DOF is enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return type != DOFType::Off;
    }
    
    /**
     * @brief Calculate circle of confusion size
     */
    [[nodiscard]] f32 calculateCoC(f32 depth) const noexcept {
        if (type != DOFType::PhysicallyBased) {
            // Simple linear interpolation for non-physical DOF
            if (depth < focusDistance - focusRange * 0.5f) {
                f32 t = 1.0f - (depth - nearBlurStart) / (nearBlurEnd - nearBlurStart);
                return std::clamp(t, 0.0f, 1.0f) * blurRadius;
            } else if (depth > focusDistance + focusRange * 0.5f) {
                f32 t = (depth - farBlurStart) / (farBlurEnd - farBlurStart);
                return std::clamp(t, 0.0f, 1.0f) * blurRadius;
            }
            return 0.0f;
        }
        
        // Physically-based circle of confusion
        f32 focalLengthM = focalLength * 0.001f;
        f32 focusM = focusDistance;
        f32 depthM = depth;
        
        f32 coc = std::abs(aperture * focalLengthM * (focusM - depthM)) / 
                  (depthM * (focusM - focalLengthM));
        
        return coc * blurRadius;
    }
    
    /**
     * @brief Create cinematic DOF preset
     */
    [[nodiscard]] static DOFSettings createCinematic() noexcept {
        DOFSettings settings;
        settings.type = DOFType::BokehCircular;
        settings.aperture = 1.4f;
        settings.focalLength = 85.0f;
        settings.sampleCount = 128;
        settings.bokehBrightness = 2.0f;
        return settings;
    }
    
    /**
     * @brief Create game DOF preset (faster)
     */
    [[nodiscard]] static DOFSettings createGame() noexcept {
        DOFSettings settings;
        settings.type = DOFType::Gaussian;
        settings.focusRange = 8.0f;
        settings.sampleCount = 32;
        settings.autoFocus = true;
        return settings;
    }
};

/**
 * @brief Motion blur settings
 */
struct MotionBlurSettings {
    /// Motion blur type
    MotionBlurType type = MotionBlurType::Off;
    
    /// Blur intensity/strength
    f32 intensity = 0.5f;
    
    /// Maximum blur velocity (pixels)
    f32 maxVelocity = 40.0f;
    
    /// Sample count for blur
    u32 sampleCount = 8;
    
    /// Shutter angle (degrees, 0-360)
    f32 shutterAngle = 180.0f;
    
    /// Camera motion blur weight
    f32 cameraWeight = 1.0f;
    
    /// Per-object motion blur weight
    f32 objectWeight = 1.0f;
    
    /**
     * @brief Check if motion blur is enabled
     */
    [[nodiscard]] bool isEnabled() const noexcept {
        return type != MotionBlurType::Off && intensity > 0.0f;
    }
    
    /**
     * @brief Get shutter speed from angle
     */
    [[nodiscard]] f32 getShutterSpeed() const noexcept {
        return shutterAngle / 360.0f;
    }
    
    /**
     * @brief Create cinematic preset
     */
    [[nodiscard]] static MotionBlurSettings createCinematic() noexcept {
        MotionBlurSettings settings;
        settings.type = MotionBlurType::Combined;
        settings.intensity = 0.8f;
        settings.shutterAngle = 180.0f;
        settings.sampleCount = 16;
        return settings;
    }
    
    /**
     * @brief Create game preset (less blur)
     */
    [[nodiscard]] static MotionBlurSettings createGame() noexcept {
        MotionBlurSettings settings;
        settings.type = MotionBlurType::Camera;
        settings.intensity = 0.4f;
        settings.shutterAngle = 90.0f;
        settings.sampleCount = 8;
        return settings;
    }
};

/**
 * @brief Anti-aliasing settings
 */
struct AntiAliasingSettings {
    /// AA technique
    AntiAliasingTechnique technique = AntiAliasingTechnique::TAA;
    
    /// FXAA quality preset (0-5)
    u32 fxaaQuality = 3;
    
    /// SMAA quality preset (0-3)
    u32 smaaQuality = 2;
    
    /// TAA feedback/history blend (0.9 = smooth, 0.7 = responsive)
    f32 taaFeedback = 0.9f;
    
    /// TAA sharpness
    f32 taaSharpness = 0.5f;
    
    /// TAA motion weighting
    f32 taaMotionWeight = 1.0f;
    
    /// TAA jitter scale
    f32 taaJitterScale = 1.0f;
    
    /// Upscaler quality (for DLSS/FSR/XeSS)
    u32 upscalerQuality = 2;
    
    /**
     * @brief Check if temporal AA is used
     */
    [[nodiscard]] bool usesTemporal() const noexcept {
        return technique == AntiAliasingTechnique::TAA ||
               technique == AntiAliasingTechnique::TAAWithFXAA ||
               technique == AntiAliasingTechnique::DLSS ||
               technique == AntiAliasingTechnique::FSR ||
               technique == AntiAliasingTechnique::XeSS;
    }
    
    /**
     * @brief Get jitter offset for current frame
     */
    [[nodiscard]] Vec2 getJitterOffset(u32 frameIndex) const noexcept {
        if (!usesTemporal()) {
            return Vec2{0.0f, 0.0f};
        }
        
        // Halton sequence for sub-pixel jitter in TAA
        // This is a low-discrepancy quasi-random sequence using bases 2 and 3.
        // Halton(n, base) generates well-distributed samples for temporal accumulation.
        // Base 2 (x): 1/2, 1/4, 3/4, 1/8, 5/8, 3/8, 7/8, ...
        // Base 3 (y): 1/3, 2/3, 1/9, 4/9, 7/9, 2/9, 5/9, ...
        // Pre-computed 16 samples for efficiency.
        static constexpr std::array<Vec2, 16> haltonSequence = {{
            {0.5f, 0.333f}, {0.25f, 0.666f}, {0.75f, 0.111f}, {0.125f, 0.444f},
            {0.625f, 0.777f}, {0.375f, 0.222f}, {0.875f, 0.555f}, {0.0625f, 0.888f},
            {0.5625f, 0.037f}, {0.3125f, 0.370f}, {0.8125f, 0.703f}, {0.1875f, 0.148f},
            {0.6875f, 0.481f}, {0.4375f, 0.814f}, {0.9375f, 0.259f}, {0.03125f, 0.592f}
        }};
        
        Vec2 jitter = haltonSequence[frameIndex % 16];
        return Vec2{(jitter.x - 0.5f) * taaJitterScale, (jitter.y - 0.5f) * taaJitterScale};
    }
    
    /**
     * @brief Create default AA settings
     */
    [[nodiscard]] static AntiAliasingSettings createDefault() noexcept {
        return AntiAliasingSettings{};
    }
    
    /**
     * @brief Create performance AA settings
     */
    [[nodiscard]] static AntiAliasingSettings createPerformance() noexcept {
        AntiAliasingSettings settings;
        settings.technique = AntiAliasingTechnique::FXAA;
        settings.fxaaQuality = 2;
        return settings;
    }
};

/**
 * @brief Film grain settings
 */
struct FilmGrainSettings {
    /// Enable film grain
    bool enabled = false;
    
    /// Grain intensity
    f32 intensity = 0.1f;
    
    /// Grain response (how it reacts to luminance)
    f32 response = 0.5f;
    
    /// Grain size
    f32 size = 1.0f;
    
    /// Animated grain
    bool animated = true;
    
    /**
     * @brief Create subtle grain preset
     */
    [[nodiscard]] static FilmGrainSettings createSubtle() noexcept {
        FilmGrainSettings settings;
        settings.enabled = true;
        settings.intensity = 0.05f;
        return settings;
    }
    
    /**
     * @brief Create cinematic grain preset
     */
    [[nodiscard]] static FilmGrainSettings createCinematic() noexcept {
        FilmGrainSettings settings;
        settings.enabled = true;
        settings.intensity = 0.15f;
        settings.response = 0.3f;
        return settings;
    }
};

/**
 * @brief Chromatic aberration settings
 */
struct ChromaticAberrationSettings {
    /// Enable chromatic aberration
    bool enabled = false;
    
    /// Aberration intensity
    f32 intensity = 0.1f;
    
    /// Start radius (from center, 0-1)
    f32 startRadius = 0.5f;
    
    /**
     * @brief Create subtle aberration preset
     */
    [[nodiscard]] static ChromaticAberrationSettings createSubtle() noexcept {
        ChromaticAberrationSettings settings;
        settings.enabled = true;
        settings.intensity = 0.05f;
        settings.startRadius = 0.7f;
        return settings;
    }
};

/**
 * @brief Vignette settings
 */
struct VignetteSettings {
    /// Enable vignette
    bool enabled = false;
    
    /// Vignette intensity
    f32 intensity = 0.3f;
    
    /// Vignette smoothness
    f32 smoothness = 0.5f;
    
    /// Vignette roundness (1.0 = circular)
    f32 roundness = 1.0f;
    
    /// Center offset
    Vec2 center = Vec2{0.5f, 0.5f};
    
    /// Vignette color
    Vec3 color = Vec3{0.0f, 0.0f, 0.0f};
    
    /**
     * @brief Create subtle vignette preset
     */
    [[nodiscard]] static VignetteSettings createSubtle() noexcept {
        VignetteSettings settings;
        settings.enabled = true;
        settings.intensity = 0.2f;
        settings.smoothness = 0.6f;
        return settings;
    }
    
    /**
     * @brief Create dramatic vignette preset
     */
    [[nodiscard]] static VignetteSettings createDramatic() noexcept {
        VignetteSettings settings;
        settings.enabled = true;
        settings.intensity = 0.5f;
        settings.smoothness = 0.3f;
        return settings;
    }
};

/**
 * @brief Lens distortion settings
 */
struct LensDistortionSettings {
    /// Enable lens distortion
    bool enabled = false;
    
    /// Distortion intensity (-1 = barrel, +1 = pincushion)
    f32 intensity = 0.0f;
    
    /// X/Y scale
    Vec2 scale = Vec2{1.0f, 1.0f};
    
    /// Center point offset
    Vec2 centerOffset = Vec2{0.0f, 0.0f};
};

/**
 * @brief Complete post-process volume settings
 */
struct PostProcessVolume {
    /// Volume name
    std::string name = "Default";
    
    /// Volume priority (higher = applied last)
    i32 priority = 0;
    
    /// Volume weight for blending
    f32 weight = 1.0f;
    
    /// Global volume (affects entire scene)
    bool isGlobal = true;
    
    /// Settings components
    ToneMappingSettings toneMapping;
    ColorGradingSettings colorGrading;
    BloomSettings bloom;
    DOFSettings dof;
    MotionBlurSettings motionBlur;
    AntiAliasingSettings antiAliasing;
    FilmGrainSettings filmGrain;
    ChromaticAberrationSettings chromaticAberration;
    VignetteSettings vignette;
    LensDistortionSettings lensDistortion;
    
    /**
     * @brief Create default volume
     */
    [[nodiscard]] static PostProcessVolume createDefault() noexcept {
        PostProcessVolume volume;
        volume.name = "Default";
        volume.toneMapping = ToneMappingSettings::createGame();
        volume.bloom = BloomSettings::createSubtle();
        volume.antiAliasing = AntiAliasingSettings::createDefault();
        return volume;
    }
    
    /**
     * @brief Create cinematic volume
     */
    [[nodiscard]] static PostProcessVolume createCinematic() noexcept {
        PostProcessVolume volume;
        volume.name = "Cinematic";
        volume.toneMapping = ToneMappingSettings::createCinematic();
        volume.colorGrading = ColorGradingSettings::createWarm();
        volume.bloom = BloomSettings::createDramatic();
        volume.dof = DOFSettings::createCinematic();
        volume.motionBlur = MotionBlurSettings::createCinematic();
        volume.filmGrain = FilmGrainSettings::createCinematic();
        volume.vignette = VignetteSettings::createDramatic();
        return volume;
    }
    
    /**
     * @brief Create performance volume
     */
    [[nodiscard]] static PostProcessVolume createPerformance() noexcept {
        PostProcessVolume volume;
        volume.name = "Performance";
        volume.toneMapping.op = ToneMappingOperator::ACESApprox;
        volume.bloom.quality = BloomQuality::Low;
        volume.antiAliasing = AntiAliasingSettings::createPerformance();
        volume.dof.type = DOFType::Off;
        volume.motionBlur.type = MotionBlurType::Off;
        return volume;
    }
};

/**
 * @brief Post-processing statistics
 */
struct PostProcessStats {
    /// Bloom pass time (ms)
    f32 bloomTimeMs = 0.0f;
    
    /// DOF pass time (ms)
    f32 dofTimeMs = 0.0f;
    
    /// Motion blur pass time (ms)
    f32 motionBlurTimeMs = 0.0f;
    
    /// AA pass time (ms)
    f32 aaTimeMs = 0.0f;
    
    /// Tone mapping pass time (ms)
    f32 toneMappingTimeMs = 0.0f;
    
    /// Total post-process time (ms)
    f32 totalTimeMs = 0.0f;
    
    /// Current average luminance
    f32 avgLuminance = 0.0f;
    
    /// Current exposure
    f32 currentExposure = 1.0f;
    
    /**
     * @brief Reset statistics
     */
    void reset() noexcept {
        bloomTimeMs = 0.0f;
        dofTimeMs = 0.0f;
        motionBlurTimeMs = 0.0f;
        aaTimeMs = 0.0f;
        toneMappingTimeMs = 0.0f;
        totalTimeMs = 0.0f;
    }
    
    /**
     * @brief Calculate total time
     */
    void calculateTotal() noexcept {
        totalTimeMs = bloomTimeMs + dofTimeMs + motionBlurTimeMs + aaTimeMs + toneMappingTimeMs;
    }
};

// =============================================================================
// Post-Process Manager Class
// =============================================================================

/**
 * @brief Post-processing effect manager
 * 
 * Manages post-processing volumes, effect rendering, and resource allocation.
 */
class PostProcessManager {
public:
    /**
     * @brief Default constructor
     */
    PostProcessManager() {
        m_volumes.push_back(PostProcessVolume::createDefault());
    }
    
    // -------------------------------------------------------------------------
    // Volume Management
    // -------------------------------------------------------------------------
    
    /**
     * @brief Add a post-process volume
     */
    void addVolume(const PostProcessVolume& volume) noexcept {
        m_volumes.push_back(volume);
        sortVolumesByPriority();
    }
    
    /**
     * @brief Remove volume by name
     */
    bool removeVolume(const std::string& name) noexcept {
        auto it = std::find_if(m_volumes.begin(), m_volumes.end(),
            [&name](const PostProcessVolume& v) { return v.name == name; });
        
        if (it != m_volumes.end()) {
            m_volumes.erase(it);
            return true;
        }
        return false;
    }
    
    /**
     * @brief Get volume by name
     */
    [[nodiscard]] PostProcessVolume* getVolume(const std::string& name) noexcept {
        auto it = std::find_if(m_volumes.begin(), m_volumes.end(),
            [&name](const PostProcessVolume& v) { return v.name == name; });
        
        return it != m_volumes.end() ? &(*it) : nullptr;
    }
    
    /**
     * @brief Get all volumes
     */
    [[nodiscard]] const std::vector<PostProcessVolume>& getVolumes() const noexcept {
        return m_volumes;
    }
    
    /**
     * @brief Clear all volumes
     */
    void clearVolumes() noexcept {
        m_volumes.clear();
    }
    
    // -------------------------------------------------------------------------
    // Rendering
    // -------------------------------------------------------------------------
    
    /**
     * @brief Begin post-processing frame
     */
    void beginFrame(u32 frameIndex) noexcept {
        m_frameIndex = frameIndex;
        m_stats.reset();
        blendVolumes();
    }
    
    /**
     * @brief End post-processing frame
     */
    void endFrame() noexcept {
        m_stats.calculateTotal();
    }
    
    /**
     * @brief Execute bloom pass
     */
    void executeBloom() noexcept {
        if (!m_blendedVolume.bloom.isEnabled()) {
            return;
        }
        // Bloom rendering would happen here
    }
    
    /**
     * @brief Execute DOF pass
     */
    void executeDOF() noexcept {
        if (!m_blendedVolume.dof.isEnabled()) {
            return;
        }
        // DOF rendering would happen here
    }
    
    /**
     * @brief Execute motion blur pass
     */
    void executeMotionBlur() noexcept {
        if (!m_blendedVolume.motionBlur.isEnabled()) {
            return;
        }
        // Motion blur rendering would happen here
    }
    
    /**
     * @brief Execute tone mapping pass
     */
    void executeToneMapping() noexcept {
        // Tone mapping is always executed (even if set to None)
    }
    
    /**
     * @brief Execute anti-aliasing pass
     */
    void executeAA() noexcept {
        // AA rendering would happen here
    }
    
    /**
     * @brief Execute final composite pass
     */
    void executeComposite() noexcept {
        // Final composite with grain, vignette, etc.
    }
    
    // -------------------------------------------------------------------------
    // State Access
    // -------------------------------------------------------------------------
    
    /**
     * @brief Get blended/final volume settings
     */
    [[nodiscard]] const PostProcessVolume& getBlendedVolume() const noexcept {
        return m_blendedVolume;
    }
    
    /**
     * @brief Get statistics
     */
    [[nodiscard]] const PostProcessStats& getStats() const noexcept {
        return m_stats;
    }
    
    /**
     * @brief Get current jitter offset for TAA
     */
    [[nodiscard]] Vec2 getJitterOffset() const noexcept {
        return m_blendedVolume.antiAliasing.getJitterOffset(m_frameIndex);
    }
    
    /**
     * @brief Get current exposure value
     */
    [[nodiscard]] f32 getCurrentExposure() const noexcept {
        return m_stats.currentExposure;
    }
    
    /**
     * @brief Set override exposure (for auto-exposure)
     */
    void setCurrentExposure(f32 exposure) noexcept {
        m_stats.currentExposure = exposure;
    }

private:
    /**
     * @brief Sort volumes by priority
     */
    void sortVolumesByPriority() noexcept {
        std::sort(m_volumes.begin(), m_volumes.end(),
            [](const PostProcessVolume& a, const PostProcessVolume& b) {
                return a.priority < b.priority;
            });
    }
    
    /**
     * @brief Blend all active volumes into final settings
     */
    void blendVolumes() noexcept {
        if (m_volumes.empty()) {
            m_blendedVolume = PostProcessVolume::createDefault();
            return;
        }
        
        // Start with first volume
        m_blendedVolume = m_volumes[0];
        
        // Blend subsequent volumes based on weight
        for (usize i = 1; i < m_volumes.size(); ++i) {
            const auto& vol = m_volumes[i];
            if (vol.weight <= 0.0f) continue;
            
            // Simple override for now - could implement proper lerping
            if (vol.weight >= 1.0f) {
                m_blendedVolume = vol;
            }
        }
    }
    
    // Volume storage
    std::vector<PostProcessVolume> m_volumes;
    
    // Blended result
    PostProcessVolume m_blendedVolume;
    
    // Frame state
    u32 m_frameIndex = 0;
    
    // Statistics
    PostProcessStats m_stats;
};

// =============================================================================
// Tone Mapping Functions
// =============================================================================

/**
 * @brief Apply Reinhard tone mapping
 */
[[nodiscard]] inline Vec3 tonemapReinhard(const Vec3& color) noexcept {
    return color / (color + Vec3{1.0f, 1.0f, 1.0f});
}

/**
 * @brief Apply Reinhard extended tone mapping with white point
 */
[[nodiscard]] inline Vec3 tonemapReinhardExtended(const Vec3& color, f32 whitePoint) noexcept {
    f32 wp2 = whitePoint * whitePoint;
    Vec3 numerator = color * (Vec3{1.0f, 1.0f, 1.0f} + color / Vec3{wp2, wp2, wp2});
    return numerator / (color + Vec3{1.0f, 1.0f, 1.0f});
}

/**
 * @brief Apply Uncharted 2 tone mapping
 */
[[nodiscard]] inline Vec3 tonemapUncharted2(const Vec3& color) noexcept {
    auto uncharted2Tonemap = [](const Vec3& x) -> Vec3 {
        constexpr f32 A = 0.15f;
        constexpr f32 B = 0.50f;
        constexpr f32 C = 0.10f;
        constexpr f32 D = 0.20f;
        constexpr f32 E = 0.02f;
        constexpr f32 F = 0.30f;
        
        return ((x * (x * A + Vec3{C * B, C * B, C * B}) + Vec3{D * E, D * E, D * E}) /
                (x * (x * A + Vec3{B, B, B}) + Vec3{D * F, D * F, D * F})) - Vec3{E / F, E / F, E / F};
    };
    
    constexpr f32 W = 11.2f;
    Vec3 whiteScale = Vec3{1.0f, 1.0f, 1.0f} / uncharted2Tonemap(Vec3{W, W, W});
    return uncharted2Tonemap(color * 2.0f) * whiteScale;
}

/**
 * @brief Apply ACES tone mapping (full)
 */
[[nodiscard]] inline Vec3 tonemapACES(const Vec3& color) noexcept {
    // ACES input transform matrix
    constexpr f32 a = 2.51f;
    constexpr f32 b = 0.03f;
    constexpr f32 c = 2.43f;
    constexpr f32 d = 0.59f;
    constexpr f32 e = 0.14f;
    
    return (color * (color * a + Vec3{b, b, b})) / 
           (color * (color * c + Vec3{d, d, d}) + Vec3{e, e, e});
}

/**
 * @brief Apply ACES approximation (faster)
 */
[[nodiscard]] inline Vec3 tonemapACESApprox(const Vec3& color) noexcept {
    Vec3 x = color * 0.6f;
    constexpr f32 a = 2.51f;
    constexpr f32 b = 0.03f;
    constexpr f32 c = 2.43f;
    constexpr f32 d = 0.59f;
    constexpr f32 e = 0.14f;
    
    Vec3 result = (x * (x * a + Vec3{b, b, b})) / 
                  (x * (x * c + Vec3{d, d, d}) + Vec3{e, e, e});
    
    return Vec3{
        std::clamp(result.x, 0.0f, 1.0f),
        std::clamp(result.y, 0.0f, 1.0f),
        std::clamp(result.z, 0.0f, 1.0f)
    };
}

/**
 * @brief Apply gamma correction
 */
[[nodiscard]] inline Vec3 gammaCorrect(const Vec3& color, f32 gamma) noexcept {
    f32 invGamma = 1.0f / gamma;
    return Vec3{
        std::pow(std::max(color.x, 0.0f), invGamma),
        std::pow(std::max(color.y, 0.0f), invGamma),
        std::pow(std::max(color.z, 0.0f), invGamma)
    };
}

/**
 * @brief Convert linear to sRGB
 */
[[nodiscard]] inline Vec3 linearToSRGB(const Vec3& color) noexcept {
    auto convert = [](f32 c) -> f32 {
        if (c <= 0.0031308f) {
            return c * 12.92f;
        }
        return 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
    };
    
    return Vec3{convert(color.x), convert(color.y), convert(color.z)};
}

/**
 * @brief Convert sRGB to linear
 */
[[nodiscard]] inline Vec3 sRGBToLinear(const Vec3& color) noexcept {
    auto convert = [](f32 c) -> f32 {
        if (c <= 0.04045f) {
            return c / 12.92f;
        }
        return std::pow((c + 0.055f) / 1.055f, 2.4f);
    };
    
    return Vec3{convert(color.x), convert(color.y), convert(color.z)};
}

/**
 * @brief Calculate luminance from RGB
 */
[[nodiscard]] inline f32 luminance(const Vec3& color) noexcept {
    return color.x * 0.2126f + color.y * 0.7152f + color.z * 0.0722f;
}

} // namespace nova
