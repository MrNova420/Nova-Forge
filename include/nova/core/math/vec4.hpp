// =============================================================================
// NovaCore Engine - 4D Vector
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// 4D vector type optimized for SIMD operations.
// Primary use: Homogeneous coordinates, quaternion representation, colors (RGBA).
// =============================================================================

#pragma once

#include "nova/core/math/math_common.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec3.hpp"

namespace nova::math {

/// @brief 4D vector with x, y, z, w components
/// @note Aligned to 16 bytes for optimal SIMD performance
struct alignas(16) Vec4 {
    f32 x, y, z, w;
    
    // ==========================================================================
    // Constructors
    // ==========================================================================
    
    /// Default constructor (zero vector)
    constexpr Vec4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    
    /// Scalar constructor (same value for all components)
    constexpr explicit Vec4(f32 scalar) noexcept : x(scalar), y(scalar), z(scalar), w(scalar) {}
    
    /// Component constructor
    constexpr Vec4(f32 x_, f32 y_, f32 z_, f32 w_) noexcept : x(x_), y(y_), z(z_), w(w_) {}
    
    /// Construct from Vec3 and w component
    constexpr Vec4(const Vec3& xyz, f32 w_) noexcept : x(xyz.x), y(xyz.y), z(xyz.z), w(w_) {}
    
    /// Construct from Vec2 and z, w components
    constexpr Vec4(const Vec2& xy, f32 z_, f32 w_) noexcept : x(xy.x), y(xy.y), z(z_), w(w_) {}
    
    /// Construct from two Vec2
    constexpr Vec4(const Vec2& xy, const Vec2& zw) noexcept : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}
    
    // ==========================================================================
    // Static Factory Methods
    // ==========================================================================
    
    /// Zero vector (0, 0, 0, 0)
    [[nodiscard]] static constexpr Vec4 zero() noexcept { return Vec4(0.0f, 0.0f, 0.0f, 0.0f); }
    
    /// One vector (1, 1, 1, 1)
    [[nodiscard]] static constexpr Vec4 one() noexcept { return Vec4(1.0f, 1.0f, 1.0f, 1.0f); }
    
    /// Unit X vector (1, 0, 0, 0)
    [[nodiscard]] static constexpr Vec4 unitX() noexcept { return Vec4(1.0f, 0.0f, 0.0f, 0.0f); }
    
    /// Unit Y vector (0, 1, 0, 0)
    [[nodiscard]] static constexpr Vec4 unitY() noexcept { return Vec4(0.0f, 1.0f, 0.0f, 0.0f); }
    
    /// Unit Z vector (0, 0, 1, 0)
    [[nodiscard]] static constexpr Vec4 unitZ() noexcept { return Vec4(0.0f, 0.0f, 1.0f, 0.0f); }
    
    /// Unit W vector (0, 0, 0, 1)
    [[nodiscard]] static constexpr Vec4 unitW() noexcept { return Vec4(0.0f, 0.0f, 0.0f, 1.0f); }
    
