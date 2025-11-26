/**
 * @file physics_types.hpp
 * @brief NovaCore Physics System - Core Types and Constants
 * 
 * Defines fundamental physics types, constants, and configuration:
 * - Vector and matrix types for physics calculations
 * - Physical units and constants
 * - Collision masks and layers
 * - Physics configuration structures
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include <nova/core/types/types.hpp>
#include <nova/core/math/math.hpp>
#include <limits>
#include <cstdint>

namespace nova::physics {

using namespace nova::math;

// =============================================================================
// Physical Constants
// =============================================================================

/// Standard gravity (m/s²)
constexpr f32 GRAVITY_EARTH = 9.80665f;

/// Default gravity vector (pointing down in Y-negative)
constexpr Vec3 DEFAULT_GRAVITY = Vec3(0.0f, -GRAVITY_EARTH, 0.0f);

/// Physics epsilon for floating point comparisons
constexpr f32 PHYSICS_EPSILON = 1e-6f;

/// Maximum linear velocity (m/s) - prevents numerical instability
constexpr f32 MAX_LINEAR_VELOCITY = 500.0f;

/// Maximum angular velocity (rad/s) - prevents numerical instability
constexpr f32 MAX_ANGULAR_VELOCITY = 100.0f;

/// Default physics timestep (60 Hz)
constexpr f32 DEFAULT_TIMESTEP = 1.0f / 60.0f;

/// Maximum substeps per frame
constexpr u32 MAX_SUBSTEPS = 8;

/// Default linear damping
constexpr f32 DEFAULT_LINEAR_DAMPING = 0.0f;

/// Default angular damping
constexpr f32 DEFAULT_ANGULAR_DAMPING = 0.05f;

/// Sleep linear velocity threshold (m/s)
constexpr f32 SLEEP_LINEAR_VELOCITY = 0.05f;

/// Sleep angular velocity threshold (rad/s)
constexpr f32 SLEEP_ANGULAR_VELOCITY = 0.05f;

/// Time to sleep (seconds of low velocity before sleeping)
constexpr f32 SLEEP_TIME_THRESHOLD = 0.5f;

// =============================================================================
// Motion Types
// =============================================================================

/**
 * @brief Defines how a body moves in the physics simulation
 */
enum class MotionType : u8 {
    /// Static bodies never move (infinite mass, zero velocity)
    Static = 0,
    
    /// Kinematic bodies move via velocity but ignore forces
    Kinematic = 1,
    
    /// Dynamic bodies respond to forces and collisions
    Dynamic = 2
};

/**
 * @brief Defines the quality of motion for dynamic bodies
 */
enum class MotionQuality : u8 {
    /// Discrete collision detection (fast, may tunnel)
    Discrete = 0,
    
    /// Linear continuous collision detection (prevents tunneling)
    LinearCast = 1
};

// =============================================================================
// Collision Types
// =============================================================================

/**
 * @brief Collision layer type (up to 16 layers)
 */
using CollisionLayer = u16;

/**
 * @brief Collision mask type for layer filtering
 */
using CollisionMask = u16;

/**
 * @brief Predefined collision layers
 */
namespace CollisionLayers {
    constexpr CollisionLayer None        = 0;
    constexpr CollisionLayer Default     = 1 << 0;
    constexpr CollisionLayer Static      = 1 << 1;
    constexpr CollisionLayer Dynamic     = 1 << 2;
    constexpr CollisionLayer Kinematic   = 1 << 3;
    constexpr CollisionLayer Trigger     = 1 << 4;
    constexpr CollisionLayer Character   = 1 << 5;
    constexpr CollisionLayer Projectile  = 1 << 6;
    constexpr CollisionLayer Debris      = 1 << 7;
    constexpr CollisionLayer Vehicle     = 1 << 8;
    constexpr CollisionLayer Water       = 1 << 9;
    constexpr CollisionLayer UI          = 1 << 10;
    constexpr CollisionLayer Custom1     = 1 << 11;
    constexpr CollisionLayer Custom2     = 1 << 12;
    constexpr CollisionLayer Custom3     = 1 << 13;
    constexpr CollisionLayer Custom4     = 1 << 14;
    constexpr CollisionLayer All         = 0xFFFF;
}

