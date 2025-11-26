// =============================================================================
// NovaCore Engine - Quaternion
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Unit quaternion for 3D rotations. More compact than matrices, avoids gimbal
// lock, and supports smooth interpolation (slerp).
// =============================================================================

#pragma once

#include "nova/core/math/math_common.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/math/mat3.hpp"
#include "nova/core/math/mat4.hpp"

namespace nova::math {

/// @brief Unit quaternion for 3D rotations (w, x, y, z format)
/// @note Stored as [x, y, z, w] internally for SIMD alignment with Vec4
struct alignas(16) Quat {
    f32 x, y, z, w;
    
    // ==========================================================================
    // Constructors
    // ==========================================================================
    
    /// Default constructor (identity quaternion - no rotation)
    constexpr Quat() noexcept : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    
    /// Component constructor (x, y, z, w)
    constexpr Quat(f32 x_, f32 y_, f32 z_, f32 w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}
    
    /// Construct from Vec4 (x, y, z, w)
    constexpr explicit Quat(const Vec4& v) noexcept : x(v.x), y(v.y), z(v.z), w(v.w) {}
    
    // ==========================================================================
    // Static Factory Methods
    // ==========================================================================
    
    /// Identity quaternion (no rotation)
    [[nodiscard]] static constexpr Quat identity() noexcept { return Quat(); }
    
    /// Create from axis-angle representation
    [[nodiscard]] static Quat fromAxisAngle(const Vec3& axis, f32 radians) noexcept {
        f32 halfAngle = radians * 0.5f;
        f32 s = std::sin(halfAngle);
        Vec3 n = axis.normalized();
        return Quat(n.x * s, n.y * s, n.z * s, std::cos(halfAngle));
    }
    
    /// Create from Euler angles (pitch, yaw, roll in radians)
    /// Order: YXZ (yaw first, then pitch, then roll)
    [[nodiscard]] static Quat fromEuler(f32 pitch, f32 yaw, f32 roll) noexcept {
        f32 cy = std::cos(yaw * 0.5f);
        f32 sy = std::sin(yaw * 0.5f);
        f32 cp = std::cos(pitch * 0.5f);
        f32 sp = std::sin(pitch * 0.5f);
        f32 cr = std::cos(roll * 0.5f);
        f32 sr = std::sin(roll * 0.5f);
        
        return Quat(
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        );
    }
    
    /// Create from Euler angles vector (x=pitch, y=yaw, z=roll)
    [[nodiscard]] static Quat fromEuler(const Vec3& euler) noexcept {
        return fromEuler(euler.x, euler.y, euler.z);
    }
    
    /// Create quaternion that rotates from one direction to another
    [[nodiscard]] static Quat fromToRotation(const Vec3& from, const Vec3& to) noexcept {
        Vec3 f = from.normalized();
        Vec3 t = to.normalized();
        
        f32 d = f.dot(t);
        
        if (d >= 1.0f - F32_EPSILON) {
            return Quat::identity();
        }
        
        if (d <= -1.0f + F32_EPSILON) {
            // 180 degree rotation - find perpendicular axis
            Vec3 axis = Vec3::unitX().cross(f);
            if (axis.lengthSquared() < F32_EPSILON) {
                axis = Vec3::unitY().cross(f);
            }
            return fromAxisAngle(axis.normalized(), PI_F32);
        }
        
        Vec3 axis = f.cross(t);
        f32 w = std::sqrt((1.0f + d) * 2.0f);
        f32 invW = 1.0f / w;
        
        return Quat(axis.x * invW, axis.y * invW, axis.z * invW, w * 0.5f);
    }
    
