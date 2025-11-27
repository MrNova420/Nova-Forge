/**
 * @file audio_system.hpp
 * @brief NovaCore Audio System™ - Main Audio Engine
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 * 
 * Central audio management system providing:
 * - Sound and music playback
 * - 3D spatial audio
 * - Audio mixing and bus routing
 * - Real-time audio effects
 * - Platform-agnostic audio backend
 */

#pragma once

#include "audio_types.hpp"

#include <memory>
#include <unordered_map>
#include <queue>

namespace nova::audio {

// ============================================================================
// Forward Declarations
// ============================================================================

class AudioMixer;
class AudioDevice;
class AudioDecoder;

// ============================================================================
// Audio System
// ============================================================================

/**
 * @brief Central audio management system
 * 
 * The AudioSystem provides a high-level interface for all audio operations:
 * 
 * Usage:
 * @code
 *     auto& audio = AudioSystem::get();
 *     
 *     // Load and play a sound
 *     auto clip = audio.loadClip("sounds/explosion.ogg");
 *     auto handle = audio.play(clip);
 *     
 *     // 3D spatial audio
 *     PlayParams params = PlayParams::spatial(Vec3(10, 0, 0));
 *     audio.play(clip, params);
 *     
 *     // Update listener position
 *     audio.setListenerPosition(0, camera.position);
 *     audio.setListenerOrientation(0, camera.rotation);
 * @endcode
 */
class AudioSystem {
public:
    /**
     * @brief Get singleton instance
     */
    static AudioSystem& get();
    
    /**
     * @brief Initialize the audio system
     * @param deviceName Optional device name (nullptr for default)
     * @return true if initialization succeeded
     */
    bool initialize(const char* deviceName = nullptr);
    
    /**
     * @brief Shutdown the audio system
     */
    void shutdown();
    
    /**
     * @brief Update audio system (call each frame)
     * @param deltaTime Time since last frame
     */
    void update(f32 deltaTime);
    
    /**
     * @brief Check if audio is initialized
     */
    bool isInitialized() const { return m_initialized; }
    
    // ========================================================================
    // Audio Clip Management
    // ========================================================================
    
    /**
     * @brief Load an audio clip from file
     * @param path File path
     * @param mode Load mode (streaming/decompressed)
     * @return Loaded audio clip
     */
    std::shared_ptr<AudioClip> loadClip(const std::string& path,
                                        LoadMode mode = LoadMode::Decompressed);
    
    /**
     * @brief Load audio clip asynchronously
     * @param path File path
     * @param mode Load mode
     * @param callback Called when loading completes
     */
    void loadClipAsync(const std::string& path,
                       LoadMode mode,
                       std::function<void(std::shared_ptr<AudioClip>)> callback);
    
    /**
     * @brief Unload an audio clip
     */
    void unloadClip(std::shared_ptr<AudioClip> clip);
    
    /**
     * @brief Unload all audio clips
     */
    void unloadAllClips();
    
    /**
     * @brief Get a loaded clip by path
     */
    std::shared_ptr<AudioClip> getClip(const std::string& path) const;
    
    // ========================================================================
    // Sound Playback
    // ========================================================================
    
    /**
     * @brief Play a sound
     * @param clip Audio clip to play
     * @param params Playback parameters
     * @return Handle to playing sound
     */
    SoundHandle play(std::shared_ptr<AudioClip> clip,
                     const PlayParams& params = PlayParams::defaultParams());
    
    /**
     * @brief Play a sound at position (3D)
     */
    SoundHandle playAtPosition(std::shared_ptr<AudioClip> clip,
                               const Vec3& position,
                               f32 volume = 1.0f);
    
    /**
     * @brief Play a one-shot sound (fire and forget)
     */
    void playOneShot(std::shared_ptr<AudioClip> clip, f32 volume = 1.0f);
    
    /**
     * @brief Play a one-shot sound at position
     */
    void playOneShotAtPosition(std::shared_ptr<AudioClip> clip,
                               const Vec3& position,
                               f32 volume = 1.0f);
    
    /**
     * @brief Stop a playing sound
     * @param fadeTime Fade out duration (0 = immediate)
     */
    void stop(SoundHandle handle, f32 fadeTime = 0.0f);
    
    /**
     * @brief Stop all playing sounds
     */
    void stopAll(f32 fadeTime = 0.0f);
    
    /**
     * @brief Pause a sound
     */
    void pause(SoundHandle handle);
    
    /**
     * @brief Resume a paused sound
     */
    void resume(SoundHandle handle);
    
    /**
     * @brief Pause all sounds
     */
    void pauseAll();
    
    /**
     * @brief Resume all paused sounds
     */
    void resumeAll();
    
    /**
     * @brief Check if sound is playing
     */
    bool isPlaying(SoundHandle handle) const;
    
