# =============================================================================
# NovaForge Platform - API Module
# =============================================================================
# Platform: NovaForge | Engine: NovaCore
# Company: WeNova Interactive (operating as Kayden Shawn Massengill)
#
# The API module provides unified access to all NovaForge functionality
# including engine systems, platform services, and security.
# =============================================================================

# =============================================================================
# API Library Sources
# =============================================================================

set(NOVA_API_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_services.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_security.cpp
)

set(NOVA_API_HEADERS
    ${NOVA_INCLUDE_DIR}/nova/api/api.hpp
    ${NOVA_INCLUDE_DIR}/nova/api/api_types.hpp
    ${NOVA_INCLUDE_DIR}/nova/api/api_engine.hpp
    ${NOVA_INCLUDE_DIR}/nova/api/api_platform.hpp
    ${NOVA_INCLUDE_DIR}/nova/api/api_services.hpp
    ${NOVA_INCLUDE_DIR}/nova/api/api_security.hpp
)

# =============================================================================
# Create API Library
# =============================================================================
add_library(nova_api STATIC
    ${NOVA_API_SOURCES}
    ${NOVA_API_HEADERS}
)

# Alias for cleaner usage
add_library(Nova::Api ALIAS nova_api)

# =============================================================================
# Target Properties
# =============================================================================
target_include_directories(nova_api
    PUBLIC
        $<BUILD_INTERFACE:${NOVA_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# C++23 requirement
target_compile_features(nova_api PUBLIC cxx_std_23)

# Link to core library
target_link_libraries(nova_api
    PUBLIC
        nova_core
)

# Compile definitions
target_compile_definitions(nova_api
    PUBLIC
        $<$<CONFIG:Debug>:NOVA_DEBUG=1>
        $<$<CONFIG:Release>:NOVA_RELEASE=1>
    PRIVATE
        NOVA_API_BUILDING=1
)

# Compile options
target_compile_options(nova_api
    PRIVATE
        ${NOVA_COMMON_WARNINGS}
        $<$<CONFIG:Debug>:${NOVA_DEBUG_FLAGS}>
        $<$<CONFIG:Release>:${NOVA_RELEASE_FLAGS}>
)

# =============================================================================
# Installation
# =============================================================================
install(TARGETS nova_api
    EXPORT NovaApiTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${NOVA_INCLUDE_DIR}/nova/api
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nova
)
