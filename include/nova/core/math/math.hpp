// =============================================================================
// NovaCore Engine - Math Library
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Comprehensive math library with SIMD optimizations for:
// - ARM NEON (mobile devices)
// - x86 AVX2/SSE (desktop)
// - WebAssembly SIMD (web)
//
// All math types are designed for:
// - Maximum performance (SIMD where beneficial)
// - Cache efficiency (16-byte aligned, SoA-friendly)
// - Type safety (no implicit conversions)
// - Intuitive API (operator overloads, named functions)
// =============================================================================

#pragma once

// Include all math components
#include "nova/core/math/math_common.hpp"
#include "nova/core/math/vec2.hpp"
#include "nova/core/math/vec3.hpp"
#include "nova/core/math/vec4.hpp"
#include "nova/core/math/mat3.hpp"
#include "nova/core/math/mat4.hpp"
#include "nova/core/math/quat.hpp"
#include "nova/core/math/transform.hpp"

namespace nova::math {

// =============================================================================
// Math Library Version
// =============================================================================
inline constexpr u32 MATH_VERSION_MAJOR = 0;
inline constexpr u32 MATH_VERSION_MINOR = 1;
inline constexpr u32 MATH_VERSION_PATCH = 0;

// =============================================================================
// SIMD Detection Summary
// =============================================================================
#if NOVA_SIMD_NEON
    inline constexpr const char* SIMD_TYPE = "ARM NEON";
#elif NOVA_SIMD_AVX2
    inline constexpr const char* SIMD_TYPE = "x86 AVX2";
#elif NOVA_SIMD_AVX512
    inline constexpr const char* SIMD_TYPE = "x86 AVX-512";
#elif NOVA_SIMD_WASM
    inline constexpr const char* SIMD_TYPE = "WebAssembly SIMD";
#else
    inline constexpr const char* SIMD_TYPE = "Scalar";
#endif

} // namespace nova::math
