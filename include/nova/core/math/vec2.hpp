// =============================================================================
// NovaCore Engine - 2D Vector
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// 2D vector type for positions, directions, and texture coordinates.
// =============================================================================

#pragma once

#include "nova/core/math/math_common.hpp"

namespace nova::math {

/// @brief 2D vector with x and y components
struct Vec2 {
    f32 x, y;
    
    // ==========================================================================
    // Constructors
    // ==========================================================================
    
    /// Default constructor (zero vector)
    constexpr Vec2() noexcept : x(0.0f), y(0.0f) {}
    
    /// Scalar constructor (same value for both components)
    constexpr explicit Vec2(f32 scalar) noexcept : x(scalar), y(scalar) {}
    
    /// Component constructor
    constexpr Vec2(f32 x_, f32 y_) noexcept : x(x_), y(y_) {}
    
    // ==========================================================================
    // Static Factory Methods
    // ==========================================================================
    
    /// Zero vector (0, 0)
    [[nodiscard]] static constexpr Vec2 zero() noexcept { return Vec2(0.0f, 0.0f); }
    
    /// One vector (1, 1)
    [[nodiscard]] static constexpr Vec2 one() noexcept { return Vec2(1.0f, 1.0f); }
    
    /// Unit X vector (1, 0)
    [[nodiscard]] static constexpr Vec2 unitX() noexcept { return Vec2(1.0f, 0.0f); }
    
    /// Unit Y vector (0, 1)
    [[nodiscard]] static constexpr Vec2 unitY() noexcept { return Vec2(0.0f, 1.0f); }
    
    /// Up direction (0, 1) in screen coordinates
    [[nodiscard]] static constexpr Vec2 up() noexcept { return Vec2(0.0f, 1.0f); }
    
    /// Down direction (0, -1)
    [[nodiscard]] static constexpr Vec2 down() noexcept { return Vec2(0.0f, -1.0f); }
    
    /// Left direction (-1, 0)
    [[nodiscard]] static constexpr Vec2 left() noexcept { return Vec2(-1.0f, 0.0f); }
    
    /// Right direction (1, 0)
    [[nodiscard]] static constexpr Vec2 right() noexcept { return Vec2(1.0f, 0.0f); }
    
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
    
    // ==========================================================================
    // Arithmetic Operators
    // ==========================================================================
    
    /// Unary negation
    [[nodiscard]] constexpr Vec2 operator-() const noexcept {
        return Vec2(-x, -y);
    }
    
    /// Vector addition
    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const noexcept {
        return Vec2(x + other.x, y + other.y);
    }
    
    /// Vector subtraction
    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const noexcept {
        return Vec2(x - other.x, y - other.y);
    }
    
    /// Component-wise multiplication
    [[nodiscard]] constexpr Vec2 operator*(const Vec2& other) const noexcept {
        return Vec2(x * other.x, y * other.y);
    }
    
    /// Component-wise division
    [[nodiscard]] constexpr Vec2 operator/(const Vec2& other) const noexcept {
        return Vec2(x / other.x, y / other.y);
    }
    
    /// Scalar multiplication
    [[nodiscard]] constexpr Vec2 operator*(f32 scalar) const noexcept {
        return Vec2(x * scalar, y * scalar);
    }
    
    /// Scalar division
    [[nodiscard]] constexpr Vec2 operator/(f32 scalar) const noexcept {
        f32 inv = 1.0f / scalar;
        return Vec2(x * inv, y * inv);
    }
    
    // Compound assignment operators
    constexpr Vec2& operator+=(const Vec2& other) noexcept {
        x += other.x; y += other.y;
        return *this;
    }
    
    constexpr Vec2& operator-=(const Vec2& other) noexcept {
        x -= other.x; y -= other.y;
        return *this;
    }
    
    constexpr Vec2& operator*=(const Vec2& other) noexcept {
        x *= other.x; y *= other.y;
        return *this;
    }
    
    constexpr Vec2& operator/=(const Vec2& other) noexcept {
        x /= other.x; y /= other.y;
        return *this;
    }
    
    constexpr Vec2& operator*=(f32 scalar) noexcept {
        x *= scalar; y *= scalar;
        return *this;
    }
    
    constexpr Vec2& operator/=(f32 scalar) noexcept {
        f32 inv = 1.0f / scalar;
        x *= inv; y *= inv;
        return *this;
    }
    
    // ==========================================================================
    // Comparison Operators
    // ==========================================================================
    
    [[nodiscard]] constexpr bool operator==(const Vec2& other) const noexcept {
        return x == other.x && y == other.y;
    }
    
    [[nodiscard]] constexpr bool operator!=(const Vec2& other) const noexcept {
        return !(*this == other);
    }
    
    // ==========================================================================
    // Vector Operations
    // ==========================================================================
    
    /// Dot product
    [[nodiscard]] constexpr f32 dot(const Vec2& other) const noexcept {
        return x * other.x + y * other.y;
    }
    
    /// Cross product (returns scalar - z component of 3D cross product)
    [[nodiscard]] constexpr f32 cross(const Vec2& other) const noexcept {
        return x * other.y - y * other.x;
    }
    
    /// Squared length (faster than length)
    [[nodiscard]] constexpr f32 lengthSquared() const noexcept {
        return x * x + y * y;
    }
    
    /// Vector length
    [[nodiscard]] f32 length() const noexcept {
        return std::sqrt(lengthSquared());
    }
    
    /// Normalized vector (unit length)
    [[nodiscard]] Vec2 normalized() const noexcept {
        f32 len = length();
        if (len > 0.0f) {
            f32 invLen = 1.0f / len;
            return Vec2(x * invLen, y * invLen);
        }
        return Vec2::zero();
    }
    
