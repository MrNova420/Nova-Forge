// =============================================================================
// NovaCore Engine - 4x4 Matrix
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// 4x4 matrix for 3D transformations (translate, rotate, scale).
// Column-major storage for GPU compatibility (OpenGL/Vulkan/Metal).
// =============================================================================

#pragma once

#include "nova/core/math/math_common.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/math/mat3.hpp"

namespace nova::math {

/// @brief 4x4 matrix in column-major order
/// @note Aligned to 64 bytes for cache line and SIMD optimization
struct alignas(64) Mat4 {
    // Column-major storage: columns[0] is first column
    Vec4 columns[4];
    
    // ==========================================================================
    // Constructors
    // ==========================================================================
    
    /// Default constructor (identity matrix)
    constexpr Mat4() noexcept 
        : columns{
            Vec4(1.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 1.0f, 0.0f, 0.0f),
            Vec4(0.0f, 0.0f, 1.0f, 0.0f),
            Vec4(0.0f, 0.0f, 0.0f, 1.0f)
        } {}
    
    /// Construct from column vectors
    constexpr Mat4(const Vec4& col0, const Vec4& col1, const Vec4& col2, const Vec4& col3) noexcept
        : columns{col0, col1, col2, col3} {}
    
    /// Construct from scalar (diagonal matrix)
    constexpr explicit Mat4(f32 diagonal) noexcept
        : columns{
            Vec4(diagonal, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, diagonal, 0.0f, 0.0f),
            Vec4(0.0f, 0.0f, diagonal, 0.0f),
            Vec4(0.0f, 0.0f, 0.0f, diagonal)
        } {}
    
    /// Construct from 3x3 matrix (upper-left) with identity for rest
    constexpr explicit Mat4(const Mat3& m) noexcept
        : columns{
            Vec4(m.columns[0], 0.0f),
            Vec4(m.columns[1], 0.0f),
            Vec4(m.columns[2], 0.0f),
            Vec4(0.0f, 0.0f, 0.0f, 1.0f)
        } {}
    
    // ==========================================================================
    // Static Factory Methods
    // ==========================================================================
    
    /// Identity matrix
    [[nodiscard]] static constexpr Mat4 identity() noexcept { return Mat4(); }
    
    /// Zero matrix
    [[nodiscard]] static constexpr Mat4 zero() noexcept {
        return Mat4(Vec4::zero(), Vec4::zero(), Vec4::zero(), Vec4::zero());
    }
    
    /// Create translation matrix
    [[nodiscard]] static constexpr Mat4 translate(const Vec3& t) noexcept {
        Mat4 m;
        m.columns[3] = Vec4(t, 1.0f);
        return m;
    }
    
    /// Create rotation matrix around X axis
    [[nodiscard]] static Mat4 rotateX(f32 radians) noexcept {
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        Mat4 m;
        m.columns[1].y = c;  m.columns[1].z = s;
        m.columns[2].y = -s; m.columns[2].z = c;
        return m;
    }
    
    /// Create rotation matrix around Y axis
    [[nodiscard]] static Mat4 rotateY(f32 radians) noexcept {
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        Mat4 m;
        m.columns[0].x = c;  m.columns[0].z = -s;
        m.columns[2].x = s;  m.columns[2].z = c;
        return m;
    }
    
    /// Create rotation matrix around Z axis
    [[nodiscard]] static Mat4 rotateZ(f32 radians) noexcept {
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        Mat4 m;
        m.columns[0].x = c;  m.columns[0].y = s;
        m.columns[1].x = -s; m.columns[1].y = c;
        return m;
    }
    
    /// Create rotation matrix around arbitrary axis
    [[nodiscard]] static Mat4 rotate(const Vec3& axis, f32 radians) noexcept {
        return Mat4(Mat3::rotate(axis, radians));
    }
    
    /// Create scale matrix
    [[nodiscard]] static constexpr Mat4 scale(const Vec3& s) noexcept {
        Mat4 m;
        m.columns[0].x = s.x;
        m.columns[1].y = s.y;
        m.columns[2].z = s.z;
        return m;
    }
    
