/**
 * @file audio_system.cpp
 * @brief NovaCore Audio System™ - Main Audio Engine Implementation
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/core/audio/audio_system.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>

namespace nova::audio {

// ============================================================================
// Singleton
// ============================================================================

AudioSystem& AudioSystem::get() {
    static AudioSystem instance;
    return instance;
}

// ============================================================================
// Initialization
// ============================================================================

bool AudioSystem::initialize(const char* deviceName) {
    if (m_initialized) {
        return true;
    }
    
    // Initialize output format
    m_outputFormat = AudioFormat::stereo48000();
    
    // Create master bus
    AudioBus masterBus;
    masterBus.name = "Master";
    masterBus.id = 0;
    m_buses.push_back(masterBus);
    
    // Create default buses
    createBus("Music", 0);
    createBus("SFX", 0);
    createBus("Voice", 0);
    createBus("Ambient", 0);
    
    // Initialize listeners
    for (auto& listener : m_listeners) {
        listener = AudioListener();
    }
    m_listenerCount = 1;
    
    m_initialized = true;
    return true;
}

void AudioSystem::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // Stop all sounds
    stopAll(0.0f);
    
    // Unload all clips
    unloadAllClips();
    
    // Clear buses
    m_buses.clear();
    
    // Clear instances
    m_instances.clear();
    
    m_device.reset();
    m_initialized = false;
}

void AudioSystem::update(f32 deltaTime) {
    if (!m_initialized) {
        return;
    }
    
    // Process async loading
    while (!m_loadQueue.empty()) {
        auto& request = m_loadQueue.front();
        auto clip = loadClip(request.path, request.mode);
        if (request.callback) {
            request.callback(clip);
        }
        m_loadQueue.pop();
    }
    
    // Update fades
    updateFades(deltaTime);
    
    // Update 3D audio
    update3DAudio();
    
    // Update crossfade
    if (m_crossfadeDuration > 0.0f && m_previousMusic.isValid()) {
        m_crossfadeTime += deltaTime;
        f32 t = std::min(m_crossfadeTime / m_crossfadeDuration, 1.0f);
        
        // Fade out old music
        if (auto* instance = findInstance(m_previousMusic)) {
            instance->params.volume = m_musicVolume * (1.0f - t);
        }
        
        // Fade in new music
        if (auto* instance = findInstance(m_currentMusic)) {
            instance->params.volume = m_musicVolume * t;
        }
        
        // Complete crossfade
        if (t >= 1.0f) {
            stop(m_previousMusic);
            m_previousMusic = SoundHandle::invalid();
            m_crossfadeDuration = 0.0f;
        }
    }
    
    // Process finished sounds
    processFinishedSounds();
    
    // Update instances
    for (auto& instance : m_instances) {
        if (instance.state == SoundState::Playing) {
            instance.currentTime += deltaTime * instance.params.pitch;
            
            // Check for loop
            if (instance.clip && instance.currentTime >= instance.clip->duration) {
                if (instance.params.mode == PlaybackMode::Loop ||
                    (instance.params.mode == PlaybackMode::LoopCount && instance.loopsRemaining > 0)) {
                    
                    instance.currentTime = 0.0f;
                    if (instance.loopsRemaining > 0) {
                        instance.loopsRemaining--;
                    }
                    
                    // Fire loop callback
                    if (m_soundLoopCallback) {
                        u32 loopIndex = instance.params.loopCount - instance.loopsRemaining;
                        m_soundLoopCallback(instance.handle, loopIndex);
                    }
                } else {
                    instance.state = SoundState::Stopped;
                }
            }
        }
    }
}

// ============================================================================
// Audio Clip Management
// ============================================================================

std::shared_ptr<AudioClip> AudioSystem::loadClip(const std::string& path, LoadMode mode) {
    // Check if already loaded
    auto it = m_clips.find(path);
    if (it != m_clips.end()) {
        return it->second;
    }
    
    // Create new clip
    auto clip = std::make_shared<AudioClip>();
    clip->name = path;
    clip->path = path;
    clip->loadMode = mode;
    
    // Detect codec from extension
    std::string ext = path.substr(path.find_last_of('.') + 1);
    if (ext == "wav") clip->codec = AudioCodec::WAV;
    else if (ext == "ogg") clip->codec = AudioCodec::OGG;
    else if (ext == "mp3") clip->codec = AudioCodec::MP3;
    else if (ext == "flac") clip->codec = AudioCodec::FLAC;
    else clip->codec = AudioCodec::Unknown;
    
    // Load file data
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        clip->data.resize(static_cast<usize>(size));
        if (file.read(reinterpret_cast<char*>(clip->data.data()), size)) {
            clip->isLoaded = true;
            
            // Parse audio header based on codec
            if (clip->codec == AudioCodec::WAV && clip->data.size() >= 44) {
                // Parse WAV header (RIFF format)
                // RIFF header: "RIFF" (4) + file size (4) + "WAVE" (4)
                // fmt chunk: "fmt " (4) + chunk size (4) + audio format (2) + channels (2)
                //            + sample rate (4) + byte rate (4) + block align (2) + bits per sample (2)
                // data chunk: "data" (4) + data size (4) + audio data
                
                const u8* data = clip->data.data();
                
                // Verify RIFF header
                if (data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F' &&
                    data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E') {
                    
                    // Find fmt chunk (usually at offset 12)
                    usize offset = 12;
                    while (offset + 8 <= clip->data.size()) {
                        // Bounds check for chunk header read
                        if (offset + 8 > clip->data.size()) break;
                        
                        u32 chunkId = *reinterpret_cast<const u32*>(data + offset);
                        u32 chunkSize = *reinterpret_cast<const u32*>(data + offset + 4);
                        
                        // "fmt " chunk (0x20746d66 in little-endian)
                        if (chunkId == 0x20746d66) {
                            // Bounds check for fmt chunk data (need at least 16 bytes)
                            if (offset + 24 > clip->data.size()) break;
                            
                            u16 audioFormat = *reinterpret_cast<const u16*>(data + offset + 8);
                            u16 numChannels = *reinterpret_cast<const u16*>(data + offset + 10);
                            u32 sampleRate = *reinterpret_cast<const u32*>(data + offset + 12);
                            // u32 byteRate = *reinterpret_cast<const u32*>(data + offset + 16);
                            // u16 blockAlign = *reinterpret_cast<const u16*>(data + offset + 20);
                            u16 bitsPerSample = *reinterpret_cast<const u16*>(data + offset + 22);
                            
                            // Set format
                            clip->format.sampleRate = sampleRate;
                            clip->format.channels = numChannels;
                            clip->format.bitsPerSample = bitsPerSample;
                            clip->format.isPlanar = false;
                            clip->format.isFloat = (audioFormat == 3); // IEEE float format
                            
                            offset += 8 + chunkSize;
                        }
                        // "data" chunk (0x61746164 in little-endian)
                        else if (chunkId == 0x61746164) {
                            // Calculate sample count and duration
                            u32 dataSize = chunkSize;
                            u32 bytesPerSample = clip->format.bitsPerSample / 8;
                            u32 bytesPerFrame = bytesPerSample * clip->format.channels;
                            
                            if (bytesPerFrame > 0) {
                                clip->sampleCount = dataSize / bytesPerFrame;
                                clip->duration = static_cast<f32>(clip->sampleCount) / 
                                               static_cast<f32>(clip->format.sampleRate);
                            }
                            break;
                        }
                        else {
                            // Skip unknown chunk
                            offset += 8 + chunkSize;
                            // Ensure even alignment
                            if (chunkSize % 2 != 0) offset++;
                        }
                    }
                }
            }
            else {
                // For non-WAV formats, use reasonable defaults
                // In a full implementation, we would use codec-specific decoders
                clip->format = AudioFormat::stereo44100();
                clip->sampleCount = 0;
                clip->duration = 0.0f;
            }
        }
    }
    
    clip->isStreaming = (mode == LoadMode::Streaming);
    
    m_clips[path] = clip;
    return clip;
}

void AudioSystem::loadClipAsync(const std::string& path,
                                LoadMode mode,
                                std::function<void(std::shared_ptr<AudioClip>)> callback) {
    m_loadQueue.push({path, mode, callback});
}

void AudioSystem::unloadClip(std::shared_ptr<AudioClip> clip) {
    if (!clip) return;
    
    // Stop any instances using this clip
    for (auto& instance : m_instances) {
        if (instance.clip == clip) {
            instance.state = SoundState::Stopped;
        }
    }
    
    // Remove from map
    m_clips.erase(clip->path);
}

void AudioSystem::unloadAllClips() {
    stopAll(0.0f);
    m_clips.clear();
}

std::shared_ptr<AudioClip> AudioSystem::getClip(const std::string& path) const {
    auto it = m_clips.find(path);
    return it != m_clips.end() ? it->second : nullptr;
}

// ============================================================================
// Sound Playback
// ============================================================================

SoundHandle AudioSystem::play(std::shared_ptr<AudioClip> clip, const PlayParams& params) {
    if (!clip || !clip->isLoaded) {
        return SoundHandle::invalid();
    }
    
    SoundHandle handle = allocateHandle();
    
    SoundInstance instance;
    instance.handle = handle;
    instance.clip = clip;
    instance.params = params;
    instance.state = SoundState::Playing;
    instance.currentTime = params.startTime;
    instance.loopsRemaining = params.loopCount;
    
    if (params.fadeInTime > 0.0f) {
        instance.state = SoundState::Starting;
        instance.fadeTarget = params.volume;
        instance.fadeRate = params.volume / params.fadeInTime;
        instance.params.volume = 0.0f;
    }
    
    m_instances.push_back(instance);
    
    return handle;
}

SoundHandle AudioSystem::playAtPosition(std::shared_ptr<AudioClip> clip,
                                        const Vec3& position,
                                        f32 volume) {
    PlayParams params = PlayParams::spatial(position);
    params.volume = volume;
    return play(clip, params);
}

void AudioSystem::playOneShot(std::shared_ptr<AudioClip> clip, f32 volume) {
    PlayParams params;
    params.volume = volume;
    params.priority = SoundPriority::Low;
    play(clip, params);
}

void AudioSystem::playOneShotAtPosition(std::shared_ptr<AudioClip> clip,
                                        const Vec3& position,
                                        f32 volume) {
    PlayParams params = PlayParams::spatial(position);
    params.volume = volume;
    params.priority = SoundPriority::Low;
    play(clip, params);
}

void AudioSystem::stop(SoundHandle handle, f32 fadeTime) {
    auto* instance = findInstance(handle);
    if (!instance) return;
    
    if (fadeTime > 0.0f) {
        instance->state = SoundState::Stopping;
        instance->fadeTarget = 0.0f;
        instance->fadeRate = instance->params.volume / fadeTime;
    } else {
        instance->state = SoundState::Stopped;
    }
}

void AudioSystem::stopAll(f32 fadeTime) {
    for (auto& instance : m_instances) {
        if (instance.state == SoundState::Playing || instance.state == SoundState::Starting) {
            if (fadeTime > 0.0f) {
                instance.state = SoundState::Stopping;
                instance.fadeTarget = 0.0f;
                instance.fadeRate = instance.params.volume / fadeTime;
            } else {
                instance.state = SoundState::Stopped;
            }
        }
    }
}

void AudioSystem::pause(SoundHandle handle) {
    auto* instance = findInstance(handle);
    if (instance && instance->state == SoundState::Playing) {
        instance->state = SoundState::Paused;
    }
}

void AudioSystem::resume(SoundHandle handle) {
    auto* instance = findInstance(handle);
    if (instance && instance->state == SoundState::Paused) {
        instance->state = SoundState::Playing;
    }
}

void AudioSystem::pauseAll() {
    for (auto& instance : m_instances) {
        if (instance.state == SoundState::Playing) {
            instance.state = SoundState::Paused;
        }
    }
}

void AudioSystem::resumeAll() {
    for (auto& instance : m_instances) {
        if (instance.state == SoundState::Paused) {
            instance.state = SoundState::Playing;
        }
    }
}

bool AudioSystem::isPlaying(SoundHandle handle) const {
    auto* instance = findInstance(handle);
    return instance && instance->state == SoundState::Playing;
}

SoundState AudioSystem::getState(SoundHandle handle) const {
    auto* instance = findInstance(handle);
    return instance ? instance->state : SoundState::Stopped;
}

SoundInfo AudioSystem::getSoundInfo(SoundHandle handle) const {
    SoundInfo info;
    auto* instance = findInstance(handle);
    if (instance) {
        info.handle = instance->handle;
        info.state = instance->state;
        info.currentTime = instance->currentTime;
        info.duration = instance->clip ? instance->clip->duration : 0.0f;
        info.volume = instance->params.volume;
        info.pitch = instance->params.pitch;
        info.loopsRemaining = instance->loopsRemaining;
        info.bus = instance->params.bus;
        info.is3D = instance->params.spatialize;
    }
    return info;
}

// ============================================================================
// Sound Properties
// ============================================================================

void AudioSystem::setVolume(SoundHandle handle, f32 volume) {
    auto* instance = findInstance(handle);
    if (instance) {
        instance->params.volume = std::clamp(volume, AudioConfig::MIN_VOLUME, AudioConfig::MAX_VOLUME);
    }
}

f32 AudioSystem::getVolume(SoundHandle handle) const {
    auto* instance = findInstance(handle);
    return instance ? instance->params.volume : 0.0f;
}

void AudioSystem::setPitch(SoundHandle handle, f32 pitch) {
    auto* instance = findInstance(handle);
    if (instance) {
        instance->params.pitch = std::clamp(pitch, AudioConfig::MIN_PITCH, AudioConfig::MAX_PITCH);
    }
}

f32 AudioSystem::getPitch(SoundHandle handle) const {
    auto* instance = findInstance(handle);
    return instance ? instance->params.pitch : 1.0f;
}

void AudioSystem::setPan(SoundHandle handle, f32 pan) {
    auto* instance = findInstance(handle);
    if (instance) {
        instance->params.pan = std::clamp(pan, -1.0f, 1.0f);
    }
}

void AudioSystem::setPlaybackPosition(SoundHandle handle, f32 time) {
    auto* instance = findInstance(handle);
    if (instance && instance->clip) {
        instance->currentTime = std::clamp(time, 0.0f, instance->clip->duration);
    }
}

f32 AudioSystem::getPlaybackPosition(SoundHandle handle) const {
    auto* instance = findInstance(handle);
    return instance ? instance->currentTime : 0.0f;
}

void AudioSystem::fade(SoundHandle handle, f32 targetVolume, f32 duration) {
    auto* instance = findInstance(handle);
    if (instance && duration > 0.0f) {
        instance->fadeTarget = std::clamp(targetVolume, AudioConfig::MIN_VOLUME, AudioConfig::MAX_VOLUME);
        instance->fadeRate = std::abs(targetVolume - instance->params.volume) / duration;
    }
}

// ============================================================================
// 3D Audio
// ============================================================================

void AudioSystem::setPosition(SoundHandle handle, const Vec3& position) {
    auto* instance = findInstance(handle);
    if (instance) {
        instance->params.source3D.position = position;
    }
}

void AudioSystem::setVelocity(SoundHandle handle, const Vec3& velocity) {
    auto* instance = findInstance(handle);
    if (instance) {
        instance->params.source3D.velocity = velocity;
    }
}

void AudioSystem::setDirection(SoundHandle handle, const Vec3& direction) {
    auto* instance = findInstance(handle);
    if (instance) {
        instance->params.source3D.direction = direction.normalized();
    }
}

void AudioSystem::setDistanceRange(SoundHandle handle, f32 minDist, f32 maxDist) {
    auto* instance = findInstance(handle);
    if (instance) {
        instance->params.source3D.minDistance = std::max(0.01f, minDist);
        instance->params.source3D.maxDistance = std::max(minDist, maxDist);
    }
}

void AudioSystem::setCone(SoundHandle handle, f32 innerAngle, f32 outerAngle, f32 outerGain) {
    auto* instance = findInstance(handle);
    if (instance) {
        instance->params.source3D.innerConeAngle = std::clamp(innerAngle, 0.0f, 360.0f);
        instance->params.source3D.outerConeAngle = std::clamp(outerAngle, innerAngle, 360.0f);
        instance->params.source3D.outerConeGain = std::clamp(outerGain, 0.0f, 1.0f);
    }
}

// ============================================================================
// Listener Management
// ============================================================================

void AudioSystem::setListenerPosition(u32 index, const Vec3& position) {
    if (index < AudioConfig::MAX_LISTENERS) {
        m_listeners[index].position = position;
    }
}

void AudioSystem::setListenerVelocity(u32 index, const Vec3& velocity) {
    if (index < AudioConfig::MAX_LISTENERS) {
        m_listeners[index].velocity = velocity;
    }
}

void AudioSystem::setListenerOrientation(u32 index, const Quat& orientation) {
    if (index < AudioConfig::MAX_LISTENERS) {
        m_listeners[index].orientation = orientation;
    }
}

void AudioSystem::setListenerOrientation(u32 index, const Vec3& forward, const Vec3& up) {
    if (index < AudioConfig::MAX_LISTENERS) {
        m_listeners[index].orientation = Quat::lookRotation(forward, up);
    }
}

void AudioSystem::setListenerGain(u32 index, f32 gain) {
    if (index < AudioConfig::MAX_LISTENERS) {
        m_listeners[index].gain = std::clamp(gain, 0.0f, 2.0f);
    }
}

const AudioListener& AudioSystem::getListener(u32 index) const {
    static AudioListener defaultListener;
    if (index < AudioConfig::MAX_LISTENERS) {
        return m_listeners[index];
    }
    return defaultListener;
}

void AudioSystem::setListenerCount(u32 count) {
    m_listenerCount = std::min(count, AudioConfig::MAX_LISTENERS);
}

// ============================================================================
// Music Playback
// ============================================================================

SoundHandle AudioSystem::playMusic(std::shared_ptr<AudioClip> clip, const CrossfadeParams& crossfade) {
    // Store previous music for crossfade
    if (m_currentMusic.isValid()) {
        m_previousMusic = m_currentMusic;
        m_crossfadeTime = 0.0f;
        m_crossfadeDuration = crossfade.duration;
    }
    
    // Play new music
    PlayParams params = PlayParams::music();
    params.volume = (crossfade.duration > 0.0f && m_previousMusic.isValid()) ? 0.0f : m_musicVolume;
    params.bus = 1;  // Music bus
    
    m_currentMusic = play(clip, params);
    return m_currentMusic;
}

void AudioSystem::stopMusic(f32 fadeTime) {
    if (m_currentMusic.isValid()) {
        stop(m_currentMusic, fadeTime);
        m_currentMusic = SoundHandle::invalid();
    }
}

void AudioSystem::pauseMusic() {
    if (m_currentMusic.isValid()) {
        pause(m_currentMusic);
    }
}

void AudioSystem::resumeMusic() {
    if (m_currentMusic.isValid()) {
        resume(m_currentMusic);
    }
}

void AudioSystem::setMusicVolume(f32 volume) {
    m_musicVolume = std::clamp(volume, AudioConfig::MIN_VOLUME, AudioConfig::MAX_VOLUME);
    if (m_currentMusic.isValid()) {
        setVolume(m_currentMusic, m_musicVolume);
    }
}

bool AudioSystem::isMusicPlaying() const {
    return m_currentMusic.isValid() && isPlaying(m_currentMusic);
}

// ============================================================================
// Audio Bus
// ============================================================================

u32 AudioSystem::createBus(const std::string& name, u32 outputBus) {
    AudioBus bus;
    bus.name = name;
    bus.id = static_cast<u32>(m_buses.size());
    bus.outputBus = outputBus;
    
    m_buses.push_back(bus);
    
    // Add as input to parent bus
    if (outputBus < m_buses.size()) {
        m_buses[outputBus].inputBuses.push_back(bus.id);
    }
    
    return bus.id;
}

AudioBus* AudioSystem::getBus(const std::string& name) {
    for (auto& bus : m_buses) {
        if (bus.name == name) {
            return &bus;
        }
    }
    return nullptr;
}

AudioBus* AudioSystem::getBus(u32 id) {
    if (id < m_buses.size()) {
        return &m_buses[id];
    }
    return nullptr;
}

AudioBus* AudioSystem::getMasterBus() {
    return m_buses.empty() ? nullptr : &m_buses[0];
}

void AudioSystem::setBusVolume(u32 busId, f32 volume) {
    if (auto* bus = getBus(busId)) {
        bus->volume = std::clamp(volume, AudioConfig::MIN_VOLUME, AudioConfig::MAX_VOLUME);
    }
}

void AudioSystem::setBusMute(u32 busId, bool mute) {
    if (auto* bus = getBus(busId)) {
        bus->mute = mute;
    }
}

void AudioSystem::setBusSolo(u32 busId, bool solo) {
    if (auto* bus = getBus(busId)) {
        bus->solo = solo;
    }
}

void AudioSystem::addBusEffect(u32 busId, const EffectParams& effect) {
    if (auto* bus = getBus(busId)) {
        if (bus->effects.size() < AudioConfig::MAX_EFFECTS_PER_BUS) {
            bus->effects.push_back(effect);
        }
    }
}

void AudioSystem::removeBusEffect(u32 busId, u32 effectIndex) {
    if (auto* bus = getBus(busId)) {
        if (effectIndex < bus->effects.size()) {
            bus->effects.erase(bus->effects.begin() + effectIndex);
        }
    }
}

void AudioSystem::clearBusEffects(u32 busId) {
    if (auto* bus = getBus(busId)) {
        bus->effects.clear();
    }
}

// ============================================================================
// Global Settings
// ============================================================================

void AudioSystem::setMasterVolume(f32 volume) {
    m_masterVolume = std::clamp(volume, AudioConfig::MIN_VOLUME, AudioConfig::MAX_VOLUME);
}

void AudioSystem::setMute(bool mute) {
    m_isMuted = mute;
}

// ============================================================================
// Device Information
// ============================================================================

std::vector<std::string> AudioSystem::getDeviceNames() const {
    // Platform-specific implementation
    return {"Default"};
}

std::string AudioSystem::getCurrentDeviceName() const {
    return "Default";
}

u32 AudioSystem::getActiveVoiceCount() const {
    u32 count = 0;
    for (const auto& instance : m_instances) {
        if (instance.state == SoundState::Playing) {
            count++;
        }
    }
    return count;
}

// ============================================================================
// Callbacks
// ============================================================================

void AudioSystem::setSoundFinishedCallback(SoundFinishedCallback callback) {
    m_soundFinishedCallback = std::move(callback);
}

void AudioSystem::setSoundLoopCallback(SoundLoopCallback callback) {
    m_soundLoopCallback = std::move(callback);
}

// ============================================================================
// Internal Methods
// ============================================================================

SoundHandle AudioSystem::allocateHandle() {
    SoundHandle handle;
    handle.id = m_nextHandleId++;
    handle.generation = m_handleGeneration;
    return handle;
}

void AudioSystem::freeHandle(SoundHandle handle) {
    // Increment generation to invalidate handle
    m_handleGeneration++;
}

AudioSystem::SoundInstance* AudioSystem::findInstance(SoundHandle handle) {
    for (auto& instance : m_instances) {
        if (instance.handle == handle) {
            return &instance;
        }
    }
    return nullptr;
}

const AudioSystem::SoundInstance* AudioSystem::findInstance(SoundHandle handle) const {
    for (const auto& instance : m_instances) {
        if (instance.handle == handle) {
            return &instance;
        }
    }
    return nullptr;
}

void AudioSystem::processFinishedSounds() {
    for (auto it = m_instances.begin(); it != m_instances.end();) {
        if (it->state == SoundState::Stopped) {
            // Fire callback
            if (m_soundFinishedCallback) {
                m_soundFinishedCallback(it->handle);
            }
            
            freeHandle(it->handle);
            it = m_instances.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioSystem::updateFades(f32 deltaTime) {
    for (auto& instance : m_instances) {
        if (instance.fadeRate > 0.0f) {
            f32 delta = instance.fadeRate * deltaTime;
            
            if (instance.params.volume < instance.fadeTarget) {
                instance.params.volume = std::min(instance.params.volume + delta, instance.fadeTarget);
            } else {
                instance.params.volume = std::max(instance.params.volume - delta, instance.fadeTarget);
            }
            
            // Check if fade complete
            if (std::abs(instance.params.volume - instance.fadeTarget) < 0.001f) {
                instance.params.volume = instance.fadeTarget;
                instance.fadeRate = 0.0f;
                
                if (instance.state == SoundState::Starting) {
                    instance.state = SoundState::Playing;
                } else if (instance.state == SoundState::Stopping) {
                    instance.state = SoundState::Stopped;
                }
            }
        }
    }
}

void AudioSystem::update3DAudio() {
    if (m_listenerCount == 0) return;
    
    // Get primary listener
    const auto& listener = m_listeners[0];
    
    for (auto& instance : m_instances) {
        if (!instance.params.spatialize) continue;
        
        const auto& source = instance.params.source3D;
        
        // Calculate distance attenuation
        f32 attenuation = calculateAttenuation(source, listener.position);
        
        // Calculate doppler shift
        f32 dopplerPitch = calculateDoppler(source, listener);
        
        // Calculate pan based on relative position
        Vec3 toSource = source.position - listener.position;
        // Calculate right vector from forward and up (cross product)
        Vec3 right = listener.forward.cross(listener.up).normalized();
        f32 pan = toSource.normalized().dot(right);
        
        // Apply to instance
        // Note: Would modulate audio based on these values
    }
}

f32 AudioSystem::calculateAttenuation(const AudioSource3D& source, const Vec3& listenerPos) {
    f32 distance = (source.position - listenerPos).length();
    
    if (distance <= source.minDistance) {
        return 1.0f;
    }
    if (distance >= source.maxDistance) {
        return 0.0f;
    }
    
    f32 range = source.maxDistance - source.minDistance;
    f32 normalizedDist = (distance - source.minDistance) / range;
    
    switch (source.attenuation) {
        case AttenuationModel::None:
            return 1.0f;
            
        case AttenuationModel::Linear:
            return 1.0f - normalizedDist;
            
        case AttenuationModel::Inverse:
            return source.minDistance / distance;
            
        case AttenuationModel::InverseSquare:
            return (source.minDistance * source.minDistance) / (distance * distance);
            
        case AttenuationModel::Logarithmic:
            return std::log(source.maxDistance / distance) / std::log(source.maxDistance / source.minDistance);
            
        default:
            return 1.0f;
    }
}

f32 AudioSystem::calculateDoppler(const AudioSource3D& source, const AudioListener& listener) {
    if (m_dopplerFactor == 0.0f || source.dopplerLevel == 0.0f) {
        return 1.0f;
    }
    
    Vec3 toListener = listener.position - source.position;
    f32 distance = toListener.length();
    if (distance < 0.001f) {
        return 1.0f;
    }
    
    Vec3 direction = toListener / distance;
    
    f32 sourceSpeed = source.velocity.dot(direction);
    f32 listenerSpeed = listener.velocity.dot(direction);
    
    f32 doppler = m_dopplerFactor * source.dopplerLevel;
    f32 pitch = (m_speedOfSound + listenerSpeed * doppler) / 
                (m_speedOfSound + sourceSpeed * doppler);
    
    return std::clamp(pitch, 0.5f, 2.0f);
}

} // namespace nova::audio
