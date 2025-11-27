/**
 * @file test_physics.cpp
 * @brief Comprehensive tests for Nova Physics System
 * @copyright (c) 2025 WeNova Interactive. All rights reserved.
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <nova/core/physics/physics_types.hpp>

using namespace nova;
using namespace nova::physics;
using Catch::Approx;

// =============================================================================
// Physical Constants Tests
// =============================================================================

TEST_CASE("Physics: Constants", "[physics][constants]") {
    SECTION("Gravity") {
        REQUIRE(GRAVITY_EARTH == Approx(9.80665f));
        REQUIRE(DEFAULT_GRAVITY.y == Approx(-9.80665f));
        REQUIRE(DEFAULT_GRAVITY.x == Approx(0.0f));
        REQUIRE(DEFAULT_GRAVITY.z == Approx(0.0f));
    }
    
    SECTION("Velocity limits") {
        REQUIRE(MAX_LINEAR_VELOCITY == Approx(500.0f));
        REQUIRE(MAX_ANGULAR_VELOCITY == Approx(100.0f));
    }
    
    SECTION("Timestep") {
        REQUIRE(DEFAULT_TIMESTEP == Approx(1.0f / 60.0f));
        REQUIRE(MAX_SUBSTEPS == 8);
    }
    
    SECTION("Damping defaults") {
        REQUIRE(DEFAULT_LINEAR_DAMPING == Approx(0.0f));
        REQUIRE(DEFAULT_ANGULAR_DAMPING == Approx(0.05f));
    }
    
    SECTION("Sleep thresholds") {
        REQUIRE(SLEEP_LINEAR_VELOCITY == Approx(0.05f));
        REQUIRE(SLEEP_ANGULAR_VELOCITY == Approx(0.05f));
        REQUIRE(SLEEP_TIME_THRESHOLD == Approx(0.5f));
    }
}

// =============================================================================
// Motion Type Tests
// =============================================================================

TEST_CASE("Physics: MotionType", "[physics][types]") {
    SECTION("Enum values") {
        REQUIRE(static_cast<u8>(MotionType::Static) == 0);
        REQUIRE(static_cast<u8>(MotionType::Kinematic) == 1);
        REQUIRE(static_cast<u8>(MotionType::Dynamic) == 2);
    }
    
    SECTION("Motion quality") {
        REQUIRE(static_cast<u8>(MotionQuality::Discrete) == 0);
        REQUIRE(static_cast<u8>(MotionQuality::LinearCast) == 1);
    }
}

// =============================================================================
// Collision Layer Tests
// =============================================================================

TEST_CASE("Physics: CollisionLayers", "[physics][collision]") {
    SECTION("Layer values") {
        REQUIRE(CollisionLayers::None == 0);
        REQUIRE(CollisionLayers::Default == 1);
        REQUIRE(CollisionLayers::Static == 2);
        REQUIRE(CollisionLayers::Dynamic == 4);
        REQUIRE(CollisionLayers::All == 0xFFFF);
    }
    
    SECTION("Layer masking") {
        CollisionMask mask = CollisionLayers::Default | CollisionLayers::Dynamic;
        REQUIRE((mask & CollisionLayers::Default) != 0);
        REQUIRE((mask & CollisionLayers::Dynamic) != 0);
        REQUIRE((mask & CollisionLayers::Static) == 0);
    }
    
    SECTION("All layers include everything") {
        REQUIRE((CollisionLayers::All & CollisionLayers::Default) != 0);
        REQUIRE((CollisionLayers::All & CollisionLayers::Static) != 0);
        REQUIRE((CollisionLayers::All & CollisionLayers::Dynamic) != 0);
        REQUIRE((CollisionLayers::All & CollisionLayers::Character) != 0);
    }
}

// =============================================================================
// Shape Type Tests
// =============================================================================

TEST_CASE("Physics: ShapeType", "[physics][shapes]") {
    SECTION("All shape types defined") {
        REQUIRE(ShapeType::Sphere != ShapeType::Box);
        REQUIRE(ShapeType::Capsule != ShapeType::Cylinder);
        REQUIRE(ShapeType::ConvexHull != ShapeType::TriangleMesh);
        REQUIRE(ShapeType::HeightField != ShapeType::Compound);
        REQUIRE(ShapeType::Plane != ShapeType::Sphere);
    }
}

// =============================================================================
// Physics Material Tests
// =============================================================================

TEST_CASE("Physics: PhysicsMaterial", "[physics][material]") {
    SECTION("Default material") {
        PhysicsMaterial mat = PhysicsMaterial::defaultMaterial();
        REQUIRE(mat.staticFriction == Approx(0.5f));
        REQUIRE(mat.dynamicFriction == Approx(0.5f));
        REQUIRE(mat.restitution == Approx(0.0f));
        REQUIRE(mat.density == Approx(1000.0f));
    }
    
    SECTION("Rubber material") {
        PhysicsMaterial mat = PhysicsMaterial::rubber();
        REQUIRE(mat.staticFriction == Approx(1.0f));
        REQUIRE(mat.dynamicFriction == Approx(0.8f));
        REQUIRE(mat.restitution == Approx(0.8f));
        REQUIRE(mat.density == Approx(1100.0f));
    }
    
    SECTION("Ice material") {
        PhysicsMaterial mat = PhysicsMaterial::ice();
        REQUIRE(mat.staticFriction == Approx(0.05f));
        REQUIRE(mat.dynamicFriction == Approx(0.02f));
        REQUIRE(mat.restitution == Approx(0.1f));
        REQUIRE(mat.density == Approx(917.0f));
    }
    
    SECTION("Metal material") {
        PhysicsMaterial mat = PhysicsMaterial::metal();
        REQUIRE(mat.staticFriction == Approx(0.6f));
        REQUIRE(mat.dynamicFriction == Approx(0.4f));
        REQUIRE(mat.restitution == Approx(0.2f));
        REQUIRE(mat.density == Approx(7800.0f));
    }
    
    SECTION("Wood material") {
        PhysicsMaterial mat = PhysicsMaterial::wood();
        REQUIRE(mat.staticFriction == Approx(0.5f));
        REQUIRE(mat.dynamicFriction == Approx(0.4f));
        REQUIRE(mat.restitution == Approx(0.3f));
        REQUIRE(mat.density == Approx(700.0f));
    }
    
    SECTION("Bouncy ball material") {
        PhysicsMaterial mat = PhysicsMaterial::bouncyBall();
        REQUIRE(mat.staticFriction == Approx(0.8f));
        REQUIRE(mat.dynamicFriction == Approx(0.6f));
        REQUIRE(mat.restitution == Approx(0.95f));
        REQUIRE(mat.density == Approx(1200.0f));
    }
    
    SECTION("Friction relationships") {
        // Static friction should be >= dynamic friction for realistic materials
        PhysicsMaterial rubber = PhysicsMaterial::rubber();
        REQUIRE(rubber.staticFriction >= rubber.dynamicFriction);
        
        PhysicsMaterial metal = PhysicsMaterial::metal();
        REQUIRE(metal.staticFriction >= metal.dynamicFriction);
        
        PhysicsMaterial wood = PhysicsMaterial::wood();
        REQUIRE(wood.staticFriction >= wood.dynamicFriction);
    }
    
    SECTION("Restitution bounds") {
        // Restitution should be between 0 and 1 for realistic materials
        PhysicsMaterial bouncy = PhysicsMaterial::bouncyBall();
        REQUIRE(bouncy.restitution >= 0.0f);
        REQUIRE(bouncy.restitution <= 1.0f);
        
        PhysicsMaterial ice = PhysicsMaterial::ice();
        REQUIRE(ice.restitution >= 0.0f);
        REQUIRE(ice.restitution <= 1.0f);
    }
}

// =============================================================================
// Physics Epsilon Tests
// =============================================================================

TEST_CASE("Physics: Epsilon", "[physics][precision]") {
    SECTION("Epsilon value") {
        REQUIRE(PHYSICS_EPSILON == Approx(1e-6f));
        REQUIRE(PHYSICS_EPSILON > 0.0f);
        REQUIRE(PHYSICS_EPSILON < 0.001f);
    }
    
    SECTION("Near-zero comparison") {
        f32 small = PHYSICS_EPSILON * 0.5f;
        f32 notSmall = PHYSICS_EPSILON * 2.0f;
        
        REQUIRE(small < PHYSICS_EPSILON);
        REQUIRE(notSmall > PHYSICS_EPSILON);
    }
}
