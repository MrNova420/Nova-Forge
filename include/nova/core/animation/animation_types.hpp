/**
 * @file animation_types.hpp
 * @brief NovaCore Animation System™ - Core Animation Types
 * 
 * Production-grade animation types for the NovaCore engine.
 * Supports skeletal animation, blending, IK, and procedural animation.
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace nova::animation {

// Use types from nova namespace (types.hpp)
using nova::u8;
using nova::u32;
using nova::u64;
using nova::i32;
using nova::i64;
using nova::f32;
using nova::f64;
using nova::usize;
using nova::Handle;

// Use types from nova::math namespace
using nova::math::Vec2;
using nova::math::Vec3;
using nova::math::Vec4;
using nova::math::Mat3;
using nova::math::Mat4;
using nova::math::Quat;
using nova::math::Transform;

// ============================================================================
// Forward Declarations
// ============================================================================

class AnimationClip;
class AnimationState;
class AnimationController;
class Skeleton;
class Bone;

// ============================================================================
// Handle Types
// ============================================================================

/// Handle to an animation clip
using AnimationClipHandle = Handle<struct AnimationClipTag>;

/// Handle to a skeleton
using SkeletonHandle = Handle<struct SkeletonTag>;

/// Handle to an animation state machine
using AnimationControllerHandle = Handle<struct AnimationControllerTag>;

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Interpolation mode for animation keyframes
 */
enum class InterpolationMode : u8 {
    Step,           ///< No interpolation (snap to keyframe)
    Linear,         ///< Linear interpolation
    Bezier,         ///< Cubic bezier interpolation
    Hermite,        ///< Hermite spline interpolation
    CatmullRom,     ///< Catmull-Rom spline interpolation
    Custom          ///< Custom interpolation curve
};

/**
 * @brief Animation playback state
 */
enum class PlaybackState : u8 {
    Stopped,        ///< Animation is stopped at beginning
    Playing,        ///< Animation is playing forward
    PlayingReverse, ///< Animation is playing backward
    Paused,         ///< Animation is paused at current time
    Finished        ///< Animation has finished (non-looping)
};

/**
 * @brief Animation wrap mode
 */
enum class WrapMode : u8 {
    Once,           ///< Play once and stop
    Loop,           ///< Loop continuously
    PingPong,       ///< Play forward, then backward, repeat
    ClampForever,   ///< Play once, hold last frame
    Mirror          ///< Mirror animation at loop point
};

/**
 * @brief Animation blend mode
 */
enum class BlendMode : u8 {
    Override,       ///< Completely override lower layers
    Additive,       ///< Add to lower layers
    Multiply,       ///< Multiply with lower layers
    Difference      ///< Apply difference from rest pose
};

/**
 * @brief Bone transform component mask
 */
enum class TransformMask : u8 {
    None        = 0,
    Position    = 1 << 0,
    Rotation    = 1 << 1,
    Scale       = 1 << 2,
    All         = Position | Rotation | Scale
};

