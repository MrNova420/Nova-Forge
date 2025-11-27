/**
 * @file rigid_body.hpp
 * @brief NovaCore Physics System - Rigid Body
 * 
 * Provides rigid body dynamics including:
 * - Position and orientation integration
 * - Force and impulse application
 * - Velocity management
 * - Sleep state handling
 * - ECS integration via BodyId
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "physics_types.hpp"
#include "collision_shape.hpp"
#include <memory>
#include <vector>

namespace nova::physics {

/**
 * @brief Unique identifier for a rigid body
 */
using BodyId = u32;
constexpr BodyId INVALID_BODY_ID = 0;

/**
 * @brief Descriptor for creating a rigid body
 */
struct RigidBodyDesc {
    /// Initial position
    Vec3 position = Vec3::zero();
    
    /// Initial orientation
    Quat orientation = Quat::identity();
    
    /// Initial linear velocity
    Vec3 linearVelocity = Vec3::zero();
    
    /// Initial angular velocity
    Vec3 angularVelocity = Vec3::zero();
    
    /// Motion type (static, kinematic, dynamic)
    MotionType motionType = MotionType::Dynamic;
    
    /// Motion quality
    MotionQuality motionQuality = MotionQuality::Discrete;
    
    /// Collision shape (required for non-static bodies)
    std::shared_ptr<CollisionShape> shape;
    
    /// Physics material
    PhysicsMaterial material;
    
    /// Mass (0 = auto-calculate from shape and density)
    f32 mass = 0.0f;
    
    /// Linear damping [0, 1]
    f32 linearDamping = DEFAULT_LINEAR_DAMPING;
    
    /// Angular damping [0, 1]
    f32 angularDamping = DEFAULT_ANGULAR_DAMPING;
    
    /// Gravity scale (0 = no gravity, 1 = normal, 2 = double, etc.)
    f32 gravityScale = 1.0f;
    
    /// Collision layer
    CollisionLayer layer = CollisionLayers::Default;
    
    /// Collision mask (which layers to collide with)
    CollisionMask mask = CollisionLayers::All;
    
    /// Body flags
    BodyFlags flags = BodyFlags::Default;
    
    /// Is this a sensor/trigger (no physical response)
    bool isSensor = false;
    
    /// Allow sleeping
    bool allowSleep = true;
    
    /// Start sleeping
    bool startSleeping = false;
    
    /// User data pointer
    void* userData = nullptr;
    
    /// Debug name
    std::string name;
    
    // Factory methods for common configurations
    
    /**
     * @brief Create a static body descriptor
     */
    static RigidBodyDesc staticBody(std::shared_ptr<CollisionShape> shape) {
        RigidBodyDesc desc;
        desc.motionType = MotionType::Static;
        desc.shape = std::move(shape);
        desc.flags = BodyFlags::Active | BodyFlags::IsStatic;
        return desc;
    }
    
    /**
     * @brief Create a kinematic body descriptor
     */
    static RigidBodyDesc kinematicBody(std::shared_ptr<CollisionShape> shape) {
        RigidBodyDesc desc;
        desc.motionType = MotionType::Kinematic;
        desc.shape = std::move(shape);
        desc.flags = BodyFlags::Active | BodyFlags::IsKinematic | BodyFlags::CanSleep;
        return desc;
    }
    
    /**
     * @brief Create a dynamic body descriptor
     */
    static RigidBodyDesc dynamicBody(std::shared_ptr<CollisionShape> shape, f32 mass = 0.0f) {
        RigidBodyDesc desc;
        desc.motionType = MotionType::Dynamic;
        desc.shape = std::move(shape);
        desc.mass = mass;
        return desc;
    }
    
    /**
     * @brief Create a sensor/trigger body descriptor
     */
    static RigidBodyDesc sensorBody(std::shared_ptr<CollisionShape> shape) {
        RigidBodyDesc desc;
        desc.motionType = MotionType::Static;
        desc.shape = std::move(shape);
        desc.isSensor = true;
        desc.flags = BodyFlags::Active | BodyFlags::IsSensor;
        return desc;
    }
};

/**
 * @brief Rigid body in the physics simulation
 * 
 * Represents a physical object that can move and collide.
 * Bodies are managed by PhysicsWorld and should not be
 * created directly.
 */
class RigidBody {
public:
    /**
     * @brief Create a rigid body from a descriptor
     * @param id Unique body ID
     * @param desc Body descriptor
     */
    RigidBody(BodyId id, const RigidBodyDesc& desc);
    
    ~RigidBody() = default;
    
