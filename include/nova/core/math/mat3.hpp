// =============================================================================
// NovaCore Engine - 3x3 Matrix
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// 3x3 matrix for rotations and 2D transformations.
// Column-major storage for GPU compatibility.
// =============================================================================

#pragma once

#include "nova/core/math/math_common.hpp"
#include "nova/core/math/vec3.hpp"

namespace nova::math {

/// @brief 3x3 matrix in column-major order
/// @note Used for rotations and upper-left portion of 4x4 matrices
struct Mat3 {
    // Column-major storage: columns[0] is first column
    Vec3 columns[3];
    
    // ==========================================================================
    // Constructors
    // ==========================================================================
    
    /// Default constructor (identity matrix)
    constexpr Mat3() noexcept 
        : columns{Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)} {}
    
    /// Construct from column vectors
    constexpr Mat3(const Vec3& col0, const Vec3& col1, const Vec3& col2) noexcept
        : columns{col0, col1, col2} {}
    
    /// Construct from scalar (diagonal matrix)
    constexpr explicit Mat3(f32 diagonal) noexcept
        : columns{Vec3(diagonal, 0.0f, 0.0f), Vec3(0.0f, diagonal, 0.0f), Vec3(0.0f, 0.0f, diagonal)} {}
    
    /// Construct from individual elements (row-major input for convenience)
    constexpr Mat3(
        f32 m00, f32 m01, f32 m02,
        f32 m10, f32 m11, f32 m12,
        f32 m20, f32 m21, f32 m22
    ) noexcept : columns{
        Vec3(m00, m10, m20),  // Column 0
        Vec3(m01, m11, m21),  // Column 1
        Vec3(m02, m12, m22)   // Column 2
    } {}
    
    // ==========================================================================
    // Static Factory Methods
    // ==========================================================================
    
    /// Identity matrix
    [[nodiscard]] static constexpr Mat3 identity() noexcept { return Mat3(); }
    
    /// Zero matrix
    [[nodiscard]] static constexpr Mat3 zero() noexcept {
        return Mat3(Vec3::zero(), Vec3::zero(), Vec3::zero());
    }
    
    /// Create rotation matrix around X axis
    [[nodiscard]] static Mat3 rotateX(f32 radians) noexcept {
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        return Mat3(
            Vec3(1.0f, 0.0f, 0.0f),
            Vec3(0.0f, c, s),
            Vec3(0.0f, -s, c)
        );
    }
    
    /// Create rotation matrix around Y axis
    [[nodiscard]] static Mat3 rotateY(f32 radians) noexcept {
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        return Mat3(
            Vec3(c, 0.0f, -s),
            Vec3(0.0f, 1.0f, 0.0f),
            Vec3(s, 0.0f, c)
        );
    }
    
    /// Create rotation matrix around Z axis
    [[nodiscard]] static Mat3 rotateZ(f32 radians) noexcept {
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        return Mat3(
            Vec3(c, s, 0.0f),
            Vec3(-s, c, 0.0f),
            Vec3(0.0f, 0.0f, 1.0f)
        );
    }
    
    /// Create rotation matrix around arbitrary axis
    [[nodiscard]] static Mat3 rotate(const Vec3& axis, f32 radians) noexcept {
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        f32 t = 1.0f - c;
        Vec3 n = axis.normalized();
        
        return Mat3(
            Vec3(t * n.x * n.x + c,        t * n.x * n.y + s * n.z, t * n.x * n.z - s * n.y),
            Vec3(t * n.x * n.y - s * n.z,  t * n.y * n.y + c,       t * n.y * n.z + s * n.x),
            Vec3(t * n.x * n.z + s * n.y,  t * n.y * n.z - s * n.x, t * n.z * n.z + c)
        );
    }
    
    /// Create scale matrix
    [[nodiscard]] static constexpr Mat3 scale(const Vec3& s) noexcept {
        return Mat3(
            Vec3(s.x, 0.0f, 0.0f),
            Vec3(0.0f, s.y, 0.0f),
            Vec3(0.0f, 0.0f, s.z)
        );
    }
    
    /// Create uniform scale matrix
    [[nodiscard]] static constexpr Mat3 scale(f32 s) noexcept {
        return scale(Vec3(s, s, s));
    }
    
    // ==========================================================================
    // Accessors
    // ==========================================================================
    
    /// Column access
    [[nodiscard]] constexpr Vec3& operator[](usize col) noexcept {
        return columns[col];
    }
    
    [[nodiscard]] constexpr const Vec3& operator[](usize col) const noexcept {
        return columns[col];
    }
    
    /// Get element at (row, col)
    [[nodiscard]] constexpr f32& at(usize row, usize col) noexcept {
        return columns[col][row];
    }
    
    [[nodiscard]] constexpr f32 at(usize row, usize col) const noexcept {
        return columns[col][row];
    }
    