// =============================================================================
// Shape Types
// =============================================================================

/**
 * @brief Types of collision shapes
 */
enum class ShapeType : u8 {
    /// Sphere shape (radius only)
    Sphere = 0,
    
    /// Axis-aligned bounding box
    Box = 1,
    
    /// Capsule (cylinder with hemispherical caps)
    Capsule = 2,
    
    /// Cylinder (finite height cylinder)
    Cylinder = 3,
    
    /// Convex hull (arbitrary convex polyhedron)
    ConvexHull = 4,
    
    /// Triangle mesh (static only)
    TriangleMesh = 5,
    
    /// Height field (terrain)
    HeightField = 6,
    
    /// Compound shape (multiple sub-shapes)
    Compound = 7,
    
    /// Plane (infinite half-space)
    Plane = 8
};

// =============================================================================
// Physics Material
// =============================================================================

/**
 * @brief Physical material properties
 */
struct PhysicsMaterial {
    /// Static friction coefficient [0, 1+]
    f32 staticFriction = 0.5f;
    
    /// Dynamic friction coefficient [0, 1+]
    f32 dynamicFriction = 0.5f;
    
    /// Restitution (bounciness) [0, 1]
    f32 restitution = 0.0f;
    
    /// Density (kg/m³) for mass calculation
    f32 density = 1000.0f;
    
    /// Default material
    static PhysicsMaterial defaultMaterial() {
        return PhysicsMaterial{};
    }
    
    /// Rubber material (high friction, high bounce)
    static PhysicsMaterial rubber() {
        PhysicsMaterial mat;
        mat.staticFriction = 1.0f;
        mat.dynamicFriction = 0.8f;
        mat.restitution = 0.8f;
        mat.density = 1100.0f;
        return mat;
    }
    
    /// Ice material (low friction)
    static PhysicsMaterial ice() {
        PhysicsMaterial mat;
        mat.staticFriction = 0.05f;
        mat.dynamicFriction = 0.02f;
        mat.restitution = 0.1f;
        mat.density = 917.0f;
        return mat;
    }
    
    /// Metal material
    static PhysicsMaterial metal() {
        PhysicsMaterial mat;
        mat.staticFriction = 0.6f;
        mat.dynamicFriction = 0.4f;
        mat.restitution = 0.2f;
        mat.density = 7800.0f;
        return mat;
    }
    
    /// Wood material
    static PhysicsMaterial wood() {
        PhysicsMaterial mat;
        mat.staticFriction = 0.5f;
        mat.dynamicFriction = 0.4f;
        mat.restitution = 0.3f;
        mat.density = 700.0f;
        return mat;
    }
    
    /// Bouncy ball material
    static PhysicsMaterial bouncyBall() {
        PhysicsMaterial mat;
        mat.staticFriction = 0.8f;
        mat.dynamicFriction = 0.6f;
        mat.restitution = 0.95f;
        mat.density = 1200.0f;
        return mat;
    }
};

// =============================================================================
// Mass Properties
// =============================================================================

/**
 * @brief Mass properties of a rigid body
 */
struct MassProperties {
    /// Total mass (kg)
    f32 mass = 1.0f;
    
    /// Inverse mass (0 for infinite/static)
    f32 inverseMass = 1.0f;
    
    /// Center of mass in local space
    Vec3 centerOfMass = Vec3::zero();
    
    /// Inertia tensor (diagonal, local space)
    Vec3 inertia = Vec3::one();
    
    /// Inverse inertia tensor (diagonal, local space)
    Vec3 inverseInertia = Vec3::one();
    
