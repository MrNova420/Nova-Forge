/**
 * @file render.hpp
 * @brief Main include header for NovaCore rendering system
 * 
 * The NovaCore rendering system provides:
 * - Multi-backend support (Vulkan, Metal, WebGPU, OpenGL fallback)
 * - Render graph for automatic resource management
 * - GPU-driven rendering for 10M+ entities
 * - UCRT (Universal Continual Ray Tracing) path
 * - Quality tier auto-detection and scaling
 * 
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for complete architecture
 * @see TECHNOLOGY_ANALYSIS.md for rendering backend details
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include "render_device.hpp"
#include "render_context.hpp"
#include "render_pass.hpp"
#include "render_pipeline.hpp"
#include "command_buffer.hpp"
#include "swap_chain.hpp"
#include "buffer.hpp"
#include "texture.hpp"
#include "shader.hpp"

namespace nova::render {

/**
 * @brief Rendering system version
 */
constexpr u32 RENDER_VERSION_MAJOR = 1;
constexpr u32 RENDER_VERSION_MINOR = 0;
constexpr u32 RENDER_VERSION_PATCH = 0;

/**
 * @brief Rendering configuration constants
 */
namespace config {
    /// Maximum number of frames in flight
    constexpr u32 MAX_FRAMES_IN_FLIGHT = 3;
    
    /// Maximum number of render passes in a frame
    constexpr u32 MAX_RENDER_PASSES = 64;
    
    /// Maximum descriptor sets per pipeline
    constexpr u32 MAX_DESCRIPTOR_SETS = 8;
    
    /// Maximum push constant size (128 bytes for Vulkan compatibility)
    constexpr usize MAX_PUSH_CONSTANT_SIZE = 128;
    
    /// Default swapchain image count
    constexpr u32 DEFAULT_SWAPCHAIN_IMAGES = 3;
    
    /// Maximum bindless resources
    constexpr u32 MAX_BINDLESS_TEXTURES = 16384;
    constexpr u32 MAX_BINDLESS_BUFFERS = 4096;
}

} // namespace nova::render