    // Non-copyable, movable
    RigidBody(const RigidBody&) = delete;
    RigidBody& operator=(const RigidBody&) = delete;
    RigidBody(RigidBody&&) = default;
    RigidBody& operator=(RigidBody&&) = default;
    
    // =========================================================================
    // Identity
    // =========================================================================
    
    /**
     * @brief Get the body ID
     */
    [[nodiscard]] BodyId getId() const { return m_id; }
    
    /**
     * @brief Get the debug name
     */
    [[nodiscard]] const std::string& getName() const { return m_name; }
    
    /**
     * @brief Set the debug name
     */
    void setName(const std::string& name) { m_name = name; }
    
    // =========================================================================
    // Transform
    // =========================================================================
    
    /**
     * @brief Get current position
     */
    [[nodiscard]] const Vec3& getPosition() const { return m_position; }
    
    /**
     * @brief Set position (teleport)
     */
    void setPosition(const Vec3& position);
    
    /**
     * @brief Get current orientation
     */
    [[nodiscard]] const Quat& getOrientation() const { return m_orientation; }
    
    /**
     * @brief Set orientation
     */
    void setOrientation(const Quat& orientation);
    
    /**
     * @brief Get the transformation matrix
     */
    [[nodiscard]] Mat4 getTransformMatrix() const;
    
    /**
     * @brief Get the inverse transformation matrix
     */
    [[nodiscard]] Mat4 getInverseTransformMatrix() const;
    
    /**
     * @brief Set both position and orientation
     */
    void setTransform(const Vec3& position, const Quat& orientation);
    
    /**
     * @brief Move by a delta (for kinematic bodies)
     */
    void moveKinematic(const Vec3& targetPosition, const Quat& targetOrientation, f32 deltaTime);
    
    // =========================================================================
    // Velocity
    // =========================================================================
    
    /**
     * @brief Get linear velocity
     */
    [[nodiscard]] const Vec3& getLinearVelocity() const { return m_linearVelocity; }
    
    /**
     * @brief Set linear velocity
     */
    void setLinearVelocity(const Vec3& velocity);
    
    /**
     * @brief Get angular velocity
     */
    [[nodiscard]] const Vec3& getAngularVelocity() const { return m_angularVelocity; }
    
    /**
     * @brief Set angular velocity
     */
    void setAngularVelocity(const Vec3& velocity);
    
    /**
     * @brief Get velocity at a world point
     */
    [[nodiscard]] Vec3 getVelocityAtPoint(const Vec3& worldPoint) const;
    
    // =========================================================================
    // Forces and Impulses
    // =========================================================================
    
    /**
     * @brief Apply force at center of mass
     * @param force Force in world space (Newtons)
     */
    void applyForce(const Vec3& force);
    
    /**
     * @brief Apply force at a world point
     * @param force Force in world space (Newtons)
     * @param point Application point in world space
     */
    void applyForceAtPoint(const Vec3& force, const Vec3& point);
    
    /**
     * @brief Apply torque
     * @param torque Torque in world space (Newton-meters)
     */
    void applyTorque(const Vec3& torque);
    
    /**
     * @brief Apply impulse at center of mass
     * @param impulse Impulse in world space (kg*m/s)
     */
    void applyImpulse(const Vec3& impulse);
    
    /**
     * @brief Apply impulse at a world point
     * @param impulse Impulse in world space (kg*m/s)
     * @param point Application point in world space
     */
    void applyImpulseAtPoint(const Vec3& impulse, const Vec3& point);
    
    /**
     * @brief Apply angular impulse
     * @param impulse Angular impulse (kg*m²/s)
     */
    void applyAngularImpulse(const Vec3& impulse);
    
    /**
     * @brief Clear all accumulated forces
     */
    void clearForces();
    
    /**
     * @brief Get accumulated force
     */
    [[nodiscard]] const Vec3& getAccumulatedForce() const { return m_accumulatedForce; }
    
    /**
     * @brief Get accumulated torque
     */
    [[nodiscard]] const Vec3& getAccumulatedTorque() const { return m_accumulatedTorque; }
    
    // =========================================================================
    // Mass Properties
    // =========================================================================
    
    /**
     * @brief Get mass
     */
    [[nodiscard]] f32 getMass() const { return m_massProperties.mass; }
    
    /**
     * @brief Get inverse mass (0 for static bodies)
     */
    [[nodiscard]] f32 getInverseMass() const { return m_massProperties.inverseMass; }
    