    /// Create uniform scale matrix
    [[nodiscard]] static constexpr Mat4 scale(f32 s) noexcept {
        return scale(Vec3(s, s, s));
    }
    
    /// Create look-at view matrix (right-handed)
    [[nodiscard]] static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) noexcept {
        Vec3 f = (target - eye).normalized(); // Forward
        Vec3 r = f.cross(up).normalized();    // Right
        Vec3 u = r.cross(f);                   // Up
        
        Mat4 m;
        m.columns[0] = Vec4(r.x, u.x, -f.x, 0.0f);
        m.columns[1] = Vec4(r.y, u.y, -f.y, 0.0f);
        m.columns[2] = Vec4(r.z, u.z, -f.z, 0.0f);
        m.columns[3] = Vec4(-r.dot(eye), -u.dot(eye), f.dot(eye), 1.0f);
        return m;
    }
    
    /// Create perspective projection matrix (right-handed, depth [0,1], reversed-Z for better precision)
    [[nodiscard]] static Mat4 perspective(f32 fovY, f32 aspect, f32 nearZ, f32 farZ) noexcept {
        f32 tanHalfFov = std::tan(fovY * 0.5f);
        
        Mat4 m = Mat4::zero();
        m.columns[0].x = 1.0f / (aspect * tanHalfFov);
        m.columns[1].y = 1.0f / tanHalfFov;
        // Reversed-Z for better depth precision
        m.columns[2].z = nearZ / (farZ - nearZ);
        m.columns[2].w = -1.0f;
        m.columns[3].z = (farZ * nearZ) / (farZ - nearZ);
        return m;
    }
    
    /// Create infinite perspective projection (far plane at infinity)
    [[nodiscard]] static Mat4 perspectiveInfinite(f32 fovY, f32 aspect, f32 nearZ) noexcept {
        f32 tanHalfFov = std::tan(fovY * 0.5f);
        
        Mat4 m = Mat4::zero();
        m.columns[0].x = 1.0f / (aspect * tanHalfFov);
        m.columns[1].y = 1.0f / tanHalfFov;
        m.columns[2].z = 0.0f;
        m.columns[2].w = -1.0f;
        m.columns[3].z = nearZ;
        return m;
    }
    
