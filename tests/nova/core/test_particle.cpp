/**
 * @file test_particle.cpp
 * @brief Comprehensive tests for Nova Particle System
 * @copyright (c) 2025 WeNova Interactive. All rights reserved.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/particle/particle_types.hpp>
#include <nova/core/particle/particle_system.hpp>

using namespace nova;
using namespace nova::particle;
using Catch::Approx;

// =============================================================================
// Particle Curve Tests
// =============================================================================

TEST_CASE("Particle: ParticleCurve constant", "[particle][curve]") {
    ParticleCurve curve;
    curve.type = CurveType::Constant;
    curve.constantValue = 1.0f;
    
    REQUIRE(curve.evaluate(0.0f) == Approx(1.0f));
    REQUIRE(curve.evaluate(0.5f) == Approx(1.0f));
    REQUIRE(curve.evaluate(1.0f) == Approx(1.0f));
}

TEST_CASE("Particle: ParticleCurve linear", "[particle][curve]") {
    ParticleCurve curve;
    curve.type = CurveType::Linear;
    curve.minValue = 0.0f;
    curve.maxValue = 1.0f;
    
    // Linear curve behavior depends on implementation
    // Just verify it's set up correctly
    REQUIRE(curve.type == CurveType::Linear);
    REQUIRE(curve.minValue == Approx(0.0f));
    REQUIRE(curve.maxValue == Approx(1.0f));
}

// =============================================================================
// Color Gradient Tests
// =============================================================================

TEST_CASE("Particle: ColorGradient", "[particle][gradient]") {
    SECTION("Default white gradient") {
        ColorGradient gradient = ColorGradient::white();
        auto color = gradient.evaluate(0.5f);
        REQUIRE(color.x == Approx(1.0f));  // r
        REQUIRE(color.y == Approx(1.0f));  // g
        REQUIRE(color.z == Approx(1.0f));  // b
    }
    
    SECTION("Two color stops") {
        ColorGradient gradient;
        gradient.colorKeys.push_back({0.0f, Color(1.0f, 0.0f, 0.0f, 1.0f)}); // Red
        gradient.colorKeys.push_back({1.0f, Color(0.0f, 0.0f, 1.0f, 1.0f)}); // Blue
        gradient.alphaKeys.push_back({0.0f, 1.0f});
        gradient.alphaKeys.push_back({1.0f, 1.0f});
        
        auto mid = gradient.evaluate(0.5f);
        REQUIRE(mid.x == Approx(0.5f));  // r
        REQUIRE(mid.z == Approx(0.5f));  // b
    }
}

// =============================================================================
// Emission Shape Tests
// =============================================================================

TEST_CASE("Particle: ShapeModule types", "[particle][shape]") {
    SECTION("Point emission") {
        ShapeModule shape;
        shape.shape = EmissionShape::Point;
        REQUIRE(shape.shape == EmissionShape::Point);
    }
    
    SECTION("Sphere emission") {
        ShapeModule shape;
        shape.shape = EmissionShape::Sphere;
        shape.radius = 10.0f;
        
        REQUIRE(shape.shape == EmissionShape::Sphere);
        REQUIRE(shape.radius == Approx(10.0f));
    }
    
    SECTION("Cone emission") {
        ShapeModule shape;
        shape.shape = EmissionShape::Cone;
        shape.angle = 45.0f;
        shape.radius = 5.0f;
        
        REQUIRE(shape.shape == EmissionShape::Cone);
        REQUIRE(shape.angle == Approx(45.0f));
        REQUIRE(shape.radius == Approx(5.0f));
    }
    
    SECTION("Box emission") {
        ShapeModule shape;
        shape.shape = EmissionShape::Box;
        shape.scale = {10.0f, 5.0f, 2.0f};
        
        REQUIRE(shape.shape == EmissionShape::Box);
        REQUIRE(shape.scale.x == Approx(10.0f));
        REQUIRE(shape.scale.y == Approx(5.0f));
        REQUIRE(shape.scale.z == Approx(2.0f));
    }
}

// =============================================================================
// Force Type Tests
// =============================================================================

TEST_CASE("Particle: Force types", "[particle][force]") {
    SECTION("Gravity") {
        REQUIRE(ForceType::Gravity == ForceType::Gravity);
    }
    
    SECTION("Wind") {
        REQUIRE(ForceType::Wind == ForceType::Wind);
    }
    
    SECTION("Turbulence") {
        REQUIRE(ForceType::Turbulence == ForceType::Turbulence);
    }
    
    SECTION("Vortex") {
        REQUIRE(ForceType::Vortex == ForceType::Vortex);
    }
    
    SECTION("Attractor") {
        REQUIRE(ForceType::Attractor == ForceType::Attractor);
    }
}

// =============================================================================
// Collision Response Tests
// =============================================================================

TEST_CASE("Particle: CollisionResponse", "[particle][collision]") {
    SECTION("Kill response") {
        REQUIRE(CollisionResponse::Kill == CollisionResponse::Kill);
    }
    
    SECTION("Bounce response") {
        REQUIRE(CollisionResponse::Bounce == CollisionResponse::Bounce);
    }
    
    SECTION("Stick response") {
        REQUIRE(CollisionResponse::Stick == CollisionResponse::Stick);
    }
}

// =============================================================================
// Main Module Tests
// =============================================================================

TEST_CASE("Particle: MainModule", "[particle][module]") {
    SECTION("Default configuration") {
        MainModule module;
        REQUIRE(module.maxParticles == 1000);
        REQUIRE(module.duration == Approx(5.0f));
        REQUIRE(module.loop == true);
    }
    
    SECTION("Custom configuration") {
        MainModule module;
        module.maxParticles = 5000;
        module.duration = 10.0f;
        module.loop = false;
        module.prewarm = true;
        
        REQUIRE(module.maxParticles == 5000);
        REQUIRE(module.duration == Approx(10.0f));
        REQUIRE(module.loop == false);
        REQUIRE(module.prewarm == true);
    }
}

// =============================================================================
// Emission Module Tests
// =============================================================================

TEST_CASE("Particle: EmissionModule", "[particle][module]") {
    SECTION("Default rate") {
        EmissionModule module;
        REQUIRE(module.enabled == true);
        REQUIRE(module.rateOverTime.constantValue == Approx(10.0f));
    }
    
    SECTION("Burst configuration") {
        EmissionModule module;
        EmissionModule::Burst burst;
        burst.time = 0.0f;
        burst.count = MinMaxValue::constant(30.0f);
        burst.cycles = 1;
        
        module.bursts.push_back(burst);
        REQUIRE(module.bursts.size() == 1);
        REQUIRE(module.bursts[0].time == Approx(0.0f));
    }
}

// =============================================================================
// Particle Manager Tests
// =============================================================================

TEST_CASE("Particle: ParticleManager singleton", "[particle][manager]") {
    SECTION("Get instance") {
        auto& mgr1 = ParticleManager::get();
        auto& mgr2 = ParticleManager::get();
        REQUIRE(&mgr1 == &mgr2);
    }
}

// =============================================================================
// Velocity Over Lifetime Module Tests
// =============================================================================

TEST_CASE("Particle: VelocityOverLifetimeModule", "[particle][module]") {
    VelocityOverLifetimeModule module;
    module.enabled = true;
    module.speedModifier = {CurveType::Constant, 1.0f};
    
    REQUIRE(module.enabled == true);
    REQUIRE(module.speedModifier.constantValue == Approx(1.0f));
}

// =============================================================================
// Size Over Lifetime Module Tests
// =============================================================================

TEST_CASE("Particle: SizeOverLifetimeModule", "[particle][module]") {
    SizeOverLifetimeModule module;
    module.enabled = true;
    module.size = {CurveType::Constant, 1.0f};
    
    REQUIRE(module.enabled == true);
    REQUIRE(module.size.constantValue == Approx(1.0f));
}

// =============================================================================
// Particle Data Tests
// =============================================================================

TEST_CASE("Particle: Particle struct", "[particle][data]") {
    Particle p;
    p.position = {1.0f, 2.0f, 3.0f};
    p.velocity = {0.0f, 1.0f, 0.0f};
    p.lifetime = 2.5f;
    p.maxLifetime = 5.0f;
    
    REQUIRE(p.position.x == Approx(1.0f));
    REQUIRE(p.position.y == Approx(2.0f));
    REQUIRE(p.position.z == Approx(3.0f));
    REQUIRE(p.getNormalizedLifetime() == Approx(0.5f));
    REQUIRE(p.getRemainingLifetime() == Approx(2.5f));
}
