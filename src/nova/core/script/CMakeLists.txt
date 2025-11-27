# NovaCore Script System™ - Build Configuration
# NovaForge Platform | NovaCore Engine
# Copyright (c) 2025 WeNova Interactive (operating as Kayden Shawn Massengill)

set(NOVA_SCRIPT_SOURCES
    script_engine.cpp
)

set(NOVA_SCRIPT_HEADERS
    ${CMAKE_SOURCE_DIR}/include/nova/core/script/script.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/core/script/script_types.hpp
    ${CMAKE_SOURCE_DIR}/include/nova/core/script/script_engine.hpp
)

add_library(nova_script STATIC
    ${NOVA_SCRIPT_SOURCES}
    ${NOVA_SCRIPT_HEADERS}
)

target_include_directories(nova_script
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nova_script
    PUBLIC
        nova_core
        nova_ecs
)

target_compile_features(nova_script PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(nova_script PRIVATE /W4)
else()
    target_compile_options(nova_script PRIVATE -Wall -Wextra -Wpedantic)
endif()

source_group("Source Files" FILES ${NOVA_SCRIPT_SOURCES})
source_group("Header Files" FILES ${NOVA_SCRIPT_HEADERS})
