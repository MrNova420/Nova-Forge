// =============================================================================
// NovaCore Engine - Transform
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Combined position, rotation, and scale transformation.
// Optimized for game object hierarchies and skeletal animation.
// =============================================================================

#pragma once

#include "nova/core/math/vec3.hpp"
#include "nova/core/math/quat.hpp"
#include "nova/core/math/mat4.hpp"

namespace nova::math {

/// @brief Transform representing position, rotation, and scale
/// @note More efficient than Mat4 for composition and interpolation
struct Transform {
    Vec3 position;
    Quat rotation;
    Vec3 scale;
    
    // ==========================================================================
    // Constructors
    // ==========================================================================
    
    /// Default constructor (identity transform)
    constexpr Transform() noexcept
        : position(Vec3::zero())
        , rotation(Quat::identity())
        , scale(Vec3::one()) {}
    
    /// Construct from position only
    constexpr explicit Transform(const Vec3& pos) noexcept
        : position(pos)
        , rotation(Quat::identity())
        , scale(Vec3::one()) {}
    
    /// Construct from position and rotation
    constexpr Transform(const Vec3& pos, const Quat& rot) noexcept
        : position(pos)
        , rotation(rot)
        , scale(Vec3::one()) {}
    
    /// Construct from position, rotation, and scale
    constexpr Transform(const Vec3& pos, const Quat& rot, const Vec3& scl) noexcept
        : position(pos)
        , rotation(rot)
        , scale(scl) {}
    
    // ==========================================================================
    // Static Factory Methods
    // ==========================================================================
    
    /// Identity transform
    [[nodiscard]] static constexpr Transform identity() noexcept { return Transform(); }
    
    /// Create translation-only transform
    [[nodiscard]] static constexpr Transform fromPosition(const Vec3& pos) noexcept {
        return Transform(pos);
    }
    
    /// Create rotation-only transform
    [[nodiscard]] static constexpr Transform fromRotation(const Quat& rot) noexcept {
        return Transform(Vec3::zero(), rot);
    }
    
    /// Create scale-only transform
    [[nodiscard]] static constexpr Transform fromScale(const Vec3& scl) noexcept {
        return Transform(Vec3::zero(), Quat::identity(), scl);
    }
    
    /// Create uniform scale transform
    [[nodiscard]] static constexpr Transform fromScale(f32 uniformScale) noexcept {
        return Transform(Vec3::zero(), Quat::identity(), Vec3(uniformScale));
    }
    
    /// Create from 4x4 matrix (extracts position, rotation, scale)
    [[nodiscard]] static Transform fromMatrix(const Mat4& m) noexcept {
        Transform t;
        
        // Extract position
        t.position = m.translation();
        
        // Extract scale
        t.scale.x = Vec3(m.columns[0].x, m.columns[0].y, m.columns[0].z).length();
        t.scale.y = Vec3(m.columns[1].x, m.columns[1].y, m.columns[1].z).length();
        t.scale.z = Vec3(m.columns[2].x, m.columns[2].y, m.columns[2].z).length();
        
        // Extract rotation (remove scale from matrix)
        Mat3 rotMat;
        if (t.scale.x > F32_EPSILON) {
            rotMat.columns[0] = Vec3(m.columns[0].x, m.columns[0].y, m.columns[0].z) / t.scale.x;
        }
        if (t.scale.y > F32_EPSILON) {
            rotMat.columns[1] = Vec3(m.columns[1].x, m.columns[1].y, m.columns[1].z) / t.scale.y;
        }
        if (t.scale.z > F32_EPSILON) {
            rotMat.columns[2] = Vec3(m.columns[2].x, m.columns[2].y, m.columns[2].z) / t.scale.z;
        }
        t.rotation = Quat::fromMatrix(rotMat);
        
        return t;
    }
    
    // ==========================================================================
    // Conversion
    // ==========================================================================
    
    /// Convert to 4x4 matrix
    [[nodiscard]] Mat4 toMatrix() const noexcept {
        Mat4 result = rotation.toMat4();
        
        // Apply scale to rotation columns
        result.columns[0] *= scale.x;
        result.columns[1] *= scale.y;
        result.columns[2] *= scale.z;
        
        // Set translation
        result.columns[3] = Vec4(position, 1.0f);
        
        return result;
    }
    
    /// Convert to inverse 4x4 matrix
    [[nodiscard]] Mat4 toInverseMatrix() const noexcept {
        return inverse().toMatrix();
    }
    
    // ==========================================================================
    // Transform Operations
    // ==========================================================================
    
    /// Combine two transforms (this * other)
    [[nodiscard]] Transform operator*(const Transform& other) const noexcept {
        return Transform(
            position + rotation * (scale * other.position),
            rotation * other.rotation,
            scale * other.scale
        );
    }
    
    /// Combine transforms in place
    Transform& operator*=(const Transform& other) noexcept {
        *this = *this * other;
        return *this;
    }
    
