/**
 * @file audio_types.hpp
 * @brief NovaCore Audio System™ - Core Type Definitions
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Defines the fundamental types for the audio system including:
 * - Audio formats and channel configurations
 * - Sound and music playback
 * - 3D spatial audio
 * - Audio effects and filters
 * - Mixer and bus routing
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace nova::audio {

using namespace nova;
using namespace nova::math;

// ============================================================================
// Configuration Constants
// ============================================================================

namespace AudioConfig {
    constexpr u32 DEFAULT_SAMPLE_RATE = 48000;
    constexpr u32 DEFAULT_BUFFER_SIZE = 1024;
    constexpr u32 MAX_CHANNELS = 32;
    constexpr u32 MAX_BUSES = 16;
    constexpr u32 MAX_EFFECTS_PER_BUS = 8;
    constexpr u32 MAX_LISTENERS = 4;
    
    constexpr f32 MIN_VOLUME = 0.0f;
    constexpr f32 MAX_VOLUME = 2.0f;
    constexpr f32 MIN_PITCH = 0.25f;
    constexpr f32 MAX_PITCH = 4.0f;
    
    constexpr f32 DEFAULT_DOPPLER_FACTOR = 1.0f;
    constexpr f32 SPEED_OF_SOUND = 343.0f;  // m/s at 20°C
}

// ============================================================================
// Audio Format Types
// ============================================================================

/**
 * @brief Audio sample format
 */
enum class SampleFormat : u8 {
    Int8,           // 8-bit signed integer
    Int16,          // 16-bit signed integer
    Int24,          // 24-bit signed integer
    Int32,          // 32-bit signed integer
    Float32,        // 32-bit floating point
    Float64         // 64-bit floating point
};

/**
 * @brief Channel layout configuration
 */
enum class ChannelLayout : u8 {
    Mono = 1,
    Stereo = 2,
    Surround21 = 3,     // 2.1 (stereo + sub)
    Quad = 4,           // Quadraphonic
    Surround51 = 6,     // 5.1 surround
    Surround71 = 8,     // 7.1 surround
    Ambisonic1 = 4,     // First-order ambisonics
    Ambisonic2 = 9,     // Second-order ambisonics
    Ambisonic3 = 16     // Third-order ambisonics
};

/**
 * @brief Audio codec/compression format
 */
enum class AudioCodec : u8 {
    Unknown,
    PCM,            // Uncompressed PCM
    WAV,            // WAV container (PCM)
    OGG,            // Ogg Vorbis
    MP3,            // MPEG Layer 3
    FLAC,           // Free Lossless Audio Codec
    AAC,            // Advanced Audio Coding
    OPUS,           // Opus codec
    ADPCM           // Adaptive PCM
};

/**
 * @brief Audio format descriptor
 */
struct AudioFormat {
    SampleFormat sampleFormat = SampleFormat::Float32;
    ChannelLayout channelLayout = ChannelLayout::Stereo;
    u32 sampleRate = AudioConfig::DEFAULT_SAMPLE_RATE;
    u32 bitDepth = 32;
    u32 channels = 2;
    
    u32 bytesPerSample() const;
    u32 bytesPerFrame() const;
    u32 bytesPerSecond() const;
    
    static AudioFormat stereo44100();
    static AudioFormat stereo48000();
    static AudioFormat mono44100();
    static AudioFormat surround51();
};

// ============================================================================
// Audio Resource Types
// ============================================================================

/**
 * @brief Sound playback mode
 */
enum class PlaybackMode : u8 {
    Once,           // Play once then stop
    Loop,           // Loop forever
    LoopCount,      // Loop N times
    PingPong        // Play forward then backward
};

/**
 * @brief Sound load mode
 */
enum class LoadMode : u8 {
    Streaming,      // Stream from disk (for music/long audio)
    Decompressed,   // Decompress fully into memory
    Compressed      // Keep compressed in memory, decompress on play
};

/**
 * @brief Sound priority for voice stealing
 */
enum class SoundPriority : u8 {
    Lowest = 0,
    Low = 64,
    Normal = 128,
    High = 192,
    Highest = 255
};

/**
 * @brief Handle to a playing sound instance
 */