    /**
     * @brief Get sound state
     */
    SoundState getState(SoundHandle handle) const;
    
    /**
     * @brief Get sound info
     */
    SoundInfo getSoundInfo(SoundHandle handle) const;
    
    // ========================================================================
    // Sound Properties
    // ========================================================================
    
    /**
     * @brief Set sound volume
     */
    void setVolume(SoundHandle handle, f32 volume);
    
    /**
     * @brief Get sound volume
     */
    f32 getVolume(SoundHandle handle) const;
    
    /**
     * @brief Set sound pitch
     */
    void setPitch(SoundHandle handle, f32 pitch);
    
    /**
     * @brief Get sound pitch
     */
    f32 getPitch(SoundHandle handle) const;
    
    /**
     * @brief Set sound pan (-1 to 1)
     */
    void setPan(SoundHandle handle, f32 pan);
    
    /**
     * @brief Set playback position in seconds
     */
    void setPlaybackPosition(SoundHandle handle, f32 time);
    
    /**
     * @brief Get playback position in seconds
     */
    f32 getPlaybackPosition(SoundHandle handle) const;
    
    /**
     * @brief Fade sound volume over time
     */
    void fade(SoundHandle handle, f32 targetVolume, f32 duration);
    
    // ========================================================================
    // 3D Audio
    // ========================================================================
    
    /**
     * @brief Set sound 3D position
     */
    void setPosition(SoundHandle handle, const Vec3& position);
    
    /**
     * @brief Set sound 3D velocity (for Doppler)
     */
    void setVelocity(SoundHandle handle, const Vec3& velocity);
    
    /**
     * @brief Set sound 3D direction (for cone)
     */
    void setDirection(SoundHandle handle, const Vec3& direction);
    
    /**
     * @brief Set sound min/max distance
     */
    void setDistanceRange(SoundHandle handle, f32 minDist, f32 maxDist);
    
    /**
     * @brief Set sound cone parameters
     */
    void setCone(SoundHandle handle, f32 innerAngle, f32 outerAngle, f32 outerGain);
    
    // ========================================================================
    // Listener Management
    // ========================================================================
    
    /**
     * @brief Get number of active listeners
     */
    u32 getListenerCount() const { return m_listenerCount; }
    
    /**
     * @brief Set listener position
     */
    void setListenerPosition(u32 index, const Vec3& position);
    
    /**
     * @brief Set listener velocity
     */
    void setListenerVelocity(u32 index, const Vec3& velocity);
    
    /**
     * @brief Set listener orientation
     */
    void setListenerOrientation(u32 index, const Quat& orientation);
    
    /**
     * @brief Set listener orientation from forward/up vectors
     */
    void setListenerOrientation(u32 index, const Vec3& forward, const Vec3& up);
    
    /**
     * @brief Set listener gain
     */
    void setListenerGain(u32 index, f32 gain);
    
    /**
     * @brief Get listener
     */
    const AudioListener& getListener(u32 index) const;
    
    /**
     * @brief Set number of active listeners (split-screen)
     */
    void setListenerCount(u32 count);
    
    // ========================================================================
    // Music Playback
    // ========================================================================
    
    /**
     * @brief Play music with crossfade from current
     */
    SoundHandle playMusic(std::shared_ptr<AudioClip> clip,
                          const CrossfadeParams& crossfade = CrossfadeParams());
    
    /**
     * @brief Stop current music
     */
    void stopMusic(f32 fadeTime = 1.0f);
    
    /**
     * @brief Pause music
     */
    void pauseMusic();
    
    /**
     * @brief Resume music
     */
    void resumeMusic();
    
    /**
     * @brief Set music volume
     */
    void setMusicVolume(f32 volume);
    
    /**
     * @brief Get music volume
     */
    f32 getMusicVolume() const { return m_musicVolume; }
    
    /**
     * @brief Check if music is playing
     */
    bool isMusicPlaying() const;
    
    // ========================================================================
    // Audio Bus/Mixer
    // ========================================================================
    
    /**
     * @brief Create an audio bus
     * @param name Bus name
     * @param outputBus Parent bus ID (0 = master)
     * @return Bus ID
     */
    u32 createBus(const std::string& name, u32 outputBus = 0);
    
    /**
     * @brief Get bus by name
     */
    AudioBus* getBus(const std::string& name);
    
    /**
     * @brief Get bus by ID
     */
    AudioBus* getBus(u32 id);
    
    /**
     * @brief Get master bus
     */
    AudioBus* getMasterBus();
    
    /**
     * @brief Set bus volume
     */
    void setBusVolume(u32 busId, f32 volume);
    
    /**
     * @brief Set bus mute
     */
    void setBusMute(u32 busId, bool mute);
    
    /**
     * @brief Set bus solo
     */
    void setBusSolo(u32 busId, bool solo);
    
    /**
     * @brief Add effect to bus
     */
    void addBusEffect(u32 busId, const EffectParams& effect);
    
