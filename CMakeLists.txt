# ============================================================================
# NovaCore Engine - Root CMake Configuration
# ============================================================================
# Platform: NovaForge | Engine: NovaCore
# Company: WeNova Interactive (operating as Kayden Shawn Massengill)
# Technology: C++23 (95%) + Mojo (3%) + Rust (2%)
# Target: 10+ Billion Devices, Mobile-First, Universal Platform
# ============================================================================

cmake_minimum_required(VERSION 3.20)

# Project definition
project(NovaCore
    VERSION 0.1.0
    DESCRIPTION "NovaCore - World's Best Mobile-First Game Engine"
    HOMEPAGE_URL "https://novaforge.dev"
    LANGUAGES CXX
)

# ============================================================================
# C++23 Standard Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Build Types
# ============================================================================
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile;Shipping" CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Release, Profile, Shipping)" FORCE)
endif()

# ============================================================================
# Platform Detection
# ============================================================================
if(ANDROID)
    set(NOVA_PLATFORM "Android")
    set(NOVA_GRAPHICS_API "Vulkan")
    message(STATUS "NovaCore: Building for Android with Vulkan")
elseif(IOS)
    set(NOVA_PLATFORM "iOS")
    set(NOVA_GRAPHICS_API "Metal")
    message(STATUS "NovaCore: Building for iOS with Metal")
elseif(EMSCRIPTEN)
    set(NOVA_PLATFORM "Web")
    set(NOVA_GRAPHICS_API "WebGPU")
    message(STATUS "NovaCore: Building for Web with WebGPU")
elseif(WIN32)
    set(NOVA_PLATFORM "Windows")
    set(NOVA_GRAPHICS_API "Vulkan")
    message(STATUS "NovaCore: Building for Windows with Vulkan")
elseif(APPLE)
    set(NOVA_PLATFORM "macOS")
    set(NOVA_GRAPHICS_API "Metal")
    message(STATUS "NovaCore: Building for macOS with Metal")
elseif(UNIX)
    set(NOVA_PLATFORM "Linux")
    set(NOVA_GRAPHICS_API "Vulkan")
    message(STATUS "NovaCore: Building for Linux with Vulkan")
else()
    set(NOVA_PLATFORM "Unknown")
    set(NOVA_GRAPHICS_API "OpenGL")
    message(WARNING "NovaCore: Unknown platform, falling back to OpenGL")
endif()

# ============================================================================
# Compiler Flags
# ============================================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    # Common flags for Clang and GCC
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wformat=2
    )
    
    # NovaCore Quality Standard: Warnings as errors in Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Shipping")
        add_compile_options(-Werror)
    endif()
    
    # No exceptions (use std::expected) - NovaCore standard
    add_compile_options(-fno-exceptions)
    
    # No RTTI (custom reflection system) - NovaCore standard
    add_compile_options(-fno-rtti)
    
    # Build-type specific flags
    add_compile_options(
        $<$<CONFIG:Debug>:-g -O0 -DNOVA_DEBUG=1>
        $<$<CONFIG:Release>:-O3 -DNDEBUG -flto>
        $<$<CONFIG:Profile>:-O2 -g -fno-omit-frame-pointer -DNOVA_PROFILE=1>
        $<$<CONFIG:Shipping>:-O3 -DNDEBUG -flto -DNOVA_SHIPPING=1>
    )
    
    # ARM NEON SIMD optimization for mobile
    if(NOVA_PLATFORM STREQUAL "Android" OR NOVA_PLATFORM STREQUAL "iOS")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            # ARM64 has NEON by default, enable vectorization
            add_compile_options(-ftree-vectorize)
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
            # ARM32 requires explicit NEON flags
            add_compile_options(-mfpu=neon -mfloat-abi=softfp)
        endif()
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC flags
    add_compile_options(
        /W4
        /permissive-
        /Zc:__cplusplus
        /EHsc
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Shipping")
        add_compile_options(/WX)  # Warnings as errors
    endif()
    
    add_compile_options(
        $<$<CONFIG:Debug>:/Od /Zi /DNOVA_DEBUG=1>
        $<$<CONFIG:Release>:/O2 /DNDEBUG /GL>
        $<$<CONFIG:Profile>:/O2 /Zi /DNOVA_PROFILE=1>
        $<$<CONFIG:Shipping>:/O2 /DNDEBUG /GL /DNOVA_SHIPPING=1>
    )
endif()

# ============================================================================
# Include Directories
# ============================================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# ============================================================================
# Options
# ============================================================================
option(NOVA_BUILD_TESTS "Build NovaCore unit tests" ON)
option(NOVA_BUILD_EXAMPLES "Build NovaCore examples" ON)
option(NOVA_BUILD_EDITOR "Build NovaCore editor" ON)
option(NOVA_ENABLE_PROFILING "Enable Tracy profiler integration" ON)

# ============================================================================
# Define NovaCore Library
# ============================================================================
add_library(NovaCore STATIC
    # Core Types
    src/core/types/types.cpp
    
    # Memory System
    src/core/memory/allocator.cpp
    
    # Math Library
    src/core/math/math.cpp
    
    # Logging System
    src/core/logging/logger.cpp
    
    # Platform Abstraction
    src/core/platform/platform.cpp
)

target_include_directories(NovaCore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Platform-specific definitions
target_compile_definitions(NovaCore PUBLIC
    NOVA_PLATFORM_${NOVA_PLATFORM}
    NOVA_GRAPHICS_${NOVA_GRAPHICS_API}
)

# ============================================================================
# Tests
# ============================================================================
if(NOVA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# Installation
# ============================================================================
install(TARGETS NovaCore
    EXPORT NovaCoreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/nova
    DESTINATION include
)

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "===============================================")
message(STATUS "NovaCore Engine v${PROJECT_VERSION}")
message(STATUS "===============================================")
message(STATUS "Platform:        ${NOVA_PLATFORM}")
message(STATUS "Graphics API:    ${NOVA_GRAPHICS_API}")
message(STATUS "Build Type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:    C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:        ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Tests:           ${NOVA_BUILD_TESTS}")
message(STATUS "Examples:        ${NOVA_BUILD_EXAMPLES}")
message(STATUS "Editor:          ${NOVA_BUILD_EDITOR}")
message(STATUS "Profiling:       ${NOVA_ENABLE_PROFILING}")
message(STATUS "===============================================")
message(STATUS "")