    /**
     * @brief Create mass properties for a given mass
     * @param m Mass in kg
     * @return Mass properties
     */
    static MassProperties fromMass(f32 m) {
        MassProperties props;
        props.mass = m;
        props.inverseMass = (m > 0.0f) ? (1.0f / m) : 0.0f;
        return props;
    }
    
    /**
     * @brief Create infinite mass properties (static body)
     * @return Mass properties with zero inverse mass
     */
    static MassProperties infinite() {
        MassProperties props;
        props.mass = std::numeric_limits<f32>::infinity();
        props.inverseMass = 0.0f;
        props.inverseInertia = Vec3::zero();
        return props;
    }
    
    /**
     * @brief Calculate mass properties for a sphere
     * @param radius Sphere radius
     * @param density Density in kg/m³
     * @return Mass properties
     */
    static MassProperties sphere(f32 radius, f32 density) {
        f32 volume = (4.0f / 3.0f) * PI * radius * radius * radius;
        f32 m = volume * density;
        f32 i = (2.0f / 5.0f) * m * radius * radius;
        
        MassProperties props;
        props.mass = m;
        props.inverseMass = 1.0f / m;
        props.inertia = Vec3(i, i, i);
        props.inverseInertia = Vec3(1.0f / i, 1.0f / i, 1.0f / i);
        return props;
    }
    
    /**
     * @brief Calculate mass properties for a box
     * @param halfExtents Half-extents of the box
     * @param density Density in kg/m³
     * @return Mass properties
     */
    static MassProperties box(Vec3 halfExtents, f32 density) {
        f32 volume = 8.0f * halfExtents.x * halfExtents.y * halfExtents.z;
        f32 m = volume * density;
        
        f32 w2 = 4.0f * halfExtents.x * halfExtents.x;
        f32 h2 = 4.0f * halfExtents.y * halfExtents.y;
        f32 d2 = 4.0f * halfExtents.z * halfExtents.z;
        
        f32 ix = (m / 12.0f) * (h2 + d2);
        f32 iy = (m / 12.0f) * (w2 + d2);
        f32 iz = (m / 12.0f) * (w2 + h2);
        
        MassProperties props;
        props.mass = m;
        props.inverseMass = 1.0f / m;
        props.inertia = Vec3(ix, iy, iz);
        props.inverseInertia = Vec3(1.0f / ix, 1.0f / iy, 1.0f / iz);
        return props;
    }
    
    /**
     * @brief Calculate mass properties for a capsule
     * @param radius Capsule radius
     * @param height Capsule height (cylinder portion)
     * @param density Density in kg/m³
     * @return Mass properties
     */
    static MassProperties capsule(f32 radius, f32 height, f32 density) {
        // Cylinder volume + 2 hemisphere volumes
        f32 cylVolume = PI * radius * radius * height;
        f32 sphereVolume = (4.0f / 3.0f) * PI * radius * radius * radius;
        f32 volume = cylVolume + sphereVolume;
        f32 m = volume * density;
        
        // Inertia tensor for capsule (approximation)
        f32 cylMass = cylVolume * density;
        f32 sphereMass = sphereVolume * density;
        
        // Cylinder inertia
        f32 cylIy = cylMass * radius * radius * 0.5f;
        f32 cylIxz = (cylMass / 12.0f) * (3.0f * radius * radius + height * height);
        
        // Hemisphere inertia (using parallel axis theorem)
        f32 hemiI = (2.0f / 5.0f) * sphereMass * radius * radius;
        f32 hemiOffset = height * 0.5f + (3.0f / 8.0f) * radius;
        f32 hemiIxz = hemiI + sphereMass * hemiOffset * hemiOffset;
        
        f32 ix = cylIxz + hemiIxz;
        f32 iy = cylIy + hemiI;
        f32 iz = cylIxz + hemiIxz;
        
        MassProperties props;
        props.mass = m;
        props.inverseMass = 1.0f / m;
        props.inertia = Vec3(ix, iy, iz);
        props.inverseInertia = Vec3(1.0f / ix, 1.0f / iy, 1.0f / iz);
        return props;
    }
};

