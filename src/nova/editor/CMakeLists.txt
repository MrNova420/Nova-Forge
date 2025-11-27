# NovaCore Editor™ - Build Configuration
# NovaForge Platform | NovaCore Engine
# Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)

# Editor library sources
set(NOVA_EDITOR_SOURCES
    editor_context.cpp
    editor_command.cpp
    editor_gizmo.cpp
)

# Editor library headers
set(NOVA_EDITOR_HEADERS
    ${CMAKE_SOURCE_DIR}/include/nova/editor/editor.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/editor/editor_types.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/editor/editor_context.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/editor/editor_command.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/editor/editor_panel.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/editor/editor_gizmo.hpp
)

# Create editor library
add_library(nova_editor STATIC
    ${NOVA_EDITOR_SOURCES}
    ${NOVA_EDITOR_HEADERS}
)

# Include directories
target_include_directories(nova_editor
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(nova_editor
    PUBLIC
        nova_core
)

# C++23 features
target_compile_features(nova_editor PUBLIC cxx_std_23)

# Compiler warnings
if(MSVC)
    target_compile_options(nova_editor PRIVATE /W4)
else()
    target_compile_options(nova_editor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Platform-specific configuration
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    # Android-specific settings for mobile editor
    target_compile_definitions(nova_editor PRIVATE NOVA_PLATFORM_ANDROID)
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # iOS-specific settings
    target_compile_definitions(nova_editor PRIVATE NOVA_PLATFORM_IOS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(nova_editor PRIVATE NOVA_PLATFORM_LINUX)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(nova_editor PRIVATE NOVA_PLATFORM_WINDOWS)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(nova_editor PRIVATE NOVA_PLATFORM_MACOS)
endif()

# IDE source groups
source_group("Source Files" FILES ${NOVA_EDITOR_SOURCES})
source_group("Header Files" FILES ${NOVA_EDITOR_HEADERS})