    /**
     * @brief Get inertia tensor (local space)
     */
    [[nodiscard]] const Vec3& getInertia() const { return m_massProperties.inertia; }
    
    /**
     * @brief Get inverse inertia tensor (local space)
     */
    [[nodiscard]] const Vec3& getInverseInertia() const { return m_massProperties.inverseInertia; }
    
    /**
     * @brief Get center of mass in local space
     */
    [[nodiscard]] const Vec3& getLocalCenterOfMass() const { return m_massProperties.centerOfMass; }
    
    /**
     * @brief Get center of mass in world space
     */
    [[nodiscard]] Vec3 getWorldCenterOfMass() const;
    
    /**
     * @brief Set mass properties manually
     */
    void setMassProperties(const MassProperties& props) { m_massProperties = props; }
    
    /**
     * @brief Recalculate mass properties from shape
     */
    void recalculateMassProperties();
    
    // =========================================================================
    // Damping
    // =========================================================================
    
    /**
     * @brief Get linear damping
     */
    [[nodiscard]] f32 getLinearDamping() const { return m_linearDamping; }
    
    /**
     * @brief Set linear damping
     */
    void setLinearDamping(f32 damping) { m_linearDamping = damping; }
    
    /**
     * @brief Get angular damping
     */
    [[nodiscard]] f32 getAngularDamping() const { return m_angularDamping; }
    
    /**
     * @brief Set angular damping
     */
    void setAngularDamping(f32 damping) { m_angularDamping = damping; }
    
    // =========================================================================
    // Gravity
    // =========================================================================
    
    /**
     * @brief Get gravity scale
     */
    [[nodiscard]] f32 getGravityScale() const { return m_gravityScale; }
    
    /**
     * @brief Set gravity scale
     */
    void setGravityScale(f32 scale) { m_gravityScale = scale; }
    
    // =========================================================================
    // Motion Type
    // =========================================================================
    
    /**
     * @brief Get motion type
     */
    [[nodiscard]] MotionType getMotionType() const { return m_motionType; }
    
    /**
     * @brief Set motion type
     */
    void setMotionType(MotionType type);
    
    /**
     * @brief Check if body is static
     */
    [[nodiscard]] bool isStatic() const { return m_motionType == MotionType::Static; }
    
    /**
     * @brief Check if body is kinematic
     */
    [[nodiscard]] bool isKinematic() const { return m_motionType == MotionType::Kinematic; }
    
    /**
     * @brief Check if body is dynamic
     */
    [[nodiscard]] bool isDynamic() const { return m_motionType == MotionType::Dynamic; }
    
    // =========================================================================
    // Collision
    // =========================================================================
    
    /**
     * @brief Get collision shape
     */
    [[nodiscard]] const std::shared_ptr<CollisionShape>& getShape() const { return m_shape; }
    
    /**
     * @brief Set collision shape
     */
    void setShape(std::shared_ptr<CollisionShape> shape);
    
    /**
     * @brief Get collision layer
     */
    [[nodiscard]] CollisionLayer getLayer() const { return m_layer; }
    
    /**
     * @brief Set collision layer
     */
    void setLayer(CollisionLayer layer) { m_layer = layer; }
    
    /**
     * @brief Get collision mask
     */
    [[nodiscard]] CollisionMask getMask() const { return m_mask; }
    
    /**
     * @brief Set collision mask
     */
    void setMask(CollisionMask mask) { m_mask = mask; }
    
    /**
     * @brief Check if this body should collide with another
     */
    [[nodiscard]] bool shouldCollideWith(const RigidBody& other) const;
    
    /**
     * @brief Check if body is a sensor
     */
    [[nodiscard]] bool isSensor() const { return m_isSensor; }
    
    /**
     * @brief Set sensor mode
     */
    void setSensor(bool sensor) { m_isSensor = sensor; }
    
    /**
     * @brief Get world AABB
     */
    [[nodiscard]] AABB getWorldBounds() const;
    
    // =========================================================================
    // Material
    // =========================================================================
    
    /**
     * @brief Get physics material
     */
    [[nodiscard]] const PhysicsMaterial& getMaterial() const { return m_material; }
    
    /**
     * @brief Set physics material
     */
    void setMaterial(const PhysicsMaterial& material) { m_material = material; }
    
    // =========================================================================
    // Sleep State
    // =========================================================================
    
    /**
     * @brief Check if body is sleeping
     */
    [[nodiscard]] bool isSleeping() const { return hasFlag(BodyFlags::Sleeping); }
    