struct SoundHandle {
    u32 id = 0;
    u32 generation = 0;
    
    bool isValid() const { return id != 0; }
    bool operator==(const SoundHandle& other) const {
        return id == other.id && generation == other.generation;
    }
    bool operator!=(const SoundHandle& other) const { return !(*this == other); }
    
    static SoundHandle invalid() { return SoundHandle{}; }
};

/**
 * @brief Audio clip data (loaded sound file)
 */
struct AudioClip {
    std::string name;
    std::string path;
    AudioFormat format;
    AudioCodec codec = AudioCodec::Unknown;
    LoadMode loadMode = LoadMode::Decompressed;
    
    std::vector<u8> data;       // Audio data (compressed or decompressed)
    u64 sampleCount = 0;
    f32 duration = 0.0f;        // Duration in seconds
    
    bool isLoaded = false;
    bool isStreaming = false;
};

// ============================================================================
// 3D Audio Types
// ============================================================================

/**
 * @brief 3D sound distance attenuation model
 */
enum class AttenuationModel : u8 {
    None,           // No distance attenuation
    Linear,         // Linear falloff
    Inverse,        // Inverse distance (1/d)
    InverseSquare,  // Inverse square (1/d²)
    Logarithmic,    // Logarithmic falloff
    Custom          // Custom curve
};

/**
 * @brief 3D rolloff mode
 */
enum class RolloffMode : u8 {
    Linear,
    Logarithmic,
    Custom
};

/**
 * @brief 3D audio source properties
 */
struct AudioSource3D {
    Vec3 position = Vec3::zero();
    Vec3 velocity = Vec3::zero();
    Vec3 direction = Vec3::forward();
    
    f32 minDistance = 1.0f;         // Distance at which volume is 100%
    f32 maxDistance = 100.0f;       // Distance at which sound is inaudible
    AttenuationModel attenuation = AttenuationModel::InverseSquare;
    RolloffMode rolloff = RolloffMode::Logarithmic;
    
    f32 innerConeAngle = 360.0f;    // Degrees (360 = omnidirectional)
    f32 outerConeAngle = 360.0f;
    f32 outerConeGain = 0.0f;       // Volume multiplier outside outer cone
    
    f32 dopplerLevel = 1.0f;        // Doppler effect intensity
    f32 spreadAngle = 0.0f;         // Sound spread (0 = point, 180 = hemisphere)
    
    bool spatialize = true;
};

/**
 * @brief 3D audio listener properties
 */
struct AudioListener {
    Vec3 position = Vec3::zero();
    Vec3 velocity = Vec3::zero();
    Quat orientation = Quat::identity();
    
    f32 gain = 1.0f;
    bool isActive = true;
    
    Vec3 forward() const { return orientation * Vec3::forward(); }
    Vec3 up() const { return orientation * Vec3::up(); }
    Vec3 right() const { return orientation * Vec3::right(); }
};

// ============================================================================
// Audio Effect Types
// ============================================================================

/**
 * @brief Audio effect type
 */
enum class EffectType : u8 {
    None,
    // Dynamics
    Compressor,
    Limiter,
    Gate,
    Expander,
    
    // EQ/Filters
    LowPassFilter,
    HighPassFilter,
    BandPassFilter,
    NotchFilter,
    Equalizer,
    
    // Time-based
    Delay,
    Reverb,
    Echo,
    Chorus,
    Flanger,
    Phaser,
    
    // Distortion
    Distortion,
    Overdrive,
    Bitcrusher,
    
    // Modulation
    Tremolo,
    Vibrato,
    RingMod,
    
    // Spatial
    Panner,
    StereoWidth,
    
    // Utility
    Gain,
    Normalize,
    DcOffset
};

/**
 * @brief Base audio effect parameters
 */
struct EffectParams {
    EffectType type = EffectType::None;
    f32 wetDry = 1.0f;      // 0 = dry, 1 = wet
    bool bypass = false;
};

/**
 * @brief Reverb effect parameters
 */
