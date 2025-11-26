// =============================================================================
// NovaCore Engine - Core Types
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// This file defines the fundamental types used throughout NovaCore Engine.
// All types are designed for:
// - Maximum performance (cache-friendly, SIMD-ready)
// - Mobile-first (memory-efficient)
// - Type safety (strong typing, no implicit conversions)
// - Cross-platform compatibility (all target platforms)
// =============================================================================

#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <limits>
#include <bit>
#include <compare>

namespace nova {

// =============================================================================
// Integer Types - Fixed-width for cross-platform consistency
// =============================================================================

/// 8-bit signed integer
using i8 = std::int8_t;

/// 16-bit signed integer
using i16 = std::int16_t;

/// 32-bit signed integer
using i32 = std::int32_t;

/// 64-bit signed integer
using i64 = std::int64_t;

/// 8-bit unsigned integer
using u8 = std::uint8_t;

/// 16-bit unsigned integer
using u16 = std::uint16_t;

/// 32-bit unsigned integer
using u32 = std::uint32_t;

/// 64-bit unsigned integer
using u64 = std::uint64_t;

// =============================================================================
// Floating-Point Types
// =============================================================================

/// 32-bit floating-point (single precision)
using f32 = float;

/// 64-bit floating-point (double precision)
using f64 = double;

// Compile-time validation of floating-point sizes
static_assert(sizeof(f32) == 4, "f32 must be 4 bytes");
static_assert(sizeof(f64) == 8, "f64 must be 8 bytes");
static_assert(std::numeric_limits<f32>::is_iec559, "f32 must be IEEE 754");
static_assert(std::numeric_limits<f64>::is_iec559, "f64 must be IEEE 754");

// =============================================================================
// Size and Pointer Types
// =============================================================================

/// Size type (platform-native unsigned)
using usize = std::size_t;

/// Signed size type (for differences)
using isize = std::ptrdiff_t;

/// Pointer-sized unsigned integer
using uptr = std::uintptr_t;

/// Pointer-sized signed integer
using iptr = std::intptr_t;

// =============================================================================
// Type Limits
// =============================================================================

namespace limits {
    /// Maximum value for i8
    inline constexpr i8 I8_MAX = std::numeric_limits<i8>::max();
    /// Minimum value for i8
    inline constexpr i8 I8_MIN = std::numeric_limits<i8>::min();
    
    /// Maximum value for i16
    inline constexpr i16 I16_MAX = std::numeric_limits<i16>::max();
    /// Minimum value for i16
    inline constexpr i16 I16_MIN = std::numeric_limits<i16>::min();
    
    /// Maximum value for i32
    inline constexpr i32 I32_MAX = std::numeric_limits<i32>::max();
    /// Minimum value for i32
    inline constexpr i32 I32_MIN = std::numeric_limits<i32>::min();
    
    /// Maximum value for i64
    inline constexpr i64 I64_MAX = std::numeric_limits<i64>::max();
    /// Minimum value for i64
    inline constexpr i64 I64_MIN = std::numeric_limits<i64>::min();
    
    /// Maximum value for u8
    inline constexpr u8 U8_MAX = std::numeric_limits<u8>::max();
    
    /// Maximum value for u16
    inline constexpr u16 U16_MAX = std::numeric_limits<u16>::max();
    
    /// Maximum value for u32
    inline constexpr u32 U32_MAX = std::numeric_limits<u32>::max();
    
    /// Maximum value for u64
    inline constexpr u64 U64_MAX = std::numeric_limits<u64>::max();
    
    /// Largest finite f32 value
    inline constexpr f32 F32_MAX = std::numeric_limits<f32>::max();
    /// Smallest positive f32 value
    inline constexpr f32 F32_MIN = std::numeric_limits<f32>::min();
    /// Positive infinity for f32
    inline constexpr f32 F32_INFINITY = std::numeric_limits<f32>::infinity();
    /// Epsilon for f32 comparisons
    inline constexpr f32 F32_EPSILON = std::numeric_limits<f32>::epsilon();
    
    /// Largest finite f64 value
    inline constexpr f64 F64_MAX = std::numeric_limits<f64>::max();
    /// Smallest positive f64 value
    inline constexpr f64 F64_MIN = std::numeric_limits<f64>::min();
    /// Positive infinity for f64
    inline constexpr f64 F64_INFINITY = std::numeric_limits<f64>::infinity();
    /// Epsilon for f64 comparisons
    inline constexpr f64 F64_EPSILON = std::numeric_limits<f64>::epsilon();
    
