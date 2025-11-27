# NovaCore Resource System™ - Build Configuration
# NovaForge Platform | NovaCore Engine
# Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)

set(NOVA_RESOURCE_SOURCES
    resource_manager.cpp
)

set(NOVA_RESOURCE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/nova/core/resource/resource.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/core/resource/resource_types.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/core/resource/resource_manager.hpp
)

add_library(nova_resource STATIC
    ${NOVA_RESOURCE_SOURCES}
    ${NOVA_RESOURCE_HEADERS}
)

target_include_directories(nova_resource
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nova_resource
    PUBLIC
        nova_core
)

target_compile_features(nova_resource PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(nova_resource PRIVATE /W4)
else()
    target_compile_options(nova_resource PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Thread support
find_package(Threads REQUIRED)
target_link_libraries(nova_resource PRIVATE Threads::Threads)

source_group("Source Files" FILES ${NOVA_RESOURCE_SOURCES})
source_group("Header Files" FILES ${NOVA_RESOURCE_HEADERS})