struct ReverbParams : EffectParams {
    f32 roomSize = 0.5f;        // 0-1
    f32 damping = 0.5f;         // 0-1
    f32 decay = 1.5f;           // Seconds
    f32 preDelay = 0.01f;       // Seconds
    f32 earlyReflections = 0.5f;
    f32 diffusion = 0.5f;
    f32 density = 0.5f;
    f32 hfCutoff = 5000.0f;     // Hz
    
    static ReverbParams hall();
    static ReverbParams room();
    static ReverbParams cathedral();
    static ReverbParams bathroom();
    static ReverbParams cave();
};

/**
 * @brief Delay effect parameters
 */
struct DelayParams : EffectParams {
    f32 delayTime = 0.25f;      // Seconds
    f32 feedback = 0.3f;        // 0-1
    f32 lowPassCutoff = 5000.0f;
    bool sync = false;          // Sync to tempo
    f32 syncBeats = 1.0f;       // Beats if synced
};

/**
 * @brief Compressor effect parameters
 */
struct CompressorParams : EffectParams {
    f32 threshold = -20.0f;     // dB
    f32 ratio = 4.0f;           // x:1
    f32 attack = 0.01f;         // Seconds
    f32 release = 0.1f;         // Seconds
    f32 knee = 0.0f;            // dB
    f32 makeupGain = 0.0f;      // dB
};

/**
 * @brief Low-pass filter parameters
 */
struct LowPassParams : EffectParams {
    f32 cutoff = 5000.0f;       // Hz
    f32 resonance = 0.707f;     // Q factor
    u32 order = 2;              // Filter order
};

/**
 * @brief High-pass filter parameters
 */
struct HighPassParams : EffectParams {
    f32 cutoff = 100.0f;        // Hz
    f32 resonance = 0.707f;
    u32 order = 2;
};

/**
 * @brief Equalizer band
 */
struct EQBand {
    f32 frequency = 1000.0f;    // Center frequency Hz
    f32 gain = 0.0f;            // dB
    f32 bandwidth = 1.0f;       // Octaves
    
    enum class Type : u8 {
        Peak,
        LowShelf,
        HighShelf,
        LowPass,
        HighPass
    } type = Type::Peak;
};

/**
 * @brief Equalizer parameters
 */
struct EqualizerParams : EffectParams {
    std::vector<EQBand> bands;
    
    static EqualizerParams threeband();
    static EqualizerParams fiveband();
    static EqualizerParams tenband();
};

// ============================================================================
// Audio Mixer Types
// ============================================================================

/**
 * @brief Audio bus (mixing channel)
 */
struct AudioBus {
    std::string name;
    u32 id = 0;
    
    f32 volume = 1.0f;          // 0-2 (1 = unity)
    f32 pan = 0.0f;             // -1 (left) to 1 (right)
    bool mute = false;
    bool solo = false;
    
    std::vector<EffectParams> effects;
    
    u32 outputBus = 0;          // Parent bus ID (0 = master)
    std::vector<u32> inputBuses;
    
    // Metering
    f32 peakLeft = 0.0f;
    f32 peakRight = 0.0f;
    f32 rmsLeft = 0.0f;
    f32 rmsRight = 0.0f;
};

/**
 * @brief Sound playback parameters
 */
struct PlayParams {
    f32 volume = 1.0f;
    f32 pitch = 1.0f;
    f32 pan = 0.0f;
    PlaybackMode mode = PlaybackMode::Once;
    u32 loopCount = 0;
    f32 startTime = 0.0f;       // Start position in seconds
    f32 fadeInTime = 0.0f;
    f32 fadeOutTime = 0.0f;
    SoundPriority priority = SoundPriority::Normal;
    u32 bus = 0;                // Target bus ID
    
    // 3D settings (if spatialize is true)
    bool spatialize = false;
    AudioSource3D source3D;
    
    static PlayParams defaultParams();
    static PlayParams loop();
    static PlayParams music();
    static PlayParams spatial(const Vec3& position);
};

/**
 * @brief Music crossfade settings
 */
struct CrossfadeParams {
    f32 duration = 1.0f;        // Seconds
    
    enum class Curve : u8 {
        Linear,
        EqualPower,
        SCurve
    } curve = Curve::EqualPower;
};

// ============================================================================
// Audio State Types
// ============================================================================

/**
 * @brief Sound instance state
 */
