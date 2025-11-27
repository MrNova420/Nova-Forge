# NovaCore Audio System™ - Build Configuration
# NovaForge Platform | NovaCore Engine
# Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)

set(NOVA_AUDIO_SOURCES
    audio_system.cpp
)

set(NOVA_AUDIO_HEADERS
    ${CMAKE_SOURCE_DIR}/include/nova/core/audio/audio.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/core/audio/audio_types.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/core/audio/audio_system.hpp
)

add_library(nova_audio STATIC
    ${NOVA_AUDIO_SOURCES}
    ${NOVA_AUDIO_HEADERS}
)

target_include_directories(nova_audio
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nova_audio
    PUBLIC
        nova_core
)

target_compile_features(nova_audio PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(nova_audio PRIVATE /W4)
else()
    target_compile_options(nova_audio PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Platform-specific audio backends
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_compile_definitions(nova_audio PRIVATE NOVA_PLATFORM_ANDROID)
    # OpenSL ES / AAudio backend
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_compile_definitions(nova_audio PRIVATE NOVA_PLATFORM_IOS)
    # Core Audio backend
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(nova_audio PRIVATE NOVA_PLATFORM_WINDOWS)
    # WASAPI backend
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(nova_audio PRIVATE NOVA_PLATFORM_LINUX)
    # ALSA/PulseAudio backend
endif()

source_group("Source Files" FILES ${NOVA_AUDIO_SOURCES})
source_group("Header Files" FILES ${NOVA_AUDIO_HEADERS})