    /// Create quaternion that looks in a direction
    [[nodiscard]] static Quat lookRotation(const Vec3& forward, const Vec3& up = Vec3::up()) noexcept {
        Vec3 f = forward.normalized();
        Vec3 r = up.cross(f).normalized();
        Vec3 u = f.cross(r);
        
        // Build rotation matrix columns and convert to quaternion
        f32 m00 = r.x, m01 = u.x, m02 = f.x;
        f32 m10 = r.y, m11 = u.y, m12 = f.y;
        f32 m20 = r.z, m21 = u.z, m22 = f.z;
        
        f32 trace = m00 + m11 + m22;
        Quat q;
        
        if (trace > 0.0f) {
            f32 s = std::sqrt(trace + 1.0f) * 2.0f;
            q.w = 0.25f * s;
            q.x = (m21 - m12) / s;
            q.y = (m02 - m20) / s;
            q.z = (m10 - m01) / s;
        } else if (m00 > m11 && m00 > m22) {
            f32 s = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;
            q.w = (m21 - m12) / s;
            q.x = 0.25f * s;
            q.y = (m01 + m10) / s;
            q.z = (m02 + m20) / s;
        } else if (m11 > m22) {
            f32 s = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;
            q.w = (m02 - m20) / s;
            q.x = (m01 + m10) / s;
            q.y = 0.25f * s;
            q.z = (m12 + m21) / s;
        } else {
            f32 s = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;
            q.w = (m10 - m01) / s;
            q.x = (m02 + m20) / s;
            q.y = (m12 + m21) / s;
            q.z = 0.25f * s;
        }
        
        return q.normalized();
    }
    
    /// Create from rotation matrix
    [[nodiscard]] static Quat fromMatrix(const Mat3& m) noexcept {
        f32 trace = m.columns[0].x + m.columns[1].y + m.columns[2].z;
        Quat q;
        
        if (trace > 0.0f) {
            f32 s = std::sqrt(trace + 1.0f) * 2.0f;
            q.w = 0.25f * s;
            q.x = (m.columns[1].z - m.columns[2].y) / s;
            q.y = (m.columns[2].x - m.columns[0].z) / s;
            q.z = (m.columns[0].y - m.columns[1].x) / s;
        } else if (m.columns[0].x > m.columns[1].y && m.columns[0].x > m.columns[2].z) {
            f32 s = std::sqrt(1.0f + m.columns[0].x - m.columns[1].y - m.columns[2].z) * 2.0f;
            q.w = (m.columns[1].z - m.columns[2].y) / s;
            q.x = 0.25f * s;
            q.y = (m.columns[1].x + m.columns[0].y) / s;
            q.z = (m.columns[2].x + m.columns[0].z) / s;
        } else if (m.columns[1].y > m.columns[2].z) {
            f32 s = std::sqrt(1.0f + m.columns[1].y - m.columns[0].x - m.columns[2].z) * 2.0f;
            q.w = (m.columns[2].x - m.columns[0].z) / s;
            q.x = (m.columns[1].x + m.columns[0].y) / s;
            q.y = 0.25f * s;
            q.z = (m.columns[2].y + m.columns[1].z) / s;
        } else {
            f32 s = std::sqrt(1.0f + m.columns[2].z - m.columns[0].x - m.columns[1].y) * 2.0f;
            q.w = (m.columns[0].y - m.columns[1].x) / s;
            q.x = (m.columns[2].x + m.columns[0].z) / s;
            q.y = (m.columns[2].y + m.columns[1].z) / s;
            q.z = 0.25f * s;
        }
        
        return q.normalized();
    }
    
    // ==========================================================================
    // Accessors
    // ==========================================================================
    
    /// Get as Vec4
    [[nodiscard]] constexpr Vec4 toVec4() const noexcept {
        return Vec4(x, y, z, w);
    }
    
    /// Get as pointer to float array
    [[nodiscard]] constexpr f32* data() noexcept { return &x; }
    [[nodiscard]] constexpr const f32* data() const noexcept { return &x; }
    
    // ==========================================================================
    // Quaternion Operations
    // ==========================================================================
    
    /// Quaternion multiplication (rotation composition)
    [[nodiscard]] constexpr Quat operator*(const Quat& other) const noexcept {
        return Quat(
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w,
            w * other.w - x * other.x - y * other.y - z * other.z
        );
    }
    
    /// Rotate a vector by this quaternion
    [[nodiscard]] Vec3 operator*(const Vec3& v) const noexcept {
        // Optimized quaternion-vector multiplication
        Vec3 qv(x, y, z);
        Vec3 t = qv.cross(v) * 2.0f;
        return v + t * w + qv.cross(t);
    }
    