    /// Get row
    [[nodiscard]] constexpr Vec3 row(usize r) const noexcept {
        return Vec3(columns[0][r], columns[1][r], columns[2][r]);
    }
    
    /// Get column
    [[nodiscard]] constexpr Vec3 col(usize c) const noexcept {
        return columns[c];
    }
    
    /// Get as pointer to float array (column-major)
    [[nodiscard]] constexpr f32* data() noexcept { return columns[0].data(); }
    [[nodiscard]] constexpr const f32* data() const noexcept { return columns[0].data(); }
    
    // ==========================================================================
    // Matrix Operations
    // ==========================================================================
    
    /// Matrix multiplication
    [[nodiscard]] constexpr Mat3 operator*(const Mat3& other) const noexcept {
        Mat3 result;
        for (usize c = 0; c < 3; ++c) {
            for (usize r = 0; r < 3; ++r) {
                result.columns[c][r] = 
                    columns[0][r] * other.columns[c][0] +
                    columns[1][r] * other.columns[c][1] +
                    columns[2][r] * other.columns[c][2];
            }
        }
        return result;
    }
    
    /// Matrix-vector multiplication
    [[nodiscard]] constexpr Vec3 operator*(const Vec3& v) const noexcept {
        return Vec3(
            columns[0].x * v.x + columns[1].x * v.y + columns[2].x * v.z,
            columns[0].y * v.x + columns[1].y * v.y + columns[2].y * v.z,
            columns[0].z * v.x + columns[1].z * v.y + columns[2].z * v.z
        );
    }
    
    /// Scalar multiplication
    [[nodiscard]] constexpr Mat3 operator*(f32 scalar) const noexcept {
        return Mat3(columns[0] * scalar, columns[1] * scalar, columns[2] * scalar);
    }
    
    /// Matrix addition
    [[nodiscard]] constexpr Mat3 operator+(const Mat3& other) const noexcept {
        return Mat3(columns[0] + other.columns[0], columns[1] + other.columns[1], columns[2] + other.columns[2]);
    }
    
    /// Matrix subtraction
    [[nodiscard]] constexpr Mat3 operator-(const Mat3& other) const noexcept {
        return Mat3(columns[0] - other.columns[0], columns[1] - other.columns[1], columns[2] - other.columns[2]);
    }
    
    /// Transpose
    [[nodiscard]] constexpr Mat3 transposed() const noexcept {
        return Mat3(
            Vec3(columns[0].x, columns[1].x, columns[2].x),
            Vec3(columns[0].y, columns[1].y, columns[2].y),
            Vec3(columns[0].z, columns[1].z, columns[2].z)
        );
    }
    
    /// Determinant
    [[nodiscard]] constexpr f32 determinant() const noexcept {
        return columns[0].x * (columns[1].y * columns[2].z - columns[2].y * columns[1].z)
             - columns[1].x * (columns[0].y * columns[2].z - columns[2].y * columns[0].z)
             + columns[2].x * (columns[0].y * columns[1].z - columns[1].y * columns[0].z);
    }
    
    /// Inverse
    [[nodiscard]] Mat3 inverse() const noexcept {
        f32 det = determinant();
        if (std::abs(det) < limits::F32_EPSILON) {
            return Mat3::identity();
        }
        
        f32 invDet = 1.0f / det;
        
        Mat3 result;
        result.columns[0].x = (columns[1].y * columns[2].z - columns[2].y * columns[1].z) * invDet;
        result.columns[0].y = (columns[2].y * columns[0].z - columns[0].y * columns[2].z) * invDet;
        result.columns[0].z = (columns[0].y * columns[1].z - columns[1].y * columns[0].z) * invDet;
        
        result.columns[1].x = (columns[2].x * columns[1].z - columns[1].x * columns[2].z) * invDet;
        result.columns[1].y = (columns[0].x * columns[2].z - columns[2].x * columns[0].z) * invDet;
        result.columns[1].z = (columns[1].x * columns[0].z - columns[0].x * columns[1].z) * invDet;
        
        result.columns[2].x = (columns[1].x * columns[2].y - columns[2].x * columns[1].y) * invDet;
        result.columns[2].y = (columns[2].x * columns[0].y - columns[0].x * columns[2].y) * invDet;
        result.columns[2].z = (columns[0].x * columns[1].y - columns[1].x * columns[0].y) * invDet;
        
        return result;
    }
    
    /// Check equality
    [[nodiscard]] constexpr bool operator==(const Mat3& other) const noexcept {
        return columns[0] == other.columns[0] && 
               columns[1] == other.columns[1] && 
               columns[2] == other.columns[2];
    }
    
    [[nodiscard]] constexpr bool operator!=(const Mat3& other) const noexcept {
        return !(*this == other);
    }
};

// Free function: scalar * matrix
[[nodiscard]] constexpr Mat3 operator*(f32 scalar, const Mat3& m) noexcept {
    return m * scalar;
}

} // namespace nova::math

namespace nova {
    using Mat3 = math::Mat3;
}