    /// Normalize this vector in place
    Vec2& normalize() noexcept {
        *this = normalized();
        return *this;
    }
    
    /// Check if vector is approximately zero
    [[nodiscard]] bool isNearZero(f32 epsilon = limits::F32_EPSILON * 100.0f) const noexcept {
        return lengthSquared() <= epsilon * epsilon;
    }
    
    /// Check if approximately equal to another vector
    [[nodiscard]] bool isNearEqual(const Vec2& other, f32 epsilon = limits::F32_EPSILON * 100.0f) const noexcept {
        return nearEqual(x, other.x, epsilon) && nearEqual(y, other.y, epsilon);
    }
    
    /// Distance to another point
    [[nodiscard]] f32 distanceTo(const Vec2& other) const noexcept {
        return (*this - other).length();
    }
    
    /// Squared distance to another point (faster)
    [[nodiscard]] constexpr f32 distanceSquaredTo(const Vec2& other) const noexcept {
        return (*this - other).lengthSquared();
    }
    
    /// Perpendicular vector (rotated 90 degrees counter-clockwise)
    [[nodiscard]] constexpr Vec2 perpendicular() const noexcept {
        return Vec2(-y, x);
    }
    
    /// Reflect vector around a normal
    [[nodiscard]] constexpr Vec2 reflect(const Vec2& normal) const noexcept {
        return *this - normal * (2.0f * dot(normal));
    }
    
    /// Project this vector onto another vector
    [[nodiscard]] Vec2 projectOnto(const Vec2& other) const noexcept {
        f32 d = other.dot(other);
        if (d > 0.0f) {
            return other * (dot(other) / d);
        }
        return Vec2::zero();
    }
    
    /// Angle from this vector to another (in radians)
    [[nodiscard]] f32 angleTo(const Vec2& other) const noexcept {
        return std::atan2(cross(other), dot(other));
    }
    
    /// Angle of this vector from positive X axis (in radians)
    [[nodiscard]] f32 angle() const noexcept {
        return std::atan2(y, x);
    }
    
    /// Create vector from angle (in radians)
    [[nodiscard]] static Vec2 fromAngle(f32 radians) noexcept {
        return Vec2(std::cos(radians), std::sin(radians));
    }
    
    /// Rotate vector by angle (in radians)
    [[nodiscard]] Vec2 rotated(f32 radians) const noexcept {
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        return Vec2(x * c - y * s, x * s + y * c);
    }
    
    /// Component-wise min
    [[nodiscard]] constexpr Vec2 min(const Vec2& other) const noexcept {
        return Vec2(std::min(x, other.x), std::min(y, other.y));
    }
    
    /// Component-wise max
    [[nodiscard]] constexpr Vec2 max(const Vec2& other) const noexcept {
        return Vec2(std::max(x, other.x), std::max(y, other.y));
    }
    
    /// Component-wise absolute value
    [[nodiscard]] Vec2 abs() const noexcept {
        return Vec2(std::abs(x), std::abs(y));
    }
    
    /// Component-wise floor
    [[nodiscard]] Vec2 floor() const noexcept {
        return Vec2(std::floor(x), std::floor(y));
    }
    
    /// Component-wise ceil
    [[nodiscard]] Vec2 ceil() const noexcept {
        return Vec2(std::ceil(x), std::ceil(y));
    }
    
    /// Component-wise round
    [[nodiscard]] Vec2 round() const noexcept {
        return Vec2(std::round(x), std::round(y));
    }
    
    /// Clamp components to range
    [[nodiscard]] constexpr Vec2 clamp(const Vec2& minVal, const Vec2& maxVal) const noexcept {
        return Vec2(
            math::clamp(x, minVal.x, maxVal.x),
            math::clamp(y, minVal.y, maxVal.y)
        );
    }
    
    /// Linear interpolation
    [[nodiscard]] constexpr Vec2 lerp(const Vec2& other, f32 t) const noexcept {
        return Vec2(
            math::lerp(x, other.x, t),
            math::lerp(y, other.y, t)
        );
    }
};

// =============================================================================
// Free Functions
// =============================================================================

/// Scalar multiplication (scalar * vector)
[[nodiscard]] constexpr Vec2 operator*(f32 scalar, const Vec2& v) noexcept {
    return v * scalar;
}

/// Dot product free function
[[nodiscard]] constexpr f32 dot(const Vec2& a, const Vec2& b) noexcept {
    return a.dot(b);
}

/// Cross product free function
[[nodiscard]] constexpr f32 cross(const Vec2& a, const Vec2& b) noexcept {
    return a.cross(b);
}

/// Length free function
[[nodiscard]] inline f32 length(const Vec2& v) noexcept {
    return v.length();
}

/// Normalize free function
[[nodiscard]] inline Vec2 normalize(const Vec2& v) noexcept {
    return v.normalized();
}

/// Distance free function
[[nodiscard]] inline f32 distance(const Vec2& a, const Vec2& b) noexcept {
    return a.distanceTo(b);
}

/// Linear interpolation free function
[[nodiscard]] constexpr Vec2 lerp(const Vec2& a, const Vec2& b, f32 t) noexcept {
    return a.lerp(b, t);
}

/// Component-wise min free function
[[nodiscard]] constexpr Vec2 min(const Vec2& a, const Vec2& b) noexcept {
    return a.min(b);
}

/// Component-wise max free function
[[nodiscard]] constexpr Vec2 max(const Vec2& a, const Vec2& b) noexcept {
    return a.max(b);
}

/// Reflect free function
[[nodiscard]] constexpr Vec2 reflect(const Vec2& v, const Vec2& normal) noexcept {
    return v.reflect(normal);
}

} // namespace nova::math

// Bring common types into nova namespace
namespace nova {
    using Vec2 = math::Vec2;
}