    /// Scalar multiplication
    [[nodiscard]] constexpr Quat operator*(f32 scalar) const noexcept {
        return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
    }
    
    /// Quaternion addition
    [[nodiscard]] constexpr Quat operator+(const Quat& other) const noexcept {
        return Quat(x + other.x, y + other.y, z + other.z, w + other.w);
    }
    
    /// Quaternion subtraction
    [[nodiscard]] constexpr Quat operator-(const Quat& other) const noexcept {
        return Quat(x - other.x, y - other.y, z - other.z, w - other.w);
    }
    
    /// Negation
    [[nodiscard]] constexpr Quat operator-() const noexcept {
        return Quat(-x, -y, -z, -w);
    }
    
    // Compound assignment
    constexpr Quat& operator*=(const Quat& other) noexcept {
        *this = *this * other;
        return *this;
    }
    
    /// Dot product
    [[nodiscard]] constexpr f32 dot(const Quat& other) const noexcept {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
    
    /// Squared length
    [[nodiscard]] constexpr f32 lengthSquared() const noexcept {
        return x * x + y * y + z * z + w * w;
    }
    
    /// Length
    [[nodiscard]] f32 length() const noexcept {
        return std::sqrt(lengthSquared());
    }
    
    /// Normalized quaternion (unit quaternion)
    [[nodiscard]] Quat normalized() const noexcept {
        f32 len = length();
        if (len > F32_EPSILON) {
            f32 invLen = 1.0f / len;
            return Quat(x * invLen, y * invLen, z * invLen, w * invLen);
        }
        return Quat::identity();
    }
    
    /// Normalize in place
    Quat& normalize() noexcept {
        *this = normalized();
        return *this;
    }
    
    /// Conjugate (inverse for unit quaternions)
    [[nodiscard]] constexpr Quat conjugate() const noexcept {
        return Quat(-x, -y, -z, w);
    }
    
    /// Inverse
    [[nodiscard]] Quat inverse() const noexcept {
        f32 lenSq = lengthSquared();
        if (lenSq > F32_EPSILON) {
            f32 invLenSq = 1.0f / lenSq;
            return Quat(-x * invLenSq, -y * invLenSq, -z * invLenSq, w * invLenSq);
        }
        return Quat::identity();
    }
    
    // ==========================================================================
    // Conversion Methods
    // ==========================================================================
    
    /// Convert to 3x3 rotation matrix
    [[nodiscard]] Mat3 toMat3() const noexcept {
        f32 xx = x * x, yy = y * y, zz = z * z;
        f32 xy = x * y, xz = x * z, yz = y * z;
        f32 wx = w * x, wy = w * y, wz = w * z;
        
        return Mat3(
            Vec3(1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy)),
            Vec3(2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx)),
            Vec3(2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy))
        );
    }
    
    /// Convert to 4x4 rotation matrix
    [[nodiscard]] Mat4 toMat4() const noexcept {
        return Mat4(toMat3());
    }
    
    /// Convert to Euler angles (pitch, yaw, roll) in radians
    [[nodiscard]] Vec3 toEuler() const noexcept {
        Vec3 euler;
        
        // Roll (x-axis rotation)
        f32 sinr = 2.0f * (w * x + y * z);
        f32 cosr = 1.0f - 2.0f * (x * x + y * y);
        euler.z = std::atan2(sinr, cosr);
        
        // Pitch (y-axis rotation)
        f32 sinp = 2.0f * (w * y - z * x);
        if (std::abs(sinp) >= 1.0f) {
            euler.x = std::copysign(PI_F32 / 2.0f, sinp);
        } else {
            euler.x = std::asin(sinp);
        }
        
        // Yaw (z-axis rotation)
        f32 siny = 2.0f * (w * z + x * y);
        f32 cosy = 1.0f - 2.0f * (y * y + z * z);
        euler.y = std::atan2(siny, cosy);
        
        return euler;
    }
    
    /// Get axis of rotation
    [[nodiscard]] Vec3 axis() const noexcept {
        f32 sinHalfAngle = std::sqrt(1.0f - w * w);
        if (sinHalfAngle < F32_EPSILON) {
            return Vec3::unitX();
        }
        return Vec3(x, y, z) / sinHalfAngle;
    }
    
    /// Get angle of rotation in radians
    [[nodiscard]] f32 angle() const noexcept {
        return 2.0f * std::acos(clamp(w, -1.0f, 1.0f));
    }
    
    // ==========================================================================
    // Direction Accessors
    // ==========================================================================
    
    /// Get forward direction (-Z in right-handed coordinates)
    [[nodiscard]] Vec3 forward() const noexcept {
        return *this * Vec3::forward();
    }
    
    /// Get right direction (+X)
    [[nodiscard]] Vec3 right() const noexcept {
        return *this * Vec3::right();
    }
    
    /// Get up direction (+Y)
    [[nodiscard]] Vec3 up() const noexcept {
        return *this * Vec3::up();
    }
    
    // ==========================================================================
    // Interpolation
    // ==========================================================================
    
    /// Linear interpolation (not normalized - faster but may not be unit length)
    [[nodiscard]] constexpr Quat lerp(const Quat& other, f32 t) const noexcept {
        return Quat(
            x + (other.x - x) * t,
            y + (other.y - y) * t,
            z + (other.z - z) * t,
            w + (other.w - w) * t
        );
    }
    
    /// Normalized linear interpolation (nlerp - usually good enough)
    [[nodiscard]] Quat nlerp(const Quat& other, f32 t) const noexcept {
        Quat target = other;
        // Ensure we take the short path
        if (dot(target) < 0.0f) {
            target = -target;
        }
        return lerp(target, t).normalized();
    }
    
    /// Spherical linear interpolation (slerp - constant angular velocity)
    [[nodiscard]] Quat slerp(const Quat& other, f32 t) const noexcept {
        Quat target = other;
        f32 d = dot(target);
        
        // Ensure we take the short path
        if (d < 0.0f) {
            target = -target;
            d = -d;
        }
        
        // If quaternions are very close, use nlerp to avoid division issues
        if (d > 0.9995f) {
            return nlerp(target, t);
        }
        
        f32 theta = std::acos(d);
        f32 sinTheta = std::sin(theta);
        f32 t0 = std::sin((1.0f - t) * theta) / sinTheta;
        f32 t1 = std::sin(t * theta) / sinTheta;
        
        return Quat(
            x * t0 + target.x * t1,
            y * t0 + target.y * t1,
            z * t0 + target.z * t1,
            w * t0 + target.w * t1
        );
    }
    
    // ==========================================================================
    // Comparison
    // ==========================================================================
    
    [[nodiscard]] constexpr bool operator==(const Quat& other) const noexcept {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }
    
    [[nodiscard]] constexpr bool operator!=(const Quat& other) const noexcept {
        return !(*this == other);
    }
    
    /// Check if approximately equal (accounts for quaternion double-cover)
    [[nodiscard]] bool isNearEqual(const Quat& other, f32 epsilon = F32_EPSILON * 100.0f) const noexcept {
        // Quaternions q and -q represent the same rotation
        f32 d = std::abs(dot(other));
        return d > 1.0f - epsilon;
    }
};

// =============================================================================
// Free Functions
// =============================================================================

/// Scalar multiplication
[[nodiscard]] constexpr Quat operator*(f32 scalar, const Quat& q) noexcept {
    return q * scalar;
}

/// Dot product
[[nodiscard]] constexpr f32 dot(const Quat& a, const Quat& b) noexcept {
    return a.dot(b);
}

/// Normalize
[[nodiscard]] inline Quat normalize(const Quat& q) noexcept {
    return q.normalized();
}

/// Conjugate
[[nodiscard]] constexpr Quat conjugate(const Quat& q) noexcept {
    return q.conjugate();
}

/// Inverse
[[nodiscard]] inline Quat inverse(const Quat& q) noexcept {
    return q.inverse();
}

/// Spherical linear interpolation
[[nodiscard]] inline Quat slerp(const Quat& a, const Quat& b, f32 t) noexcept {
    return a.slerp(b, t);
}

/// Normalized linear interpolation
[[nodiscard]] inline Quat nlerp(const Quat& a, const Quat& b, f32 t) noexcept {
    return a.nlerp(b, t);
}

} // namespace nova::math

namespace nova {
    using Quat = math::Quat;
}