    /// Create orthographic projection matrix
    [[nodiscard]] static constexpr Mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 nearZ, f32 farZ) noexcept {
        Mat4 m;
        m.columns[0].x = 2.0f / (right - left);
        m.columns[1].y = 2.0f / (top - bottom);
        m.columns[2].z = 1.0f / (nearZ - farZ);
        m.columns[3].x = -(right + left) / (right - left);
        m.columns[3].y = -(top + bottom) / (top - bottom);
        m.columns[3].z = nearZ / (nearZ - farZ);
        return m;
    }
    
    // ==========================================================================
    // Accessors
    // ==========================================================================
    
    /// Column access
    [[nodiscard]] constexpr Vec4& operator[](usize col) noexcept {
        return columns[col];
    }
    
    [[nodiscard]] constexpr const Vec4& operator[](usize col) const noexcept {
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
    [[nodiscard]] constexpr Vec4 row(usize r) const noexcept {
        return Vec4(columns[0][r], columns[1][r], columns[2][r], columns[3][r]);
    }
    
    /// Get translation component
    [[nodiscard]] constexpr Vec3 translation() const noexcept {
        return columns[3].xyz();
    }
    
    /// Set translation component
    constexpr void setTranslation(const Vec3& t) noexcept {
        columns[3].x = t.x;
        columns[3].y = t.y;
        columns[3].z = t.z;
    }
    
    /// Get upper-left 3x3 matrix (rotation/scale)
    [[nodiscard]] constexpr Mat3 upper3x3() const noexcept {
        return Mat3(columns[0].xyz(), columns[1].xyz(), columns[2].xyz());
    }
    
    /// Get as pointer to float array (column-major)
    [[nodiscard]] constexpr f32* data() noexcept { return columns[0].data(); }
    [[nodiscard]] constexpr const f32* data() const noexcept { return columns[0].data(); }
    
    // ==========================================================================
    // Matrix Operations
    // ==========================================================================
    
    /// Matrix multiplication
    [[nodiscard]] Mat4 operator*(const Mat4& other) const noexcept {
        Mat4 result;
        for (usize c = 0; c < 4; ++c) {
            for (usize r = 0; r < 4; ++r) {
                result.columns[c][r] = 
                    columns[0][r] * other.columns[c][0] +
                    columns[1][r] * other.columns[c][1] +
                    columns[2][r] * other.columns[c][2] +
                    columns[3][r] * other.columns[c][3];
            }
        }
        return result;
    }
    
    /// Matrix-vector multiplication (Vec4)
    [[nodiscard]] constexpr Vec4 operator*(const Vec4& v) const noexcept {
        return Vec4(
            columns[0].x * v.x + columns[1].x * v.y + columns[2].x * v.z + columns[3].x * v.w,
            columns[0].y * v.x + columns[1].y * v.y + columns[2].y * v.z + columns[3].y * v.w,
            columns[0].z * v.x + columns[1].z * v.y + columns[2].z * v.z + columns[3].z * v.w,
            columns[0].w * v.x + columns[1].w * v.y + columns[2].w * v.z + columns[3].w * v.w
        );
    }
    
    /// Transform point (Vec3, w=1)
    [[nodiscard]] constexpr Vec3 transformPoint(const Vec3& p) const noexcept {
        return Vec3(
            columns[0].x * p.x + columns[1].x * p.y + columns[2].x * p.z + columns[3].x,
            columns[0].y * p.x + columns[1].y * p.y + columns[2].y * p.z + columns[3].y,
            columns[0].z * p.x + columns[1].z * p.y + columns[2].z * p.z + columns[3].z
        );
    }
    
    /// Transform direction (Vec3, w=0, no translation)
    [[nodiscard]] constexpr Vec3 transformDirection(const Vec3& d) const noexcept {
        return Vec3(
            columns[0].x * d.x + columns[1].x * d.y + columns[2].x * d.z,
            columns[0].y * d.x + columns[1].y * d.y + columns[2].y * d.z,
            columns[0].z * d.x + columns[1].z * d.y + columns[2].z * d.z
        );
    }
    
    /// Scalar multiplication
    [[nodiscard]] constexpr Mat4 operator*(f32 scalar) const noexcept {
        return Mat4(columns[0] * scalar, columns[1] * scalar, columns[2] * scalar, columns[3] * scalar);
    }
    
    /// Matrix addition
    [[nodiscard]] constexpr Mat4 operator+(const Mat4& other) const noexcept {
        return Mat4(
            columns[0] + other.columns[0],
            columns[1] + other.columns[1],
            columns[2] + other.columns[2],
            columns[3] + other.columns[3]
        );
    }
    
    /// Transpose
    [[nodiscard]] constexpr Mat4 transposed() const noexcept {
        return Mat4(
            Vec4(columns[0].x, columns[1].x, columns[2].x, columns[3].x),
            Vec4(columns[0].y, columns[1].y, columns[2].y, columns[3].y),
            Vec4(columns[0].z, columns[1].z, columns[2].z, columns[3].z),
            Vec4(columns[0].w, columns[1].w, columns[2].w, columns[3].w)
        );
    }
    
    /// Inverse (optimized for affine transforms)
    [[nodiscard]] Mat4 inverse() const noexcept;
    
    /// Inverse for affine transforms (faster, assumes w row is [0,0,0,1])
    [[nodiscard]] Mat4 inverseAffine() const noexcept {
        Mat3 r = upper3x3().inverse();
        Vec3 t = r * (-translation());
        
        Mat4 result(r);
        result.columns[3] = Vec4(t, 1.0f);
        return result;
    }
    
    /// Determinant
    [[nodiscard]] f32 determinant() const noexcept;
    
    /// Check equality
    [[nodiscard]] constexpr bool operator==(const Mat4& other) const noexcept {
        return columns[0] == other.columns[0] && 
               columns[1] == other.columns[1] && 
               columns[2] == other.columns[2] &&
               columns[3] == other.columns[3];
    }
    
    [[nodiscard]] constexpr bool operator!=(const Mat4& other) const noexcept {
        return !(*this == other);
    }
};

