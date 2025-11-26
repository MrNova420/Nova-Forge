# ============================================================================
# NovaCore Engine - Android NDK Toolchain Configuration
# ============================================================================
# Platform: NovaForge | Engine: NovaCore
# Company: WeNova Interactive (operating as Kayden Shawn Massengill)
# Target: Android 6.0+ (API 23+), ARM64-v8a primary, ARMv7-a fallback
# ============================================================================

# This file configures CMake for Android NDK cross-compilation
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/android.toolchain.cmake ...

# ============================================================================
# Android Platform Settings
# ============================================================================

# Minimum Android API level (Android 6.0 Marshmallow)
# This enables support for ~97% of active Android devices
set(ANDROID_PLATFORM android-24 CACHE STRING "Android platform level")

# Primary ABI: ARM64-v8a for modern devices (2016+)
# Supports 95%+ of active Android devices as of 2024
set(ANDROID_ABI arm64-v8a CACHE STRING "Android ABI")

# Use shared C++ STL for smaller binary size
set(ANDROID_STL c++_shared CACHE STRING "Android STL")

# ============================================================================
# Compiler Configuration
# ============================================================================

# Use Clang (default in modern NDK)
set(ANDROID_TOOLCHAIN clang CACHE STRING "Android toolchain")

# ARM mode (vs Thumb) for better performance on ARM64
set(ANDROID_ARM_MODE arm CACHE STRING "Android ARM mode")

# ============================================================================
# NovaCore-Specific Android Settings
# ============================================================================

# Enable ARM NEON SIMD intrinsics
# Note: NEON is mandatory on ARM64, optional on ARM32
set(ANDROID_ARM_NEON ON CACHE BOOL "Enable ARM NEON SIMD")

# Enable hardware floating-point
set(ANDROID_ARM_HARD_FLOAT OFF CACHE BOOL "Use hard float ABI")

# ============================================================================
# Include NDK's CMake toolchain
# ============================================================================
# This must be set before project() is called in the main CMakeLists.txt
# The actual NDK toolchain file path is typically:
# ${ANDROID_NDK}/build/cmake/android.toolchain.cmake

if(DEFINED ENV{ANDROID_NDK})
    set(ANDROID_NDK $ENV{ANDROID_NDK})
elseif(DEFINED ENV{ANDROID_NDK_HOME})
    set(ANDROID_NDK $ENV{ANDROID_NDK_HOME})
elseif(DEFINED ENV{NDK_ROOT})
    set(ANDROID_NDK $ENV{NDK_ROOT})
endif()

if(ANDROID_NDK)
    set(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake" CACHE PATH "Android NDK toolchain")
    message(STATUS "NovaCore: Using Android NDK at ${ANDROID_NDK}")
else()
    message(WARNING "NovaCore: ANDROID_NDK not set. Set ANDROID_NDK environment variable.")
endif()

# ============================================================================
# Vulkan Configuration for Android
# ============================================================================

# Use Vulkan as primary graphics API on Android
set(NOVA_GRAPHICS_API "Vulkan" CACHE STRING "Graphics API")

# Vulkan 1.1 is available on Android 8.0+ (API 26)
# Vulkan 1.0 is available on Android 7.0+ (API 24)
# We target Vulkan 1.0 for maximum compatibility
set(NOVA_VULKAN_VERSION "1.0" CACHE STRING "Vulkan version")

# ============================================================================
# Build Variants
# ============================================================================

# ABI variants for distribution
set(NOVA_ANDROID_ABIS
    arm64-v8a    # Primary: 64-bit ARM (2016+)
    armeabi-v7a  # Fallback: 32-bit ARM (2010+)
    CACHE STRING "Android ABIs to build"
)

# ============================================================================
# Performance Optimization
# ============================================================================

# Link-time optimization for release builds
set(ANDROID_LTO ON CACHE BOOL "Enable LTO for release builds")

# Strip debug symbols in release builds
set(ANDROID_STRIP ON CACHE BOOL "Strip symbols in release")

# ============================================================================
# Debug Settings
# ============================================================================

# Enable Android AddressSanitizer for debug builds
set(NOVA_ANDROID_ASAN OFF CACHE BOOL "Enable AddressSanitizer")

# ============================================================================
# Output Settings
# ============================================================================

# Output APK settings
set(NOVA_ANDROID_APP_NAME "NovaForge" CACHE STRING "Android app name")
set(NOVA_ANDROID_PACKAGE_NAME "dev.novaforge.engine" CACHE STRING "Android package name")
set(NOVA_ANDROID_VERSION_CODE 1 CACHE STRING "Android version code")
set(NOVA_ANDROID_VERSION_NAME "0.1.0" CACHE STRING "Android version name")
