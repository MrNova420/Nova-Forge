// =============================================================================
// NovaCore Engine - 3D Vector
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// 3D vector type with SIMD optimizations where beneficial.
// Primary use: 3D positions, directions, colors (RGB), physics vectors.
// =============================================================================

#pragma once

#include "nova/core/math/math_common.hpp"
#include "nova/core/math/vec2.hpp"

namespace nova::math {

/// @brief 3D vector with x, y, z components
/// @note Aligned to 16 bytes for SIMD operations (w component is padding)
struct alignas(16) Vec3 {
    f32 x, y, z;
    f32 _pad; // Padding for 16-byte alignment, ignored in operations
    
    // ==========================================================================
    // Constructors
    // ==========================================================================
    
    /// Default constructor (zero vector)
    constexpr Vec3() noexcept : x(0.0f), y(0.0f), z(0.0f), _pad(0.0f) {}
    
    /// Scalar constructor (same value for all components)
    constexpr explicit Vec3(f32 scalar) noexcept : x(scalar), y(scalar), z(scalar), _pad(0.0f) {}
    
    /// Component constructor
    constexpr Vec3(f32 x_, f32 y_, f32 z_) noexcept : x(x_), y(y_), z(z_), _pad(0.0f) {}
    
    /// Construct from Vec2 and z component
    constexpr Vec3(const Vec2& xy, f32 z_) noexcept : x(xy.x), y(xy.y), z(z_), _pad(0.0f) {}
    
    // ==========================================================================
    // Static Factory Methods
    // ==========================================================================
    
    /// Zero vector (0, 0, 0)
    [[nodiscard]] static constexpr Vec3 zero() noexcept { return Vec3(0.0f, 0.0f, 0.0f); }
    
    /// One vector (1, 1, 1)
    [[nodiscard]] static constexpr Vec3 one() noexcept { return Vec3(1.0f, 1.0f, 1.0f); }
    
    /// Unit X vector (1, 0, 0)
    [[nodiscard]] static constexpr Vec3 unitX() noexcept { return Vec3(1.0f, 0.0f, 0.0f); }
    
    /// Unit Y vector (0, 1, 0)
    [[nodiscard]] static constexpr Vec3 unitY() noexcept { return Vec3(0.0f, 1.0f, 0.0f); }
    
    /// Unit Z vector (0, 0, 1)
    [[nodiscard]] static constexpr Vec3 unitZ() noexcept { return Vec3(0.0f, 0.0f, 1.0f); }
    
    /// Up direction (0, 1, 0) - Y-up convention
    [[nodiscard]] static constexpr Vec3 up() noexcept { return Vec3(0.0f, 1.0f, 0.0f); }
    
    /// Down direction (0, -1, 0)
    [[nodiscard]] static constexpr Vec3 down() noexcept { return Vec3(0.0f, -1.0f, 0.0f); }
    
    /// Forward direction (0, 0, -1) - Right-handed, -Z forward
    [[nodiscard]] static constexpr Vec3 forward() noexcept { return Vec3(0.0f, 0.0f, -1.0f); }
    
    /// Back direction (0, 0, 1)
    [[nodiscard]] static constexpr Vec3 back() noexcept { return Vec3(0.0f, 0.0f, 1.0f); }
    
    /// Left direction (-1, 0, 0)
    [[nodiscard]] static constexpr Vec3 left() noexcept { return Vec3(-1.0f, 0.0f, 0.0f); }
    
    /// Right direction (1, 0, 0)
    [[nodiscard]] static constexpr Vec3 right() noexcept { return Vec3(1.0f, 0.0f, 0.0f); }
    
    // ==========================================================================
    // Accessors
    // ==========================================================================
    
    /// Array access operator
    [[nodiscard]] constexpr f32& operator[](usize index) noexcept {
        return (&x)[index];
    }
    
    /// Const array access operator
    [[nodiscard]] constexpr const f32& operator[](usize index) const noexcept {
        return (&x)[index];
    }
    
    /// Get as pointer to float array
    [[nodiscard]] constexpr f32* data() noexcept { return &x; }
    [[nodiscard]] constexpr const f32* data() const noexcept { return &x; }
    