// Free function: scalar * matrix
[[nodiscard]] constexpr Mat4 operator*(f32 scalar, const Mat4& m) noexcept {
    return m * scalar;
}

// =============================================================================
// Mat4 Method Implementations (complex methods)
// =============================================================================

inline f32 Mat4::determinant() const noexcept {
    f32 a00 = columns[0].x, a01 = columns[1].x, a02 = columns[2].x, a03 = columns[3].x;
    f32 a10 = columns[0].y, a11 = columns[1].y, a12 = columns[2].y, a13 = columns[3].y;
    f32 a20 = columns[0].z, a21 = columns[1].z, a22 = columns[2].z, a23 = columns[3].z;
    f32 a30 = columns[0].w, a31 = columns[1].w, a32 = columns[2].w, a33 = columns[3].w;
    
    f32 b00 = a00 * a11 - a01 * a10;
    f32 b01 = a00 * a12 - a02 * a10;
    f32 b02 = a00 * a13 - a03 * a10;
    f32 b03 = a01 * a12 - a02 * a11;
    f32 b04 = a01 * a13 - a03 * a11;
    f32 b05 = a02 * a13 - a03 * a12;
    f32 b06 = a20 * a31 - a21 * a30;
    f32 b07 = a20 * a32 - a22 * a30;
    f32 b08 = a20 * a33 - a23 * a30;
    f32 b09 = a21 * a32 - a22 * a31;
    f32 b10 = a21 * a33 - a23 * a31;
    f32 b11 = a22 * a33 - a23 * a32;
    
    return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
}

inline Mat4 Mat4::inverse() const noexcept {
    f32 a00 = columns[0].x, a01 = columns[1].x, a02 = columns[2].x, a03 = columns[3].x;
    f32 a10 = columns[0].y, a11 = columns[1].y, a12 = columns[2].y, a13 = columns[3].y;
    f32 a20 = columns[0].z, a21 = columns[1].z, a22 = columns[2].z, a23 = columns[3].z;
    f32 a30 = columns[0].w, a31 = columns[1].w, a32 = columns[2].w, a33 = columns[3].w;
    
    f32 b00 = a00 * a11 - a01 * a10;
    f32 b01 = a00 * a12 - a02 * a10;
    f32 b02 = a00 * a13 - a03 * a10;
    f32 b03 = a01 * a12 - a02 * a11;
    f32 b04 = a01 * a13 - a03 * a11;
    f32 b05 = a02 * a13 - a03 * a12;
    f32 b06 = a20 * a31 - a21 * a30;
    f32 b07 = a20 * a32 - a22 * a30;
    f32 b08 = a20 * a33 - a23 * a30;
    f32 b09 = a21 * a32 - a22 * a31;
    f32 b10 = a21 * a33 - a23 * a31;
    f32 b11 = a22 * a33 - a23 * a32;
    
    f32 det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
    
    if (std::abs(det) < limits::F32_EPSILON) {
        return Mat4::identity();
    }
    
    f32 invDet = 1.0f / det;
    
    Mat4 result;
    result.columns[0].x = ( a11 * b11 - a12 * b10 + a13 * b09) * invDet;
    result.columns[0].y = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
    result.columns[0].z = ( a10 * b10 - a11 * b08 + a13 * b06) * invDet;
    result.columns[0].w = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
    
    result.columns[1].x = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
    result.columns[1].y = ( a00 * b11 - a02 * b08 + a03 * b07) * invDet;
    result.columns[1].z = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
    result.columns[1].w = ( a00 * b09 - a01 * b07 + a02 * b06) * invDet;
    
    result.columns[2].x = ( a31 * b05 - a32 * b04 + a33 * b03) * invDet;
    result.columns[2].y = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
    result.columns[2].z = ( a30 * b04 - a31 * b02 + a33 * b00) * invDet;
    result.columns[2].w = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
    
    result.columns[3].x = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
    result.columns[3].y = ( a20 * b05 - a22 * b02 + a23 * b01) * invDet;
    result.columns[3].z = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
    result.columns[3].w = ( a20 * b03 - a21 * b01 + a22 * b00) * invDet;
    
    return result;
}

} // namespace nova::math

namespace nova {
    using Mat4 = math::Mat4;
}