    /**
     * @brief Check if body is active (not sleeping)
     */
    [[nodiscard]] bool isActive() const { return hasFlag(BodyFlags::Active) && !isSleeping(); }
    
    /**
     * @brief Wake up the body
     */
    void wakeUp();
    
    /**
     * @brief Put the body to sleep
     */
    void sleep();
    
    /**
     * @brief Check if body can sleep
     */
    [[nodiscard]] bool canSleep() const { return hasFlag(BodyFlags::CanSleep); }
    
    /**
     * @brief Set whether body can sleep
     */
    void setCanSleep(bool canSleep);
    
    /**
     * @brief Get time spent at low velocity
     */
    [[nodiscard]] f32 getSleepTimer() const { return m_sleepTimer; }
    
    /**
     * @brief Reset sleep timer (called when body moves)
     */
    void resetSleepTimer() { m_sleepTimer = 0.0f; }
    
    /**
     * @brief Update sleep timer
     */
    void updateSleepTimer(f32 deltaTime);
    
    // =========================================================================
    // Flags
    // =========================================================================
    
    /**
     * @brief Get body flags
     */
    [[nodiscard]] BodyFlags getFlags() const { return m_flags; }
    
    /**
     * @brief Set body flags
     */
    void setFlags(BodyFlags flags) { m_flags = flags; }
    
    /**
     * @brief Add a flag
     */
    void addFlag(BodyFlags flag) { m_flags = m_flags | flag; }
    
    /**
     * @brief Remove a flag
     */
    void removeFlag(BodyFlags flag) { 
        m_flags = static_cast<BodyFlags>(static_cast<u32>(m_flags) & ~static_cast<u32>(flag)); 
    }
    
    /**
     * @brief Check if flag is set
     */
    [[nodiscard]] bool hasFlag(BodyFlags flag) const { return nova::physics::hasFlag(m_flags, flag); }
    
    // =========================================================================
    // User Data
    // =========================================================================
    
    /**
     * @brief Get user data
     */
    [[nodiscard]] void* getUserData() const { return m_userData; }
    
    /**
     * @brief Set user data
     */
    void setUserData(void* data) { m_userData = data; }
    
    // =========================================================================
    // Simulation (called by PhysicsWorld)
    // =========================================================================
    
    /**
     * @brief Integrate velocities (apply forces)
     */
    void integrateVelocities(f32 deltaTime, const Vec3& gravity);
    
    /**
     * @brief Integrate positions
     */
    void integratePositions(f32 deltaTime);
    
    /**
     * @brief Apply damping
     */
    void applyDamping(f32 deltaTime);
    
    /**
     * @brief Clamp velocities to maximum values
     */
    void clampVelocities();
    
    /**
     * @brief Store current state for interpolation
     */
    void storeState();
    
    /**
     * @brief Get interpolated state
     * @param alpha Interpolation factor [0, 1]
     * @return Interpolated state
     */
    [[nodiscard]] BodyState getInterpolatedState(f32 alpha) const;

private:
    // Identity
    BodyId m_id;
    std::string m_name;
    
    // Transform
    Vec3 m_position = Vec3::zero();
    Quat m_orientation = Quat::identity();
    
    // Previous state for interpolation
    Vec3 m_previousPosition = Vec3::zero();
    Quat m_previousOrientation = Quat::identity();
    
    // Velocity
    Vec3 m_linearVelocity = Vec3::zero();
    Vec3 m_angularVelocity = Vec3::zero();
    
    // Accumulated forces (cleared each step)
    Vec3 m_accumulatedForce = Vec3::zero();
    Vec3 m_accumulatedTorque = Vec3::zero();
    
    // Mass properties
    MassProperties m_massProperties;
    
    // Damping
    f32 m_linearDamping = DEFAULT_LINEAR_DAMPING;
    f32 m_angularDamping = DEFAULT_ANGULAR_DAMPING;
    
    // Gravity
    f32 m_gravityScale = 1.0f;
    
    // Motion type
    MotionType m_motionType = MotionType::Dynamic;
    MotionQuality m_motionQuality = MotionQuality::Discrete;
    
    // Collision
    std::shared_ptr<CollisionShape> m_shape;
    CollisionLayer m_layer = CollisionLayers::Default;
    CollisionMask m_mask = CollisionLayers::All;
    bool m_isSensor = false;
    
    // Material
    PhysicsMaterial m_material;
    
    // Sleep
    f32 m_sleepTimer = 0.0f;
    
    // Flags
    BodyFlags m_flags = BodyFlags::Default;
    
    // User data
    void* m_userData = nullptr;
};

} // namespace nova::physics
