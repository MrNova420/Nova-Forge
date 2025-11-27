# NovaCore Platform™ - Build Configuration
# NovaForge Platform | NovaCore Engine
# Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)

set(NOVA_PLATFORM_SOURCES
    application.cpp
    window.cpp
)

set(NOVA_PLATFORM_HEADERS
    ${CMAKE_SOURCE_DIR}/include/nova/platform/platform.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/platform/platform_types.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/platform/application.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/platform/window.hpp
)

add_library(nova_platform STATIC
    ${NOVA_PLATFORM_SOURCES}
    ${NOVA_PLATFORM_HEADERS}
)

target_include_directories(nova_platform
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nova_platform
    PUBLIC
        nova_core
)

target_compile_features(nova_platform PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(nova_platform PRIVATE /W4)
else()
    target_compile_options(nova_platform PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(nova_platform PRIVATE
        user32
        gdi32
        shell32
        ole32
    )
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    target_link_libraries(nova_platform PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
    )
elseif(UNIX)
    find_package(X11)
    if(X11_FOUND)
        target_link_libraries(nova_platform PRIVATE ${X11_LIBRARIES})
        target_include_directories(nova_platform PRIVATE ${X11_INCLUDE_DIR})
    endif()
endif()

source_group("Source Files" FILES ${NOVA_PLATFORM_SOURCES})
source_group("Header Files" FILES ${NOVA_PLATFORM_HEADERS})