enum class SoundState : u8 {
    Stopped,
    Playing,
    Paused,
    Stopping,        // Fading out
    Starting         // Fading in
};

/**
 * @brief Sound instance info
 */
struct SoundInfo {
    SoundHandle handle;
    SoundState state = SoundState::Stopped;
    f32 currentTime = 0.0f;
    f32 duration = 0.0f;
    f32 volume = 1.0f;
    f32 pitch = 1.0f;
    u32 loopsRemaining = 0;
    u32 bus = 0;
    bool is3D = false;
};

// ============================================================================
// Audio Callback Types
// ============================================================================

using SoundFinishedCallback = std::function<void(SoundHandle)>;
using SoundLoopCallback = std::function<void(SoundHandle, u32 loopIndex)>;
using AudioDataCallback = std::function<void(f32* samples, u32 frameCount, u32 channels)>;

// ============================================================================
// Inline Implementations
// ============================================================================

inline u32 AudioFormat::bytesPerSample() const {
    switch (sampleFormat) {
        case SampleFormat::Int8: return 1;
        case SampleFormat::Int16: return 2;
        case SampleFormat::Int24: return 3;
        case SampleFormat::Int32: return 4;
        case SampleFormat::Float32: return 4;
        case SampleFormat::Float64: return 8;
    }
    return 4;
}

inline u32 AudioFormat::bytesPerFrame() const {
    return bytesPerSample() * channels;
}

inline u32 AudioFormat::bytesPerSecond() const {
    return bytesPerFrame() * sampleRate;
}

inline AudioFormat AudioFormat::stereo44100() {
    AudioFormat fmt;
    fmt.sampleRate = 44100;
    fmt.channels = 2;
    return fmt;
}

inline AudioFormat AudioFormat::stereo48000() {
    return AudioFormat();  // Default
}

inline AudioFormat AudioFormat::mono44100() {
    AudioFormat fmt;
    fmt.sampleRate = 44100;
    fmt.channels = 1;
    fmt.channelLayout = ChannelLayout::Mono;
    return fmt;
}

inline AudioFormat AudioFormat::surround51() {
    AudioFormat fmt;
    fmt.channels = 6;
    fmt.channelLayout = ChannelLayout::Surround51;
    return fmt;
}

inline ReverbParams ReverbParams::hall() {
    ReverbParams p;
    p.type = EffectType::Reverb;
    p.roomSize = 0.8f;
    p.decay = 2.5f;
    p.damping = 0.3f;
    return p;
}

inline ReverbParams ReverbParams::room() {
    ReverbParams p;
    p.type = EffectType::Reverb;
    p.roomSize = 0.5f;
    p.decay = 0.8f;
    p.damping = 0.5f;
    return p;
}

inline ReverbParams ReverbParams::cathedral() {
    ReverbParams p;
    p.type = EffectType::Reverb;
    p.roomSize = 1.0f;
    p.decay = 5.0f;
    p.damping = 0.2f;
    return p;
}

inline ReverbParams ReverbParams::bathroom() {
    ReverbParams p;
    p.type = EffectType::Reverb;
    p.roomSize = 0.3f;
    p.decay = 0.4f;
    p.damping = 0.7f;
    p.earlyReflections = 0.8f;
    return p;
}

inline ReverbParams ReverbParams::cave() {
    ReverbParams p;
    p.type = EffectType::Reverb;
    p.roomSize = 0.9f;
    p.decay = 4.0f;
    p.damping = 0.4f;
    p.diffusion = 0.8f;
    return p;
}

inline PlayParams PlayParams::defaultParams() {
    return PlayParams();
}

inline PlayParams PlayParams::loop() {
    PlayParams p;
    p.mode = PlaybackMode::Loop;
    return p;
}

inline PlayParams PlayParams::music() {
    PlayParams p;
    p.mode = PlaybackMode::Loop;
    p.priority = SoundPriority::High;
    p.fadeInTime = 0.5f;
    p.fadeOutTime = 0.5f;
    return p;
}

inline PlayParams PlayParams::spatial(const Vec3& position) {
    PlayParams p;
    p.spatialize = true;
    p.source3D.position = position;
    return p;
}

} // namespace nova::audio