// =============================================================================
// Bounding Volumes
// =============================================================================

/**
 * @brief Axis-Aligned Bounding Box
 */
struct AABB {
    Vec3 min = Vec3(std::numeric_limits<f32>::max());
    Vec3 max = Vec3(std::numeric_limits<f32>::lowest());
    
    /// Check if AABB is valid (min < max)
    [[nodiscard]] bool isValid() const {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }
    
    /// Get center of AABB
    [[nodiscard]] Vec3 getCenter() const {
        return (min + max) * 0.5f;
    }
    
    /// Get extents (half-size)
    [[nodiscard]] Vec3 getExtents() const {
        return (max - min) * 0.5f;
    }
    
    /// Get size (full dimensions)
    [[nodiscard]] Vec3 getSize() const {
        return max - min;
    }
    
    /// Get surface area
    [[nodiscard]] f32 getSurfaceArea() const {
        Vec3 d = max - min;
        return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
    }
    
    /// Get volume
    [[nodiscard]] f32 getVolume() const {
        Vec3 d = max - min;
        return d.x * d.y * d.z;
    }
    
    /// Expand to include a point
    void expandToInclude(const Vec3& point) {
        min.x = (point.x < min.x) ? point.x : min.x;
        min.y = (point.y < min.y) ? point.y : min.y;
        min.z = (point.z < min.z) ? point.z : min.z;
        max.x = (point.x > max.x) ? point.x : max.x;
        max.y = (point.y > max.y) ? point.y : max.y;
        max.z = (point.z > max.z) ? point.z : max.z;
    }
    
    /// Expand to include another AABB
    void expandToInclude(const AABB& other) {
        min.x = (other.min.x < min.x) ? other.min.x : min.x;
        min.y = (other.min.y < min.y) ? other.min.y : min.y;
        min.z = (other.min.z < min.z) ? other.min.z : min.z;
        max.x = (other.max.x > max.x) ? other.max.x : max.x;
        max.y = (other.max.y > max.y) ? other.max.y : max.y;
        max.z = (other.max.z > max.z) ? other.max.z : max.z;
    }
    
    /// Add padding to AABB
    void expand(f32 padding) {
        min -= Vec3(padding);
        max += Vec3(padding);
    }
    
    /// Check if this AABB overlaps another
    [[nodiscard]] bool overlaps(const AABB& other) const {
        if (max.x < other.min.x || min.x > other.max.x) return false;
        if (max.y < other.min.y || min.y > other.max.y) return false;
        if (max.z < other.min.z || min.z > other.max.z) return false;
        return true;
    }
    
    /// Check if this AABB contains a point
    [[nodiscard]] bool contains(const Vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
    
    /// Check if this AABB fully contains another
    [[nodiscard]] bool contains(const AABB& other) const {
        return other.min.x >= min.x && other.max.x <= max.x &&
               other.min.y >= min.y && other.max.y <= max.y &&
               other.min.z >= min.z && other.max.z <= max.z;
    }
    
    /// Create AABB from center and half-extents
    static AABB fromCenterExtents(const Vec3& center, const Vec3& extents) {
        AABB aabb;
        aabb.min = center - extents;
        aabb.max = center + extents;
        return aabb;
    }
    
    /// Create AABB from min and max points
    static AABB fromMinMax(const Vec3& minPt, const Vec3& maxPt) {
        AABB aabb;
        aabb.min = minPt;
        aabb.max = maxPt;
        return aabb;
    }
    
    /// Create AABB for a sphere
    static AABB fromSphere(const Vec3& center, f32 radius) {
        return fromCenterExtents(center, Vec3(radius));
    }
};

/**
 * @brief Bounding Sphere
 */
struct BoundingSphere {
    Vec3 center = Vec3::zero();
    f32 radius = 0.0f;
    