    /// Maximum value for usize
    inline constexpr usize USIZE_MAX = std::numeric_limits<usize>::max();
    /// Maximum value for isize
    inline constexpr isize ISIZE_MAX = std::numeric_limits<isize>::max();
    /// Minimum value for isize
    inline constexpr isize ISIZE_MIN = std::numeric_limits<isize>::min();
} // namespace limits

// =============================================================================
// Mathematical Constants
// =============================================================================

namespace math {
    /// Pi constant (32-bit)
    inline constexpr f32 PI_F32 = 3.14159265358979323846f;
    /// Pi constant (64-bit)
    inline constexpr f64 PI_F64 = 3.14159265358979323846;
    
    /// Tau (2*Pi) constant (32-bit)
    inline constexpr f32 TAU_F32 = PI_F32 * 2.0f;
    /// Tau (2*Pi) constant (64-bit)
    inline constexpr f64 TAU_F64 = PI_F64 * 2.0;
    
    /// Euler's number e (32-bit)
    inline constexpr f32 E_F32 = 2.71828182845904523536f;
    /// Euler's number e (64-bit)
    inline constexpr f64 E_F64 = 2.71828182845904523536;
    
    /// Square root of 2 (32-bit)
    inline constexpr f32 SQRT2_F32 = 1.41421356237309504880f;
    /// Square root of 2 (64-bit)
    inline constexpr f64 SQRT2_F64 = 1.41421356237309504880;
    
    /// Square root of 3 (32-bit)
    inline constexpr f32 SQRT3_F32 = 1.73205080756887729353f;
    /// Square root of 3 (64-bit)
    inline constexpr f64 SQRT3_F64 = 1.73205080756887729353;
    
    /// Golden ratio (32-bit)
    inline constexpr f32 PHI_F32 = 1.61803398874989484820f;
    /// Golden ratio (64-bit)
    inline constexpr f64 PHI_F64 = 1.61803398874989484820;
    
    /// Degrees to radians conversion factor (32-bit)
    inline constexpr f32 DEG_TO_RAD_F32 = PI_F32 / 180.0f;
    /// Radians to degrees conversion factor (32-bit)
    inline constexpr f32 RAD_TO_DEG_F32 = 180.0f / PI_F32;
    
    /// Degrees to radians conversion factor (64-bit)
    inline constexpr f64 DEG_TO_RAD_F64 = PI_F64 / 180.0;
    /// Radians to degrees conversion factor (64-bit)
    inline constexpr f64 RAD_TO_DEG_F64 = 180.0 / PI_F64;
} // namespace math

// =============================================================================
// Handle Types - Type-safe handles for engine resources
// =============================================================================

/// @brief Type-safe handle for engine resources
/// @tparam Tag Unique tag type to distinguish handle types
/// @tparam T Underlying integer type (default: u32)
template<typename Tag, typename T = u32>
struct Handle {
    using value_type = T;
    
    /// Invalid handle constant
    static constexpr T INVALID_VALUE = std::numeric_limits<T>::max();
    
    /// The underlying value
    T value;
    
    /// Default constructor creates invalid handle
    constexpr Handle() noexcept : value(INVALID_VALUE) {}
    
    /// Construct from value
    constexpr explicit Handle(T v) noexcept : value(v) {}
    
    /// Check if handle is valid
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return value != INVALID_VALUE;
    }
    
    /// Explicit conversion to bool
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return isValid();
    }
    
    /// Get underlying value
    [[nodiscard]] constexpr T get() const noexcept {
        return value;
    }
    
    /// Create invalid handle
    [[nodiscard]] static constexpr Handle invalid() noexcept {
        return Handle{};
    }
    
    /// Comparison operators
    [[nodiscard]] constexpr auto operator<=>(const Handle&) const noexcept = default;
};

// =============================================================================
// Entity Types - For Entity-Component-Worker (ECW) system
// =============================================================================

/// Entity handle tag
struct EntityTag {};

/// Entity ID type - 64-bit for version + index encoding
/// Format: [32-bit version][32-bit index]
using EntityId = Handle<EntityTag, u64>;