inline TransformMask operator|(TransformMask a, TransformMask b) {
    return static_cast<TransformMask>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline TransformMask operator&(TransformMask a, TransformMask b) {
    return static_cast<TransformMask>(static_cast<u8>(a) & static_cast<u8>(b));
}

/**
 * @brief IK solver type
 */
enum class IKSolverType : u8 {
    None,           ///< No IK
    TwoBone,        ///< Two-bone IK (arm/leg)
    FABRIK,         ///< Forward And Backward Reaching IK
    CCD,            ///< Cyclic Coordinate Descent
    Jacobian,       ///< Jacobian transpose/pseudo-inverse
    FullBody        ///< Full body IK
};

/**
 * @brief Animation event type
 */
enum class AnimationEventType : u8 {
    Custom,         ///< Custom user event
    Sound,          ///< Sound playback event
    Particle,       ///< Particle spawn event
    Footstep,       ///< Footstep event for audio/VFX
    Attack,         ///< Attack hit frame
    Notify,         ///< Generic notification
    Branch,         ///< State machine branch point
    RootMotion      ///< Root motion marker
};

// ============================================================================
// Keyframe Structures
// ============================================================================

/**
 * @brief Single position keyframe
 */
struct PositionKeyframe {
    f32 time = 0.0f;            ///< Time in seconds
    Vec3 position{};            ///< Position value
    InterpolationMode interp = InterpolationMode::Linear;
    Vec3 inTangent{};           ///< Tangent for bezier/hermite
    Vec3 outTangent{};          ///< Tangent for bezier/hermite
};

/**
 * @brief Single rotation keyframe
 */
struct RotationKeyframe {
    f32 time = 0.0f;            ///< Time in seconds
    Quat rotation{};            ///< Rotation value
    InterpolationMode interp = InterpolationMode::Linear;
};

/**
 * @brief Single scale keyframe
 */
struct ScaleKeyframe {
    f32 time = 0.0f;            ///< Time in seconds
    Vec3 scale{1.0f, 1.0f, 1.0f}; ///< Scale value
    InterpolationMode interp = InterpolationMode::Linear;
    Vec3 inTangent{};           ///< Tangent for bezier/hermite
    Vec3 outTangent{};          ///< Tangent for bezier/hermite
};

/**
 * @brief Animation event keyframe
 */
struct AnimationEvent {
    f32 time = 0.0f;            ///< Time in seconds
    AnimationEventType type = AnimationEventType::Custom;
    std::string name;           ///< Event name/identifier
    std::string stringParam;    ///< String parameter
    i32 intParam = 0;           ///< Integer parameter
    f32 floatParam = 0.0f;      ///< Float parameter
};

// ============================================================================
// Bone and Skeleton
// ============================================================================

/**
 * @brief Bone local transform
 */
struct BoneTransform {
    Vec3 position{};            ///< Local position
    Quat rotation{};            ///< Local rotation
    Vec3 scale{1.0f, 1.0f, 1.0f}; ///< Local scale
    
    /// Get transform matrix
    Mat4 toMatrix() const {
        return Mat4::translate(position) * rotation.toMat4() * Mat4::scale(scale);
    }
    
    /// Interpolate between transforms
    static BoneTransform lerp(const BoneTransform& a, const BoneTransform& b, f32 t) {
        BoneTransform result;
        result.position = a.position.lerp(b.position, t);
        result.rotation = a.rotation.slerp(b.rotation, t);
        result.scale = a.scale.lerp(b.scale, t);
        return result;
    }
    
    /// Blend transforms with weight
    static BoneTransform blend(const BoneTransform& a, const BoneTransform& b, f32 weight) {
        return lerp(a, b, weight);
    }
    
    /// Additive blend
    static BoneTransform additive(const BoneTransform& base, const BoneTransform& add, f32 weight) {
        BoneTransform result;
        result.position = base.position + add.position * weight;
        result.rotation = base.rotation * Quat{}.slerp(add.rotation, weight);
        result.scale = base.scale * Vec3{1.0f, 1.0f, 1.0f}.lerp(add.scale, weight);
        return result;
    }
};

/**
 * @brief Bone definition in skeleton
 */
struct BoneInfo {
    std::string name;           ///< Bone name
    i32 parentIndex = -1;       ///< Parent bone index (-1 for root)
    BoneTransform bindPose;     ///< Bind pose transform
    Mat4 inverseBindMatrix{};   ///< Inverse bind pose matrix for skinning
    
    // Limits for procedural animation / physics
    Vec3 minRotation{-math::PI_F32, -math::PI_F32, -math::PI_F32};
    Vec3 maxRotation{math::PI_F32, math::PI_F32, math::PI_F32};
};

/**
 * @brief Complete skeleton definition
 */
struct SkeletonData {
    std::string name;
    std::vector<BoneInfo> bones;
    std::unordered_map<std::string, i32> boneNameToIndex;
    
    /// Get bone index by name
    i32 findBone(const std::string& boneName) const {
        auto it = boneNameToIndex.find(boneName);
        return (it != boneNameToIndex.end()) ? it->second : -1;
    }
    
    /// Get number of bones
    usize getBoneCount() const { return bones.size(); }
    
    /// Check if valid
    bool isValid() const { return !bones.empty(); }
};

// ============================================================================
// Animation Clip
// ============================================================================

/**
 * @brief Animation channel for a single bone
 */
struct AnimationChannel {
    i32 boneIndex = -1;         ///< Target bone index
    std::string boneName;       ///< Target bone name (for remapping)
    
    std::vector<PositionKeyframe> positionKeys;
    std::vector<RotationKeyframe> rotationKeys;
    std::vector<ScaleKeyframe> scaleKeys;
    
    /// Check if channel has any data
    bool isEmpty() const {
        return positionKeys.empty() && rotationKeys.empty() && scaleKeys.empty();
    }
    
    /// Get channel duration
    f32 getDuration() const {
        f32 duration = 0.0f;
        if (!positionKeys.empty()) {
            duration = std::max(duration, positionKeys.back().time);
        }
        if (!rotationKeys.empty()) {
            duration = std::max(duration, rotationKeys.back().time);
        }
        if (!scaleKeys.empty()) {
            duration = std::max(duration, scaleKeys.back().time);
        }
        return duration;
    }
};

/**
 * @brief Animation clip data
 */
struct AnimationClipData {
    std::string name;           ///< Clip name
    f32 duration = 0.0f;        ///< Total duration in seconds
    f32 framesPerSecond = 30.0f; ///< Original FPS for time conversion
    
    std::vector<AnimationChannel> channels;
    std::vector<AnimationEvent> events;
    
    // Root motion
    bool hasRootMotion = false;
    Vec3 rootMotionPosition{};  ///< Total root motion translation
    f32 rootMotionRotation = 0.0f; ///< Total root motion rotation (Y-axis)
    
    /// Calculate duration from channels
    void calculateDuration() {
        duration = 0.0f;
        for (const auto& channel : channels) {
            duration = std::max(duration, channel.getDuration());
        }
    }
};

// ============================================================================
// Animation Instance / Playback
// ============================================================================

/**
 * @brief Animation playback parameters
 */
struct PlaybackParams {
    f32 speed = 1.0f;           ///< Playback speed multiplier
    f32 startTime = 0.0f;       ///< Start time offset
    f32 blendInTime = 0.0f;     ///< Blend in duration
    f32 blendOutTime = 0.0f;    ///< Blend out duration
    WrapMode wrapMode = WrapMode::Loop;
    bool normalized = false;     ///< Use normalized time (0-1)
    
    /// Default params for one-shot animation
    static PlaybackParams oneShot(f32 blendIn = 0.2f, f32 blendOut = 0.2f) {
        PlaybackParams p;
        p.wrapMode = WrapMode::Once;
        p.blendInTime = blendIn;
        p.blendOutTime = blendOut;
        return p;
    }
    
    /// Default params for looping animation
    static PlaybackParams looping(f32 blendIn = 0.2f) {
        PlaybackParams p;
        p.wrapMode = WrapMode::Loop;
        p.blendInTime = blendIn;
        return p;
    }
};

/**
 * @brief Animation instance state
 */
struct AnimationInstance {
    AnimationClipHandle clipHandle;
    PlaybackState state = PlaybackState::Stopped;
    f32 currentTime = 0.0f;     ///< Current playback time
    f32 weight = 1.0f;          ///< Current blend weight
    f32 speed = 1.0f;           ///< Current speed multiplier
    WrapMode wrapMode = WrapMode::Loop;
    BlendMode blendMode = BlendMode::Override;
    i32 loopCount = 0;          ///< Number of times looped
    
    // Blend state
    f32 blendInTime = 0.0f;
    f32 blendOutTime = 0.0f;
    f32 blendInProgress = 0.0f;
    f32 blendOutProgress = 0.0f;
    bool isBlendingIn = false;
    bool isBlendingOut = false;
    
    /// Get normalized time (0-1)
    f32 getNormalizedTime(f32 duration) const {
        return (duration > 0.0f) ? (currentTime / duration) : 0.0f;
    }
    
    /// Check if playing
    bool isPlaying() const {
        return state == PlaybackState::Playing || state == PlaybackState::PlayingReverse;
    }
    
    /// Check if finished
    bool isFinished() const {
        return state == PlaybackState::Finished;
    }
};

// ============================================================================
// Animation Layer
// ============================================================================

/**
 * @brief Animation layer for blending multiple animations
 */
struct AnimationLayer {
    std::string name;
    i32 index = 0;              ///< Layer index (higher = on top)
    f32 weight = 1.0f;          ///< Layer weight
    BlendMode blendMode = BlendMode::Override;
    TransformMask mask = TransformMask::All;
    
    // Bone mask - which bones are affected
    std::vector<f32> boneMask;  ///< Per-bone weight (0-1)
    
    // Active animations on this layer
    std::vector<AnimationInstance> animations;
    
    /// Check if layer affects a bone
    f32 getBoneWeight(i32 boneIndex) const {
        if (boneMask.empty() || boneIndex < 0 || 
            static_cast<usize>(boneIndex) >= boneMask.size()) {
            return 1.0f;
        }
        return boneMask[boneIndex];
    }
};

// ============================================================================
// IK Types
// ============================================================================

/**
 * @brief IK target for a limb
 */
struct IKTarget {
    Vec3 position{};            ///< Target world position
    Quat rotation{};            ///< Target world rotation (optional)
    f32 positionWeight = 1.0f;  ///< Position influence (0-1)
    f32 rotationWeight = 0.0f;  ///< Rotation influence (0-1)
    bool isActive = false;      ///< Is IK target active
};

/**
 * @brief IK chain definition
 */
struct IKChain {
    std::string name;
    IKSolverType solverType = IKSolverType::TwoBone;
    
    // Bone indices in chain (tip to root order)
    std::vector<i32> boneIndices;
    
    // Target
    IKTarget target;
    
    // Solver parameters
    i32 maxIterations = 10;         ///< Max solver iterations
    f32 tolerance = 0.001f;         ///< Position tolerance for convergence
    f32 weight = 1.0f;              ///< IK weight (0 = FK only, 1 = full IK)
    f32 maxAnglePerJoint = 0.5f;    ///< Maximum angle change per joint per iteration (radians)
    
    // Pole vector for orientation (e.g., knee/elbow direction)
    Vec3 poleVector{0.0f, 0.0f, 1.0f};
    bool usePoleVector = false;
    
    // Twist distribution along chain
    f32 twistDistribution = 0.5f;
};

/**
 * @brief Look-at IK constraint
 */
struct LookAtConstraint {
    std::string name;
    i32 boneIndex = -1;         ///< Head/eye bone index
    Vec3 targetPosition{};      ///< Look at target
    Vec3 upVector{0.0f, 1.0f, 0.0f}; ///< Up reference
    f32 weight = 1.0f;          ///< Constraint weight
    f32 maxAngle = math::PI_F32; ///< Maximum rotation angle
    bool isActive = false;
};

// ============================================================================
// Animation State Machine
// ============================================================================

/**
 * @brief Transition condition operator
 */
enum class ConditionOperator : u8 {
    Equal,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual
};

/**
 * @brief Animation parameter type
 */
enum class AnimParamType : u8 {
    Float,
    Int,
    Bool,
    Trigger
};

/**
 * @brief Animation parameter value
 */
struct AnimParam {
    AnimParamType type = AnimParamType::Float;
    union {
        f32 floatValue;
        i32 intValue;
        bool boolValue;
    };
    std::string name;
    
    AnimParam() : floatValue(0.0f) {}
    
    static AnimParam makeFloat(const std::string& paramName, f32 value = 0.0f) {
        AnimParam p;
        p.type = AnimParamType::Float;
        p.name = paramName;
        p.floatValue = value;
        return p;
    }
    
    static AnimParam makeInt(const std::string& paramName, i32 value = 0) {
        AnimParam p;
        p.type = AnimParamType::Int;
        p.name = paramName;
        p.intValue = value;
        return p;
    }
    
    static AnimParam makeBool(const std::string& paramName, bool value = false) {
        AnimParam p;
        p.type = AnimParamType::Bool;
        p.name = paramName;
        p.boolValue = value;
        return p;
    }
    
    static AnimParam makeTrigger(const std::string& paramName) {
        AnimParam p;
        p.type = AnimParamType::Trigger;
        p.name = paramName;
        p.boolValue = false;
        return p;
    }
};

/**
 * @brief Transition condition
 */
struct TransitionCondition {
    std::string paramName;
    ConditionOperator op = ConditionOperator::Equal;
    AnimParam threshold;
};

/**
 * @brief State transition definition
 */
struct StateTransition {
    std::string name;
    i32 sourceStateIndex = -1;      ///< -1 = any state
    i32 targetStateIndex = -1;
    
    std::vector<TransitionCondition> conditions;
    
    f32 duration = 0.2f;            ///< Transition duration
    f32 offset = 0.0f;              ///< Offset into target animation
    bool hasExitTime = false;       ///< Wait for exit time
    f32 exitTime = 0.0f;            ///< Normalized exit time (0-1)
    
    /// Check if all conditions are met
    bool checkConditions(const std::unordered_map<std::string, AnimParam>& params) const;
};

/**
 * @brief Animation state in state machine
 */
struct AnimationStateData {
    std::string name;
    AnimationClipHandle clipHandle;
    
    f32 speed = 1.0f;
    WrapMode wrapMode = WrapMode::Loop;
    bool mirror = false;            ///< Mirror animation
    
    // Blend tree (optional, for complex states)
    std::vector<AnimationClipHandle> blendTreeClips;
    std::string blendParamX;        ///< Parameter for X-axis blend
    std::string blendParamY;        ///< Parameter for Y-axis blend
    
    // State events
    std::function<void()> onEnter;
    std::function<void()> onExit;
    std::function<void(f32)> onUpdate;
};

/**
 * @brief Animation state machine data
 */
struct AnimationStateMachineData {
    std::string name;
    
    std::vector<AnimationStateData> states;
    std::vector<StateTransition> transitions;
    std::unordered_map<std::string, AnimParam> parameters;
    
    i32 defaultStateIndex = 0;
    
    /// Get state index by name
    i32 findState(const std::string& stateName) const {
        for (usize i = 0; i < states.size(); ++i) {
            if (states[i].name == stateName) {
                return static_cast<i32>(i);
            }
        }
        return -1;
    }
    
    /// Add a parameter
    void addParameter(const AnimParam& param) {
        parameters[param.name] = param;
    }
};

// ============================================================================
// Animation Pose
// ============================================================================

/**
 * @brief Complete animation pose for all bones
 */
struct AnimationPose {
    std::vector<BoneTransform> localTransforms;     ///< Local space transforms
    std::vector<Mat4> worldTransforms;              ///< World space matrices
    std::vector<Mat4> skinningMatrices;             ///< Final skinning matrices
    
    /// Initialize pose for skeleton
    void initialize(usize boneCount) {
        localTransforms.resize(boneCount);
        worldTransforms.resize(boneCount, Mat4::identity());
        skinningMatrices.resize(boneCount, Mat4::identity());
    }
    
    /// Reset to identity
    void reset() {
        for (auto& t : localTransforms) {
            t = BoneTransform{};
        }
        for (auto& m : worldTransforms) {
            m = Mat4::identity();
        }
        for (auto& m : skinningMatrices) {
            m = Mat4::identity();
        }
    }
    
    /// Copy from another pose
    void copyFrom(const AnimationPose& other) {
        localTransforms = other.localTransforms;
        worldTransforms = other.worldTransforms;
        skinningMatrices = other.skinningMatrices;
    }
    
    /// Blend with another pose
    void blendWith(const AnimationPose& other, f32 weight) {
        if (localTransforms.size() != other.localTransforms.size()) {
            return;
        }
        for (usize i = 0; i < localTransforms.size(); ++i) {
            localTransforms[i] = BoneTransform::blend(localTransforms[i], 
                                                       other.localTransforms[i], 
                                                       weight);
        }
    }
};

// ============================================================================
// Animation Statistics
// ============================================================================

/**
 * @brief Animation system statistics
 */
struct AnimationStats {
    u32 activeSamplers = 0;         ///< Number of active animation samplers
    u32 totalBones = 0;             ///< Total bones being animated
    u32 ikChainsEvaluated = 0;      ///< IK chains evaluated this frame
    u32 eventsTriggered = 0;        ///< Events triggered this frame
    f64 evaluationTimeMs = 0.0;     ///< Time spent in animation evaluation
    f64 ikSolveTimeMs = 0.0;        ///< Time spent in IK solving
    f64 blendingTimeMs = 0.0;       ///< Time spent in pose blending
};

// ============================================================================
// Callbacks
// ============================================================================

/// Animation event callback
using AnimationEventCallback = std::function<void(const AnimationEvent&)>;

/// Animation finished callback
using AnimationFinishedCallback = std::function<void(AnimationClipHandle)>;

/// State machine state changed callback
using StateChangedCallback = std::function<void(const std::string& oldState, 
                                                 const std::string& newState)>;

} // namespace nova::animation
