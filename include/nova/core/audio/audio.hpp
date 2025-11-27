/**
 * @file audio.hpp
 * @brief NovaCore Audio System™ - Main Include Header
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include "audio_types.hpp"
#include "audio_system.hpp"

namespace nova::audio {

/**
 * @brief Audio system version
 */
namespace version {
    constexpr u32 MAJOR = 1;
    constexpr u32 MINOR = 0;
    constexpr u32 PATCH = 0;
    constexpr const char* STRING = "1.0.0";
}

/**
 * @brief Initialize the audio system
 */
inline bool initializeAudio(const char* deviceName = nullptr) {
    return AudioSystem::get().initialize(deviceName);
}

/**
 * @brief Shutdown the audio system
 */
inline void shutdownAudio() {
    AudioSystem::get().shutdown();
}

/**
 * @brief Update audio (call each frame)
 */
inline void updateAudio(f32 deltaTime) {
    AudioSystem::get().update(deltaTime);
}

/**
 * @brief Get the audio system singleton
 */
inline AudioSystem& getAudio() {
    return AudioSystem::get();
}

// Convenience functions
inline SoundHandle playSound(std::shared_ptr<AudioClip> clip, f32 volume = 1.0f) {
    PlayParams params;
    params.volume = volume;
    return AudioSystem::get().play(clip, params);
}

inline SoundHandle playSoundAtPosition(std::shared_ptr<AudioClip> clip, const Vec3& pos, f32 volume = 1.0f) {
    return AudioSystem::get().playAtPosition(clip, pos, volume);
}

inline void stopSound(SoundHandle handle, f32 fadeTime = 0.0f) {
    AudioSystem::get().stop(handle, fadeTime);
}

inline void setMasterVolume(f32 volume) {
    AudioSystem::get().setMasterVolume(volume);
}

} // namespace nova::audio
