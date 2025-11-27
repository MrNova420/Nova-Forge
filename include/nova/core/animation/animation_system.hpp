/**
 * @file animation_system.hpp
 * @brief NovaCore Animation System™ - Main Animation Manager
 * 
 * Production-grade animation manager for the NovaCore engine.
 * Handles animation playback, blending, IK, and state machines.
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include "animation_types.hpp"
#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>

namespace nova::animation {

// Already using nova::types and nova::math from animation_types.hpp

/**
 * @brief Animation system configuration
 */
struct AnimationSystemConfig {
    u32 maxSkeletons = 256;             ///< Maximum concurrent skeletons
    u32 maxClips = 1024;                ///< Maximum loaded animation clips
    u32 maxActiveAnimations = 512;      ///< Maximum concurrent active animations
    u32 maxIKChains = 128;              ///< Maximum IK chains
    u32 maxLayers = 8;                  ///< Maximum animation layers per entity
    f32 defaultBlendTime = 0.2f;        ///< Default blend duration
    bool enableRootMotion = true;       ///< Enable root motion by default
    bool enableIK = true;               ///< Enable IK by default
    bool enableEvents = true;           ///< Enable animation events
};

/**
 * @brief Animation sampler for a single entity
 */
class AnimationSampler {
public:
    AnimationSampler() = default;
    ~AnimationSampler() = default;
    
    /// Initialize with skeleton
    void initialize(const SkeletonData& skeleton);
    
    /// Reset sampler
    void reset();
    
    /// Update all animations
    void update(f32 deltaTime);
    
    /// Get final pose
    const AnimationPose& getPose() const { return m_finalPose; }
    
    // Layer management
    AnimationLayer* addLayer(const std::string& name, i32 index = -1);
    AnimationLayer* getLayer(const std::string& name);
    AnimationLayer* getLayer(i32 index);
    void removeLayer(const std::string& name);
    void setLayerWeight(const std::string& name, f32 weight);
    
    // Animation playback
    AnimationInstance* play(AnimationClipHandle clip, 
                            const PlaybackParams& params = {},
                            const std::string& layer = "Base");
    void stop(AnimationClipHandle clip);
    void stopAll();
    void pause(AnimationClipHandle clip);
    void resume(AnimationClipHandle clip);
    
    // Crossfade
    void crossfade(AnimationClipHandle toClip, f32 duration = 0.2f,
                   const std::string& layer = "Base");
    
    // IK
    void setIKTarget(const std::string& chainName, const IKTarget& target);
    void enableIK(const std::string& chainName, bool enabled);
    void addIKChain(const IKChain& chain);
    void removeIKChain(const std::string& chainName);
    
    // Look-at
    void setLookAtTarget(const Vec3& target);
    void enableLookAt(bool enabled);
    
    // Events
    void setEventCallback(AnimationEventCallback callback);
    void setFinishedCallback(AnimationFinishedCallback callback);
    
    // Root motion
    Vec3 consumeRootMotionDelta();
    f32 consumeRootRotationDelta();
    
private:
    SkeletonData m_skeleton;
    AnimationPose m_finalPose;
    AnimationPose m_tempPose;
    
    std::vector<AnimationLayer> m_layers;
    std::unordered_map<std::string, usize> m_layerMap;
    
    std::vector<IKChain> m_ikChains;
    std::unordered_map<std::string, usize> m_ikChainMap;
    
    LookAtConstraint m_lookAt;
    
    Vec3 m_rootMotionDelta{};
    f32 m_rootRotationDelta = 0.0f;
    
    AnimationEventCallback m_eventCallback;
    AnimationFinishedCallback m_finishedCallback;
    
    // Internal methods
    void sampleAnimation(const AnimationClipData& clip, f32 time, 
                         AnimationPose& outPose);
    void blendLayers();
    void solveIK();
    void calculateWorldTransforms();
    void calculateSkinningMatrices();
    void processEvents(const AnimationClipData& clip, f32 prevTime, f32 currTime);
    
    // Interpolation helpers
    Vec3 interpolatePosition(const std::vector<PositionKeyframe>& keys, f32 time);
    Quat interpolateRotation(const std::vector<RotationKeyframe>& keys, f32 time);
    Vec3 interpolateScale(const std::vector<ScaleKeyframe>& keys, f32 time);
};

/**
 * @brief Animation state machine instance
 */
class AnimationStateMachine {
public:
    AnimationStateMachine() = default;
    ~AnimationStateMachine() = default;
    
    /// Initialize with state machine data
    void initialize(const AnimationStateMachineData& data);
    
