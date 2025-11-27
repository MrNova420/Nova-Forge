/**
 * @file rigid_body.cpp
 * @brief NovaCore Physics System - Rigid Body Implementation
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/physics/rigid_body.hpp"
#include <cmath>
#include <algorithm>

namespace nova::physics {

RigidBody::RigidBody(BodyId id, const RigidBodyDesc& desc)
    : m_id(id)
    , m_name(desc.name)
    , m_position(desc.position)
    , m_orientation(desc.orientation)
    , m_previousPosition(desc.position)
    , m_previousOrientation(desc.orientation)
    , m_linearVelocity(desc.linearVelocity)
    , m_angularVelocity(desc.angularVelocity)
    , m_linearDamping(desc.linearDamping)
    , m_angularDamping(desc.angularDamping)
    , m_gravityScale(desc.gravityScale)
    , m_motionType(desc.motionType)
    , m_motionQuality(desc.motionQuality)
    , m_shape(desc.shape)
    , m_layer(desc.layer)
    , m_mask(desc.mask)
    , m_isSensor(desc.isSensor)
    , m_material(desc.material)
    , m_flags(desc.flags)
    , m_userData(desc.userData)
{
    // Calculate mass properties
    if (m_motionType == MotionType::Static) {
        m_massProperties = MassProperties::infinite();
    } else if (desc.mass > 0.0f) {
        m_massProperties = MassProperties::fromMass(desc.mass);
        if (m_shape) {
            // Scale inertia from shape
            MassProperties shapeProps = m_shape->calculateMassProperties(m_material.density);
            f32 scale = desc.mass / shapeProps.mass;
            m_massProperties.inertia = shapeProps.inertia * scale;
            m_massProperties.inverseInertia = Vec3(
                (shapeProps.inertia.x > 0.0f) ? (1.0f / (shapeProps.inertia.x * scale)) : 0.0f,
                (shapeProps.inertia.y > 0.0f) ? (1.0f / (shapeProps.inertia.y * scale)) : 0.0f,
                (shapeProps.inertia.z > 0.0f) ? (1.0f / (shapeProps.inertia.z * scale)) : 0.0f
            );
            m_massProperties.centerOfMass = shapeProps.centerOfMass;
        }
    } else if (m_shape) {
        m_massProperties = m_shape->calculateMassProperties(m_material.density);
    }
    
    // Handle initial sleep state
    if (!desc.allowSleep) {
        removeFlag(BodyFlags::CanSleep);
    }
    
    if (desc.startSleeping && desc.allowSleep && m_motionType != MotionType::Static) {
        addFlag(BodyFlags::Sleeping);
    }
}

void RigidBody::setPosition(const Vec3& position) {
    m_position = position;
    wakeUp();
}

void RigidBody::setOrientation(const Quat& orientation) {
    m_orientation = orientation.normalized();
    wakeUp();
}

Mat4 RigidBody::getTransformMatrix() const {
    Mat4 translation = Mat4::translate(m_position);
    Mat4 rotation = m_orientation.toMat4();
    return translation * rotation;
}

Mat4 RigidBody::getInverseTransformMatrix() const {
    Mat4 invRotation = m_orientation.inverse().toMat4();
    Mat4 invTranslation = Mat4::translate(-m_position);
    return invRotation * invTranslation;
}

void RigidBody::setTransform(const Vec3& position, const Quat& orientation) {
    m_position = position;
    m_orientation = orientation.normalized();
    wakeUp();
}

void RigidBody::moveKinematic(const Vec3& targetPosition, const Quat& targetOrientation, f32 deltaTime) {
    if (m_motionType != MotionType::Kinematic || deltaTime <= 0.0f) {
        return;
    }
    
    // Calculate required velocities to reach target
    m_linearVelocity = (targetPosition - m_position) / deltaTime;
    
    // Calculate angular velocity from quaternion difference
    Quat deltaRot = targetOrientation * m_orientation.inverse();
    
    // Convert quaternion to axis-angle
    f32 angle = 2.0f * std::acos(std::clamp(deltaRot.w, -1.0f, 1.0f));
    
    if (std::abs(angle) > PHYSICS_EPSILON) {
        f32 sinHalfAngle = std::sqrt(1.0f - deltaRot.w * deltaRot.w);
        if (sinHalfAngle > PHYSICS_EPSILON) {
            Vec3 axis(deltaRot.x / sinHalfAngle, deltaRot.y / sinHalfAngle, deltaRot.z / sinHalfAngle);
            m_angularVelocity = axis * (angle / deltaTime);
        }
    } else {
        m_angularVelocity = Vec3::zero();
    }
    
    wakeUp();
}

void RigidBody::setLinearVelocity(const Vec3& velocity) {
    if (m_motionType != MotionType::Dynamic) return;
    m_linearVelocity = velocity;
    wakeUp();
}

void RigidBody::setAngularVelocity(const Vec3& velocity) {
    if (m_motionType != MotionType::Dynamic) return;
    m_angularVelocity = velocity;
    wakeUp();
}

Vec3 RigidBody::getVelocityAtPoint(const Vec3& worldPoint) const {
    Vec3 r = worldPoint - getWorldCenterOfMass();
    return m_linearVelocity + m_angularVelocity.cross(r);
}

void RigidBody::applyForce(const Vec3& force) {
    if (m_motionType != MotionType::Dynamic) return;
    m_accumulatedForce += force;
    wakeUp();
}

void RigidBody::applyForceAtPoint(const Vec3& force, const Vec3& point) {
    if (m_motionType != MotionType::Dynamic) return;
    
    m_accumulatedForce += force;
    
    Vec3 r = point - getWorldCenterOfMass();
    m_accumulatedTorque += r.cross(force);
    
    wakeUp();
}

void RigidBody::applyTorque(const Vec3& torque) {
    if (m_motionType != MotionType::Dynamic) return;
    m_accumulatedTorque += torque;
    wakeUp();
}

void RigidBody::applyImpulse(const Vec3& impulse) {
    if (m_motionType != MotionType::Dynamic) return;
    m_linearVelocity += impulse * m_massProperties.inverseMass;
    wakeUp();
}

void RigidBody::applyImpulseAtPoint(const Vec3& impulse, const Vec3& point) {
    if (m_motionType != MotionType::Dynamic) return;
    
    m_linearVelocity += impulse * m_massProperties.inverseMass;
    
    Vec3 r = point - getWorldCenterOfMass();
    Vec3 angularImpulse = r.cross(impulse);
    
    // Transform to local space, apply inverse inertia, transform back
    Vec3 localAngImp = m_orientation.inverse() * angularImpulse;
    Vec3 localAngVel = localAngImp * m_massProperties.inverseInertia;
    m_angularVelocity += m_orientation * localAngVel;
    
    wakeUp();
}

void RigidBody::applyAngularImpulse(const Vec3& impulse) {
    if (m_motionType != MotionType::Dynamic) return;
    
    Vec3 localImpulse = m_orientation.inverse() * impulse;
    Vec3 localAngVel = localImpulse * m_massProperties.inverseInertia;
    m_angularVelocity += m_orientation * localAngVel;
    
    wakeUp();
}

void RigidBody::clearForces() {
    m_accumulatedForce = Vec3::zero();
    m_accumulatedTorque = Vec3::zero();
}

Vec3 RigidBody::getWorldCenterOfMass() const {
    return m_position + m_orientation * m_massProperties.centerOfMass;
}

void RigidBody::recalculateMassProperties() {
    if (m_motionType == MotionType::Static) {
        m_massProperties = MassProperties::infinite();
    } else if (m_shape) {
        m_massProperties = m_shape->calculateMassProperties(m_material.density);
    }
}

void RigidBody::setMotionType(MotionType type) {
    if (m_motionType == type) return;
    
    m_motionType = type;
    
    if (type == MotionType::Static) {
        m_massProperties = MassProperties::infinite();
        m_linearVelocity = Vec3::zero();
        m_angularVelocity = Vec3::zero();
        m_accumulatedForce = Vec3::zero();
        m_accumulatedTorque = Vec3::zero();
        addFlag(BodyFlags::IsStatic);
        removeFlag(BodyFlags::IsKinematic);
    } else if (type == MotionType::Kinematic) {
        m_accumulatedForce = Vec3::zero();
        m_accumulatedTorque = Vec3::zero();
        addFlag(BodyFlags::IsKinematic);
        removeFlag(BodyFlags::IsStatic);
        recalculateMassProperties();
    } else {
        removeFlag(BodyFlags::IsStatic);
        removeFlag(BodyFlags::IsKinematic);
        recalculateMassProperties();
    }
}

void RigidBody::setShape(std::shared_ptr<CollisionShape> shape) {
    m_shape = std::move(shape);
    recalculateMassProperties();
}

bool RigidBody::shouldCollideWith(const RigidBody& other) const {
    return (m_layer & other.m_mask) != 0 && (other.m_layer & m_mask) != 0;
}

AABB RigidBody::getWorldBounds() const {
    if (!m_shape) {
        return AABB::fromCenterExtents(m_position, Vec3(0.1f));
    }
    return m_shape->getWorldBounds(m_position, m_orientation);
}

void RigidBody::wakeUp() {
    if (isSleeping()) {
        removeFlag(BodyFlags::Sleeping);
        m_sleepTimer = 0.0f;
    }
}

void RigidBody::sleep() {
    if (canSleep() && !isStatic()) {
        addFlag(BodyFlags::Sleeping);
        m_linearVelocity = Vec3::zero();
        m_angularVelocity = Vec3::zero();
    }
}

void RigidBody::setCanSleep(bool canSleep) {
    if (canSleep) {
        addFlag(BodyFlags::CanSleep);
    } else {
        removeFlag(BodyFlags::CanSleep);
        wakeUp();
    }
}

void RigidBody::updateSleepTimer(f32 deltaTime) {
    if (!canSleep() || isStatic()) return;
    
    f32 linearSpeed = m_linearVelocity.length();
    f32 angularSpeed = m_angularVelocity.length();
    
    if (linearSpeed < SLEEP_LINEAR_VELOCITY && angularSpeed < SLEEP_ANGULAR_VELOCITY) {
        m_sleepTimer += deltaTime;
        if (m_sleepTimer >= SLEEP_TIME_THRESHOLD) {
            sleep();
        }
    } else {
        m_sleepTimer = 0.0f;
    }
}

void RigidBody::integrateVelocities(f32 deltaTime, const Vec3& gravity) {
    if (m_motionType != MotionType::Dynamic || isSleeping()) return;
    
    // Apply gravity
    Vec3 acceleration = gravity * m_gravityScale;
    
    // Apply accumulated forces
    acceleration += m_accumulatedForce * m_massProperties.inverseMass;
    
    // Integrate linear velocity
    m_linearVelocity += acceleration * deltaTime;
    
    // Apply accumulated torque
    Vec3 localTorque = m_orientation.inverse() * m_accumulatedTorque;
    Vec3 localAngAcc = localTorque * m_massProperties.inverseInertia;
    Vec3 angularAcceleration = m_orientation * localAngAcc;
    
    // Integrate angular velocity
    m_angularVelocity += angularAcceleration * deltaTime;
    
    // Clear forces
    clearForces();
}

void RigidBody::integratePositions(f32 deltaTime) {
    if (m_motionType == MotionType::Static || isSleeping()) return;
    
    // Store previous state for interpolation
    m_previousPosition = m_position;
    m_previousOrientation = m_orientation;
    
    // Integrate position
    m_position += m_linearVelocity * deltaTime;
    
    // Integrate orientation using quaternion derivative
    f32 angSpeed = m_angularVelocity.length();
    if (angSpeed > PHYSICS_EPSILON) {
        Vec3 axis = m_angularVelocity / angSpeed;
        f32 angle = angSpeed * deltaTime;
        Quat deltaRot = Quat::fromAxisAngle(axis, angle);
        m_orientation = deltaRot * m_orientation;
        m_orientation = m_orientation.normalized();
    }
}

void RigidBody::applyDamping(f32 deltaTime) {
    if (m_motionType != MotionType::Dynamic || isSleeping()) return;
    
    // Exponential damping
    f32 linearDampFactor = std::pow(1.0f - m_linearDamping, deltaTime);
    f32 angularDampFactor = std::pow(1.0f - m_angularDamping, deltaTime);
    
    m_linearVelocity *= linearDampFactor;
    m_angularVelocity *= angularDampFactor;
}

void RigidBody::clampVelocities() {
    // Clamp linear velocity
    f32 linearSpeedSq = m_linearVelocity.lengthSquared();
    if (linearSpeedSq > MAX_LINEAR_VELOCITY * MAX_LINEAR_VELOCITY) {
        m_linearVelocity = m_linearVelocity.normalized() * MAX_LINEAR_VELOCITY;
    }
    
    // Clamp angular velocity
    f32 angularSpeedSq = m_angularVelocity.lengthSquared();
    if (angularSpeedSq > MAX_ANGULAR_VELOCITY * MAX_ANGULAR_VELOCITY) {
        m_angularVelocity = m_angularVelocity.normalized() * MAX_ANGULAR_VELOCITY;
    }
}

void RigidBody::storeState() {
    m_previousPosition = m_position;
    m_previousOrientation = m_orientation;
}

BodyState RigidBody::getInterpolatedState(f32 alpha) const {
    BodyState state;
    state.position = m_previousPosition.lerp(m_position, alpha);
    state.orientation = m_previousOrientation.slerp(m_orientation, alpha);
    state.linearVelocity = m_linearVelocity;
    state.angularVelocity = m_angularVelocity;
    return state;
}

} // namespace nova::physics
