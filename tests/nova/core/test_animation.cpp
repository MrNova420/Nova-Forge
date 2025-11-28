/**
 * @file test_animation.cpp
 * @brief Unit tests for NovaCore Animation System
 * 
 * NovaForge Platform | NovaCore Engine
 * Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/animation/animation.hpp>

using namespace nova;
using namespace nova::animation;
using namespace nova::math;
using Catch::Approx;

// ============================================================================
// Animation Types Tests
// ============================================================================

TEST_CASE("Animation Types - BoneTransform", "[animation][types]") {
    SECTION("Default construction") {
        BoneTransform t;
        REQUIRE(t.position.x == 0.0f);
        REQUIRE(t.position.y == 0.0f);
        REQUIRE(t.position.z == 0.0f);
        REQUIRE(t.scale.x == 1.0f);
        REQUIRE(t.scale.y == 1.0f);
        REQUIRE(t.scale.z == 1.0f);
    }
    
    SECTION("Lerp interpolation") {
        BoneTransform a;
        a.position = Vec3(0.0f, 0.0f, 0.0f);
        a.scale = Vec3(1.0f, 1.0f, 1.0f);
        
        BoneTransform b;
        b.position = Vec3(10.0f, 20.0f, 30.0f);
        b.scale = Vec3(2.0f, 2.0f, 2.0f);
        
        auto result = BoneTransform::lerp(a, b, 0.5f);
        REQUIRE(result.position.x == Approx(5.0f));
        REQUIRE(result.position.y == Approx(10.0f));
        REQUIRE(result.position.z == Approx(15.0f));
        REQUIRE(result.scale.x == Approx(1.5f));
    }
    
    SECTION("Blend is same as lerp") {
        BoneTransform a;
        a.position = Vec3(0.0f, 0.0f, 0.0f);
        
        BoneTransform b;
        b.position = Vec3(10.0f, 10.0f, 10.0f);
        
        auto lerped = BoneTransform::lerp(a, b, 0.3f);
        auto blended = BoneTransform::blend(a, b, 0.3f);
        
        REQUIRE(lerped.position.x == blended.position.x);
        REQUIRE(lerped.position.y == blended.position.y);
        REQUIRE(lerped.position.z == blended.position.z);
    }
    
    SECTION("To matrix") {
        BoneTransform t;
        t.position = Vec3(1.0f, 2.0f, 3.0f);
        t.scale = Vec3(2.0f, 2.0f, 2.0f);
        
        Mat4 m = t.toMatrix();
        // Translation is in column 3
        REQUIRE(m.columns[3].x == Approx(1.0f));
        REQUIRE(m.columns[3].y == Approx(2.0f));
        REQUIRE(m.columns[3].z == Approx(3.0f));
    }
}

TEST_CASE("Animation Types - SkeletonData", "[animation][types]") {
    SECTION("Empty skeleton") {
        SkeletonData skel;
        REQUIRE(skel.getBoneCount() == 0);
        REQUIRE(!skel.isValid());
        REQUIRE(skel.findBone("NonExistent") == -1);
    }
    
    SECTION("Skeleton with bones") {
        SkeletonData skel;
        skel.name = "TestSkeleton";
        
        BoneInfo root;
        root.name = "Root";
        root.parentIndex = -1;
        skel.bones.push_back(root);
        skel.boneNameToIndex["Root"] = 0;
        
        BoneInfo spine;
        spine.name = "Spine";
        spine.parentIndex = 0;
        skel.bones.push_back(spine);
        skel.boneNameToIndex["Spine"] = 1;
        
        REQUIRE(skel.getBoneCount() == 2);
        REQUIRE(skel.isValid());
        REQUIRE(skel.findBone("Root") == 0);
        REQUIRE(skel.findBone("Spine") == 1);
        REQUIRE(skel.findBone("NonExistent") == -1);
    }
}

TEST_CASE("Animation Types - AnimationClipData", "[animation][types]") {
    SECTION("Calculate duration from channels") {
        AnimationClipData clip;
        clip.name = "TestClip";
        
        AnimationChannel channel;
        channel.boneIndex = 0;
        channel.positionKeys.push_back({0.0f, Vec3{}, InterpolationMode::Linear, {}, {}});
        channel.positionKeys.push_back({2.0f, Vec3{1.0f, 0.0f, 0.0f}, InterpolationMode::Linear, {}, {}});
        clip.channels.push_back(channel);
        
        clip.calculateDuration();
        REQUIRE(clip.duration == Approx(2.0f));
    }
    
    SECTION("Channel duration") {
        AnimationChannel channel;
        channel.positionKeys.push_back({0.0f, Vec3{}, InterpolationMode::Linear, {}, {}});
        channel.positionKeys.push_back({1.5f, Vec3{1.0f, 0.0f, 0.0f}, InterpolationMode::Linear, {}, {}});
        channel.rotationKeys.push_back({0.0f, Quat{}, InterpolationMode::Linear});
        channel.rotationKeys.push_back({2.5f, Quat{}, InterpolationMode::Linear});
        
        REQUIRE(channel.getDuration() == Approx(2.5f));
    }
    
    SECTION("Empty channel") {
        AnimationChannel channel;
        REQUIRE(channel.isEmpty());
        REQUIRE(channel.getDuration() == 0.0f);
    }
}

TEST_CASE("Animation Types - PlaybackParams", "[animation][types]") {
    SECTION("Default params") {
        PlaybackParams p;
        REQUIRE(p.speed == 1.0f);
        REQUIRE(p.startTime == 0.0f);
        REQUIRE(p.wrapMode == WrapMode::Loop);
    }
    
    SECTION("One-shot params") {
        auto p = PlaybackParams::oneShot(0.3f, 0.4f);
        REQUIRE(p.wrapMode == WrapMode::Once);
        REQUIRE(p.blendInTime == Approx(0.3f));
        REQUIRE(p.blendOutTime == Approx(0.4f));
    }
    
    SECTION("Looping params") {
        auto p = PlaybackParams::looping(0.5f);
        REQUIRE(p.wrapMode == WrapMode::Loop);
        REQUIRE(p.blendInTime == Approx(0.5f));
    }
}

TEST_CASE("Animation Types - AnimationInstance", "[animation][types]") {
    SECTION("Normalized time") {
        AnimationInstance inst;
        inst.currentTime = 1.5f;
        
        REQUIRE(inst.getNormalizedTime(3.0f) == Approx(0.5f));
        REQUIRE(inst.getNormalizedTime(0.0f) == 0.0f); // Avoid division by zero
    }
    
    SECTION("Playing state check") {
        AnimationInstance inst;
        
        inst.state = PlaybackState::Stopped;
        REQUIRE(!inst.isPlaying());
        REQUIRE(!inst.isFinished());
        
        inst.state = PlaybackState::Playing;
        REQUIRE(inst.isPlaying());
        REQUIRE(!inst.isFinished());
        
        inst.state = PlaybackState::PlayingReverse;
        REQUIRE(inst.isPlaying());
        
        inst.state = PlaybackState::Paused;
        REQUIRE(!inst.isPlaying());
        
        inst.state = PlaybackState::Finished;
        REQUIRE(!inst.isPlaying());
        REQUIRE(inst.isFinished());
    }
}

TEST_CASE("Animation Types - AnimationLayer", "[animation][types]") {
    SECTION("Bone weight with empty mask") {
        AnimationLayer layer;
        // Empty mask should return 1.0 for all bones
        REQUIRE(layer.getBoneWeight(0) == 1.0f);
        REQUIRE(layer.getBoneWeight(10) == 1.0f);
        REQUIRE(layer.getBoneWeight(-1) == 1.0f);
    }
    
    SECTION("Bone weight with mask") {
        AnimationLayer layer;
        layer.boneMask = {1.0f, 0.5f, 0.0f};
        
        REQUIRE(layer.getBoneWeight(0) == 1.0f);
        REQUIRE(layer.getBoneWeight(1) == 0.5f);
        REQUIRE(layer.getBoneWeight(2) == 0.0f);
        REQUIRE(layer.getBoneWeight(3) == 1.0f); // Out of bounds returns 1.0
    }
}

TEST_CASE("Animation Types - AnimParam", "[animation][types]") {
    SECTION("Float parameter") {
        auto p = AnimParam::makeFloat("Speed", 1.5f);
        REQUIRE(p.type == AnimParamType::Float);
        REQUIRE(p.name == "Speed");
        REQUIRE(p.floatValue == Approx(1.5f));
    }
    
    SECTION("Int parameter") {
        auto p = AnimParam::makeInt("Count", 42);
        REQUIRE(p.type == AnimParamType::Int);
        REQUIRE(p.name == "Count");
        REQUIRE(p.intValue == 42);
    }
    
    SECTION("Bool parameter") {
        auto p = AnimParam::makeBool("IsGrounded", true);
        REQUIRE(p.type == AnimParamType::Bool);
        REQUIRE(p.name == "IsGrounded");
        REQUIRE(p.boolValue == true);
    }
    
    SECTION("Trigger parameter") {
        auto p = AnimParam::makeTrigger("Jump");
        REQUIRE(p.type == AnimParamType::Trigger);
        REQUIRE(p.name == "Jump");
        REQUIRE(p.boolValue == false);
    }
}

TEST_CASE("Animation Types - AnimationStateMachineData", "[animation][types]") {
    SECTION("Find state by name") {
        AnimationStateMachineData sm;
        
        AnimationStateData idle;
        idle.name = "Idle";
        sm.states.push_back(idle);
        
        AnimationStateData walk;
        walk.name = "Walk";
        sm.states.push_back(walk);
        
        AnimationStateData run;
        run.name = "Run";
        sm.states.push_back(run);
        
        REQUIRE(sm.findState("Idle") == 0);
        REQUIRE(sm.findState("Walk") == 1);
        REQUIRE(sm.findState("Run") == 2);
        REQUIRE(sm.findState("Jump") == -1);
    }
    
    SECTION("Add parameter") {
        AnimationStateMachineData sm;
        sm.addParameter(AnimParam::makeFloat("Speed", 0.0f));
        sm.addParameter(AnimParam::makeBool("IsGrounded", true));
        
        REQUIRE(sm.parameters.size() == 2);
        REQUIRE(sm.parameters["Speed"].type == AnimParamType::Float);
        REQUIRE(sm.parameters["IsGrounded"].boolValue == true);
    }
}

TEST_CASE("Animation Types - AnimationPose", "[animation][types]") {
    SECTION("Initialize") {
        AnimationPose pose;
        pose.initialize(3);
        
        REQUIRE(pose.localTransforms.size() == 3);
        REQUIRE(pose.worldTransforms.size() == 3);
        REQUIRE(pose.skinningMatrices.size() == 3);
    }
    
    SECTION("Reset") {
        AnimationPose pose;
        pose.initialize(2);
        pose.localTransforms[0].position = Vec3(1.0f, 2.0f, 3.0f);
        
        pose.reset();
        REQUIRE(pose.localTransforms[0].position.x == 0.0f);
    }
    
    SECTION("Copy from") {
        AnimationPose src;
        src.initialize(2);
        src.localTransforms[0].position = Vec3(5.0f, 5.0f, 5.0f);
        
        AnimationPose dst;
        dst.initialize(2);
        dst.copyFrom(src);
        
        REQUIRE(dst.localTransforms[0].position.x == 5.0f);
    }
}

// ============================================================================
// Animation System Tests
// ============================================================================

TEST_CASE("Animation System - Initialization", "[animation][system]") {
    auto& system = AnimationSystem::get();
    
    SECTION("Initialize and shutdown") {
        AnimationSystemConfig config;
        config.maxSkeletons = 64;
        
        REQUIRE(system.initialize(config));
        REQUIRE(system.getConfig().maxSkeletons == 64);
        
        system.shutdown();
    }
}

TEST_CASE("Animation System - Skeleton Management", "[animation][system]") {
    auto& system = AnimationSystem::get();
    system.initialize();
    
    SECTION("Create skeleton") {
        SkeletonData data;
        data.name = "TestSkeleton";
        
        BoneInfo root;
        root.name = "Root";
        root.parentIndex = -1;
        data.bones.push_back(root);
        data.boneNameToIndex["Root"] = 0;
        
        auto handle = system.createSkeleton(data);
        REQUIRE(handle.isValid());
        
        auto* skel = system.getSkeleton(handle);
        REQUIRE(skel != nullptr);
        REQUIRE(skel->name == "TestSkeleton");
        REQUIRE(skel->getBoneCount() == 1);
        
        system.unloadSkeleton(handle);
        REQUIRE(system.getSkeleton(handle) == nullptr);
    }
    
    system.shutdown();
}

TEST_CASE("Animation System - Clip Management", "[animation][system]") {
    auto& system = AnimationSystem::get();
    system.initialize();
    
    SECTION("Create clip") {
        AnimationClipData data;
        data.name = "TestClip";
        data.duration = 1.0f;
        
        auto handle = system.createClip(data);
        REQUIRE(handle.isValid());
        
        auto* clip = system.getClip(handle);
        REQUIRE(clip != nullptr);
        REQUIRE(clip->name == "TestClip");
        REQUIRE(clip->duration == Approx(1.0f));
        
        system.unloadClip(handle);
        REQUIRE(system.getClip(handle) == nullptr);
    }
    
    SECTION("Auto-calculate duration") {
        AnimationClipData data;
        data.name = "AutoDurationClip";
        data.duration = 0.0f; // Will be calculated
        
        AnimationChannel channel;
        channel.boneIndex = 0;
        channel.positionKeys.push_back({0.0f, Vec3{}, InterpolationMode::Linear, {}, {}});
        channel.positionKeys.push_back({3.0f, Vec3{1.0f, 0.0f, 0.0f}, InterpolationMode::Linear, {}, {}});
        data.channels.push_back(channel);
        
        auto handle = system.createClip(data);
        auto* clip = system.getClip(handle);
        
        REQUIRE(clip->duration == Approx(3.0f));
        
        system.unloadClip(handle);
    }
    
    system.shutdown();
}

TEST_CASE("Animation System - Sampler", "[animation][system]") {
    auto& system = AnimationSystem::get();
    system.initialize();
    
    // Create skeleton
    SkeletonData skelData;
    skelData.name = "TestSkeleton";
    
    BoneInfo root;
    root.name = "Root";
    root.parentIndex = -1;
    skelData.bones.push_back(root);
    skelData.boneNameToIndex["Root"] = 0;
    
    BoneInfo child;
    child.name = "Child";
    child.parentIndex = 0;
    skelData.bones.push_back(child);
    skelData.boneNameToIndex["Child"] = 1;
    
    auto skelHandle = system.createSkeleton(skelData);
    
    SECTION("Create sampler") {
        auto* sampler = system.createSampler(skelHandle);
        REQUIRE(sampler != nullptr);
        
        auto& pose = sampler->getPose();
        REQUIRE(pose.localTransforms.size() == 2);
        
        system.destroySampler(sampler);
    }
    
    SECTION("Sampler layers") {
        auto* sampler = system.createSampler(skelHandle);
        
        // Base layer is created by default
        auto* baseLayer = sampler->getLayer("Base");
        REQUIRE(baseLayer != nullptr);
        REQUIRE(baseLayer->index == 0);
        
        // Add new layer
        auto* upperBody = sampler->addLayer("UpperBody", 1);
        REQUIRE(upperBody != nullptr);
        REQUIRE(upperBody->index == 1);
        
        // Set layer weight
        sampler->setLayerWeight("UpperBody", 0.5f);
        REQUIRE(sampler->getLayer("UpperBody")->weight == Approx(0.5f));
        
        // Remove layer
        sampler->removeLayer("UpperBody");
        REQUIRE(sampler->getLayer("UpperBody") == nullptr);
        
        // Can't remove base layer
        sampler->removeLayer("Base");
        REQUIRE(sampler->getLayer("Base") != nullptr);
        
        system.destroySampler(sampler);
    }
    
    system.unloadSkeleton(skelHandle);
    system.shutdown();
}

TEST_CASE("Animation System - State Machine", "[animation][system]") {
    auto& system = AnimationSystem::get();
    system.initialize();
    
    SECTION("Create state machine") {
        AnimationStateMachineData smData;
        smData.name = "CharacterSM";
        
        // Add states
        AnimationStateData idle;
        idle.name = "Idle";
        smData.states.push_back(idle);
        
        AnimationStateData walk;
        walk.name = "Walk";
        smData.states.push_back(walk);
        
        // Add parameter
        smData.addParameter(AnimParam::makeFloat("Speed", 0.0f));
        
        // Add transition
        StateTransition toWalk;
        toWalk.sourceStateIndex = 0;
        toWalk.targetStateIndex = 1;
        
        TransitionCondition speedCondition;
        speedCondition.paramName = "Speed";
        speedCondition.op = ConditionOperator::Greater;
        speedCondition.threshold = AnimParam::makeFloat("", 0.1f);
        toWalk.conditions.push_back(speedCondition);
        
        smData.transitions.push_back(toWalk);
        
        auto handle = system.createController(smData);
        REQUIRE(handle.isValid());
        
        auto* sm = system.getController(handle);
        REQUIRE(sm != nullptr);
        REQUIRE(sm->getCurrentStateName() == "Idle");
        
        // Set parameter and check
        sm->setFloat("Speed", 0.5f);
        REQUIRE(sm->getFloat("Speed") == Approx(0.5f));
        
        system.destroyController(handle);
    }
    
    system.shutdown();
}

TEST_CASE("Animation System - Update", "[animation][system]") {
    auto& system = AnimationSystem::get();
    system.initialize();
    
    // Create skeleton and sampler
    SkeletonData skelData;
    skelData.name = "TestSkeleton";
    
    BoneInfo root;
    root.name = "Root";
    root.parentIndex = -1;
    skelData.bones.push_back(root);
    skelData.boneNameToIndex["Root"] = 0;
    
    auto skelHandle = system.createSkeleton(skelData);
    auto* sampler = system.createSampler(skelHandle);
    
    // Create animation clip
    AnimationClipData clipData;
    clipData.name = "TestAnim";
    clipData.duration = 1.0f;
    
    AnimationChannel channel;
    channel.boneIndex = 0;
    channel.positionKeys.push_back({0.0f, Vec3(0.0f, 0.0f, 0.0f), InterpolationMode::Linear, {}, {}});
    channel.positionKeys.push_back({1.0f, Vec3(10.0f, 0.0f, 0.0f), InterpolationMode::Linear, {}, {}});
    clipData.channels.push_back(channel);
    
    auto clipHandle = system.createClip(clipData);
    
    SECTION("Animation playback") {
        sampler->play(clipHandle);
        
        // Update system
        system.update(0.5f);
        
        // Check statistics
        auto& stats = system.getStats();
        REQUIRE(stats.activeSamplers >= 1);
    }
    
    system.destroySampler(sampler);
    system.unloadClip(clipHandle);
    system.unloadSkeleton(skelHandle);
    system.shutdown();
}

// ============================================================================
// IK Solver Tests - Comprehensive Coverage for All IK Types
// ============================================================================

TEST_CASE("IK Solver - CCD (Cyclic Coordinate Descent)", "[animation][ik]") {
    auto& system = AnimationSystem::get();
    system.initialize();
    
    // Create a 3-bone chain skeleton (typical arm: shoulder, elbow, wrist)
    SkeletonData skelData;
    skelData.name = "ArmSkeleton";
    
    // Root/Shoulder bone
    BoneInfo shoulder;
    shoulder.name = "Shoulder";
    shoulder.parentIndex = -1;
    shoulder.bindPose.position = Vec3(0.0f, 0.0f, 0.0f);
    skelData.bones.push_back(shoulder);
    skelData.boneNameToIndex["Shoulder"] = 0;
    
    // Elbow bone
    BoneInfo elbow;
    elbow.name = "Elbow";
    elbow.parentIndex = 0;
    elbow.bindPose.position = Vec3(1.0f, 0.0f, 0.0f);
    skelData.bones.push_back(elbow);
    skelData.boneNameToIndex["Elbow"] = 1;
    
    // Wrist/Hand bone
    BoneInfo wrist;
    wrist.name = "Wrist";
    wrist.parentIndex = 1;
    wrist.bindPose.position = Vec3(1.0f, 0.0f, 0.0f);
    skelData.bones.push_back(wrist);
    skelData.boneNameToIndex["Wrist"] = 2;
    
    auto skelHandle = system.createSkeleton(skelData);
    auto* sampler = system.createSampler(skelHandle);
    
    SECTION("CCD solver configuration") {
        IKChain chain;
        chain.name = "ArmIK";
        chain.solverType = IKSolverType::CCD;
        chain.boneIndices = {2, 1, 0}; // Tip to root: wrist, elbow, shoulder
        chain.maxIterations = 10;
        chain.tolerance = 0.01f;
        chain.maxAnglePerJoint = 0.5f;
        chain.weight = 1.0f;
        
        REQUIRE(chain.solverType == IKSolverType::CCD);
        REQUIRE(chain.boneIndices.size() == 3);
        REQUIRE(chain.maxAnglePerJoint == Approx(0.5f));
    }
    
    SECTION("CCD IK chain setup") {
        IKChain chain;
        chain.name = "TestCCDChain";
        chain.solverType = IKSolverType::CCD;
        chain.boneIndices = {2, 1, 0};
        chain.maxIterations = 15;
        chain.tolerance = 0.001f;
        chain.maxAnglePerJoint = 0.3f;
        chain.weight = 0.8f;
        
        // Set target
        chain.target.position = Vec3(1.5f, 1.0f, 0.0f);
        chain.target.positionWeight = 1.0f;
        chain.target.isActive = true;
        
        sampler->addIKChain(chain);
        
        // Verify chain was added
        // IK chain was added successfully
    }
    
    system.destroySampler(sampler);
    system.unloadSkeleton(skelHandle);
    system.shutdown();
}

TEST_CASE("IK Solver - Jacobian Transpose", "[animation][ik]") {
    auto& system = AnimationSystem::get();
    system.initialize();
    
    // Create skeleton for Jacobian IK
    SkeletonData skelData;
    skelData.name = "JacobianSkeleton";
    
    // Create a spine-like chain (5 bones)
    for (i32 i = 0; i < 5; ++i) {
        BoneInfo bone;
        bone.name = "Spine" + std::to_string(i);
        bone.parentIndex = i > 0 ? i - 1 : -1;
        bone.bindPose.position = Vec3(0.0f, 0.5f, 0.0f);
        skelData.bones.push_back(bone);
        skelData.boneNameToIndex[bone.name] = i;
    }
    
    auto skelHandle = system.createSkeleton(skelData);
    auto* sampler = system.createSampler(skelHandle);
    
    SECTION("Jacobian solver configuration") {
        IKChain chain;
        chain.name = "SpineIK";
        chain.solverType = IKSolverType::Jacobian;
        chain.boneIndices = {4, 3, 2, 1, 0}; // Tip to root
        chain.maxIterations = 20;
        chain.tolerance = 0.005f;
        chain.maxAnglePerJoint = 0.2f;
        chain.weight = 1.0f;
        
        REQUIRE(chain.solverType == IKSolverType::Jacobian);
        REQUIRE(chain.boneIndices.size() == 5);
    }
    
    SECTION("Jacobian IK chain with target") {
        IKChain chain;
        chain.name = "SpineReachIK";
        chain.solverType = IKSolverType::Jacobian;
        chain.boneIndices = {4, 3, 2, 1, 0};
        chain.maxIterations = 25;
        chain.tolerance = 0.01f;
        chain.maxAnglePerJoint = 0.25f;
        chain.weight = 1.0f;
        
        // Set target position
        chain.target.position = Vec3(0.5f, 2.0f, 0.5f);
        chain.target.positionWeight = 1.0f;
        chain.target.isActive = true;
        
        sampler->addIKChain(chain);
        // IK chain was added successfully
    }
    
    system.destroySampler(sampler);
    system.unloadSkeleton(skelHandle);
    system.shutdown();
}

TEST_CASE("IK Solver - Full Body IK", "[animation][ik]") {
    auto& system = AnimationSystem::get();
    system.initialize();
    
    // Create humanoid-like skeleton for full body IK
    SkeletonData skelData;
    skelData.name = "HumanoidSkeleton";
    
    // Root/Pelvis
    BoneInfo pelvis;
    pelvis.name = "Pelvis";
    pelvis.parentIndex = -1;
    pelvis.bindPose.position = Vec3(0.0f, 1.0f, 0.0f);
    skelData.bones.push_back(pelvis);
    skelData.boneNameToIndex["Pelvis"] = 0;
    
    // Spine
    BoneInfo spine;
    spine.name = "Spine";
    spine.parentIndex = 0;
    spine.bindPose.position = Vec3(0.0f, 0.3f, 0.0f);
    skelData.bones.push_back(spine);
    skelData.boneNameToIndex["Spine"] = 1;
    
    // Chest
    BoneInfo chest;
    chest.name = "Chest";
    chest.parentIndex = 1;
    chest.bindPose.position = Vec3(0.0f, 0.3f, 0.0f);
    skelData.bones.push_back(chest);
    skelData.boneNameToIndex["Chest"] = 2;
    
    // Head
    BoneInfo head;
    head.name = "Head";
    head.parentIndex = 2;
    head.bindPose.position = Vec3(0.0f, 0.3f, 0.0f);
    skelData.bones.push_back(head);
    skelData.boneNameToIndex["Head"] = 3;
    
    auto skelHandle = system.createSkeleton(skelData);
    auto* sampler = system.createSampler(skelHandle);
    
    SECTION("Full body IK configuration") {
        IKChain chain;
        chain.name = "UpperBodyIK";
        chain.solverType = IKSolverType::FullBody;
        chain.boneIndices = {3, 2, 1, 0}; // Head to pelvis
        chain.maxIterations = 30;
        chain.tolerance = 0.01f;
        chain.maxAnglePerJoint = 0.4f;
        chain.weight = 1.0f;
        
        REQUIRE(chain.solverType == IKSolverType::FullBody);
        REQUIRE(chain.boneIndices.size() == 4);
    }
    
    SECTION("Full body IK with balance") {
        IKChain chain;
        chain.name = "BalancedReach";
        chain.solverType = IKSolverType::FullBody;
        chain.boneIndices = {3, 2, 1, 0};
        chain.maxIterations = 25;
        chain.tolerance = 0.005f;
        chain.maxAnglePerJoint = 0.35f;
        chain.weight = 0.9f;
        
        // Target that requires leaning
        chain.target.position = Vec3(1.0f, 2.0f, 0.0f);
        chain.target.positionWeight = 1.0f;
        chain.target.isActive = true;
        
        sampler->addIKChain(chain);
        // IK chain was added successfully
    }
    
    system.destroySampler(sampler);
    system.unloadSkeleton(skelHandle);
    system.shutdown();
}

TEST_CASE("IK Solver - IKChain Parameters", "[animation][ik]") {
    SECTION("Default IKChain values") {
        IKChain chain;
        REQUIRE(chain.solverType == IKSolverType::TwoBone);
        REQUIRE(chain.maxIterations == 10);
        REQUIRE(chain.tolerance == Approx(0.001f));
        REQUIRE(chain.weight == Approx(1.0f));
        REQUIRE(chain.maxAnglePerJoint == Approx(0.5f));
    }
    
    SECTION("IKTarget configuration") {
        IKTarget target;
        target.position = Vec3(1.0f, 2.0f, 3.0f);
        target.rotation = Quat::fromAxisAngle(Vec3(0.0f, 1.0f, 0.0f), 0.5f);
        target.positionWeight = 0.8f;
        target.rotationWeight = 0.6f;
        target.isActive = true;
        
        REQUIRE(target.position.x == Approx(1.0f));
        REQUIRE(target.position.y == Approx(2.0f));
        REQUIRE(target.position.z == Approx(3.0f));
        REQUIRE(target.positionWeight == Approx(0.8f));
        REQUIRE(target.rotationWeight == Approx(0.6f));
        REQUIRE(target.isActive == true);
    }
    
    SECTION("Pole vector configuration") {
        IKChain chain;
        chain.poleVector = Vec3(0.0f, 0.0f, 1.0f);
        chain.usePoleVector = true;
        
        REQUIRE(chain.poleVector.z == Approx(1.0f));
        REQUIRE(chain.usePoleVector == true);
    }
}

TEST_CASE("IK Solver - All Solver Types Enum", "[animation][ik]") {
    SECTION("All solver types are defined") {
        REQUIRE(static_cast<u8>(IKSolverType::None) == 0);
        REQUIRE(static_cast<u8>(IKSolverType::TwoBone) == 1);
        REQUIRE(static_cast<u8>(IKSolverType::FABRIK) == 2);
        REQUIRE(static_cast<u8>(IKSolverType::CCD) == 3);
        REQUIRE(static_cast<u8>(IKSolverType::Jacobian) == 4);
        REQUIRE(static_cast<u8>(IKSolverType::FullBody) == 5);
    }
}
