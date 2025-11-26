# =============================================================================
# NovaCore Engine - Android NDK Toolchain Configuration
# =============================================================================
# Platform: NovaForge | Engine: NovaCore
# Company: WeNova Interactive (operating as Kayden Shawn Massengill)
#
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/android.toolchain.cmake ..
#
# This file wraps the official Android NDK toolchain with NovaCore-specific
# settings optimized for mobile game engine development.
# =============================================================================

cmake_minimum_required(VERSION 3.28)

# =============================================================================
# Validate NDK Environment
# =============================================================================
if(NOT DEFINED ENV{ANDROID_NDK} AND NOT DEFINED ANDROID_NDK)
    # Try common NDK locations
    if(EXISTS "$ENV{ANDROID_HOME}/ndk-bundle")
        set(ANDROID_NDK "$ENV{ANDROID_HOME}/ndk-bundle")
    elseif(EXISTS "$ENV{ANDROID_SDK_ROOT}/ndk-bundle")
        set(ANDROID_NDK "$ENV{ANDROID_SDK_ROOT}/ndk-bundle")
    elseif(EXISTS "/usr/local/lib/android/sdk/ndk-bundle")
        set(ANDROID_NDK "/usr/local/lib/android/sdk/ndk-bundle")
    else()
        message(FATAL_ERROR 
            "Android NDK not found!\n"
            "Please set ANDROID_NDK environment variable or pass -DANDROID_NDK=<path>\n"
            "Download NDK from: https://developer.android.com/ndk/downloads"
        )
    endif()
elseif(DEFINED ENV{ANDROID_NDK} AND NOT DEFINED ANDROID_NDK)
    set(ANDROID_NDK "$ENV{ANDROID_NDK}")
endif()

message(STATUS "NovaCore Android Toolchain")
message(STATUS "  NDK Path: ${ANDROID_NDK}")

# =============================================================================
# NovaCore Android Configuration
# =============================================================================

# Target Android version
# Android 7.0 (API 24) for wide device support with modern features
# - Vulkan 1.0 support
# - NEON SIMD available on all ARMv7a and ARM64 devices
# - Sufficient for 95%+ of active Android devices (2024 data)
if(NOT DEFINED ANDROID_PLATFORM)
    set(ANDROID_PLATFORM "android-24" CACHE STRING "Target Android platform")
endif()

# Target ABI - ARM64 is primary, ARMv7a for legacy support
# ARM64 (arm64-v8a): Primary target - all 2017+ devices
# ARMv7a (armeabi-v7a): Legacy support - older 32-bit devices
if(NOT DEFINED ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "Target Android ABI")
endif()

# STL configuration - Use libc++ shared for C++23 support
if(NOT DEFINED ANDROID_STL)
    set(ANDROID_STL "c++_shared" CACHE STRING "Android STL")
endif()

# ARM mode - Use ARM mode for better performance (vs Thumb)
if(NOT DEFINED ANDROID_ARM_MODE)
    set(ANDROID_ARM_MODE "arm" CACHE STRING "ARM mode")
endif()

# NEON support - Enable for SIMD optimizations
if(NOT DEFINED ANDROID_ARM_NEON)
    set(ANDROID_ARM_NEON ON CACHE BOOL "Enable NEON SIMD")
endif()

# =============================================================================
# Include Official NDK Toolchain
# =============================================================================
set(ANDROID_NDK_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")

if(NOT EXISTS "${ANDROID_NDK_TOOLCHAIN_FILE}")
    message(FATAL_ERROR 
        "NDK toolchain file not found at: ${ANDROID_NDK_TOOLCHAIN_FILE}\n"
        "Please ensure Android NDK is properly installed."
    )
endif()

include("${ANDROID_NDK_TOOLCHAIN_FILE}")

# =============================================================================
# NovaCore-Specific Compiler Flags
# =============================================================================

# Performance flags for game engine development
set(NOVA_ANDROID_FLAGS "")

# ARM64 optimizations
if(ANDROID_ABI STREQUAL "arm64-v8a")
    list(APPEND NOVA_ANDROID_FLAGS
        -march=armv8-a       # ARMv8-A baseline
        -mtune=cortex-a76    # Tune for common cores (Cortex-A76/A77/A78)
        -O3                  # Maximum optimization
        -ftree-vectorize     # Auto-vectorization
        -ffast-math          # Fast math (acceptable for games)
        -fno-math-errno      # Don't set errno for math functions
    )
    
# ARMv7a optimizations  
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    list(APPEND NOVA_ANDROID_FLAGS
        -march=armv7-a       # ARMv7-A baseline
        -mfpu=neon-vfpv4     # NEON + VFPv4 FPU
        -mfloat-abi=softfp   # Soft float ABI with hardware FP
        -O3                  # Maximum optimization
        -ftree-vectorize     # Auto-vectorization
        -ffast-math          # Fast math
    )
endif()

# Common optimization flags
list(APPEND NOVA_ANDROID_FLAGS
    -ffunction-sections      # Put each function in its own section
    -fdata-sections          # Put each data item in its own section
    -fvisibility=hidden      # Hide symbols by default
    -fvisibility-inlines-hidden
    -fno-exceptions          # No C++ exceptions (use std::expected)
    -fno-rtti                # No RTTI (custom reflection system)
)

# Link-time optimization for Release builds
if(CMAKE_BUILD_TYPE MATCHES "Release|Shipping")
    list(APPEND NOVA_ANDROID_FLAGS -flto)
endif()

# Linker flags for smaller binary size
set(NOVA_ANDROID_LINKER_FLAGS
    -Wl,--gc-sections        # Remove unused sections
    -Wl,--as-needed          # Only link needed libraries
    -Wl,-z,noexecstack       # Non-executable stack (security)
    -Wl,-z,relro             # Read-only relocations (security)
    -Wl,-z,now               # Resolve all symbols at load time
)

# Apply flags
string(REPLACE ";" " " NOVA_ANDROID_FLAGS_STR "${NOVA_ANDROID_FLAGS}")
string(REPLACE ";" " " NOVA_ANDROID_LINKER_FLAGS_STR "${NOVA_ANDROID_LINKER_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NOVA_ANDROID_FLAGS_STR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NOVA_ANDROID_FLAGS_STR}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${NOVA_ANDROID_LINKER_FLAGS_STR}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${NOVA_ANDROID_LINKER_FLAGS_STR}")

# =============================================================================
# Graphics API Configuration
# =============================================================================

# Vulkan is available on Android 7.0+ (API 24+)
set(NOVA_HAS_VULKAN ON CACHE BOOL "Vulkan support available")

# OpenGL ES 3.0 fallback for devices without Vulkan
set(NOVA_HAS_OPENGLES ON CACHE BOOL "OpenGL ES support available")

# =============================================================================
# Summary
# =============================================================================
message(STATUS "  Platform:    ${ANDROID_PLATFORM}")
message(STATUS "  ABI:         ${ANDROID_ABI}")
message(STATUS "  STL:         ${ANDROID_STL}")
message(STATUS "  NEON:        ${ANDROID_ARM_NEON}")
message(STATUS "  Vulkan:      ${NOVA_HAS_VULKAN}")
message(STATUS "  OpenGL ES:   ${NOVA_HAS_OPENGLES}")
