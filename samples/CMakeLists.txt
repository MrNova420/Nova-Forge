# =============================================================================
# NovaCore Engine - Samples
# =============================================================================
# Platform: NovaForge | Engine: NovaCore
# Company: WeNova Interactive (operating as Kayden Shawn Massengill)
#
# Sample applications demonstrating NovaCore Engine capabilities.
# =============================================================================

# =============================================================================
# Hello Triangle Sample - Nova GraphicsCore™ Demo
# =============================================================================

add_executable(hello_triangle
    hello_triangle/hello_triangle.cpp
)

target_link_libraries(hello_triangle
    PRIVATE
        Nova::Core
)

target_compile_features(hello_triangle PRIVATE cxx_std_23)

# Copy to binary directory
set_target_properties(hello_triangle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =============================================================================
# Installation
# =============================================================================

install(TARGETS hello_triangle
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/samples
)
