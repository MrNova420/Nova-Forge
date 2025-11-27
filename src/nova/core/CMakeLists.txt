# =============================================================================
# NovaCore Engine - Core Module
# =============================================================================
# Platform: NovaForge | Engine: NovaCore
# Company: WeNova Interactive (operating as Kayden Shawn Massengill)
#
# The Core module provides fundamental types, memory management, math library,
# logging, and platform abstraction. This is the foundation of NovaCore.
# =============================================================================

# =============================================================================
# Core Library Sources
# =============================================================================

# Types module
set(NOVA_CORE_TYPES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/types/types.cpp
)

set(NOVA_CORE_TYPES_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/types/types.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/types/result.hpp
)

# Memory module
set(NOVA_CORE_MEMORY_SOURCES
    # Header-only for now
)

set(NOVA_CORE_MEMORY_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/memory/memory.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/memory/allocator.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/memory/linear_allocator.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/memory/pool_allocator.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/memory/stack_allocator.hpp
)

# Math module
set(NOVA_CORE_MATH_SOURCES
    # Header-only for now
)

set(NOVA_CORE_MATH_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/math/math.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/math/math_common.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/math/vec2.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/math/vec3.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/math/vec4.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/math/mat3.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/math/mat4.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/math/quat.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/math/transform.hpp
)

# Logging module
set(NOVA_CORE_LOGGING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/logging/logger.cpp
)

set(NOVA_CORE_LOGGING_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/logging/logging.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/logging/logger.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/logging/profiler.hpp
)

# ECS module
set(NOVA_CORE_ECS_SOURCES
    # Header-only for now
)

set(NOVA_CORE_ECS_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/ecs/ecs.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/ecs/entity.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/ecs/component.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/ecs/archetype.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/ecs/world.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/ecs/query.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/ecs/system.hpp
)

# Render module
set(NOVA_CORE_RENDER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/render/render_device.cpp
)

set(NOVA_CORE_RENDER_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/render/render.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/render_types.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/render_device.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/render_context.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/render_pass.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/render_pipeline.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/command_buffer.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/swap_chain.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/buffer.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/texture.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/shader.hpp
)

# Nova GraphicsCore™ - Vulkan backend
set(NOVA_CORE_VULKAN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_swap_chain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_command_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_pipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_descriptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/render/vulkan/vulkan_resource_registry.cpp
)

set(NOVA_CORE_VULKAN_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_types.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_loader.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_device.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_swap_chain.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_command_buffer.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_pipeline.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_descriptor.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_buffer.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_shader.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_texture.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/render/vulkan/vulkan_resource_registry.hpp
)

# NovaCore Physics System - Week 7
set(NOVA_CORE_PHYSICS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/physics/collision_shape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/physics/rigid_body.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/physics/physics_world.cpp
)

set(NOVA_CORE_PHYSICS_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/physics/physics.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/physics/physics_types.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/physics/collision_shape.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/physics/rigid_body.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/physics/physics_world.hpp
)

# NovaCore Animation System
set(NOVA_CORE_ANIMATION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/animation/animation_system.cpp
)

set(NOVA_CORE_ANIMATION_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/animation/animation.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/animation/animation_types.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/animation/animation_system.hpp
)

# NovaCore Particle System
set(NOVA_CORE_PARTICLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/particle/particle_system.cpp
)

set(NOVA_CORE_PARTICLE_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/particle/particle.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/particle/particle_types.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/particle/particle_system.hpp
)

# NovaCore Network System
set(NOVA_CORE_NETWORK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/network/network_system.cpp
)

set(NOVA_CORE_NETWORK_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/core/network/network.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/network/network_types.hpp
    ${NOVA_INCLUDE_DIR}/nova/core/network/network_system.hpp
)

# Platform module
set(NOVA_CORE_PLATFORM_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/platform/platform.cpp
)

set(NOVA_CORE_PLATFORM_HEADERS
    # ${NOVA_INCLUDE_DIR}/nova/core/platform/platform.hpp
)

# Containers module
set(NOVA_CORE_CONTAINERS_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/containers/array.cpp
)

set(NOVA_CORE_CONTAINERS_HEADERS
    # ${NOVA_INCLUDE_DIR}/nova/core/containers/array.hpp
)