/// Extract entity index from EntityId
[[nodiscard]] constexpr u32 entityIndex(EntityId id) noexcept {
    return static_cast<u32>(id.value & 0xFFFFFFFF);
}

/// Extract entity version from EntityId
[[nodiscard]] constexpr u32 entityVersion(EntityId id) noexcept {
    return static_cast<u32>(id.value >> 32);
}

/// Create EntityId from index and version
[[nodiscard]] constexpr EntityId makeEntityId(u32 index, u32 version) noexcept {
    return EntityId{(static_cast<u64>(version) << 32) | index};
}

// =============================================================================
// Component Types
// =============================================================================

/// Component type tag
struct ComponentTypeTag {};

/// Component type ID - unique identifier for component types
using ComponentTypeId = Handle<ComponentTypeTag, u32>;

/// Maximum number of component types (configurable per project)
inline constexpr u32 MAX_COMPONENT_TYPES = 256;

// =============================================================================
// Bitfield Operations
// =============================================================================

/// @brief Type-safe bitfield wrapper for enum flags
/// @tparam E Enum type to wrap
template<typename E>
    requires std::is_enum_v<E>
struct Flags {
    using underlying_type = std::underlying_type_t<E>;
    
    underlying_type value;
    
    constexpr Flags() noexcept : value(0) {}
    constexpr Flags(E e) noexcept : value(static_cast<underlying_type>(e)) {}
    constexpr explicit Flags(underlying_type v) noexcept : value(v) {}
    
    [[nodiscard]] constexpr bool has(E e) const noexcept {
        auto mask = static_cast<underlying_type>(e);
        return (value & mask) == mask;
    }
    
    constexpr Flags& set(E e) noexcept {
        value |= static_cast<underlying_type>(e);
        return *this;
    }
    
    constexpr Flags& clear(E e) noexcept {
        value &= ~static_cast<underlying_type>(e);
        return *this;
    }
    
    constexpr Flags& toggle(E e) noexcept {
        value ^= static_cast<underlying_type>(e);
        return *this;
    }
    
    [[nodiscard]] constexpr bool isEmpty() const noexcept {
        return value == 0;
    }
    
    constexpr Flags operator|(Flags other) const noexcept {
        return Flags{static_cast<underlying_type>(value | other.value)};
    }
    
    constexpr Flags operator&(Flags other) const noexcept {
        return Flags{static_cast<underlying_type>(value & other.value)};
    }
    
    constexpr Flags operator^(Flags other) const noexcept {
        return Flags{static_cast<underlying_type>(value ^ other.value)};
    }
    
    constexpr Flags operator~() const noexcept {
        return Flags{static_cast<underlying_type>(~value)};
    }
    
    constexpr Flags& operator|=(Flags other) noexcept {
        value |= other.value;
        return *this;
    }
    
    constexpr Flags& operator&=(Flags other) noexcept {
        value &= other.value;
        return *this;
    }
    
    constexpr Flags& operator^=(Flags other) noexcept {
        value ^= other.value;
        return *this;
    }
    
    [[nodiscard]] constexpr auto operator<=>(const Flags&) const noexcept = default;
};

/// Macro to enable bitwise operators for an enum
#define NOVA_ENABLE_BITMASK_OPERATORS(EnumType) \
    [[nodiscard]] constexpr ::nova::Flags<EnumType> operator|(EnumType a, EnumType b) noexcept { \
        return ::nova::Flags<EnumType>{a} | ::nova::Flags<EnumType>{b}; \
    } \
    [[nodiscard]] constexpr ::nova::Flags<EnumType> operator&(EnumType a, EnumType b) noexcept { \
        return ::nova::Flags<EnumType>{a} & ::nova::Flags<EnumType>{b}; \
    } \
    [[nodiscard]] constexpr ::nova::Flags<EnumType> operator^(EnumType a, EnumType b) noexcept { \
        return ::nova::Flags<EnumType>{a} ^ ::nova::Flags<EnumType>{b}; \
    } \
    [[nodiscard]] constexpr ::nova::Flags<EnumType> operator~(EnumType a) noexcept { \
        return ~::nova::Flags<EnumType>{a}; \
    }

// =============================================================================
// Memory Size Literals
// =============================================================================

namespace literals {
    /// Kilobyte literal (1024 bytes)
    [[nodiscard]] consteval usize operator""_KB(unsigned long long value) noexcept {
        return static_cast<usize>(value) * 1024ULL;
    }
    