    /// Check if sphere overlaps another
    [[nodiscard]] bool overlaps(const BoundingSphere& other) const {
        f32 distSq = (center - other.center).lengthSquared();
        f32 radiusSum = radius + other.radius;
        return distSq <= radiusSum * radiusSum;
    }
    
    /// Check if sphere contains a point
    [[nodiscard]] bool contains(const Vec3& point) const {
        return (point - center).lengthSquared() <= radius * radius;
    }
    
    /// Check if sphere overlaps an AABB
    [[nodiscard]] bool overlaps(const AABB& aabb) const {
        // Find closest point on AABB to sphere center
        Vec3 closest;
        closest.x = (center.x < aabb.min.x) ? aabb.min.x : 
                    (center.x > aabb.max.x) ? aabb.max.x : center.x;
        closest.y = (center.y < aabb.min.y) ? aabb.min.y : 
                    (center.y > aabb.max.y) ? aabb.max.y : center.y;
        closest.z = (center.z < aabb.min.z) ? aabb.min.z : 
                    (center.z > aabb.max.z) ? aabb.max.z : center.z;
        
        return (closest - center).lengthSquared() <= radius * radius;
    }
};

// =============================================================================
// Ray and Hit Results
// =============================================================================

/**
 * @brief Ray for raycasting
 */
struct Ray {
    Vec3 origin = Vec3::zero();
    Vec3 direction = Vec3::forward();
    f32 maxDistance = std::numeric_limits<f32>::max();
    
    /// Get point along ray at distance t
    [[nodiscard]] Vec3 getPoint(f32 t) const {
        return origin + direction * t;
    }
    
    /// Create ray from two points
    static Ray fromPoints(const Vec3& from, const Vec3& to) {
        Ray ray;
        ray.origin = from;
        Vec3 diff = to - from;
        ray.maxDistance = diff.length();
        ray.direction = (ray.maxDistance > PHYSICS_EPSILON) ? 
                        diff / ray.maxDistance : Vec3::forward();
        return ray;
    }
};

/**
 * @brief Result of a raycast or shape cast
 */
struct RaycastHit {
    /// Hit occurred
    bool hit = false;
    
    /// Distance along ray to hit point
    f32 distance = 0.0f;
    
    /// Hit point in world space
    Vec3 point = Vec3::zero();
    
    /// Surface normal at hit point
    Vec3 normal = Vec3::up();
    
    /// Body ID that was hit (0 = invalid)
    u32 bodyId = 0;
    
    /// Shape index within body
    u32 shapeIndex = 0;
    
    /// Triangle index (for mesh shapes)
    u32 triangleIndex = 0;
    
    /// Barycentric coordinates (for mesh shapes)
    Vec3 barycentric = Vec3::zero();
};

// =============================================================================
// Contact Information
// =============================================================================

/**
 * @brief Single contact point between two bodies
 */
struct ContactPoint {
    /// Contact point in world space
    Vec3 position = Vec3::zero();
    
    /// Contact normal (from body A to body B)
    Vec3 normal = Vec3::up();
    
    /// Penetration depth (negative = separating)
    f32 penetration = 0.0f;
    
    /// Impulse applied along normal
    f32 normalImpulse = 0.0f;
    
    /// Impulse applied along tangent (friction)
    Vec3 tangentImpulse = Vec3::zero();
};

/**
 * @brief Contact manifold between two bodies
 */
struct ContactManifold {
    /// Body A ID
    u32 bodyA = 0;
    
    /// Body B ID
    u32 bodyB = 0;
    
    /// Shape indices
    u32 shapeIndexA = 0;
    u32 shapeIndexB = 0;
    
    /// Contact points (max 4 for stability)
    ContactPoint points[4];
    u32 pointCount = 0;
    