    /**
     * @brief Remove effect from bus
     */
    void removeBusEffect(u32 busId, u32 effectIndex);
    
    /**
     * @brief Clear all effects from bus
     */
    void clearBusEffects(u32 busId);
    
    // ========================================================================
    // Global Settings
    // ========================================================================
    
    /**
     * @brief Set master volume
     */
    void setMasterVolume(f32 volume);
    
    /**
     * @brief Get master volume
     */
    f32 getMasterVolume() const { return m_masterVolume; }
    
    /**
     * @brief Set global mute
     */
    void setMute(bool mute);
    
    /**
     * @brief Check if globally muted
     */
    bool isMuted() const { return m_isMuted; }
    
    /**
     * @brief Set doppler factor
     */
    void setDopplerFactor(f32 factor) { m_dopplerFactor = factor; }
    
    /**
     * @brief Get doppler factor
     */
    f32 getDopplerFactor() const { return m_dopplerFactor; }
    
    /**
     * @brief Set speed of sound (for Doppler)
     */
    void setSpeedOfSound(f32 speed) { m_speedOfSound = speed; }
    
    /**
     * @brief Get speed of sound
     */
    f32 getSpeedOfSound() const { return m_speedOfSound; }
    
    // ========================================================================
    // Device Information
    // ========================================================================
    
    /**
     * @brief Get audio device names
     */
    std::vector<std::string> getDeviceNames() const;
    
    /**
     * @brief Get current device name
     */
    std::string getCurrentDeviceName() const;
    
    /**
     * @brief Get output format
     */
    AudioFormat getOutputFormat() const { return m_outputFormat; }
    
    /**
     * @brief Get CPU usage (0-1)
     */
    f32 getCpuUsage() const { return m_cpuUsage; }
    
    /**
     * @brief Get number of active voices
     */
    u32 getActiveVoiceCount() const;
    
    // ========================================================================
    // Callbacks
    // ========================================================================
    
    /**
     * @brief Set callback for when a sound finishes
     */
    void setSoundFinishedCallback(SoundFinishedCallback callback);
    
    /**
     * @brief Set callback for when a sound loops
     */
    void setSoundLoopCallback(SoundLoopCallback callback);
    
private:
    AudioSystem() = default;
    ~AudioSystem() = default;
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
    
    // Internal methods
    SoundHandle allocateHandle();
    void freeHandle(SoundHandle handle);
    void processFinishedSounds();
    void updateFades(f32 deltaTime);
    void update3DAudio();
    f32 calculateAttenuation(const AudioSource3D& source, const Vec3& listenerPos);
    f32 calculateDoppler(const AudioSource3D& source, const AudioListener& listener);
    void mixAudio(f32* output, u32 frameCount);
    
    // State
    bool m_initialized = false;
    
    // Audio device
    std::unique_ptr<AudioDevice> m_device;
    AudioFormat m_outputFormat;
    
    // Clips
    std::unordered_map<std::string, std::shared_ptr<AudioClip>> m_clips;
    
    // Sound instances
    struct SoundInstance {
        SoundHandle handle;
        std::shared_ptr<AudioClip> clip;
        PlayParams params;
        SoundState state = SoundState::Stopped;
        f32 currentTime = 0.0f;
        u32 loopsRemaining = 0;
        f32 fadeTarget = 1.0f;
        f32 fadeRate = 0.0f;
        u64 samplePosition = 0;
    };
    std::vector<SoundInstance> m_instances;
    u32 m_nextHandleId = 1;
    u32 m_handleGeneration = 0;
    
    // Music
    SoundHandle m_currentMusic;
    SoundHandle m_previousMusic;
    f32 m_musicVolume = 1.0f;
    f32 m_crossfadeTime = 0.0f;
    f32 m_crossfadeDuration = 0.0f;
    
    // Listeners
    std::array<AudioListener, AudioConfig::MAX_LISTENERS> m_listeners;
    u32 m_listenerCount = 1;
    
    // Buses
    std::vector<AudioBus> m_buses;
    
    // Global settings
    f32 m_masterVolume = 1.0f;
    bool m_isMuted = false;
    f32 m_dopplerFactor = AudioConfig::DEFAULT_DOPPLER_FACTOR;
    f32 m_speedOfSound = AudioConfig::SPEED_OF_SOUND;
    
    // Performance
    f32 m_cpuUsage = 0.0f;
    
    // Callbacks
    SoundFinishedCallback m_soundFinishedCallback;
    SoundLoopCallback m_soundLoopCallback;
    
    // Async loading
    struct AsyncLoadRequest {
        std::string path;
        LoadMode mode;
        std::function<void(std::shared_ptr<AudioClip>)> callback;
    };
    std::queue<AsyncLoadRequest> m_loadQueue;
};

} // namespace nova::audio