    // Color factory methods (RGBA)
    [[nodiscard]] static constexpr Vec4 white() noexcept { return Vec4(1.0f, 1.0f, 1.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec4 black() noexcept { return Vec4(0.0f, 0.0f, 0.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec4 red() noexcept { return Vec4(1.0f, 0.0f, 0.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec4 green() noexcept { return Vec4(0.0f, 1.0f, 0.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec4 blue() noexcept { return Vec4(0.0f, 0.0f, 1.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec4 yellow() noexcept { return Vec4(1.0f, 1.0f, 0.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec4 cyan() noexcept { return Vec4(0.0f, 1.0f, 1.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec4 magenta() noexcept { return Vec4(1.0f, 0.0f, 1.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec4 transparent() noexcept { return Vec4(0.0f, 0.0f, 0.0f, 0.0f); }
    
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
    
    /// Get ZW components as Vec2
    [[nodiscard]] constexpr Vec2 zw() const noexcept { return Vec2(z, w); }
    
    /// Get XYZ components as Vec3
    [[nodiscard]] constexpr Vec3 xyz() const noexcept { return Vec3(x, y, z); }
    
    /// Get RGB components as Vec3 (alias for xyz for color usage)
    [[nodiscard]] constexpr Vec3 rgb() const noexcept { return Vec3(x, y, z); }
    
    // Color component accessors (aliases)
    [[nodiscard]] constexpr f32 r() const noexcept { return x; }
    [[nodiscard]] constexpr f32 g() const noexcept { return y; }
    [[nodiscard]] constexpr f32 b() const noexcept { return z; }
    [[nodiscard]] constexpr f32 a() const noexcept { return w; }
    
    // ==========================================================================
    // Arithmetic Operators
    // ==========================================================================
    
    /// Unary negation
    [[nodiscard]] constexpr Vec4 operator-() const noexcept {
        return Vec4(-x, -y, -z, -w);
    }
    
    /// Vector addition
    [[nodiscard]] constexpr Vec4 operator+(const Vec4& other) const noexcept {
        return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }
    
    /// Vector subtraction
    [[nodiscard]] constexpr Vec4 operator-(const Vec4& other) const noexcept {
        return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }
    
    /// Component-wise multiplication
    [[nodiscard]] constexpr Vec4 operator*(const Vec4& other) const noexcept {
        return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
    }
    
    /// Component-wise division
    [[nodiscard]] constexpr Vec4 operator/(const Vec4& other) const noexcept {
        return Vec4(x / other.x, y / other.y, z / other.z, w / other.w);
    }
    
    /// Scalar multiplication
    [[nodiscard]] constexpr Vec4 operator*(f32 scalar) const noexcept {
        return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
    }
    
    /// Scalar division
    [[nodiscard]] constexpr Vec4 operator/(f32 scalar) const noexcept {
        f32 inv = 1.0f / scalar;
        return Vec4(x * inv, y * inv, z * inv, w * inv);
    }
    
    // Compound assignment operators
    constexpr Vec4& operator+=(const Vec4& other) noexcept {
        x += other.x; y += other.y; z += other.z; w += other.w;
        return *this;
    }
    
    constexpr Vec4& operator-=(const Vec4& other) noexcept {
        x -= other.x; y -= other.y; z -= other.z; w -= other.w;
        return *this;
    }
    
    constexpr Vec4& operator*=(const Vec4& other) noexcept {
        x *= other.x; y *= other.y; z *= other.z; w *= other.w;
        return *this;
    }
    
    constexpr Vec4& operator/=(const Vec4& other) noexcept {
        x /= other.x; y /= other.y; z /= other.z; w /= other.w;
        return *this;
    }
    
    constexpr Vec4& operator*=(f32 scalar) noexcept {
        x *= scalar; y *= scalar; z *= scalar; w *= scalar;
        return *this;
    }
    
    constexpr Vec4& operator/=(f32 scalar) noexcept {
        f32 inv = 1.0f / scalar;
        x *= inv; y *= inv; z *= inv; w *= inv;
        return *this;
    }
    
    // ==========================================================================
    // Comparison Operators
    // ==========================================================================
    
    [[nodiscard]] constexpr bool operator==(const Vec4& other) const noexcept {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }
    
    [[nodiscard]] constexpr bool operator!=(const Vec4& other) const noexcept {
        return !(*this == other);
    }
    
    // ==========================================================================
    // Vector Operations
    // ==========================================================================
    
    /// Dot product
    [[nodiscard]] constexpr f32 dot(const Vec4& other) const noexcept {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
    
    /// Squared length (faster than length)
    [[nodiscard]] constexpr f32 lengthSquared() const noexcept {
        return x * x + y * y + z * z + w * w;
    }
    
    /// Vector length
    [[nodiscard]] f32 length() const noexcept {
        return std::sqrt(lengthSquared());
    }
    
    /// Normalized vector (unit length)
    [[nodiscard]] Vec4 normalized() const noexcept {
        f32 len = length();
        if (len > 0.0f) {
            f32 invLen = 1.0f / len;
            return Vec4(x * invLen, y * invLen, z * invLen, w * invLen);
        }
        return Vec4::zero();
    }
    
    /// Normalize this vector in place
    Vec4& normalize() noexcept {
        *this = normalized();
        return *this;
    }
    
    /// Check if vector is approximately zero
    [[nodiscard]] bool isNearZero(f32 epsilon = F32_EPSILON * 100.0f) const noexcept {
        return lengthSquared() <= epsilon * epsilon;
    }
    
    /// Check if approximately equal to another vector
    [[nodiscard]] bool isNearEqual(const Vec4& other, f32 epsilon = F32_EPSILON * 100.0f) const noexcept {
        return nearEqual(x, other.x, epsilon) && 
               nearEqual(y, other.y, epsilon) && 
               nearEqual(z, other.z, epsilon) &&
               nearEqual(w, other.w, epsilon);
    }
    
    /// Component-wise min
    [[nodiscard]] constexpr Vec4 min(const Vec4& other) const noexcept {
        return Vec4(
            std::min(x, other.x), 
            std::min(y, other.y), 
            std::min(z, other.z), 
            std::min(w, other.w)
        );
    }
    
    /// Component-wise max
    [[nodiscard]] constexpr Vec4 max(const Vec4& other) const noexcept {
        return Vec4(
            std::max(x, other.x), 
            std::max(y, other.y), 
            std::max(z, other.z), 
            std::max(w, other.w)
        );
    }
    
    /// Component-wise absolute value
    [[nodiscard]] Vec4 abs() const noexcept {
        return Vec4(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
    }
    
    /// Component-wise floor
    [[nodiscard]] Vec4 floor() const noexcept {
        return Vec4(std::floor(x), std::floor(y), std::floor(z), std::floor(w));
    }
    
    /// Component-wise ceil
    [[nodiscard]] Vec4 ceil() const noexcept {
        return Vec4(std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w));
    }
    
    /// Component-wise round
    [[nodiscard]] Vec4 round() const noexcept {
        return Vec4(std::round(x), std::round(y), std::round(z), std::round(w));
    }
    
    /// Clamp components to range
    [[nodiscard]] constexpr Vec4 clamp(const Vec4& minVal, const Vec4& maxVal) const noexcept {
        return Vec4(
            math::clamp(x, minVal.x, maxVal.x),
            math::clamp(y, minVal.y, maxVal.y),
            math::clamp(z, minVal.z, maxVal.z),
            math::clamp(w, minVal.w, maxVal.w)
        );
    }
    
    /// Linear interpolation
    [[nodiscard]] constexpr Vec4 lerp(const Vec4& other, f32 t) const noexcept {
        return Vec4(
            math::lerp(x, other.x, t),
            math::lerp(y, other.y, t),
            math::lerp(z, other.z, t),
            math::lerp(w, other.w, t)
        );
    }
    
    // ==========================================================================
    // Homogeneous Coordinate Operations
    // ==========================================================================
    
    /// Perspective divide (divide xyz by w)
    [[nodiscard]] Vec3 perspectiveDivide() const noexcept {
        if (std::abs(w) > F32_EPSILON) {
            f32 invW = 1.0f / w;
            return Vec3(x * invW, y * invW, z * invW);
        }
        return Vec3(x, y, z);
    }
    
    /// Create point (w = 1)
    [[nodiscard]] static constexpr Vec4 point(const Vec3& p) noexcept {
        return Vec4(p, 1.0f);
    }
    
    /// Create direction (w = 0)
    [[nodiscard]] static constexpr Vec4 direction(const Vec3& d) noexcept {
        return Vec4(d, 0.0f);
    }
    
    // ==========================================================================
    // SIMD Optimized Operations
    // ==========================================================================
    
#if NOVA_SIMD_NEON
    /// SIMD optimized dot product (NEON)
    [[nodiscard]] f32 dotSimd(const Vec4& other) const noexcept {
        float32x4_t a = vld1q_f32(data());
        float32x4_t b = vld1q_f32(other.data());
        return simdDot4(a, b);
    }
    
    /// SIMD optimized addition (NEON)
    [[nodiscard]] Vec4 addSimd(const Vec4& other) const noexcept {
        float32x4_t a = vld1q_f32(data());
        float32x4_t b = vld1q_f32(other.data());
        Vec4 result;
        vst1q_f32(result.data(), vaddq_f32(a, b));
        return result;
    }
    
    /// SIMD optimized multiplication (NEON)
    [[nodiscard]] Vec4 mulSimd(const Vec4& other) const noexcept {
        float32x4_t a = vld1q_f32(data());
        float32x4_t b = vld1q_f32(other.data());
        Vec4 result;
        vst1q_f32(result.data(), vmulq_f32(a, b));
        return result;
    }
    
#elif NOVA_SIMD_AVX2
    /// SIMD optimized dot product (AVX)
    [[nodiscard]] f32 dotSimd(const Vec4& other) const noexcept {
        __m128 a = _mm_loadu_ps(data());
        __m128 b = _mm_loadu_ps(other.data());
        return simdDot4(a, b);
    }
    
    /// SIMD optimized addition (AVX)
    [[nodiscard]] Vec4 addSimd(const Vec4& other) const noexcept {
        __m128 a = _mm_loadu_ps(data());
        __m128 b = _mm_loadu_ps(other.data());
        Vec4 result;
        _mm_storeu_ps(result.data(), _mm_add_ps(a, b));
        return result;
    }
    
    /// SIMD optimized multiplication (AVX)
    [[nodiscard]] Vec4 mulSimd(const Vec4& other) const noexcept {
        __m128 a = _mm_loadu_ps(data());
        __m128 b = _mm_loadu_ps(other.data());
        Vec4 result;
        _mm_storeu_ps(result.data(), _mm_mul_ps(a, b));
        return result;
    }
    
#else
    /// Fallback operations (no SIMD)
    [[nodiscard]] f32 dotSimd(const Vec4& other) const noexcept { return dot(other); }
    [[nodiscard]] Vec4 addSimd(const Vec4& other) const noexcept { return *this + other; }
    [[nodiscard]] Vec4 mulSimd(const Vec4& other) const noexcept { return *this * other; }
#endif
};

// =============================================================================
// Free Functions
// =============================================================================

/// Scalar multiplication (scalar * vector)
[[nodiscard]] constexpr Vec4 operator*(f32 scalar, const Vec4& v) noexcept {
    return v * scalar;
}

/// Dot product free function
[[nodiscard]] constexpr f32 dot(const Vec4& a, const Vec4& b) noexcept {
    return a.dot(b);
}

/// Length free function
[[nodiscard]] inline f32 length(const Vec4& v) noexcept {
    return v.length();
}

/// Normalize free function
[[nodiscard]] inline Vec4 normalize(const Vec4& v) noexcept {
    return v.normalized();
}

/// Linear interpolation free function
[[nodiscard]] constexpr Vec4 lerp(const Vec4& a, const Vec4& b, f32 t) noexcept {
    return a.lerp(b, t);
}

/// Component-wise min free function
[[nodiscard]] constexpr Vec4 min(const Vec4& a, const Vec4& b) noexcept {
    return a.min(b);
}

/// Component-wise max free function
[[nodiscard]] constexpr Vec4 max(const Vec4& a, const Vec4& b) noexcept {
    return a.max(b);
}

} // namespace nova::math

// Bring common types into nova namespace
namespace nova {
    using Vec4 = math::Vec4;
}
