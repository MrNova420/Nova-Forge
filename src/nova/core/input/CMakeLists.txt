# NovaCore Input System™ - Build Configuration
# NovaForge Platform | NovaCore Engine
# Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)

set(NOVA_INPUT_SOURCES
    input_system.cpp
)

set(NOVA_INPUT_HEADERS
    ${CMAKE_SOURCE_DIR}/include/nova/core/input/input.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/core/input/input_types.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/core/input/input_system.hpp
)

add_library(nova_input STATIC
    ${NOVA_INPUT_SOURCES}
    ${NOVA_INPUT_HEADERS}
)

target_include_directories(nova_input
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nova_input
    PUBLIC
        nova_core
)

target_compile_features(nova_input PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(nova_input PRIVATE /W4)
else()
    target_compile_options(nova_input PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Platform-specific
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_compile_definitions(nova_input PRIVATE NOVA_PLATFORM_ANDROID)
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_compile_definitions(nova_input PRIVATE NOVA_PLATFORM_IOS)
endif()

source_group("Source Files" FILES ${NOVA_INPUT_SOURCES})
source_group("Header Files" FILES ${NOVA_INPUT_HEADERS})