    /// Get XY components as Vec2
    [[nodiscard]] constexpr Vec2 xy() const noexcept { return Vec2(x, y); }
    
    /// Get XZ components as Vec2
    [[nodiscard]] constexpr Vec2 xz() const noexcept { return Vec2(x, z); }
    
    /// Get YZ components as Vec2
    [[nodiscard]] constexpr Vec2 yz() const noexcept { return Vec2(y, z); }
    
    // ==========================================================================
    // Arithmetic Operators
    // ==========================================================================
    
    /// Unary negation
    [[nodiscard]] constexpr Vec3 operator-() const noexcept {
        return Vec3(-x, -y, -z);
    }
    
    /// Vector addition
    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const noexcept {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    /// Vector subtraction
    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const noexcept {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    /// Component-wise multiplication
    [[nodiscard]] constexpr Vec3 operator*(const Vec3& other) const noexcept {
        return Vec3(x * other.x, y * other.y, z * other.z);
    }
    
    /// Component-wise division
    [[nodiscard]] constexpr Vec3 operator/(const Vec3& other) const noexcept {
        return Vec3(x / other.x, y / other.y, z / other.z);
    }
    
    /// Scalar multiplication
    [[nodiscard]] constexpr Vec3 operator*(f32 scalar) const noexcept {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    
    /// Scalar division
    [[nodiscard]] constexpr Vec3 operator/(f32 scalar) const noexcept {
        f32 inv = 1.0f / scalar;
        return Vec3(x * inv, y * inv, z * inv);
    }
    
    // Compound assignment operators
    constexpr Vec3& operator+=(const Vec3& other) noexcept {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }
    
    constexpr Vec3& operator-=(const Vec3& other) noexcept {
        x -= other.x; y -= other.y; z -= other.z;
        return *this;
    }
    
    constexpr Vec3& operator*=(const Vec3& other) noexcept {
        x *= other.x; y *= other.y; z *= other.z;
        return *this;
    }
    
    constexpr Vec3& operator/=(const Vec3& other) noexcept {
        x /= other.x; y /= other.y; z /= other.z;
        return *this;
    }
    
    constexpr Vec3& operator*=(f32 scalar) noexcept {
        x *= scalar; y *= scalar; z *= scalar;
        return *this;
    }
    
    constexpr Vec3& operator/=(f32 scalar) noexcept {
        f32 inv = 1.0f / scalar;
        x *= inv; y *= inv; z *= inv;
        return *this;
    }
    
    // ==========================================================================
    // Comparison Operators
    // ==========================================================================
    
    [[nodiscard]] constexpr bool operator==(const Vec3& other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }
    
    [[nodiscard]] constexpr bool operator!=(const Vec3& other) const noexcept {
        return !(*this == other);
    }
    
    // ==========================================================================
    // Vector Operations
    // ==========================================================================
    
    /// Dot product
    [[nodiscard]] constexpr f32 dot(const Vec3& other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }
    
    /// Cross product
    [[nodiscard]] constexpr Vec3 cross(const Vec3& other) const noexcept {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    
    /// Squared length (faster than length)
    [[nodiscard]] constexpr f32 lengthSquared() const noexcept {
        return x * x + y * y + z * z;
    }
    
    /// Vector length
    [[nodiscard]] f32 length() const noexcept {
        return std::sqrt(lengthSquared());
    }
    
    /// Normalized vector (unit length)
    [[nodiscard]] Vec3 normalized() const noexcept {
        f32 len = length();
        if (len > 0.0f) {
            f32 invLen = 1.0f / len;
            return Vec3(x * invLen, y * invLen, z * invLen);
        }
        return Vec3::zero();
    }
    
    /// Normalize this vector in place
    Vec3& normalize() noexcept {
        *this = normalized();
        return *this;
    }
    
    /// Check if vector is approximately zero
    [[nodiscard]] bool isNearZero(f32 epsilon = limits::F32_EPSILON * 100.0f) const noexcept {
        return lengthSquared() <= epsilon * epsilon;
    }
    
    /// Check if approximately equal to another vector
    [[nodiscard]] bool isNearEqual(const Vec3& other, f32 epsilon = limits::F32_EPSILON * 100.0f) const noexcept {
        return nearEqual(x, other.x, epsilon) && 
               nearEqual(y, other.y, epsilon) && 
               nearEqual(z, other.z, epsilon);
    }
    
    /// Distance to another point
    [[nodiscard]] f32 distanceTo(const Vec3& other) const noexcept {
        return (*this - other).length();
    }
    
    /// Squared distance to another point (faster)
    [[nodiscard]] constexpr f32 distanceSquaredTo(const Vec3& other) const noexcept {
        return (*this - other).lengthSquared();
    }
    
    /// Reflect vector around a normal
    [[nodiscard]] constexpr Vec3 reflect(const Vec3& normal) const noexcept {
        return *this - normal * (2.0f * dot(normal));
    }
    
    /// Refract vector through a surface
    [[nodiscard]] Vec3 refract(const Vec3& normal, f32 eta) const noexcept {
        f32 cosI = -dot(normal);
        f32 sin2T = eta * eta * (1.0f - cosI * cosI);
        if (sin2T > 1.0f) {
            return Vec3::zero(); // Total internal reflection
        }
        f32 cosT = std::sqrt(1.0f - sin2T);
        return *this * eta + normal * (eta * cosI - cosT);
    }
    
    /// Project this vector onto another vector
    [[nodiscard]] Vec3 projectOnto(const Vec3& other) const noexcept {
        f32 d = other.dot(other);
        if (d > 0.0f) {
            return other * (dot(other) / d);
        }
        return Vec3::zero();
    }
    
    /// Project this vector onto a plane defined by its normal
    [[nodiscard]] Vec3 projectOntoPlane(const Vec3& planeNormal) const noexcept {
        return *this - projectOnto(planeNormal);
    }
    
    /// Angle between this vector and another (in radians)
    [[nodiscard]] f32 angleTo(const Vec3& other) const noexcept {
        f32 d = dot(other);
        f32 l = length() * other.length();
        if (l > 0.0f) {
            return std::acos(nova::math::clamp(d / l, -1.0f, 1.0f));
        }
        return 0.0f;
    }
    
    /// Component-wise min
    [[nodiscard]] constexpr Vec3 min(const Vec3& other) const noexcept {
        return Vec3(std::min(x, other.x), std::min(y, other.y), std::min(z, other.z));
    }
    
    /// Component-wise max
    [[nodiscard]] constexpr Vec3 max(const Vec3& other) const noexcept {
        return Vec3(std::max(x, other.x), std::max(y, other.y), std::max(z, other.z));
    }
    
    /// Component-wise absolute value
    [[nodiscard]] Vec3 abs() const noexcept {
        return Vec3(std::abs(x), std::abs(y), std::abs(z));
    }
    
    /// Component-wise floor
    [[nodiscard]] Vec3 floor() const noexcept {
        return Vec3(std::floor(x), std::floor(y), std::floor(z));
    }
    
    /// Component-wise ceil
    [[nodiscard]] Vec3 ceil() const noexcept {
        return Vec3(std::ceil(x), std::ceil(y), std::ceil(z));
    }
    
    /// Component-wise round
    [[nodiscard]] Vec3 round() const noexcept {
        return Vec3(std::round(x), std::round(y), std::round(z));
    }
    
    /// Clamp components to range
    [[nodiscard]] constexpr Vec3 clamp(const Vec3& minVal, const Vec3& maxVal) const noexcept {
        return Vec3(
            math::clamp(x, minVal.x, maxVal.x),
            math::clamp(y, minVal.y, maxVal.y),
            math::clamp(z, minVal.z, maxVal.z)
        );
    }
    
    /// Linear interpolation
    [[nodiscard]] constexpr Vec3 lerp(const Vec3& other, f32 t) const noexcept {
        return Vec3(
            math::lerp(x, other.x, t),
            math::lerp(y, other.y, t),
            math::lerp(z, other.z, t)
        );
    }
    
    /// Spherical linear interpolation (for unit vectors)
    [[nodiscard]] Vec3 slerp(const Vec3& other, f32 t) const noexcept {
        f32 d = dot(other);
        d = math::clamp(d, -1.0f, 1.0f);
        f32 theta = std::acos(d) * t;
        Vec3 relative = (other - *this * d).normalized();
        return *this * std::cos(theta) + relative * std::sin(theta);
    }
    
    // ==========================================================================
    // SIMD Optimized Operations
    // ==========================================================================
    
#if NOVA_SIMD_NEON
    /// SIMD optimized dot product (NEON)
    [[nodiscard]] f32 dotSimd(const Vec3& other) const noexcept {
        float32x4_t a = vld1q_f32(data());
        float32x4_t b = vld1q_f32(other.data());
        float32x4_t prod = vmulq_f32(a, b);
        float32x2_t sum1 = vadd_f32(vget_low_f32(prod), vget_high_f32(prod));
        float32x2_t sum2 = vpadd_f32(sum1, sum1);
        return vget_lane_f32(sum2, 0);
    }
#elif NOVA_SIMD_AVX2
    /// SIMD optimized dot product (AVX)
    [[nodiscard]] f32 dotSimd(const Vec3& other) const noexcept {
        __m128 a = _mm_loadu_ps(data());
        __m128 b = _mm_loadu_ps(other.data());
        __m128 prod = _mm_mul_ps(a, b);
        __m128 shuf = _mm_movehdup_ps(prod);
        __m128 sums = _mm_add_ps(prod, shuf);
        shuf = _mm_movehl_ps(shuf, sums);
        sums = _mm_add_ss(sums, shuf);
        return _mm_cvtss_f32(sums);
    }
#else
    /// Fallback dot product (no SIMD)
    [[nodiscard]] f32 dotSimd(const Vec3& other) const noexcept {
        return dot(other);
    }
#endif
};

// =============================================================================
// Free Functions
// =============================================================================

/// Scalar multiplication (scalar * vector)
[[nodiscard]] constexpr Vec3 operator*(f32 scalar, const Vec3& v) noexcept {
    return v * scalar;
}

/// Dot product free function
[[nodiscard]] constexpr f32 dot(const Vec3& a, const Vec3& b) noexcept {
    return a.dot(b);
}

/// Cross product free function
[[nodiscard]] constexpr Vec3 cross(const Vec3& a, const Vec3& b) noexcept {
    return a.cross(b);
}

/// Length free function
[[nodiscard]] inline f32 length(const Vec3& v) noexcept {
    return v.length();
}

/// Normalize free function
[[nodiscard]] inline Vec3 normalize(const Vec3& v) noexcept {
    return v.normalized();
}

/// Distance free function
[[nodiscard]] inline f32 distance(const Vec3& a, const Vec3& b) noexcept {
    return a.distanceTo(b);
}

/// Linear interpolation free function
[[nodiscard]] constexpr Vec3 lerp(const Vec3& a, const Vec3& b, f32 t) noexcept {
    return a.lerp(b, t);
}

/// Spherical linear interpolation free function
[[nodiscard]] inline Vec3 slerp(const Vec3& a, const Vec3& b, f32 t) noexcept {
    return a.slerp(b, t);
}

/// Component-wise min free function
[[nodiscard]] constexpr Vec3 min(const Vec3& a, const Vec3& b) noexcept {
    return a.min(b);
}

/// Component-wise max free function
[[nodiscard]] constexpr Vec3 max(const Vec3& a, const Vec3& b) noexcept {
    return a.max(b);
}

/// Reflect free function
[[nodiscard]] constexpr Vec3 reflect(const Vec3& v, const Vec3& normal) noexcept {
    return v.reflect(normal);
}

/// Refract free function
[[nodiscard]] inline Vec3 refract(const Vec3& v, const Vec3& normal, f32 eta) noexcept {
    return v.refract(normal, eta);
}

/// Triple scalar product (a · (b × c))
[[nodiscard]] constexpr f32 tripleProduct(const Vec3& a, const Vec3& b, const Vec3& c) noexcept {
    return a.dot(b.cross(c));
}

} // namespace nova::math

// Bring common types into nova namespace
namespace nova {
    using Vec3 = math::Vec3;
}