# =============================================================================
# Aggregate all sources
# =============================================================================
set(NOVA_CORE_SOURCES
    ${NOVA_CORE_TYPES_SOURCES}
    ${NOVA_CORE_MEMORY_SOURCES}
    ${NOVA_CORE_MATH_SOURCES}
    ${NOVA_CORE_LOGGING_SOURCES}
    ${NOVA_CORE_ECS_SOURCES}
    ${NOVA_CORE_RENDER_SOURCES}
    ${NOVA_CORE_VULKAN_SOURCES}
    ${NOVA_CORE_PHYSICS_SOURCES}
    ${NOVA_CORE_ANIMATION_SOURCES}
    ${NOVA_CORE_PARTICLE_SOURCES}
    ${NOVA_CORE_NETWORK_SOURCES}
    ${NOVA_CORE_PLATFORM_SOURCES}
    ${NOVA_CORE_CONTAINERS_SOURCES}
)

set(NOVA_CORE_HEADERS
    ${NOVA_CORE_TYPES_HEADERS}
    ${NOVA_CORE_MEMORY_HEADERS}
    ${NOVA_CORE_MATH_HEADERS}
    ${NOVA_CORE_LOGGING_HEADERS}
    ${NOVA_CORE_ECS_HEADERS}
    ${NOVA_CORE_RENDER_HEADERS}
    ${NOVA_CORE_VULKAN_HEADERS}
    ${NOVA_CORE_PHYSICS_HEADERS}
    ${NOVA_CORE_ANIMATION_HEADERS}
    ${NOVA_CORE_PARTICLE_HEADERS}
    ${NOVA_CORE_NETWORK_HEADERS}
    ${NOVA_CORE_PLATFORM_HEADERS}
    ${NOVA_CORE_CONTAINERS_HEADERS}
)

# =============================================================================
# Create Core Library
# =============================================================================
add_library(nova_core STATIC
    ${NOVA_CORE_SOURCES}
    ${NOVA_CORE_HEADERS}
)

# Alias for cleaner usage
add_library(Nova::Core ALIAS nova_core)

# =============================================================================
# Target Properties
# =============================================================================
target_include_directories(nova_core
    PUBLIC
        $<BUILD_INTERFACE:${NOVA_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# C++23 requirement
target_compile_features(nova_core PUBLIC cxx_std_23)

# Compile definitions
target_compile_definitions(nova_core
    PUBLIC
        $<$<CONFIG:Debug>:NOVA_DEBUG=1>
        $<$<CONFIG:Release>:NOVA_RELEASE=1>
        $<$<CONFIG:Profile>:NOVA_PROFILE=1>
        $<$<CONFIG:Shipping>:NOVA_SHIPPING=1>
    PRIVATE
        NOVA_CORE_BUILDING=1
)

# Compile options
target_compile_options(nova_core
    PRIVATE
        ${NOVA_COMMON_WARNINGS}
        ${NOVA_PERF_FLAGS}
        $<$<CONFIG:Debug>:${NOVA_DEBUG_FLAGS}>
        $<$<CONFIG:Release>:${NOVA_RELEASE_FLAGS}>
        $<$<CONFIG:Profile>:${NOVA_PROFILE_FLAGS}>
        $<$<CONFIG:Shipping>:${NOVA_SHIPPING_FLAGS}>
)

# =============================================================================
# Platform-Specific Configuration
# =============================================================================
if(NOVA_PLATFORM_ANDROID)
    # Android-specific libraries
    target_link_libraries(nova_core
        PRIVATE
            log          # Android logging
            android      # Android native activity
            dl           # Dynamic loading for Vulkan
    )
elseif(NOVA_PLATFORM_IOS)
    # iOS-specific configuration
    # (handled by Xcode project settings)
elseif(NOVA_PLATFORM_WEB)
    # Web-specific configuration
    # (handled by Emscripten settings)
elseif(UNIX)
    # Linux/Unix - need dl for dynamic library loading
    target_link_libraries(nova_core
        PRIVATE
            dl           # Dynamic loading for Vulkan
    )
elseif(WIN32)
    # Windows - no extra libraries needed (uses LoadLibrary)
endif()

# =============================================================================
# Installation
# =============================================================================
install(TARGETS nova_core
    EXPORT NovaCoreTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${NOVA_INCLUDE_DIR}/nova/core
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nova
)