    /// Inverse transform
    [[nodiscard]] Transform inverse() const noexcept {
        Quat invRot = rotation.conjugate();
        Vec3 invScale = Vec3(
            scale.x > F32_EPSILON ? 1.0f / scale.x : 0.0f,
            scale.y > F32_EPSILON ? 1.0f / scale.y : 0.0f,
            scale.z > F32_EPSILON ? 1.0f / scale.z : 0.0f
        );
        Vec3 invPos = invRot * (-position * invScale);
        return Transform(invPos, invRot, invScale);
    }
    
    /// Transform a point
    [[nodiscard]] Vec3 transformPoint(const Vec3& point) const noexcept {
        return position + rotation * (scale * point);
    }
    
    /// Transform a direction (no translation, no scale)
    [[nodiscard]] Vec3 transformDirection(const Vec3& direction) const noexcept {
        return rotation * direction;
    }
    
    /// Transform a vector (no translation, includes scale)
    [[nodiscard]] Vec3 transformVector(const Vec3& vector) const noexcept {
        return rotation * (scale * vector);
    }
    
    /// Inverse transform a point
    [[nodiscard]] Vec3 inverseTransformPoint(const Vec3& point) const noexcept {
        Vec3 localPoint = rotation.conjugate() * (point - position);
        return Vec3(
            scale.x > F32_EPSILON ? localPoint.x / scale.x : 0.0f,
            scale.y > F32_EPSILON ? localPoint.y / scale.y : 0.0f,
            scale.z > F32_EPSILON ? localPoint.z / scale.z : 0.0f
        );
    }
    
    /// Inverse transform a direction
    [[nodiscard]] Vec3 inverseTransformDirection(const Vec3& direction) const noexcept {
        return rotation.conjugate() * direction;
    }
    
    // ==========================================================================
    // Interpolation
    // ==========================================================================
    
    /// Linear interpolation of transforms
    [[nodiscard]] Transform lerp(const Transform& other, f32 t) const noexcept {
        return Transform(
            position.lerp(other.position, t),
            rotation.nlerp(other.rotation, t),
            scale.lerp(other.scale, t)
        );
    }
    
    /// Spherical interpolation of transforms (smoother rotation)
    [[nodiscard]] Transform slerp(const Transform& other, f32 t) const noexcept {
        return Transform(
            position.lerp(other.position, t),
            rotation.slerp(other.rotation, t),
            scale.lerp(other.scale, t)
        );
    }
    
    // ==========================================================================
    // Direction Accessors
    // ==========================================================================
    
    /// Get forward direction (-Z)
    [[nodiscard]] Vec3 forward() const noexcept {
        return rotation.forward();
    }
    
    /// Get right direction (+X)
    [[nodiscard]] Vec3 right() const noexcept {
        return rotation.right();
    }
    
    /// Get up direction (+Y)
    [[nodiscard]] Vec3 up() const noexcept {
        return rotation.up();
    }
    
    // ==========================================================================
    // Modifiers
    // ==========================================================================
    
    /// Translate by offset
    constexpr void translate(const Vec3& offset) noexcept {
        position += offset;
    }
    
    /// Translate in local space
    void translateLocal(const Vec3& offset) noexcept {
        position += rotation * offset;
    }
    
    /// Rotate by quaternion
    void rotate(const Quat& rot) noexcept {
        rotation = rot * rotation;
        rotation.normalize();
    }
    
    /// Rotate around axis
    void rotateAround(const Vec3& axis, f32 radians) noexcept {
        rotate(Quat::fromAxisAngle(axis, radians));
    }
    
    /// Look at a target
    void lookAt(const Vec3& target, const Vec3& worldUp = Vec3::up()) noexcept {
        Vec3 direction = (target - position).normalized();
        if (direction.lengthSquared() > F32_EPSILON) {
            rotation = Quat::lookRotation(direction, worldUp);
        }
    }
    
    // ==========================================================================
    // Comparison
    // ==========================================================================
    
    [[nodiscard]] constexpr bool operator==(const Transform& other) const noexcept {
        return position == other.position && rotation == other.rotation && scale == other.scale;
    }
    
    [[nodiscard]] constexpr bool operator!=(const Transform& other) const noexcept {
        return !(*this == other);
    }
    
    /// Check approximate equality
    [[nodiscard]] bool isNearEqual(const Transform& other, f32 epsilon = F32_EPSILON * 100.0f) const noexcept {
        return position.isNearEqual(other.position, epsilon) &&
               rotation.isNearEqual(other.rotation, epsilon) &&
               scale.isNearEqual(other.scale, epsilon);
    }
};

// =============================================================================
// Free Functions
// =============================================================================

/// Linear interpolation
[[nodiscard]] inline Transform lerp(const Transform& a, const Transform& b, f32 t) noexcept {
    return a.lerp(b, t);
}

/// Spherical interpolation
[[nodiscard]] inline Transform slerp(const Transform& a, const Transform& b, f32 t) noexcept {
    return a.slerp(b, t);
}

/// Inverse
[[nodiscard]] inline Transform inverse(const Transform& t) noexcept {
    return t.inverse();
}

} // namespace nova::math

namespace nova {
    using Transform = math::Transform;
}