    /// Average contact normal
    Vec3 normal = Vec3::up();
    
    /// Combined friction
    f32 friction = 0.5f;
    
    /// Combined restitution
    f32 restitution = 0.0f;
    
    /// Is this a sensor/trigger contact (no response)
    bool isSensor = false;
};

// =============================================================================
// Physics World Configuration
// =============================================================================

/**
 * @brief Configuration for the physics world
 */
struct PhysicsWorldConfig {
    /// Gravity vector
    Vec3 gravity = DEFAULT_GRAVITY;
    
    /// Fixed timestep for simulation
    f32 fixedTimestep = DEFAULT_TIMESTEP;
    
    /// Maximum substeps per frame
    u32 maxSubsteps = MAX_SUBSTEPS;
    
    /// Maximum bodies in the world
    u32 maxBodies = 65536;
    
    /// Maximum contacts per step
    u32 maxContacts = 65536;
    
    /// Enable sleeping optimization
    bool enableSleeping = true;
    
    /// Enable continuous collision detection
    bool enableCCD = true;
    
    /// Number of velocity iterations
    u32 velocityIterations = 8;
    
    /// Number of position iterations
    u32 positionIterations = 3;
    
    /// Broadphase type
    enum class BroadphaseType : u8 {
        BruteForce,     ///< O(n²) - good for < 100 bodies
        SortAndSweep,   ///< O(n log n) - good for < 1000 bodies
        BVH             ///< O(n log n) - good for any size
    } broadphaseType = BroadphaseType::BVH;
    
    /// Default configuration
    static PhysicsWorldConfig defaultConfig() {
        return PhysicsWorldConfig{};
    }
    
    /// Mobile-optimized configuration
    static PhysicsWorldConfig mobileOptimized() {
        PhysicsWorldConfig config;
        config.maxSubsteps = 4;
        config.velocityIterations = 4;
        config.positionIterations = 2;
        config.maxBodies = 1024;
        config.maxContacts = 4096;
        return config;
    }
    
    /// High-quality configuration
    static PhysicsWorldConfig highQuality() {
        PhysicsWorldConfig config;
        config.maxSubsteps = 16;
        config.velocityIterations = 16;
        config.positionIterations = 6;
        config.maxBodies = 262144;
        config.maxContacts = 262144;
        return config;
    }
};

// =============================================================================
// Body State
// =============================================================================

/**
 * @brief Current state of a rigid body
 */
struct BodyState {
    /// Position in world space
    Vec3 position = Vec3::zero();
    
    /// Orientation quaternion
    Quat orientation = Quat::identity();
    
    /// Linear velocity
    Vec3 linearVelocity = Vec3::zero();
    
    /// Angular velocity
    Vec3 angularVelocity = Vec3::zero();
};

/**
 * @brief Flags for body state
 */
enum class BodyFlags : u32 {
    None = 0,
    Active = 1 << 0,        ///< Body is active in simulation
    Sleeping = 1 << 1,      ///< Body is sleeping
    CanSleep = 1 << 2,      ///< Body is allowed to sleep
    UseCCD = 1 << 3,        ///< Use continuous collision detection
    IsSensor = 1 << 4,      ///< Body is a sensor/trigger
    IsKinematic = 1 << 5,   ///< Body is kinematic
    IsStatic = 1 << 6,      ///< Body is static
    Frozen = 1 << 7,        ///< Body is temporarily frozen
    
    Default = Active | CanSleep
};

inline BodyFlags operator|(BodyFlags a, BodyFlags b) {
    return static_cast<BodyFlags>(static_cast<u32>(a) | static_cast<u32>(b));
}

inline BodyFlags operator&(BodyFlags a, BodyFlags b) {
    return static_cast<BodyFlags>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline bool hasFlag(BodyFlags flags, BodyFlags flag) {
    return (static_cast<u32>(flags) & static_cast<u32>(flag)) != 0;
}

} // namespace nova::physics
