// =============================================================================
// NovaCore Engine - Math Common Definitions
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Common math functions, constants, and SIMD intrinsic includes.
// =============================================================================

#pragma once

#include <cmath>
#include <algorithm>

#include "nova/core/types/types.hpp"

// =============================================================================
// SIMD Intrinsic Includes
// =============================================================================

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    #include <arm_neon.h>
    #define NOVA_SIMD_NEON 1
#endif

#if defined(__AVX2__)
    #include <immintrin.h>
    #define NOVA_SIMD_AVX2 1
#endif

#if defined(__AVX512F__)
    #include <immintrin.h>
    #define NOVA_SIMD_AVX512 1
#endif

#if defined(__wasm_simd128__)
    #include <wasm_simd128.h>
    #define NOVA_SIMD_WASM 1
#endif

namespace nova::math {

// =============================================================================
// Using statements for convenience
// =============================================================================
using nova::f32;
using nova::f64;
using nova::i32;
using nova::u32;

// =============================================================================
// Common Math Functions
// =============================================================================

/// Clamp value between min and max
template<typename T>
[[nodiscard]] constexpr T clamp(T value, T minVal, T maxVal) noexcept {
    return std::max(minVal, std::min(value, maxVal));
}

/// Linear interpolation
template<typename T>
[[nodiscard]] constexpr T lerp(T a, T b, T t) noexcept {
    return a + t * (b - a);
}

/// Smooth step interpolation (Hermite)
template<typename T>
[[nodiscard]] constexpr T smoothstep(T edge0, T edge1, T x) noexcept {
    T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
    return t * t * (T(3) - T(2) * t);
}

/// Smoother step interpolation (Ken Perlin's improved version)
template<typename T>
[[nodiscard]] constexpr T smootherstep(T edge0, T edge1, T x) noexcept {
    T t = clamp((x - edge0) / (edge1 - edge0), T(0), T(1));
    return t * t * t * (t * (t * T(6) - T(15)) + T(10));
}

/// Sign function (-1, 0, or 1)
template<typename T>
[[nodiscard]] constexpr T sign(T value) noexcept {
    return (T(0) < value) - (value < T(0));
}

/// Step function (0 if x < edge, else 1)
template<typename T>
[[nodiscard]] constexpr T step(T edge, T x) noexcept {
    return x < edge ? T(0) : T(1);
}

/// Fractional part of a floating-point number
[[nodiscard]] inline f32 fract(f32 x) noexcept {
    return x - std::floor(x);
}

/// Fractional part (double precision)
[[nodiscard]] inline f64 fract(f64 x) noexcept {
    return x - std::floor(x);
}

/// Modulo operation that always returns positive result
[[nodiscard]] inline f32 mod(f32 x, f32 y) noexcept {
    return x - y * std::floor(x / y);
}

/// Modulo operation (double precision)
[[nodiscard]] inline f64 mod(f64 x, f64 y) noexcept {
    return x - y * std::floor(x / y);
}

/// Wrap value into [0, max) range
template<typename T>
[[nodiscard]] constexpr T wrap(T value, T max) noexcept {
    return mod(value, max);
}

/// Wrap value into [min, max) range
template<typename T>
[[nodiscard]] constexpr T wrap(T value, T min, T max) noexcept {
    T range = max - min;
    return min + mod(value - min, range);
}

/// Map value from one range to another
template<typename T>
[[nodiscard]] constexpr T map(T value, T inMin, T inMax, T outMin, T outMax) noexcept {
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

/// Convert degrees to radians
[[nodiscard]] constexpr f32 radians(f32 degrees) noexcept {
    return degrees * DEG_TO_RAD_F32;
}

/// Convert degrees to radians (double)
[[nodiscard]] constexpr f64 radians(f64 degrees) noexcept {
    return degrees * DEG_TO_RAD_F64;
}

/// Convert radians to degrees
[[nodiscard]] constexpr f32 degrees(f32 rad) noexcept {
    return rad * RAD_TO_DEG_F32;
}

/// Convert radians to degrees (double)
[[nodiscard]] constexpr f64 degrees(f64 rad) noexcept {
    return rad * RAD_TO_DEG_F64;
}

/// Fast approximate sine using polynomial
[[nodiscard]] inline f32 fastSin(f32 x) noexcept {
    // Normalize to [-PI, PI]
    x = mod(x + PI_F32, TAU_F32) - PI_F32;
    
    // Parabolic approximation
    constexpr f32 B = 4.0f / PI_F32;
    constexpr f32 C = -4.0f / (PI_F32 * PI_F32);
    constexpr f32 P = 0.225f;
    
    f32 y = B * x + C * x * std::abs(x);
    y = P * (y * std::abs(y) - y) + y;
    
    return y;
}

/// Fast approximate cosine
[[nodiscard]] inline f32 fastCos(f32 x) noexcept {
    return fastSin(x + PI_F32 * 0.5f);
}

/// Approximate floating-point equality
[[nodiscard]] inline bool nearEqual(f32 a, f32 b, f32 epsilon = F32_EPSILON * 100.0f) noexcept {
    return std::abs(a - b) <= epsilon * std::max(1.0f, std::max(std::abs(a), std::abs(b)));
}

/// Approximate floating-point equality (double)
[[nodiscard]] inline bool nearEqual(f64 a, f64 b, f64 epsilon = F64_EPSILON * 100.0) noexcept {
    return std::abs(a - b) <= epsilon * std::max(1.0, std::max(std::abs(a), std::abs(b)));
}

/// Check if value is approximately zero
[[nodiscard]] inline bool nearZero(f32 value, f32 epsilon = F32_EPSILON * 100.0f) noexcept {
    return std::abs(value) <= epsilon;
}

/// Check if value is approximately zero (double)
[[nodiscard]] inline bool nearZero(f64 value, f64 epsilon = F64_EPSILON * 100.0) noexcept {
    return std::abs(value) <= epsilon;
}

/// Fast inverse square root (Quake III style, modernized)
[[nodiscard]] inline f32 fastInvSqrt(f32 x) noexcept {
    // Use standard library for accuracy (modern CPUs have fast rsqrt)
    return 1.0f / std::sqrt(x);
}

/// Safe divide (returns 0 if divisor is near zero)
[[nodiscard]] inline f32 safeDivide(f32 a, f32 b, f32 epsilon = F32_EPSILON) noexcept {
    return std::abs(b) > epsilon ? a / b : 0.0f;
}

/// Safe normalize scalar (clamp to [-1, 1])
[[nodiscard]] inline f32 normalizeScalar(f32 x) noexcept {
    return clamp(x, -1.0f, 1.0f);
}

// =============================================================================
// SIMD Helper Functions
// =============================================================================

#if NOVA_SIMD_NEON

/// Load 4 floats into NEON register
/// @note Assumes 16-byte aligned data (Vec4 is alignas(16))
[[nodiscard]] NOVA_FORCE_INLINE float32x4_t simdLoad4(const f32* ptr) noexcept {
    return vld1q_f32(ptr);  // ARM NEON handles both aligned/unaligned
}

/// Store NEON register to 4 floats
NOVA_FORCE_INLINE void simdStore4(f32* ptr, float32x4_t v) noexcept {
    vst1q_f32(ptr, v);
}

/// NEON 4-component dot product
[[nodiscard]] NOVA_FORCE_INLINE f32 simdDot4(float32x4_t a, float32x4_t b) noexcept {
    float32x4_t prod = vmulq_f32(a, b);
    float32x2_t sum1 = vadd_f32(vget_low_f32(prod), vget_high_f32(prod));
    float32x2_t sum2 = vpadd_f32(sum1, sum1);
    return vget_lane_f32(sum2, 0);
}

#elif NOVA_SIMD_AVX2

/// Load 4 floats into AVX register
/// @note Uses unaligned load for flexibility; Vec4 is 16-byte aligned
[[nodiscard]] NOVA_FORCE_INLINE __m128 simdLoad4(const f32* ptr) noexcept {
    return _mm_loadu_ps(ptr);  // Unaligned for general use
}

/// Store AVX register to 4 floats
NOVA_FORCE_INLINE void simdStore4(f32* ptr, __m128 v) noexcept {
    _mm_storeu_ps(ptr, v);
}

/// AVX 4-component dot product
[[nodiscard]] NOVA_FORCE_INLINE f32 simdDot4(__m128 a, __m128 b) noexcept {
    __m128 prod = _mm_mul_ps(a, b);
    __m128 shuf = _mm_movehdup_ps(prod);
    __m128 sums = _mm_add_ps(prod, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    return _mm_cvtss_f32(sums);
}

#endif

} // namespace nova::math
