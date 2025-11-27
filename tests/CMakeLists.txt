# =============================================================================
# NovaCore Engine - Test Suite
# =============================================================================
# Platform: NovaForge | Engine: NovaCore
# Company: WeNova Interactive (operating as Kayden Shawn Massengill)
#
# Test suite using Catch2 for unit and integration testing
# =============================================================================

# =============================================================================
# Fetch Catch2 (Test Framework)
# =============================================================================
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)

FetchContent_MakeAvailable(Catch2)

# =============================================================================
# Test Sources
# =============================================================================

# Core types tests
set(NOVA_TEST_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_memory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_ecs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_math.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_animation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_network.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_ui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_particle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/core/test_physics.cpp
)

# API tests
set(NOVA_TEST_API_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/nova/api/test_api.cpp
)

# All test sources
set(NOVA_TEST_SOURCES
    ${NOVA_TEST_CORE_SOURCES}
    ${NOVA_TEST_API_SOURCES}
)

# =============================================================================
# Test Executable
# =============================================================================
add_executable(nova_tests
    ${NOVA_TEST_SOURCES}
)

target_link_libraries(nova_tests
    PRIVATE
        nova_core
        nova_api
        Catch2::Catch2WithMain
)

target_include_directories(nova_tests
    PRIVATE
        ${NOVA_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/../include
)

# C++23 requirement
target_compile_features(nova_tests PRIVATE cxx_std_23)

# =============================================================================
# Register Tests with CTest
# =============================================================================
include(CTest)
include(Catch)

catch_discover_tests(nova_tests)

# =============================================================================
# Custom Test Target
# =============================================================================
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS nova_tests
    COMMENT "Running NovaCore test suite"
)
