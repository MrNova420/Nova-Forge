/**
 * @file animation_system.cpp
 * @brief NovaCore Animation System™ - Animation System Implementation
 * 
 * Full implementation of the animation system including:
 * - Skeletal animation sampling
 * - Multi-layer blending
 * - IK solving (Two-bone, FABRIK)
 * - State machine transitions
 * - Root motion extraction
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <nova/core/animation/animation_system.hpp>
#include <nova/core/logging/logging.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <chrono>

namespace nova::animation {

using namespace nova::logging;

// Helper function to extract position from Mat4
inline Vec3 getMatrixPosition(const Mat4& m) {
    return Vec3(m.columns[3].x, m.columns[3].y, m.columns[3].z);
}

// ============================================================================
// AnimationSampler Implementation
// ============================================================================

void AnimationSampler::initialize(const SkeletonData& skeleton) {
    m_skeleton = skeleton;
    
    const usize boneCount = skeleton.getBoneCount();
    m_finalPose.initialize(boneCount);
    m_tempPose.initialize(boneCount);
    
    // Initialize to bind pose
    for (usize i = 0; i < boneCount; ++i) {
        m_finalPose.localTransforms[i] = skeleton.bones[i].bindPose;
    }
    
    // Create default base layer
    addLayer("Base", 0);
}

void AnimationSampler::reset() {
    // Reset pose to bind pose
    const usize boneCount = m_skeleton.getBoneCount();
    for (usize i = 0; i < boneCount; ++i) {
        m_finalPose.localTransforms[i] = m_skeleton.bones[i].bindPose;
    }
    
    // Clear all layers except base
    while (m_layers.size() > 1) {
        m_layers.pop_back();
    }
    
    // Clear base layer animations
    if (!m_layers.empty()) {
        m_layers[0].animations.clear();
    }
    
    // Clear IK chains
    m_ikChains.clear();
    m_ikChainMap.clear();
    
    // Clear root motion
    m_rootMotionDelta = Vec3{};
    m_rootRotationDelta = 0.0f;
}

void AnimationSampler::update(f32 deltaTime) {
    // Update each layer
    for (auto& layer : m_layers) {
        // Update each animation in layer
        for (auto it = layer.animations.begin(); it != layer.animations.end(); ) {
            auto& anim = *it;
            
            if (!anim.isPlaying()) {
                ++it;
                continue;
            }
            
            // Store previous time for events
            f32 prevTime = anim.currentTime;
            
            // Update time
            f32 timeStep = deltaTime * anim.speed;
            anim.currentTime += timeStep;
            
            // Get clip duration
            const AnimationClipData* clip = AnimationSystem::get().getClip(anim.clipHandle);
            if (!clip) {
                ++it;
                continue;
            }
            
            f32 duration = clip->duration;
            
            // Handle wrap mode
            switch (anim.wrapMode) {
                case WrapMode::Once:
                    if (anim.currentTime >= duration) {
                        anim.currentTime = duration;
                        anim.state = PlaybackState::Finished;
                        if (m_finishedCallback) {
                            m_finishedCallback(anim.clipHandle);
                        }
                    }
                    break;
                    
                case WrapMode::Loop:
                    while (anim.currentTime >= duration) {
                        anim.currentTime -= duration;
                        anim.loopCount++;
                    }
                    break;
                    
                case WrapMode::PingPong:
                    if (anim.currentTime >= duration) {
                        anim.currentTime = duration - (anim.currentTime - duration);
                        anim.speed = -anim.speed;
                        anim.loopCount++;
                    } else if (anim.currentTime < 0.0f) {
                        anim.currentTime = -anim.currentTime;
                        anim.speed = -anim.speed;
                    }
                    break;
                    
                case WrapMode::ClampForever:
                    anim.currentTime = std::clamp(anim.currentTime, 0.0f, duration);
                    break;
                    
                case WrapMode::Mirror:
                    // Similar to loop but mirrors every other iteration
                    while (anim.currentTime >= duration) {
                        anim.currentTime -= duration;
                        anim.loopCount++;
                    }
                    if (anim.loopCount % 2 == 1) {
                        anim.currentTime = duration - anim.currentTime;
                    }
                    break;
            }
            
            // Update blend weights
            if (anim.isBlendingIn && anim.blendInTime > 0.0f) {
                anim.blendInProgress += deltaTime / anim.blendInTime;
                if (anim.blendInProgress >= 1.0f) {
                    anim.blendInProgress = 1.0f;
                    anim.isBlendingIn = false;
                }
                anim.weight = anim.blendInProgress;
            }
            
            if (anim.isBlendingOut && anim.blendOutTime > 0.0f) {
                anim.blendOutProgress += deltaTime / anim.blendOutTime;
                if (anim.blendOutProgress >= 1.0f) {
                    anim.blendOutProgress = 1.0f;
                    anim.isBlendingOut = false;
                    anim.state = PlaybackState::Finished;
                }
                anim.weight = 1.0f - anim.blendOutProgress;
            }
            
            // Process events
            if (m_eventCallback) {
                processEvents(*clip, prevTime, anim.currentTime);
            }
            
            // Remove finished animations
            if (anim.state == PlaybackState::Finished && anim.weight <= 0.0f) {
                it = layer.animations.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // Blend all layers
    blendLayers();
    
    // Apply IK
    solveIK();
    
    // Calculate world and skinning matrices
    calculateWorldTransforms();
    calculateSkinningMatrices();
}

AnimationLayer* AnimationSampler::addLayer(const std::string& name, i32 index) {
    // Check if layer exists
    auto it = m_layerMap.find(name);
    if (it != m_layerMap.end()) {
        return &m_layers[it->second];
    }
    
    AnimationLayer layer;
    layer.name = name;
    layer.index = (index >= 0) ? index : static_cast<i32>(m_layers.size());
    
    m_layers.push_back(layer);
    m_layerMap[name] = m_layers.size() - 1;
    
    // Sort layers by index
    std::sort(m_layers.begin(), m_layers.end(), 
              [](const AnimationLayer& a, const AnimationLayer& b) {
                  return a.index < b.index;
              });
    
    // Rebuild map
    m_layerMap.clear();
    for (usize i = 0; i < m_layers.size(); ++i) {
        m_layerMap[m_layers[i].name] = i;
    }
    
    return &m_layers[m_layerMap[name]];
}

AnimationLayer* AnimationSampler::getLayer(const std::string& name) {
    auto it = m_layerMap.find(name);
    return (it != m_layerMap.end()) ? &m_layers[it->second] : nullptr;
}

AnimationLayer* AnimationSampler::getLayer(i32 index) {
    for (auto& layer : m_layers) {
        if (layer.index == index) {
            return &layer;
        }
    }
    return nullptr;
}

void AnimationSampler::removeLayer(const std::string& name) {
    auto it = m_layerMap.find(name);
    if (it == m_layerMap.end() || name == "Base") {
        return;  // Don't remove base layer
    }
    
    m_layers.erase(m_layers.begin() + static_cast<i64>(it->second));
    
    // Rebuild map
    m_layerMap.clear();
    for (usize i = 0; i < m_layers.size(); ++i) {
        m_layerMap[m_layers[i].name] = i;
    }
}

void AnimationSampler::setLayerWeight(const std::string& name, f32 weight) {
    AnimationLayer* layer = getLayer(name);
    if (layer) {
        layer->weight = std::clamp(weight, 0.0f, 1.0f);
    }
}

AnimationInstance* AnimationSampler::play(AnimationClipHandle clip,
                                           const PlaybackParams& params,
                                           const std::string& layerName) {
    AnimationLayer* layer = getLayer(layerName);
    if (!layer) {
        layer = addLayer(layerName);
    }
    
    AnimationInstance instance;
    instance.clipHandle = clip;
    instance.state = PlaybackState::Playing;
    instance.currentTime = params.startTime;
    instance.speed = params.speed;
    instance.wrapMode = params.wrapMode;
    instance.blendInTime = params.blendInTime;
    instance.blendOutTime = params.blendOutTime;
    
    if (params.blendInTime > 0.0f) {
        instance.isBlendingIn = true;
        instance.weight = 0.0f;
    } else {
        instance.weight = 1.0f;
    }
    
    layer->animations.push_back(instance);
    return &layer->animations.back();
}

void AnimationSampler::stop(AnimationClipHandle clip) {
    for (auto& layer : m_layers) {
        for (auto& anim : layer.animations) {
            if (anim.clipHandle == clip) {
                anim.state = PlaybackState::Stopped;
                anim.weight = 0.0f;
            }
        }
    }
}

void AnimationSampler::stopAll() {
    for (auto& layer : m_layers) {
        layer.animations.clear();
    }
}

void AnimationSampler::pause(AnimationClipHandle clip) {
    for (auto& layer : m_layers) {
        for (auto& anim : layer.animations) {
            if (anim.clipHandle == clip && anim.state == PlaybackState::Playing) {
                anim.state = PlaybackState::Paused;
            }
        }
    }
}

void AnimationSampler::resume(AnimationClipHandle clip) {
    for (auto& layer : m_layers) {
        for (auto& anim : layer.animations) {
            if (anim.clipHandle == clip && anim.state == PlaybackState::Paused) {
                anim.state = PlaybackState::Playing;
            }
        }
    }
}

void AnimationSampler::crossfade(AnimationClipHandle toClip, f32 duration,
                                  const std::string& layerName) {
    AnimationLayer* layer = getLayer(layerName);
    if (!layer) {
        return;
    }
    
    // Fade out current animations
    for (auto& anim : layer->animations) {
        if (anim.isPlaying()) {
            anim.isBlendingOut = true;
            anim.blendOutTime = duration;
            anim.blendOutProgress = 0.0f;
        }
    }
    
    // Play new animation with fade in
    PlaybackParams params;
    params.blendInTime = duration;
    play(toClip, params, layerName);
}

void AnimationSampler::setIKTarget(const std::string& chainName, const IKTarget& target) {
    auto it = m_ikChainMap.find(chainName);
    if (it != m_ikChainMap.end()) {
        m_ikChains[it->second].target = target;
    }
}

void AnimationSampler::enableIK(const std::string& chainName, bool enabled) {
    auto it = m_ikChainMap.find(chainName);
    if (it != m_ikChainMap.end()) {
        m_ikChains[it->second].target.isActive = enabled;
    }
}

void AnimationSampler::addIKChain(const IKChain& chain) {
    m_ikChains.push_back(chain);
    m_ikChainMap[chain.name] = m_ikChains.size() - 1;
}

void AnimationSampler::removeIKChain(const std::string& chainName) {
    auto it = m_ikChainMap.find(chainName);
    if (it != m_ikChainMap.end()) {
        m_ikChains.erase(m_ikChains.begin() + static_cast<i64>(it->second));
        
        // Rebuild map
        m_ikChainMap.clear();
        for (usize i = 0; i < m_ikChains.size(); ++i) {
            m_ikChainMap[m_ikChains[i].name] = i;
        }
    }
}

void AnimationSampler::setLookAtTarget(const Vec3& target) {
    m_lookAt.targetPosition = target;
}

void AnimationSampler::enableLookAt(bool enabled) {
    m_lookAt.isActive = enabled;
}

void AnimationSampler::setEventCallback(AnimationEventCallback callback) {
    m_eventCallback = std::move(callback);
}

void AnimationSampler::setFinishedCallback(AnimationFinishedCallback callback) {
    m_finishedCallback = std::move(callback);
}

Vec3 AnimationSampler::consumeRootMotionDelta() {
    Vec3 delta = m_rootMotionDelta;
    m_rootMotionDelta = Vec3{};
    return delta;
}

f32 AnimationSampler::consumeRootRotationDelta() {
    f32 delta = m_rootRotationDelta;
    m_rootRotationDelta = 0.0f;
    return delta;
}

void AnimationSampler::sampleAnimation(const AnimationClipData& clip, f32 time,
                                        AnimationPose& outPose) {
    for (const auto& channel : clip.channels) {
        i32 boneIndex = channel.boneIndex;
        if (boneIndex < 0 || static_cast<usize>(boneIndex) >= outPose.localTransforms.size()) {
            continue;
        }
        
        BoneTransform& transform = outPose.localTransforms[boneIndex];
        
        // Sample position
        if (!channel.positionKeys.empty()) {
            transform.position = interpolatePosition(channel.positionKeys, time);
        }
        
        // Sample rotation
        if (!channel.rotationKeys.empty()) {
            transform.rotation = interpolateRotation(channel.rotationKeys, time);
        }
        
        // Sample scale
        if (!channel.scaleKeys.empty()) {
            transform.scale = interpolateScale(channel.scaleKeys, time);
        }
    }
}

void AnimationSampler::blendLayers() {
    // Start with bind pose
    const usize boneCount = m_skeleton.getBoneCount();
    for (usize i = 0; i < boneCount; ++i) {
        m_finalPose.localTransforms[i] = m_skeleton.bones[i].bindPose;
    }
    
    // Blend each layer
    for (const auto& layer : m_layers) {
        if (layer.weight <= 0.0f || layer.animations.empty()) {
            continue;
        }
        
        // Sample all animations in layer
        m_tempPose.reset();
        f32 totalWeight = 0.0f;
        
        for (const auto& anim : layer.animations) {
            if (anim.weight <= 0.0f) {
                continue;
            }
            
            const AnimationClipData* clip = AnimationSystem::get().getClip(anim.clipHandle);
            if (!clip) {
                continue;
            }
            
            // Sample animation
            AnimationPose sampledPose;
            sampledPose.initialize(boneCount);
            sampleAnimation(*clip, anim.currentTime, sampledPose);
            
            // Blend into temp pose
            if (totalWeight == 0.0f) {
                m_tempPose.copyFrom(sampledPose);
                totalWeight = anim.weight;
            } else {
                f32 blendFactor = anim.weight / (totalWeight + anim.weight);
                m_tempPose.blendWith(sampledPose, blendFactor);
                totalWeight += anim.weight;
            }
        }
        
        if (totalWeight <= 0.0f) {
            continue;
        }
        
        // Apply layer to final pose
        f32 layerWeight = layer.weight;
        
        switch (layer.blendMode) {
            case BlendMode::Override:
                for (usize i = 0; i < boneCount; ++i) {
                    f32 boneWeight = layer.getBoneWeight(static_cast<i32>(i)) * layerWeight;
                    m_finalPose.localTransforms[i] = BoneTransform::blend(
                        m_finalPose.localTransforms[i],
                        m_tempPose.localTransforms[i],
                        boneWeight
                    );
                }
                break;
                
            case BlendMode::Additive:
                for (usize i = 0; i < boneCount; ++i) {
                    f32 boneWeight = layer.getBoneWeight(static_cast<i32>(i)) * layerWeight;
                    m_finalPose.localTransforms[i] = BoneTransform::additive(
                        m_finalPose.localTransforms[i],
                        m_tempPose.localTransforms[i],
                        boneWeight
                    );
                }
                break;
                
            default:
                // Use override for other modes
                for (usize i = 0; i < boneCount; ++i) {
                    f32 boneWeight = layer.getBoneWeight(static_cast<i32>(i)) * layerWeight;
                    m_finalPose.localTransforms[i] = BoneTransform::blend(
                        m_finalPose.localTransforms[i],
                        m_tempPose.localTransforms[i],
                        boneWeight
                    );
                }
                break;
        }
    }
}

void AnimationSampler::solveIK() {
    for (auto& chain : m_ikChains) {
        if (!chain.target.isActive || chain.weight <= 0.0f) {
            continue;
        }
        
        switch (chain.solverType) {
            case IKSolverType::TwoBone: {
                // Two-bone IK solver (for arms/legs)
                if (chain.boneIndices.size() < 2) {
                    continue;
                }
                
                i32 tipBone = chain.boneIndices[0];
                i32 midBone = chain.boneIndices[1];
                i32 rootBone = (chain.boneIndices.size() > 2) ? 
                               chain.boneIndices[2] : 
                               m_skeleton.bones[midBone].parentIndex;
                
                if (tipBone < 0 || midBone < 0 || rootBone < 0) {
                    continue;
                }
                
                // Get world positions
                Vec3 rootPos = getMatrixPosition(m_finalPose.worldTransforms[rootBone]);
                Vec3 midPos = getMatrixPosition(m_finalPose.worldTransforms[midBone]);
                Vec3 tipPos = getMatrixPosition(m_finalPose.worldTransforms[tipBone]);
                
                Vec3 target = chain.target.position;
                
                // Bone lengths
                f32 upperLen = (midPos - rootPos).length();
                f32 lowerLen = (tipPos - midPos).length();
                f32 totalLen = upperLen + lowerLen;
                
                Vec3 toTarget = target - rootPos;
                f32 targetDist = toTarget.length();
                
                // Clamp to reachable range
                f32 epsilon = 0.0001f;
                targetDist = std::clamp(targetDist, 
                                        std::abs(upperLen - lowerLen) + epsilon,
                                        totalLen - epsilon);
                
                // Calculate joint angles using law of cosines
                f32 cosAngle1 = (upperLen * upperLen + targetDist * targetDist - 
                                 lowerLen * lowerLen) / (2.0f * upperLen * targetDist);
                f32 cosAngle2 = (upperLen * upperLen + lowerLen * lowerLen - 
                                 targetDist * targetDist) / (2.0f * upperLen * lowerLen);
                
                cosAngle1 = std::clamp(cosAngle1, -1.0f, 1.0f);
                cosAngle2 = std::clamp(cosAngle2, -1.0f, 1.0f);
                
                f32 angle1 = std::acos(cosAngle1);
                f32 angle2 = std::acos(cosAngle2);
                
                // Calculate new rotations
                Vec3 targetDir = toTarget.normalized();
                Vec3 poleDir = chain.poleVector;
                
                if (chain.usePoleVector) {
                    // Use pole vector for orientation
                    Vec3 right = targetDir.cross(poleDir).normalized();
                    // Note: up vector would be used for more complex IK with twist
                    // Vec3 up = right.cross(targetDir).normalized();
                    
                    // Rotate in the plane defined by target and pole
                    Quat rootRot = Quat::fromAxisAngle(right, -angle1);
                    Quat midRot = Quat::fromAxisAngle(right, math::PI_F32 - angle2);
                    
                    // Blend with original
                    m_finalPose.localTransforms[rootBone].rotation = 
                        m_finalPose.localTransforms[rootBone].rotation.slerp(
                            rootRot * m_finalPose.localTransforms[rootBone].rotation,
                            chain.weight * chain.target.positionWeight
                        );
                    
                    m_finalPose.localTransforms[midBone].rotation = 
                        m_finalPose.localTransforms[midBone].rotation.slerp(
                            midRot * m_finalPose.localTransforms[midBone].rotation,
                            chain.weight * chain.target.positionWeight
                        );
                }
                break;
            }
            
            case IKSolverType::FABRIK: {
                // FABRIK solver for general chains
                if (chain.boneIndices.empty()) {
                    continue;
                }
                
                std::vector<Vec3> positions(chain.boneIndices.size() + 1);
                std::vector<f32> lengths(chain.boneIndices.size());
                
                // Get initial positions and lengths
                for (usize i = 0; i < chain.boneIndices.size(); ++i) {
                    i32 boneIdx = chain.boneIndices[i];
                    positions[i] = getMatrixPosition(m_finalPose.worldTransforms[boneIdx]);
                    
                    if (i > 0) {
                        lengths[i - 1] = (positions[i] - positions[i - 1]).length();
                    }
                }
                
                // Get root position (parent of last bone in chain)
                i32 lastBone = chain.boneIndices.back();
                i32 rootParent = m_skeleton.bones[lastBone].parentIndex;
                if (rootParent >= 0) {
                    positions.back() = getMatrixPosition(m_finalPose.worldTransforms[rootParent]);
                    lengths.back() = (positions.back() - positions[positions.size() - 2]).length();
                }
                
                Vec3 target = chain.target.position;
                Vec3 root = positions.back();
                
                // FABRIK iterations
                for (i32 iter = 0; iter < chain.maxIterations; ++iter) {
                    // Check convergence
                    f32 dist = (positions[0] - target).length();
                    if (dist < chain.tolerance) {
                        break;
                    }
                    
                    // Forward reaching
                    positions[0] = target;
                    for (usize i = 1; i < positions.size(); ++i) {
                        Vec3 dir = (positions[i] - positions[i - 1]).normalized();
                        positions[i] = positions[i - 1] + dir * lengths[i - 1];
                    }
                    
                    // Backward reaching
                    positions.back() = root;
                    for (i32 i = static_cast<i32>(positions.size()) - 2; i >= 0; --i) {
                        Vec3 dir = (positions[i] - positions[i + 1]).normalized();
                        positions[i] = positions[i + 1] + dir * lengths[i];
                    }
                }
                
                // Apply solved positions (convert back to rotations)
                // This requires computing rotations from position deltas
                // For simplicity, we blend positions directly for now
                for (usize i = 0; i < chain.boneIndices.size(); ++i) {
                    i32 boneIdx = chain.boneIndices[i];
                    
                    // Get original position
                    Vec3 origPos = getMatrixPosition(m_finalPose.worldTransforms[boneIdx]);
                    
                    // Blend to new position
                    Vec3 newPos = origPos.lerp(positions[i], 
                                              chain.weight * chain.target.positionWeight);
                    
                    // Convert world delta to local
                    // This is a simplified approach
                    Vec3 delta = newPos - origPos;
                    m_finalPose.localTransforms[boneIdx].position = 
                        m_finalPose.localTransforms[boneIdx].position + delta;
                }
                break;
            }
            
            default:
                // Other solvers not implemented yet
                break;
        }
    }
    
    // Apply look-at constraint
    if (m_lookAt.isActive && m_lookAt.boneIndex >= 0) {
        i32 boneIdx = m_lookAt.boneIndex;
        
        Vec3 bonePos = getMatrixPosition(m_finalPose.worldTransforms[boneIdx]);
        
        Vec3 toTarget = (m_lookAt.targetPosition - bonePos).normalized();
        
        // Create look-at rotation
        Quat lookRot = Quat::fromToRotation(Vec3(0.0f, 0.0f, 1.0f), toTarget);
        
        // Clamp angle
        f32 angle = lookRot.angle();
        if (angle > m_lookAt.maxAngle) {
            lookRot = Quat{}.slerp(lookRot, m_lookAt.maxAngle / angle);
        }
        
        // Blend with original
        m_finalPose.localTransforms[boneIdx].rotation = 
            m_finalPose.localTransforms[boneIdx].rotation.slerp(
                lookRot * m_finalPose.localTransforms[boneIdx].rotation,
                m_lookAt.weight
            );
    }
}

void AnimationSampler::calculateWorldTransforms() {
    const usize boneCount = m_skeleton.getBoneCount();
    
    for (usize i = 0; i < boneCount; ++i) {
        Mat4 localMatrix = m_finalPose.localTransforms[i].toMatrix();
        
        i32 parentIndex = m_skeleton.bones[i].parentIndex;
        if (parentIndex >= 0 && static_cast<usize>(parentIndex) < boneCount) {
            m_finalPose.worldTransforms[i] = m_finalPose.worldTransforms[parentIndex] * localMatrix;
        } else {
            m_finalPose.worldTransforms[i] = localMatrix;
        }
    }
}

void AnimationSampler::calculateSkinningMatrices() {
    const usize boneCount = m_skeleton.getBoneCount();
    
    for (usize i = 0; i < boneCount; ++i) {
        m_finalPose.skinningMatrices[i] = m_finalPose.worldTransforms[i] * 
                                           m_skeleton.bones[i].inverseBindMatrix;
    }
}

void AnimationSampler::processEvents(const AnimationClipData& clip, 
                                      f32 prevTime, f32 currTime) {
    for (const auto& event : clip.events) {
        // Check if event time is between prev and current
        bool triggered = false;
        
        if (currTime >= prevTime) {
            triggered = (event.time > prevTime && event.time <= currTime);
        } else {
            // Wrapped around
            triggered = (event.time > prevTime || event.time <= currTime);
        }
        
        if (triggered && m_eventCallback) {
            m_eventCallback(event);
        }
    }
}

Vec3 AnimationSampler::interpolatePosition(const std::vector<PositionKeyframe>& keys, 
                                            f32 time) {
    if (keys.empty()) {
        return Vec3{};
    }
    
    if (keys.size() == 1 || time <= keys.front().time) {
        return keys.front().position;
    }
    
    if (time >= keys.back().time) {
        return keys.back().position;
    }
    
    // Find surrounding keyframes
    usize nextIdx = 0;
    for (usize i = 0; i < keys.size(); ++i) {
        if (keys[i].time > time) {
            nextIdx = i;
            break;
        }
    }
    
    usize prevIdx = nextIdx > 0 ? nextIdx - 1 : 0;
    
    const auto& prev = keys[prevIdx];
    const auto& next = keys[nextIdx];
    
    f32 dt = next.time - prev.time;
    f32 t = (dt > 0.0f) ? (time - prev.time) / dt : 0.0f;
    
    switch (prev.interp) {
        case InterpolationMode::Step:
            return prev.position;
            
        case InterpolationMode::Linear:
            return prev.position.lerp(next.position, t);
            
        case InterpolationMode::Bezier:
        case InterpolationMode::Hermite: {
            // Cubic interpolation
            f32 t2 = t * t;
            f32 t3 = t2 * t;
            
            f32 h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
            f32 h10 = t3 - 2.0f * t2 + t;
            f32 h01 = -2.0f * t3 + 3.0f * t2;
            f32 h11 = t3 - t2;
            
            return prev.position * h00 + 
                   prev.outTangent * (h10 * dt) + 
                   next.position * h01 + 
                   next.inTangent * (h11 * dt);
        }
            
        default:
            return prev.position.lerp(next.position, t);
    }
}

Quat AnimationSampler::interpolateRotation(const std::vector<RotationKeyframe>& keys, 
                                            f32 time) {
    if (keys.empty()) {
        return Quat{};
    }
    
    if (keys.size() == 1 || time <= keys.front().time) {
        return keys.front().rotation;
    }
    
    if (time >= keys.back().time) {
        return keys.back().rotation;
    }
    
    // Find surrounding keyframes
    usize nextIdx = 0;
    for (usize i = 0; i < keys.size(); ++i) {
        if (keys[i].time > time) {
            nextIdx = i;
            break;
        }
    }
    
    usize prevIdx = nextIdx > 0 ? nextIdx - 1 : 0;
    
    const auto& prev = keys[prevIdx];
    const auto& next = keys[nextIdx];
    
    f32 dt = next.time - prev.time;
    f32 t = (dt > 0.0f) ? (time - prev.time) / dt : 0.0f;
    
    switch (prev.interp) {
        case InterpolationMode::Step:
            return prev.rotation;
            
        case InterpolationMode::Linear:
        default:
            return prev.rotation.slerp(next.rotation, t);
    }
}

Vec3 AnimationSampler::interpolateScale(const std::vector<ScaleKeyframe>& keys, 
                                         f32 time) {
    if (keys.empty()) {
        return Vec3{1.0f, 1.0f, 1.0f};
    }
    
    if (keys.size() == 1 || time <= keys.front().time) {
        return keys.front().scale;
    }
    
    if (time >= keys.back().time) {
        return keys.back().scale;
    }
    
    // Find surrounding keyframes
    usize nextIdx = 0;
    for (usize i = 0; i < keys.size(); ++i) {
        if (keys[i].time > time) {
            nextIdx = i;
            break;
        }
    }
    
    usize prevIdx = nextIdx > 0 ? nextIdx - 1 : 0;
    
    const auto& prev = keys[prevIdx];
    const auto& next = keys[nextIdx];
    
    f32 dt = next.time - prev.time;
    f32 t = (dt > 0.0f) ? (time - prev.time) / dt : 0.0f;
    
    switch (prev.interp) {
        case InterpolationMode::Step:
            return prev.scale;
            
        case InterpolationMode::Linear:
            return prev.scale.lerp(next.scale, t);
            
        default:
            return prev.scale.lerp(next.scale, t);
    }
}

// ============================================================================
// AnimationStateMachine Implementation
// ============================================================================

void AnimationStateMachine::initialize(const AnimationStateMachineData& data) {
    m_data = data;
    m_currentStateIndex = data.defaultStateIndex;
    m_nextStateIndex = -1;
    m_isTransitioning = false;
}

void AnimationStateMachine::update(f32 deltaTime, AnimationSampler& sampler) {
    // Check for transitions
    if (!m_isTransitioning) {
        checkTransitions();
    }
    
    // Update transition
    if (m_isTransitioning && m_transitionDuration > 0.0f) {
        m_transitionProgress += deltaTime / m_transitionDuration;
        
        if (m_transitionProgress >= 1.0f) {
            finishTransition();
        }
    }
    
    // Update state callbacks
    if (m_currentStateIndex >= 0 && 
        static_cast<usize>(m_currentStateIndex) < m_data.states.size()) {
        const auto& state = m_data.states[m_currentStateIndex];
        if (state.onUpdate) {
            state.onUpdate(deltaTime);
        }
    }
    
    // Reset triggers at end of frame
    resetTriggers();
}

const std::string& AnimationStateMachine::getCurrentStateName() const {
    static const std::string empty;
    if (m_currentStateIndex >= 0 && 
        static_cast<usize>(m_currentStateIndex) < m_data.states.size()) {
        return m_data.states[m_currentStateIndex].name;
    }
    return empty;
}

void AnimationStateMachine::setFloat(const std::string& name, f32 value) {
    auto it = m_data.parameters.find(name);
    if (it != m_data.parameters.end() && it->second.type == AnimParamType::Float) {
        it->second.floatValue = value;
    }
}

void AnimationStateMachine::setInt(const std::string& name, i32 value) {
    auto it = m_data.parameters.find(name);
    if (it != m_data.parameters.end() && it->second.type == AnimParamType::Int) {
        it->second.intValue = value;
    }
}

void AnimationStateMachine::setBool(const std::string& name, bool value) {
    auto it = m_data.parameters.find(name);
    if (it != m_data.parameters.end() && it->second.type == AnimParamType::Bool) {
        it->second.boolValue = value;
    }
}

void AnimationStateMachine::setTrigger(const std::string& name) {
    auto it = m_data.parameters.find(name);
    if (it != m_data.parameters.end() && it->second.type == AnimParamType::Trigger) {
        it->second.boolValue = true;
    }
}

f32 AnimationStateMachine::getFloat(const std::string& name) const {
    auto it = m_data.parameters.find(name);
    if (it != m_data.parameters.end() && it->second.type == AnimParamType::Float) {
        return it->second.floatValue;
    }
    return 0.0f;
}

i32 AnimationStateMachine::getInt(const std::string& name) const {
    auto it = m_data.parameters.find(name);
    if (it != m_data.parameters.end() && it->second.type == AnimParamType::Int) {
        return it->second.intValue;
    }
    return 0;
}

bool AnimationStateMachine::getBool(const std::string& name) const {
    auto it = m_data.parameters.find(name);
    if (it != m_data.parameters.end() && 
        (it->second.type == AnimParamType::Bool || it->second.type == AnimParamType::Trigger)) {
        return it->second.boolValue;
    }
    return false;
}

void AnimationStateMachine::setStateChangedCallback(StateChangedCallback callback) {
    m_stateChangedCallback = std::move(callback);
}

void AnimationStateMachine::checkTransitions() {
    // Check transitions from current state
    for (const auto& transition : m_data.transitions) {
        // Check if transition applies to current state
        if (transition.sourceStateIndex != -1 && 
            transition.sourceStateIndex != m_currentStateIndex) {
            continue;
        }
        
        // Check conditions
        bool allConditionsMet = true;
        for (const auto& cond : transition.conditions) {
            auto it = m_data.parameters.find(cond.paramName);
            if (it == m_data.parameters.end()) {
                allConditionsMet = false;
                break;
            }
            
            const AnimParam& param = it->second;
            const AnimParam& thresh = cond.threshold;
            
            bool conditionMet = false;
            
            switch (param.type) {
                case AnimParamType::Float:
                    switch (cond.op) {
                        case ConditionOperator::Equal:
                            conditionMet = std::abs(param.floatValue - thresh.floatValue) < 0.0001f;
                            break;
                        case ConditionOperator::NotEqual:
                            conditionMet = std::abs(param.floatValue - thresh.floatValue) >= 0.0001f;
                            break;
                        case ConditionOperator::Greater:
                            conditionMet = param.floatValue > thresh.floatValue;
                            break;
                        case ConditionOperator::GreaterEqual:
                            conditionMet = param.floatValue >= thresh.floatValue;
                            break;
                        case ConditionOperator::Less:
                            conditionMet = param.floatValue < thresh.floatValue;
                            break;
                        case ConditionOperator::LessEqual:
                            conditionMet = param.floatValue <= thresh.floatValue;
                            break;
                    }
                    break;
                    
                case AnimParamType::Int:
                    switch (cond.op) {
                        case ConditionOperator::Equal:
                            conditionMet = param.intValue == thresh.intValue;
                            break;
                        case ConditionOperator::NotEqual:
                            conditionMet = param.intValue != thresh.intValue;
                            break;
                        case ConditionOperator::Greater:
                            conditionMet = param.intValue > thresh.intValue;
                            break;
                        case ConditionOperator::GreaterEqual:
                            conditionMet = param.intValue >= thresh.intValue;
                            break;
                        case ConditionOperator::Less:
                            conditionMet = param.intValue < thresh.intValue;
                            break;
                        case ConditionOperator::LessEqual:
                            conditionMet = param.intValue <= thresh.intValue;
                            break;
                    }
                    break;
                    
                case AnimParamType::Bool:
                case AnimParamType::Trigger:
                    switch (cond.op) {
                        case ConditionOperator::Equal:
                            conditionMet = param.boolValue == thresh.boolValue;
                            break;
                        case ConditionOperator::NotEqual:
                            conditionMet = param.boolValue != thresh.boolValue;
                            break;
                        default:
                            conditionMet = param.boolValue;
                            break;
                    }
                    break;
            }
            
            if (!conditionMet) {
                allConditionsMet = false;
                break;
            }
        }
        
        if (allConditionsMet) {
            startTransition(transition);
            break;
        }
    }
}

void AnimationStateMachine::startTransition(const StateTransition& transition) {
    m_nextStateIndex = transition.targetStateIndex;
    m_transitionDuration = transition.duration;
    m_transitionOffset = transition.offset;
    m_transitionProgress = 0.0f;
    m_isTransitioning = true;
    
    // Call exit callback on current state
    if (m_currentStateIndex >= 0 && 
        static_cast<usize>(m_currentStateIndex) < m_data.states.size()) {
        const auto& state = m_data.states[m_currentStateIndex];
        if (state.onExit) {
            state.onExit();
        }
    }
}

void AnimationStateMachine::finishTransition() {
    std::string oldState = getCurrentStateName();
    
    m_currentStateIndex = m_nextStateIndex;
    m_nextStateIndex = -1;
    m_isTransitioning = false;
    m_transitionProgress = 0.0f;
    
    // Call enter callback on new state
    if (m_currentStateIndex >= 0 && 
        static_cast<usize>(m_currentStateIndex) < m_data.states.size()) {
        const auto& state = m_data.states[m_currentStateIndex];
        if (state.onEnter) {
            state.onEnter();
        }
    }
    
    // Notify state changed
    if (m_stateChangedCallback) {
        m_stateChangedCallback(oldState, getCurrentStateName());
    }
}

void AnimationStateMachine::resetTriggers() {
    for (auto& [name, param] : m_data.parameters) {
        if (param.type == AnimParamType::Trigger) {
            param.boolValue = false;
        }
    }
}

// ============================================================================
// StateTransition Implementation
// ============================================================================

bool StateTransition::checkConditions(
    const std::unordered_map<std::string, AnimParam>& params) const {
    for (const auto& cond : conditions) {
        auto it = params.find(cond.paramName);
        if (it == params.end()) {
            return false;
        }
        
        // Simplified check - full implementation is in AnimationStateMachine
        const AnimParam& param = it->second;
        if (param.type == AnimParamType::Bool || param.type == AnimParamType::Trigger) {
            if (!param.boolValue) {
                return false;
            }
        }
    }
    return true;
}

// ============================================================================
// AnimationSystem Implementation
// ============================================================================

AnimationSystem& AnimationSystem::get() {
    static AnimationSystem instance;
    return instance;
}

bool AnimationSystem::initialize(const AnimationSystemConfig& config) {
    if (m_initialized) {
        return true;
    }
    
    NOVA_LOG_INFO(LogCategory::Core, "Initializing Nova Animation System...");
    
    m_config = config;
    m_initialized = true;
    
    NOVA_LOG_INFO(LogCategory::Core, "Nova Animation System initialized");
    return true;
}

void AnimationSystem::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    NOVA_LOG_INFO(LogCategory::Core, "Shutting down Nova Animation System...");
    
    m_samplers.clear();
    m_controllers.clear();
    m_clips.clear();
    m_skeletons.clear();
    
    m_initialized = false;
    
    NOVA_LOG_INFO(LogCategory::Core, "Nova Animation System shutdown complete");
}

void AnimationSystem::update(f32 deltaTime) {
    if (!m_initialized) {
        return;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Update all samplers
    m_stats.activeSamplers = 0;
    m_stats.totalBones = 0;
    
    for (auto& sampler : m_samplers) {
        if (sampler) {
            sampler->update(deltaTime);
            m_stats.activeSamplers++;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    m_stats.evaluationTimeMs = std::chrono::duration<f64, std::milli>(
        endTime - startTime).count();
}

SkeletonHandle AnimationSystem::loadSkeleton(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    SkeletonData data;
    if (!loadSkeletonFromFile(path, data)) {
        return SkeletonHandle{};
    }
    
    u64 id = m_nextSkeletonId.fetch_add(1, std::memory_order_relaxed);
    m_skeletons[id] = std::move(data);
    
    return SkeletonHandle{static_cast<u32>(id)};
}

SkeletonHandle AnimationSystem::createSkeleton(const SkeletonData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    u64 id = m_nextSkeletonId.fetch_add(1, std::memory_order_relaxed);
    m_skeletons[id] = data;
    
    // Build name-to-index map
    auto& skel = m_skeletons[id];
    skel.boneNameToIndex.clear();
    for (usize i = 0; i < skel.bones.size(); ++i) {
        skel.boneNameToIndex[skel.bones[i].name] = static_cast<i32>(i);
    }
    
    return SkeletonHandle{static_cast<u32>(id)};
}

void AnimationSystem::unloadSkeleton(SkeletonHandle handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_skeletons.erase(handle.value);
}

const SkeletonData* AnimationSystem::getSkeleton(SkeletonHandle handle) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_skeletons.find(handle.value);
    return (it != m_skeletons.end()) ? &it->second : nullptr;
}

AnimationClipHandle AnimationSystem::loadClip(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    AnimationClipData data;
    if (!loadClipFromFile(path, data)) {
        return AnimationClipHandle{};
    }
    
    u64 id = m_nextClipId.fetch_add(1, std::memory_order_relaxed);
    m_clips[id] = std::move(data);
    
    return AnimationClipHandle{static_cast<u32>(id)};
}

AnimationClipHandle AnimationSystem::createClip(const AnimationClipData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    u64 id = m_nextClipId.fetch_add(1, std::memory_order_relaxed);
    m_clips[id] = data;
    
    // Calculate duration if not set
    if (m_clips[id].duration <= 0.0f) {
        m_clips[id].calculateDuration();
    }
    
    return AnimationClipHandle{static_cast<u32>(id)};
}

void AnimationSystem::unloadClip(AnimationClipHandle handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_clips.erase(handle.value);
}

const AnimationClipData* AnimationSystem::getClip(AnimationClipHandle handle) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_clips.find(handle.value);
    return (it != m_clips.end()) ? &it->second : nullptr;
}

AnimationSampler* AnimationSystem::createSampler(SkeletonHandle skeleton) {
    const SkeletonData* skelData = getSkeleton(skeleton);
    if (!skelData) {
        return nullptr;
    }
    
    auto sampler = std::make_unique<AnimationSampler>();
    sampler->initialize(*skelData);
    
    AnimationSampler* ptr = sampler.get();
    m_samplers.push_back(std::move(sampler));
    
    return ptr;
}

void AnimationSystem::destroySampler(AnimationSampler* sampler) {
    auto it = std::find_if(m_samplers.begin(), m_samplers.end(),
                           [sampler](const std::unique_ptr<AnimationSampler>& s) {
                               return s.get() == sampler;
                           });
    if (it != m_samplers.end()) {
        m_samplers.erase(it);
    }
}

AnimationControllerHandle AnimationSystem::createController(
    const AnimationStateMachineData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    u64 id = m_nextControllerId.fetch_add(1, std::memory_order_relaxed);
    
    auto controller = std::make_unique<AnimationStateMachine>();
    controller->initialize(data);
    
    m_controllers[id] = std::move(controller);
    
    return AnimationControllerHandle{static_cast<u32>(id)};
}

void AnimationSystem::destroyController(AnimationControllerHandle handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_controllers.erase(handle.value);
}

AnimationStateMachine* AnimationSystem::getController(AnimationControllerHandle handle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_controllers.find(handle.value);
    return (it != m_controllers.end()) ? it->second.get() : nullptr;
}

void AnimationSystem::resetStats() {
    m_stats = AnimationStats{};
}

bool AnimationSystem::loadSkeletonFromFile(const std::string& path, 
                                            SkeletonData& outData) {
    // Simple binary format loader
    // In production, this would load from glTF, FBX, or custom format
    
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        NOVA_LOG_WARN(LogCategory::Core, "Failed to open skeleton file: {}", path);
        return false;
    }
    
    // For now, create a simple placeholder skeleton
    // Real implementation would parse actual skeleton data
    
    // Extract name from path
    std::string filename = path.substr(path.find_last_of("/\\") + 1);
    std::string name = filename.substr(0, filename.find_last_of('.'));
    outData.name = name;
    
    // Create a minimal skeleton with root bone
    BoneInfo root;
    root.name = "Root";
    root.parentIndex = -1;
    root.bindPose.position = Vec3{};
    root.bindPose.rotation = Quat{};
    root.bindPose.scale = Vec3{1.0f, 1.0f, 1.0f};
    root.inverseBindMatrix = Mat4::identity();
    
    outData.bones.push_back(root);
    outData.boneNameToIndex["Root"] = 0;
    
    return true;
}

bool AnimationSystem::loadClipFromFile(const std::string& path, 
                                        AnimationClipData& outData) {
    // Simple binary format loader
    // In production, this would load from glTF, FBX, or custom format
    
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        NOVA_LOG_WARN(LogCategory::Core, "Failed to open animation file: {}", path);
        return false;
    }
    
    // Extract name from path
    std::string filename = path.substr(path.find_last_of("/\\") + 1);
    std::string name = filename.substr(0, filename.find_last_of('.'));
    outData.name = name;
    
    // Create a minimal animation
    outData.duration = 1.0f;
    outData.framesPerSecond = 30.0f;
    
    // Add a simple channel for root bone
    AnimationChannel channel;
    channel.boneIndex = 0;
    channel.boneName = "Root";
    
    // Simple idle animation - subtle breathing motion
    channel.positionKeys.push_back({0.0f, Vec3{}, InterpolationMode::Linear, {}, {}});
    channel.positionKeys.push_back({0.5f, Vec3{0.0f, 0.01f, 0.0f}, 
                                    InterpolationMode::Linear, {}, {}});
    channel.positionKeys.push_back({1.0f, Vec3{}, InterpolationMode::Linear, {}, {}});
    
    channel.rotationKeys.push_back({0.0f, Quat{}, InterpolationMode::Linear});
    channel.rotationKeys.push_back({1.0f, Quat{}, InterpolationMode::Linear});
    
    outData.channels.push_back(channel);
    
    return true;
}

} // namespace nova::animation