    /// Megabyte literal (1024 * 1024 bytes)
    [[nodiscard]] consteval usize operator""_MB(unsigned long long value) noexcept {
        return static_cast<usize>(value) * 1024ULL * 1024ULL;
    }
    
    /// Gigabyte literal (1024 * 1024 * 1024 bytes)
    [[nodiscard]] consteval usize operator""_GB(unsigned long long value) noexcept {
        return static_cast<usize>(value) * 1024ULL * 1024ULL * 1024ULL;
    }
    
    /// Degrees to radians literal
    [[nodiscard]] consteval f32 operator""_deg(long double value) noexcept {
        return static_cast<f32>(value) * math::DEG_TO_RAD_F32;
    }
    
    /// Degrees to radians literal (integer)
    [[nodiscard]] consteval f32 operator""_deg(unsigned long long value) noexcept {
        return static_cast<f32>(value) * math::DEG_TO_RAD_F32;
    }
} // namespace literals

// =============================================================================
// Utility Functions
// =============================================================================

/// @brief Compute hash for a type (FNV-1a algorithm)
/// @param data Pointer to data
/// @param size Size in bytes
/// @return 64-bit hash value
[[nodiscard]] constexpr u64 fnv1aHash(const void* data, usize size) noexcept {
    constexpr u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr u64 FNV_PRIME = 1099511628211ULL;
    
    auto bytes = static_cast<const u8*>(data);
    u64 hash = FNV_OFFSET_BASIS;
    
    for (usize i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }
    
    return hash;
}

/// @brief Compile-time string hash
/// @param str String literal
/// @return 64-bit hash value
[[nodiscard]] consteval u64 constHash(const char* str) noexcept {
    constexpr u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr u64 FNV_PRIME = 1099511628211ULL;
    
    u64 hash = FNV_OFFSET_BASIS;
    while (*str) {
        hash ^= static_cast<u64>(*str++);
        hash *= FNV_PRIME;
    }
    
    return hash;
}

/// @brief Runtime string hash (FNV-1a)
/// @param str Null-terminated string
/// @return 64-bit hash value
[[nodiscard]] inline u64 runtimeHash(const char* str) noexcept {
    constexpr u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr u64 FNV_PRIME = 1099511628211ULL;
    
    u64 hash = FNV_OFFSET_BASIS;
    while (*str) {
        hash ^= static_cast<u64>(*str++);
        hash *= FNV_PRIME;
    }
    
    return hash;
}

/// @brief Align value up to given alignment
/// @param value Value to align
/// @param alignment Alignment (must be power of 2)
/// @return Aligned value
template<typename T>
    requires std::is_integral_v<T>
[[nodiscard]] constexpr T alignUp(T value, T alignment) noexcept {
    return (value + alignment - 1) & ~(alignment - 1);
}

/// @brief Align value down to given alignment
/// @param value Value to align
/// @param alignment Alignment (must be power of 2)
/// @return Aligned value
template<typename T>
    requires std::is_integral_v<T>
[[nodiscard]] constexpr T alignDown(T value, T alignment) noexcept {
    return value & ~(alignment - 1);
}

/// @brief Check if value is power of two
/// @param value Value to check
/// @return True if power of two
template<typename T>
    requires std::is_integral_v<T>
[[nodiscard]] constexpr bool isPowerOfTwo(T value) noexcept {
    return value > 0 && (value & (value - 1)) == 0;
}

/// @brief Get next power of two >= value
/// @param value Input value
/// @return Next power of two
/// @note Early return for 0 is required to prevent underflow in (value - 1)
[[nodiscard]] constexpr u32 nextPowerOfTwo(u32 value) noexcept {
    if (value == 0) return 1;  // Prevents underflow in (value - 1)
    return static_cast<u32>(1) << (32 - std::countl_zero(value - 1));
}

/// @brief Get next power of two >= value (64-bit)
/// @param value Input value
/// @return Next power of two
/// @note Early return for 0 is required to prevent underflow in (value - 1)
[[nodiscard]] constexpr u64 nextPowerOfTwo(u64 value) noexcept {
    if (value == 0) return 1;  // Prevents underflow in (value - 1)
    return static_cast<u64>(1) << (64 - std::countl_zero(value - 1));
}

} // namespace nova
