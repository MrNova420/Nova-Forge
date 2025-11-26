// =============================================================================
// NovaCore Engine - Core Types Implementation
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// Implementation file for core types. Most types are header-only for
// performance (inlining), but this file contains any necessary non-inline
// implementations and explicit template instantiations.
// =============================================================================

#include "nova/core/types/types.hpp"
#include "nova/core/types/result.hpp"

namespace nova {

// =============================================================================
// Compile-time Validations
// =============================================================================

// Validate integer type sizes
static_assert(sizeof(i8) == 1, "i8 must be 1 byte");
static_assert(sizeof(i16) == 2, "i16 must be 2 bytes");
static_assert(sizeof(i32) == 4, "i32 must be 4 bytes");
static_assert(sizeof(i64) == 8, "i64 must be 8 bytes");

static_assert(sizeof(u8) == 1, "u8 must be 1 byte");
static_assert(sizeof(u16) == 2, "u16 must be 2 bytes");
static_assert(sizeof(u32) == 4, "u32 must be 4 bytes");
static_assert(sizeof(u64) == 8, "u64 must be 8 bytes");

// Validate floating-point type sizes
static_assert(sizeof(f32) == 4, "f32 must be 4 bytes");
static_assert(sizeof(f64) == 8, "f64 must be 8 bytes");

// Validate pointer sizes are consistent
static_assert(sizeof(void*) == sizeof(uptr), "uptr must match pointer size");
static_assert(sizeof(void*) == sizeof(iptr), "iptr must match pointer size");

// Validate EntityId can encode both index and version
static_assert(sizeof(EntityId::value_type) == 8, "EntityId must be 8 bytes");

// =============================================================================
// Global Constants Validation
// =============================================================================

// Ensure math constants are within expected precision
static_assert(math::PI_F32 > 3.14f && math::PI_F32 < 3.15f, "PI_F32 out of range");
static_assert(math::TAU_F32 > 6.28f && math::TAU_F32 < 6.29f, "TAU_F32 out of range");
static_assert(math::E_F32 > 2.71f && math::E_F32 < 2.72f, "E_F32 out of range");

// =============================================================================
// Utility Function Implementations
// =============================================================================

// Most utility functions are constexpr and defined in the header.
// This file is reserved for any runtime implementations that may be needed.

} // namespace nova

// =============================================================================
// Library Information (for debugging/diagnostics)
// =============================================================================

extern "C" {
    /// Returns the NovaCore version string
    const char* nova_version() {
        return "0.1.0";
    }
    
    /// Returns the NovaCore build configuration
    const char* nova_build_config() {
        #if defined(NOVA_DEBUG)
            return "Debug";
        #elif defined(NOVA_RELEASE)
            return "Release";
        #elif defined(NOVA_PROFILE)
            return "Profile";
        #elif defined(NOVA_SHIPPING)
            return "Shipping";
        #else
            return "Unknown";
        #endif
    }
    
    /// Returns the target platform name
    const char* nova_platform() {
        #if NOVA_PLATFORM_ANDROID
            return "Android";
        #elif NOVA_PLATFORM_IOS
            return "iOS";
        #elif NOVA_PLATFORM_WEB
            return "Web";
        #elif NOVA_PLATFORM_WINDOWS
            return "Windows";
        #elif NOVA_PLATFORM_MACOS
            return "macOS";
        #elif NOVA_PLATFORM_LINUX
            return "Linux";
        #else
            return "Unknown";
        #endif
    }
}