    /// Update state machine
    void update(f32 deltaTime, AnimationSampler& sampler);
    
    /// Get current state name
    const std::string& getCurrentStateName() const;
    
    /// Get current state index
    i32 getCurrentStateIndex() const { return m_currentStateIndex; }
    
    /// Check if currently transitioning
    bool isTransitioning() const { return m_isTransitioning; }
    
    /// Get transition progress (0-1)
    f32 getTransitionProgress() const { return m_transitionProgress; }
    
    // Parameter access
    void setFloat(const std::string& name, f32 value);
    void setInt(const std::string& name, i32 value);
    void setBool(const std::string& name, bool value);
    void setTrigger(const std::string& name);
    
    f32 getFloat(const std::string& name) const;
    i32 getInt(const std::string& name) const;
    bool getBool(const std::string& name) const;
    
    /// Set state changed callback
    void setStateChangedCallback(StateChangedCallback callback);
    
private:
    AnimationStateMachineData m_data;
    
    i32 m_currentStateIndex = 0;
    i32 m_nextStateIndex = -1;
    
    bool m_isTransitioning = false;
    f32 m_transitionProgress = 0.0f;
    f32 m_transitionDuration = 0.0f;
    f32 m_transitionOffset = 0.0f;
    
    StateChangedCallback m_stateChangedCallback;
    
    void checkTransitions();
    void startTransition(const StateTransition& transition);
    void finishTransition();
    void resetTriggers();
};

/**
 * @brief Main animation system singleton
 */
class AnimationSystem {
public:
    /// Get singleton instance
    static AnimationSystem& get();
    
    /// Initialize the system
    bool initialize(const AnimationSystemConfig& config = {});
    
    /// Shutdown the system
    void shutdown();
    
    /// Update all active animations
    void update(f32 deltaTime);
    
    // Skeleton management
    SkeletonHandle loadSkeleton(const std::string& path);
    SkeletonHandle createSkeleton(const SkeletonData& data);
    void unloadSkeleton(SkeletonHandle handle);
    const SkeletonData* getSkeleton(SkeletonHandle handle) const;
    
    // Clip management
    AnimationClipHandle loadClip(const std::string& path);
    AnimationClipHandle createClip(const AnimationClipData& data);
    void unloadClip(AnimationClipHandle handle);
    const AnimationClipData* getClip(AnimationClipHandle handle) const;
    
    // Sampler management
    AnimationSampler* createSampler(SkeletonHandle skeleton);
    void destroySampler(AnimationSampler* sampler);
    
    // State machine management
    AnimationControllerHandle createController(const AnimationStateMachineData& data);
    void destroyController(AnimationControllerHandle handle);
    AnimationStateMachine* getController(AnimationControllerHandle handle);
    
    // Statistics
    const AnimationStats& getStats() const { return m_stats; }
    void resetStats();
    
    // Configuration
    const AnimationSystemConfig& getConfig() const { return m_config; }
    
private:
    AnimationSystem() = default;
    ~AnimationSystem() = default;
    
    AnimationSystem(const AnimationSystem&) = delete;
    AnimationSystem& operator=(const AnimationSystem&) = delete;
    
    bool m_initialized = false;
    AnimationSystemConfig m_config;
    AnimationStats m_stats;
    
    // Storage
    std::unordered_map<u64, SkeletonData> m_skeletons;
    std::unordered_map<u64, AnimationClipData> m_clips;
    std::unordered_map<u64, std::unique_ptr<AnimationStateMachine>> m_controllers;
    std::vector<std::unique_ptr<AnimationSampler>> m_samplers;
    
    std::atomic<u64> m_nextSkeletonId{1};
    std::atomic<u64> m_nextClipId{1};
    std::atomic<u64> m_nextControllerId{1};
    
    mutable std::mutex m_mutex;
    
    // Loading helpers
    bool loadSkeletonFromFile(const std::string& path, SkeletonData& outData);
    bool loadClipFromFile(const std::string& path, AnimationClipData& outData);
};

// ============================================================================
// Main Include Header
// ============================================================================

/**
 * @brief Initialize animation system
 */
inline bool initializeAnimation(const AnimationSystemConfig& config = {}) {
    return AnimationSystem::get().initialize(config);
}

/**
 * @brief Shutdown animation system
 */
inline void shutdownAnimation() {
    AnimationSystem::get().shutdown();
}

/**
 * @brief Update animation system
 */
inline void updateAnimation(f32 deltaTime) {
    AnimationSystem::get().update(deltaTime);
}

} // namespace nova::animation
