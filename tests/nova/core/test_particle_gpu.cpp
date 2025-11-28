/**
 * @file test_particle_gpu.cpp
 * @brief Comprehensive tests for GPU Particle Simulation System
 *
 * Copyright (c) NovaForge Platform - All rights reserved.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/render/particle_gpu.hpp>

using namespace nova;
using Catch::Approx;

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_CASE("GPUParticleConfig constants are valid", "[particle_gpu][config]") {
    SECTION("Particle limits") {
        CHECK(GPUParticleConfig::MAX_PARTICLES_PER_EMITTER >= 100000);
        CHECK(GPUParticleConfig::MAX_EMITTERS >= 64);
        CHECK(GPUParticleConfig::MAX_FORCE_FIELDS >= 16);
        CHECK(GPUParticleConfig::MAX_COLLISION_PLANES >= 8);
    }
    
    SECTION("Rendering limits") {
        CHECK(GPUParticleConfig::MAX_PARTICLE_TEXTURES >= 4);
        CHECK(GPUParticleConfig::MAX_RIBBONS_PER_EMITTER >= 256);
        CHECK(GPUParticleConfig::MAX_RIBBON_SEGMENTS >= 16);
    }
    
    SECTION("Workgroup sizes") {
        CHECK(GPUParticleConfig::PARTICLE_WORKGROUP_SIZE >= 64);
        CHECK(GPUParticleConfig::EMISSION_WORKGROUP_SIZE >= 32);
        CHECK(GPUParticleConfig::SORT_WORKGROUP_SIZE >= 256);
    }
    
    SECTION("Memory layout") {
        CHECK(GPUParticleConfig::PARTICLE_STRIDE == 64);
    }
}

// ============================================================================
// Enumeration Tests
// ============================================================================

TEST_CASE("GPUSimulationMode enum values and names", "[particle_gpu][enums]") {
    CHECK(static_cast<u8>(GPUSimulationMode::Standard) == 0);
    CHECK(static_cast<u8>(GPUSimulationMode::Persistent) == 1);
    CHECK(static_cast<u8>(GPUSimulationMode::EventDriven) == 2);
    CHECK(static_cast<u8>(GPUSimulationMode::Scripted) == 3);
    
    CHECK(std::string(getGPUSimulationModeName(GPUSimulationMode::Standard)) == "Standard");
    CHECK(std::string(getGPUSimulationModeName(GPUSimulationMode::EventDriven)) == "EventDriven");
}

TEST_CASE("GPURenderMode enum values and names", "[particle_gpu][enums]") {
    CHECK(static_cast<u8>(GPURenderMode::Billboard) == 0);
    CHECK(static_cast<u8>(GPURenderMode::StretchedBillboard) == 1);
    CHECK(static_cast<u8>(GPURenderMode::Mesh) == 2);
    CHECK(static_cast<u8>(GPURenderMode::Ribbon) == 3);
    CHECK(static_cast<u8>(GPURenderMode::Beam) == 4);
    
    CHECK(std::string(getGPURenderModeName(GPURenderMode::Billboard)) == "Billboard");
    CHECK(std::string(getGPURenderModeName(GPURenderMode::Ribbon)) == "Ribbon");
}

TEST_CASE("ForceFieldType enum values and names", "[particle_gpu][enums]") {
    CHECK(static_cast<u8>(ForceFieldType::Directional) == 0);
    CHECK(static_cast<u8>(ForceFieldType::Point) == 1);
    CHECK(static_cast<u8>(ForceFieldType::Vortex) == 2);
    CHECK(static_cast<u8>(ForceFieldType::VectorField) == 3);
    CHECK(static_cast<u8>(ForceFieldType::CurlNoise) == 4);
    CHECK(static_cast<u8>(ForceFieldType::Turbulence) == 5);
    CHECK(static_cast<u8>(ForceFieldType::Drag) == 6);
    CHECK(static_cast<u8>(ForceFieldType::Orbital) == 7);
    
    CHECK(std::string(getForceFieldTypeName(ForceFieldType::Turbulence)) == "Turbulence");
    CHECK(std::string(getForceFieldTypeName(ForceFieldType::CurlNoise)) == "CurlNoise");
}

TEST_CASE("ParticleCollisionMode enum values", "[particle_gpu][enums]") {
    CHECK(static_cast<u8>(ParticleCollisionMode::None) == 0);
    CHECK(static_cast<u8>(ParticleCollisionMode::DepthBuffer) == 1);
    CHECK(static_cast<u8>(ParticleCollisionMode::Planes) == 2);
    CHECK(static_cast<u8>(ParticleCollisionMode::SDF) == 3);
    
    CHECK(std::string(getParticleCollisionModeName(ParticleCollisionMode::DepthBuffer)) == "DepthBuffer");
}

TEST_CASE("ParticleSortMode enum values", "[particle_gpu][enums]") {
    CHECK(static_cast<u8>(ParticleSortMode::None) == 0);
    CHECK(static_cast<u8>(ParticleSortMode::OldestFirst) == 1);
    CHECK(static_cast<u8>(ParticleSortMode::NewestFirst) == 2);
    CHECK(static_cast<u8>(ParticleSortMode::ByDistance) == 3);
}

// ============================================================================
// GPU Particle Structure Tests
// ============================================================================

TEST_CASE("GPUParticle structure", "[particle_gpu][structures]") {
    SECTION("Size is at least 64 bytes") {
        CHECK(sizeof(GPUParticle) >= 64);
    }
    
    SECTION("Properties") {
        GPUParticle p;
        p.position = {1, 2, 3};
        p.velocity = {0, 1, 0};
        p.age = 0.5f;
        p.lifetime = 2.0f;
        p.size = {0.1f, 0.1f, 0.1f};
        p.rotation = 45.0f;
        p.color = {1, 1, 1, 1};
        
        CHECK(p.normalizedAge() == Approx(0.25f));
        CHECK_FALSE(p.isDead());
        CHECK(p.remainingLifetime() == Approx(1.5f));
    }
    
    SECTION("Dead particle") {
        GPUParticle p;
        p.age = 2.0f;
        p.lifetime = 1.0f;
        
        CHECK(p.isDead());
        CHECK(p.normalizedAge() == Approx(2.0f));
        CHECK(p.remainingLifetime() == Approx(0.0f));
    }
}

TEST_CASE("GPUParticleExtended structure", "[particle_gpu][structures]") {
    SECTION("Size is at least 64 bytes") {
        CHECK(sizeof(GPUParticleExtended) >= 64);
    }
    
    SECTION("Flags") {
        GPUParticleExtended p;
        p.flags = 0;
        CHECK_FALSE(p.hasCollided());
        CHECK_FALSE(p.hasTriggered());
        CHECK_FALSE(p.isRibbonHead());
        CHECK_FALSE(p.isFrozen());
        
        p.flags = GPUParticleExtended::FLAG_COLLIDED | GPUParticleExtended::FLAG_RIBBON_HEAD;
        CHECK(p.hasCollided());
        CHECK_FALSE(p.hasTriggered());
        CHECK(p.isRibbonHead());
    }
}

TEST_CASE("GPUEmitterState structure", "[particle_gpu][structures]") {
    SECTION("Size is at least 128 bytes") {
        CHECK(sizeof(GPUEmitterState) >= 128);
    }
    
    SECTION("Properties") {
        GPUEmitterState state;
        state.aliveCount = 500;
        state.maxParticles = 1000;
        state.position = {10, 0, 0};
        state.prevPosition = {9, 0, 0};
        state.deltaTime = 0.016f;
        
        CHECK(state.occupancy() == Approx(0.5f));
        CHECK_FALSE(state.isFull());
        
        Vec3 vel = state.velocity();
        CHECK(vel.x == Approx(62.5f));  // (10-9) / 0.016
    }
    
    SECTION("Full emitter") {
        GPUEmitterState state;
        state.aliveCount = 1000;
        state.maxParticles = 1000;
        CHECK(state.isFull());
        CHECK(state.occupancy() == Approx(1.0f));
    }
}

TEST_CASE("GPUForceField structure", "[particle_gpu][structures]") {
    SECTION("Size is at least 64 bytes") {
        CHECK(sizeof(GPUForceField) >= 64);
    }
    
    SECTION("Influence calculation") {
        GPUForceField field;
        field.position = {0, 0, 0};
        field.radius = 10.0f;
        field.falloff = 1.0f;
        
        // At center
        CHECK(field.calculateInfluence({0, 0, 0}) == Approx(1.0f));
        
        // At edge
        CHECK(field.calculateInfluence({10, 0, 0}) == Approx(0.0f));
        
        // Halfway
        CHECK(field.calculateInfluence({5, 0, 0}) == Approx(0.5f));
        
        // Outside
        CHECK(field.calculateInfluence({15, 0, 0}) == Approx(0.0f));
    }
}

TEST_CASE("GPUCollisionPlane structure", "[particle_gpu][structures]") {
    SECTION("Size is at least 32 bytes") {
        CHECK(sizeof(GPUCollisionPlane) >= 32);
    }
    
    SECTION("Signed distance") {
        GPUCollisionPlane plane;
        plane.normal = {0, 1, 0};  // Y-up plane
        plane.distance = 0.0f;     // At origin
        
        CHECK(plane.signedDistance({0, 5, 0}) == Approx(5.0f));
        CHECK(plane.signedDistance({0, -3, 0}) == Approx(-3.0f));
        CHECK(plane.signedDistance({0, 0, 0}) == Approx(0.0f));
    }
    
    SECTION("Projection") {
        GPUCollisionPlane plane;
        plane.normal = {0, 1, 0};
        plane.distance = 0.0f;
        
        Vec3 projected = plane.project({5, 10, 3});
        CHECK(projected.x == Approx(5.0f));
        CHECK(projected.y == Approx(0.0f));
        CHECK(projected.z == Approx(3.0f));
    }
    
    SECTION("Reflection") {
        GPUCollisionPlane plane;
        plane.normal = {0, 1, 0};
        plane.distance = 0.0f;
        
        Vec3 velocity{1, -1, 0};
        Vec3 reflected = plane.reflect(velocity);
        CHECK(reflected.x == Approx(1.0f));
        CHECK(reflected.y == Approx(1.0f));
        CHECK(reflected.z == Approx(0.0f));
    }
}

// ============================================================================
// EmissionShape Tests
// ============================================================================

TEST_CASE("EmissionShape types", "[particle_gpu][emission]") {
    SECTION("Point emission") {
        EmissionShape shape;
        shape.type = EmissionShape::Type::Point;
        shape.position = {5, 10, 15};
        
        Vec3 point = shape.getRandomPoint(12345);
        CHECK(point.x == Approx(5.0f));
        CHECK(point.y == Approx(10.0f));
        CHECK(point.z == Approx(15.0f));
    }
    
    SECTION("Box emission") {
        EmissionShape shape;
        shape.type = EmissionShape::Type::Box;
        shape.position = {0, 0, 0};
        shape.scale = {10, 10, 10};
        
        // Generate multiple points
        for (u32 i = 0; i < 100; ++i) {
            Vec3 point = shape.getRandomPoint(i);
            CHECK(point.x >= -5.0f);
            CHECK(point.x <= 5.0f);
            CHECK(point.y >= -5.0f);
            CHECK(point.y <= 5.0f);
            CHECK(point.z >= -5.0f);
            CHECK(point.z <= 5.0f);
        }
    }
    
    SECTION("Sphere emission") {
        EmissionShape shape;
        shape.type = EmissionShape::Type::Sphere;
        shape.position = {0, 0, 0};
        shape.radius = 5.0f;
        shape.radiusThickness = 1.0f;  // Solid sphere
        
        for (u32 i = 0; i < 100; ++i) {
            Vec3 point = shape.getRandomPoint(i);
            f32 dist = point.length();
            CHECK(dist <= 5.0f + 0.0001f);  // Small epsilon for floating point
        }
    }
}

// ============================================================================
// ColorGradient Tests
// ============================================================================

TEST_CASE("ColorGradient evaluation", "[particle_gpu][gradient]") {
    SECTION("Default gradient") {
        ColorGradient g;
        
        Vec4 start = g.evaluate(0.0f);
        CHECK(start.x == Approx(1.0f));
        CHECK(start.w == Approx(1.0f));
        
        Vec4 end = g.evaluate(1.0f);
        CHECK(end.w == Approx(0.0f));
    }
    
    SECTION("Interpolation") {
        ColorGradient g;
        g.keys.clear();
        g.keys.push_back({0.0f, {1, 0, 0, 1}});  // Red
        g.keys.push_back({1.0f, {0, 0, 1, 1}});  // Blue
        
        Vec4 mid = g.evaluate(0.5f);
        CHECK(mid.x == Approx(0.5f));
        CHECK(mid.z == Approx(0.5f));
    }
    
    SECTION("Fire colors preset") {
        ColorGradient g = ColorGradient::fireColors();
        CHECK(g.keys.size() == 4);
        
        Vec4 start = g.evaluate(0.0f);
        CHECK(start.x > 0.5f);  // Yellow-ish
        
        Vec4 end = g.evaluate(1.0f);
        CHECK(end.w < 0.1f);  // Mostly transparent
    }
    
    SECTION("Smoke colors preset") {
        ColorGradient g = ColorGradient::smokeColors();
        CHECK(g.keys.size() == 4);
    }
    
    SECTION("Spark colors preset") {
        ColorGradient g = ColorGradient::sparkColors();
        CHECK(g.keys.size() == 3);
    }
}

// ============================================================================
// SizeCurve Tests
// ============================================================================

TEST_CASE("SizeCurve evaluation", "[particle_gpu][curve]") {
    SECTION("Default curve") {
        SizeCurve c;
        CHECK(c.evaluate(0.0f) == Approx(1.0f));
        CHECK(c.evaluate(0.5f) == Approx(1.0f));
        CHECK(c.evaluate(1.0f) == Approx(1.0f));
    }
    
    SECTION("With multiplier") {
        SizeCurve c;
        c.multiplier = 2.0f;
        CHECK(c.evaluate(0.5f) == Approx(2.0f));
    }
    
    SECTION("Grow then shrink preset") {
        SizeCurve c = SizeCurve::growThenShrink();
        CHECK(c.evaluate(0.0f) == Approx(0.0f));
        CHECK(c.evaluate(0.2f) == Approx(1.0f));
        CHECK(c.evaluate(1.0f) == Approx(0.0f));
    }
    
    SECTION("Shrink to nothing preset") {
        SizeCurve c = SizeCurve::shrinkToNothing();
        CHECK(c.evaluate(0.0f) == Approx(1.0f));
        CHECK(c.evaluate(1.0f) == Approx(0.0f));
    }
    
    SECTION("Grow continuously preset") {
        SizeCurve c = SizeCurve::growContinuously();
        CHECK(c.evaluate(0.0f) == Approx(0.0f));
        CHECK(c.evaluate(1.0f) == Approx(1.0f));
    }
}

// ============================================================================
// GPUEmitterDesc Tests
// ============================================================================

TEST_CASE("GPUEmitterDesc configuration", "[particle_gpu][emitter]") {
    SECTION("Default values") {
        GPUEmitterDesc desc;
        CHECK(desc.maxParticles == 10000);
        CHECK(desc.simulationMode == GPUSimulationMode::Standard);
        CHECK(desc.renderMode == GPURenderMode::Billboard);
        CHECK(desc.emissionRate == Approx(100.0f));
    }
    
    SECTION("Memory estimation") {
        GPUEmitterDesc desc;
        desc.maxParticles = 10000;
        
        u32 memory = desc.estimateMemoryBytes();
        // Base (64B) + Extended (64B) + DeadList (4B) = 132B per particle
        CHECK(memory >= desc.maxParticles * 64);
    }
}

// ============================================================================
// GPUParticleEmitter Tests
// ============================================================================

TEST_CASE("GPUParticleEmitter lifecycle", "[particle_gpu][emitter]") {
    GPUEmitterDesc desc;
    desc.name = "TestEmitter";
    desc.maxParticles = 1000;
    desc.emissionRate = 100.0f;
    
    GPUParticleEmitter emitter(desc);
    
    SECTION("Initial state") {
        CHECK(emitter.desc().name == "TestEmitter");
        CHECK(emitter.isPlaying());
        CHECK(emitter.aliveCount() == 0);
    }
    
    SECTION("Position update") {
        emitter.setPosition({10, 20, 30});
        CHECK(emitter.state().position.x == Approx(10.0f));
        CHECK(emitter.state().position.y == Approx(20.0f));
    }
    
    SECTION("Color update") {
        emitter.setEmissionColor({1, 0.5f, 0, 1});
        CHECK(emitter.state().emissionColor.y == Approx(0.5f));
    }
    
    SECTION("Play/Stop") {
        emitter.stop();
        CHECK_FALSE(emitter.isPlaying());
        
        emitter.play();
        CHECK(emitter.isPlaying());
    }
    
    SECTION("Burst emission") {
        emitter.stop();  // Stop continuous emission
        emitter.burst(500);
        emitter.update(0.016f);
        
        CHECK(emitter.state().spawnCount == 500);
    }
    
    SECTION("Clear") {
        emitter.setAliveCount(500);
        emitter.clear();
        CHECK(emitter.aliveCount() == 0);
    }
}

TEST_CASE("GPUParticleEmitter update", "[particle_gpu][emitter]") {
    GPUEmitterDesc desc;
    desc.maxParticles = 1000;
    desc.emissionRate = 60.0f;  // 60 per second
    
    GPUParticleEmitter emitter(desc);
    
    SECTION("Continuous emission") {
        // Update at 60 FPS
        emitter.update(1.0f / 60.0f);
        CHECK(emitter.state().spawnCount == 1);  // 60/60 = 1
        
        // Update for full second
        emitter.update(1.0f);
        CHECK(emitter.state().spawnCount == 60);
    }
    
    SECTION("Max particle limit") {
        emitter.setAliveCount(999);
        emitter.update(1.0f);  // Would spawn 60, but only 1 slot
        CHECK(emitter.state().spawnCount == 1);
    }
    
    SECTION("Time accumulation") {
        // Multiple small updates should accumulate
        for (int i = 0; i < 60; ++i) {
            emitter.update(1.0f / 60.0f);
        }
        // After 60 updates at 1/60s each = 1 second total
        // Last update should have spawned 1
        CHECK(emitter.state().deltaTime == Approx(1.0f / 60.0f));
    }
}

// ============================================================================
// GPUParticleManager Tests
// ============================================================================

TEST_CASE("GPUParticleManager singleton", "[particle_gpu][manager]") {
    auto& manager = GPUParticleManager::get();
    
    SECTION("Singleton access") {
        auto& manager2 = GPUParticleManager::get();
        CHECK(&manager == &manager2);
    }
    
    SECTION("Initialization") {
        CHECK(manager.initialize());
        CHECK(manager.isInitialized());
    }
}

TEST_CASE("GPUParticleManager emitter management", "[particle_gpu][manager]") {
    auto& manager = GPUParticleManager::get();
    manager.initialize();
    
    SECTION("Create emitter") {
        GPUEmitterDesc desc;
        desc.name = "Test";
        desc.maxParticles = 5000;
        
        u64 handle = manager.createEmitter(desc);
        CHECK(handle != 0);
        
        GPUParticleEmitter* emitter = manager.getEmitter(handle);
        REQUIRE(emitter != nullptr);
        CHECK(emitter->desc().name == "Test");
        
        manager.destroyEmitter(handle);
        CHECK(manager.getEmitter(handle) == nullptr);
    }
    
    SECTION("Emitter count") {
        usize initialCount = manager.emitterCount();
        
        GPUEmitterDesc desc;
        u64 h1 = manager.createEmitter(desc);
        u64 h2 = manager.createEmitter(desc);
        
        CHECK(manager.emitterCount() == initialCount + 2);
        
        manager.destroyEmitter(h1);
        manager.destroyEmitter(h2);
    }
}

TEST_CASE("GPUParticleManager force fields", "[particle_gpu][manager]") {
    auto& manager = GPUParticleManager::get();
    manager.initialize();
    
    SECTION("Add force field") {
        GPUForceField field;
        field.type = static_cast<u32>(ForceFieldType::Point);
        field.position = {0, 0, 0};
        field.strength = 10.0f;
        field.radius = 5.0f;
        
        u64 handle = manager.addForceField(field);
        CHECK(handle != 0);
        
        GPUForceField* retrieved = manager.getForceField(handle);
        REQUIRE(retrieved != nullptr);
        CHECK(retrieved->strength == Approx(10.0f));
        
        manager.removeForceField(handle);
        CHECK(manager.getForceField(handle) == nullptr);
    }
    
    SECTION("Force field count") {
        usize initialCount = manager.forceFieldCount();
        
        GPUForceField field;
        u64 h = manager.addForceField(field);
        
        CHECK(manager.forceFieldCount() == initialCount + 1);
        
        manager.removeForceField(h);
    }
}

TEST_CASE("GPUParticleManager collision planes", "[particle_gpu][manager]") {
    auto& manager = GPUParticleManager::get();
    manager.initialize();
    manager.clearCollisionPlanes();
    
    SECTION("Add collision plane") {
        GPUCollisionPlane plane;
        plane.normal = {0, 1, 0};
        plane.distance = 0.0f;
        plane.bounciness = 0.5f;
        plane.friction = 0.3f;
        
        u32 index = manager.addCollisionPlane(plane);
        CHECK(index == 0);
    }
    
    SECTION("Clear collision planes") {
        GPUCollisionPlane plane;
        manager.addCollisionPlane(plane);
        manager.addCollisionPlane(plane);
        
        manager.clearCollisionPlanes();
        // Next add should be at index 0
        u32 index = manager.addCollisionPlane(plane);
        CHECK(index == 0);
    }
}

TEST_CASE("GPUParticleManager statistics", "[particle_gpu][manager]") {
    auto& manager = GPUParticleManager::get();
    manager.initialize();
    manager.resetStats();
    
    GPUEmitterDesc desc;
    desc.maxParticles = 1000;
    u64 handle = manager.createEmitter(desc);
    
    const auto& stats = manager.stats();
    CHECK(stats.totalParticles >= 1000);
    
    manager.destroyEmitter(handle);
}

// ============================================================================
// Preset Emitter Tests
// ============================================================================

TEST_CASE("Fire emitter preset", "[particle_gpu][presets]") {
    auto desc = createFireEmitterPreset(5000);
    
    CHECK(desc.name == "Fire");
    CHECK(desc.maxParticles == 5000);
    CHECK(desc.emissionShape.type == EmissionShape::Type::Cone);
    CHECK(desc.gravityModifier < 0);  // Fire rises
    CHECK(desc.colorOverLifetime.keys.size() == 4);
}

TEST_CASE("Smoke emitter preset", "[particle_gpu][presets]") {
    auto desc = createSmokeEmitterPreset(3000);
    
    CHECK(desc.name == "Smoke");
    CHECK(desc.maxParticles == 3000);
    CHECK(desc.gravityModifier < 0);  // Smoke rises
    CHECK(desc.sizeOverLifetime.multiplier > 1.0f);  // Smoke expands
}

TEST_CASE("Spark emitter preset", "[particle_gpu][presets]") {
    auto desc = createSparkEmitterPreset(1000);
    
    CHECK(desc.name == "Sparks");
    CHECK(desc.renderMode == GPURenderMode::StretchedBillboard);
    CHECK(desc.emissionRate == Approx(0.0f));  // Burst only
    CHECK(desc.collisionMode == ParticleCollisionMode::Planes);
    CHECK(desc.gravityModifier > 0);  // Sparks fall
}

TEST_CASE("Rain emitter preset", "[particle_gpu][presets]") {
    auto desc = createRainEmitterPreset(10000);
    
    CHECK(desc.name == "Rain");
    CHECK(desc.maxParticles == 10000);
    CHECK(desc.emissionShape.type == EmissionShape::Type::Box);
    CHECK(desc.collisionMode == ParticleCollisionMode::DepthBuffer);
    CHECK(desc.collisionLifetimeLoss == Approx(1.0f));  // Die on impact
}

TEST_CASE("Ribbon emitter preset", "[particle_gpu][presets]") {
    auto desc = createRibbonEmitterPreset(500);
    
    CHECK(desc.name == "Ribbon");
    CHECK(desc.renderMode == GPURenderMode::Ribbon);
    CHECK(desc.inheritVelocity == Approx(1.0f));
    CHECK(desc.ribbonSegments > 0);
}

// ============================================================================
// GPUParticleStats Tests
// ============================================================================

TEST_CASE("GPUParticleStats calculations", "[particle_gpu][stats]") {
    GPUParticleStats stats;
    stats.totalParticles = 10000;
    stats.aliveParticles = 5000;
    stats.simulationTimeMs = 1.0f;
    stats.sortTimeMs = 0.5f;
    stats.renderTimeMs = 0.3f;
    
    CHECK(stats.occupancy() == Approx(0.5f));
    CHECK(stats.totalTimeMs() == Approx(1.8f));
}

TEST_CASE("GPUParticleStats reset", "[particle_gpu][stats]") {
    GPUParticleStats stats;
    stats.totalParticles = 10000;
    stats.aliveParticles = 5000;
    stats.activeEmitters = 10;
    
    stats.reset();
    
    CHECK(stats.totalParticles == 0);
    CHECK(stats.aliveParticles == 0);
    CHECK(stats.activeEmitters == 0);
}
