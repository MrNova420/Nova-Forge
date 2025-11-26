# =============================================================================
# NovaCore Engine - Root CMake Configuration
# =============================================================================
# Platform: NovaForge | Engine: NovaCore
# Company: WeNova Interactive (operating as Kayden Shawn Massengill)
# 
# Technology Stack: C++23 (95%) + Mojo (3%) + Rust (2%)
# Target: World's Best, AAA, Enterprise-Grade, Production-Grade Game Engine
# =============================================================================

cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

# =============================================================================
# Project Definition
# =============================================================================
project(NovaCore
    VERSION 0.1.0
    DESCRIPTION "NovaCore Engine - World's Best Mobile-First Game Engine"
    HOMEPAGE_URL "https://github.com/MrNova420/Nova-Forge"
    LANGUAGES CXX
)

# Project metadata
set(NOVACORE_COMPANY "WeNova Interactive")
set(NOVACORE_PLATFORM "NovaForge")
set(NOVACORE_ENGINE "NovaCore")
set(NOVACORE_COPYRIGHT "Copyright (c) 2025 ${NOVACORE_COMPANY}")

# =============================================================================
# C++23 Standard Configuration
# =============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Build Configuration Types
# =============================================================================
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile;Shipping" CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "========================================================")
message(STATUS "NovaCore Engine v${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "========================================================")

# =============================================================================
# Platform Detection
# =============================================================================
set(NOVA_PLATFORM_ANDROID OFF)
set(NOVA_PLATFORM_IOS OFF)
set(NOVA_PLATFORM_WEB OFF)
set(NOVA_PLATFORM_WINDOWS OFF)
set(NOVA_PLATFORM_MACOS OFF)
set(NOVA_PLATFORM_LINUX OFF)

if(ANDROID)
    set(NOVA_PLATFORM_ANDROID ON)
    set(NOVA_PLATFORM_NAME "Android")
    set(NOVA_GRAPHICS_API "Vulkan")
    set(NOVA_GRAPHICS_FALLBACK "OpenGLES")
elseif(IOS)
    set(NOVA_PLATFORM_IOS ON)
    set(NOVA_PLATFORM_NAME "iOS")
    set(NOVA_GRAPHICS_API "Metal")
    set(NOVA_GRAPHICS_FALLBACK "Metal")
elseif(EMSCRIPTEN)
    set(NOVA_PLATFORM_WEB ON)
    set(NOVA_PLATFORM_NAME "Web")
    set(NOVA_GRAPHICS_API "WebGPU")
    set(NOVA_GRAPHICS_FALLBACK "WebGL2")
elseif(WIN32)
    set(NOVA_PLATFORM_WINDOWS ON)
    set(NOVA_PLATFORM_NAME "Windows")
    set(NOVA_GRAPHICS_API "Vulkan")
    set(NOVA_GRAPHICS_FALLBACK "D3D12")
elseif(APPLE)
    set(NOVA_PLATFORM_MACOS ON)
    set(NOVA_PLATFORM_NAME "macOS")
    set(NOVA_GRAPHICS_API "Metal")
    set(NOVA_GRAPHICS_FALLBACK "Metal")
elseif(UNIX)
    set(NOVA_PLATFORM_LINUX ON)
    set(NOVA_PLATFORM_NAME "Linux")
    set(NOVA_GRAPHICS_API "Vulkan")
    set(NOVA_GRAPHICS_FALLBACK "OpenGL")
else()
    message(FATAL_ERROR "Unsupported platform detected")
endif()

message(STATUS "Platform: ${NOVA_PLATFORM_NAME}")
message(STATUS "Graphics API: ${NOVA_GRAPHICS_API}")
message(STATUS "Graphics Fallback: ${NOVA_GRAPHICS_FALLBACK}")

# =============================================================================
# Build Options
# =============================================================================
option(NOVA_BUILD_TESTS "Build NovaCore test suite" ON)
option(NOVA_BUILD_TOOLS "Build NovaCore tools" ON)
option(NOVA_BUILD_EXAMPLES "Build NovaCore examples" OFF)
option(NOVA_BUILD_DOCS "Build NovaCore documentation" OFF)
option(NOVA_ENABLE_PROFILING "Enable Tracy profiler integration" ON)
option(NOVA_ENABLE_ASSERTIONS "Enable runtime assertions" ON)
option(NOVA_ENABLE_SIMD "Enable SIMD optimizations (ARM NEON / x86 AVX)" ON)
option(NOVA_ENABLE_LTO "Enable Link-Time Optimization for Release builds" ON)

# =============================================================================
# Compiler Configuration
# =============================================================================
# Common warning flags for all compilers
set(NOVA_COMMON_WARNINGS "")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    # GCC/Clang specific flags
    set(NOVA_COMMON_WARNINGS
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -Wconversion
        -Wshadow
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
    
    # GCC-specific warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND NOVA_COMMON_WARNINGS
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
        )
    endif()
    
    # Build type specific flags
    set(NOVA_DEBUG_FLAGS -g -O0 -fno-omit-frame-pointer)
    set(NOVA_RELEASE_FLAGS -O3 -DNDEBUG)
    set(NOVA_PROFILE_FLAGS -O2 -g -fno-omit-frame-pointer)
    set(NOVA_SHIPPING_FLAGS -O3 -DNDEBUG -ffunction-sections -fdata-sections)
    
    # LTO for Release/Shipping builds
    if(NOVA_ENABLE_LTO)
        list(APPEND NOVA_RELEASE_FLAGS -flto)
        list(APPEND NOVA_SHIPPING_FLAGS -flto)
    endif()
    
    # Disable exceptions and RTTI for performance (use std::expected instead)
    set(NOVA_PERF_FLAGS
        -fno-exceptions
        -fno-rtti
    )
    
elseif(MSVC)
    # MSVC specific flags
    set(NOVA_COMMON_WARNINGS
        /W4
        /WX
        /permissive-
        /w14640
        /w14826
        /w14928
    )
    
    set(NOVA_DEBUG_FLAGS /Od /Zi /RTC1)
    set(NOVA_RELEASE_FLAGS /O2 /DNDEBUG)
    set(NOVA_PROFILE_FLAGS /O2 /Zi)
    set(NOVA_SHIPPING_FLAGS /O2 /DNDEBUG /GL)
    
    # Disable exceptions and RTTI
    set(NOVA_PERF_FLAGS
        /EHs-c-
        /GR-
    )
endif()

# =============================================================================
# SIMD Configuration
# =============================================================================
if(NOVA_ENABLE_SIMD)
    if(NOVA_PLATFORM_ANDROID OR NOVA_PLATFORM_IOS)
        # ARM NEON - Always available on ARM64 (AArch64)
        # For 32-bit ARM (armeabi-v7a), explicit flags needed
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
                list(APPEND NOVA_PERF_FLAGS -mfpu=neon -mfloat-abi=softfp)
            endif()
        endif()
        set(NOVA_SIMD_TYPE "NEON")
    elseif(NOVA_PLATFORM_WINDOWS OR NOVA_PLATFORM_LINUX OR NOVA_PLATFORM_MACOS)
        # x86/x64 - Check for AVX2 support
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
        if(COMPILER_SUPPORTS_AVX2 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            list(APPEND NOVA_PERF_FLAGS -mavx2 -mfma)
            set(NOVA_SIMD_TYPE "AVX2")
        else()
            set(NOVA_SIMD_TYPE "SSE4.2")
        endif()
    elseif(NOVA_PLATFORM_WEB)
        # WebAssembly SIMD
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            list(APPEND NOVA_PERF_FLAGS -msimd128)
        endif()
        set(NOVA_SIMD_TYPE "WASM_SIMD")
    endif()
    message(STATUS "SIMD: ${NOVA_SIMD_TYPE}")
else()
    set(NOVA_SIMD_TYPE "None")
    message(STATUS "SIMD: Disabled")
endif()

# =============================================================================
# Platform-Specific Toolchain Configuration
# =============================================================================
if(NOVA_PLATFORM_ANDROID)
    # Android NDK configuration
    if(NOT DEFINED ANDROID_NDK)
        message(FATAL_ERROR "ANDROID_NDK not defined. Please set CMAKE_TOOLCHAIN_FILE to android.toolchain.cmake")
    endif()
    
    # Minimum Android API level (Android 7.0 = API 24)
    if(NOT DEFINED ANDROID_PLATFORM)
        set(ANDROID_PLATFORM android-24)
    endif()
    
    # ARM64 primary, ARMv7 for legacy support
    if(NOT DEFINED ANDROID_ABI)
        set(ANDROID_ABI arm64-v8a)
    endif()
    
    # Use libc++
    if(NOT DEFINED ANDROID_STL)
        set(ANDROID_STL c++_shared)
    endif()
    
    message(STATUS "Android Platform: ${ANDROID_PLATFORM}")
    message(STATUS "Android ABI: ${ANDROID_ABI}")
    message(STATUS "Android STL: ${ANDROID_STL}")
endif()

if(NOVA_PLATFORM_IOS)
    # iOS configuration
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Minimum iOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Target architecture")
    
    message(STATUS "iOS Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    message(STATUS "iOS Architecture: ${CMAKE_OSX_ARCHITECTURES}")
endif()

if(NOVA_PLATFORM_WEB)
    # Emscripten configuration
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    
    # WebGPU and WebAssembly settings
    set(NOVA_WEB_FLAGS
        -s USE_WEBGPU=1
        -s WASM=1
        -s ALLOW_MEMORY_GROWTH=1
        -s MAXIMUM_MEMORY=2GB
        -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']
        -s MODULARIZE=1
        -s EXPORT_NAME='NovaCore'
    )
    
    string(REPLACE ";" " " NOVA_WEB_FLAGS_STR "${NOVA_WEB_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NOVA_WEB_FLAGS_STR}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${NOVA_WEB_FLAGS_STR}")
endif()

# =============================================================================
# Include Directories
# =============================================================================
set(NOVA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(NOVA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NOVA_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(NOVA_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(NOVA_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# =============================================================================
# Configure Header Generation
# =============================================================================
configure_file(
    ${NOVA_CMAKE_DIR}/nova_config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/nova/core/config.hpp
    @ONLY
)

# =============================================================================
# Global Include Directories
# =============================================================================
include_directories(
    ${NOVA_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

# =============================================================================
# Third-Party Dependencies (Minimal, Per Blueprint)
# =============================================================================
# Note: NovaCore uses minimal external dependencies
# Most functionality is custom-built for maximum quality and mobile optimization

# Tracy Profiler (optional)
if(NOVA_ENABLE_PROFILING)
    # Tracy will be added as a subdirectory or fetched
    # For now, define the macro to enable profiling code paths
    add_compile_definitions(NOVA_PROFILING_ENABLED=1)
endif()

# =============================================================================
# NovaCore Engine Libraries
# =============================================================================
# Core module (foundation)
add_subdirectory(src/nova/core)

# Entity-Component-Worker system
# add_subdirectory(src/nova/ecw)

# Rendering system
# add_subdirectory(src/nova/render)

# Physics system
# add_subdirectory(src/nova/physics)

# Audio system
# add_subdirectory(src/nova/audio)

# Animation system
# add_subdirectory(src/nova/animation)

# Scripting system
# add_subdirectory(src/nova/script)

# AI system
# add_subdirectory(src/nova/ai)

# Asset pipeline
# add_subdirectory(src/nova/asset)

# Editor (mobile-first)
# add_subdirectory(src/nova/editor)

# Networking
# add_subdirectory(src/nova/net)

# =============================================================================
# Tests
# =============================================================================
if(NOVA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# =============================================================================
# Tools
# =============================================================================
if(NOVA_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# =============================================================================
# Installation
# =============================================================================
include(GNUInstallDirs)

install(DIRECTORY ${NOVA_INCLUDE_DIR}/nova
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/nova/core/config.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nova/core
)

# =============================================================================
# Package Configuration
# =============================================================================
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${NOVA_CMAKE_DIR}/NovaCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/NovaCoreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NovaCore
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/NovaCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/NovaCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NovaCoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NovaCore
)

# =============================================================================
# Summary
# =============================================================================
message(STATUS "========================================================")
message(STATUS "NovaCore Engine Configuration Complete")
message(STATUS "========================================================")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Platform:       ${NOVA_PLATFORM_NAME}")
message(STATUS "  Graphics:       ${NOVA_GRAPHICS_API} (fallback: ${NOVA_GRAPHICS_FALLBACK})")
message(STATUS "  SIMD:           ${NOVA_SIMD_TYPE}")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Tests:    ${NOVA_BUILD_TESTS}")
message(STATUS "  Build Tools:    ${NOVA_BUILD_TOOLS}")
message(STATUS "  Profiling:      ${NOVA_ENABLE_PROFILING}")
message(STATUS "  LTO:            ${NOVA_ENABLE_LTO}")
message(STATUS "========================================================")
