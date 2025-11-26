/**
 * @file vulkan_shader.hpp
 * @brief Nova GraphicsCore™ - Vulkan Shader Module System
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Implements Vulkan shader module creation and reflection.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 * @see NOVAFORGE_NOVACORE_ENGINE_BLUEPRINT.md for full technical specifications
 */

#pragma once

#include "vulkan_types.hpp"
#include "nova/core/types/result.hpp"
#include "nova/core/render/shader.hpp"
#include <memory>
#include <vector>
#include <span>

namespace nova::render::vulkan {

// Forward declarations
class VulkanDevice;

// ============================================================================
// Shader Stage
// ============================================================================

/**
 * @brief Shader stage type
 */
enum class ShaderStageType : u8 {
    Vertex,
    Fragment,
    Compute,
    Geometry,
    TessControl,
    TessEvaluation,
    
    Count
};

/**
 * @brief Convert shader stage to Vulkan shader stage flag
 */
constexpr VkShaderStageFlagBits toVkShaderStage(ShaderStageType stage) noexcept {
    switch (stage) {
        case ShaderStageType::Vertex:         return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStageType::Fragment:       return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStageType::Compute:        return VK_SHADER_STAGE_COMPUTE_BIT;
        case ShaderStageType::Geometry:       return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStageType::TessControl:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStageType::TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        default:                              return VK_SHADER_STAGE_VERTEX_BIT;
    }
}

// ============================================================================
// Shader Module
// ============================================================================

/**
 * @brief Shader module creation descriptor
 */
struct VulkanShaderModuleDesc {
    std::span<const u32> spirvCode;             ///< SPIR-V bytecode (must be u32 aligned)
    ShaderStageType stage = ShaderStageType::Vertex; ///< Shader stage
    std::string entryPoint = "main";            ///< Entry point name
    std::string name;                           ///< Debug name
};

/**
 * @brief Vulkan shader module wrapper
 * 
 * Wraps a VkShaderModule and its metadata.
 */
class VulkanShaderModule {
public:
    ~VulkanShaderModule();
    
    // Non-copyable
    VulkanShaderModule(const VulkanShaderModule&) = delete;
    VulkanShaderModule& operator=(const VulkanShaderModule&) = delete;
    
    // Moveable
    VulkanShaderModule(VulkanShaderModule&& other) noexcept;
    VulkanShaderModule& operator=(VulkanShaderModule&& other) noexcept;
    
    /**
     * @brief Create a shader module from SPIR-V bytecode
     * @param device Vulkan device
     * @param desc Shader module descriptor
     * @return Shader module or error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanShaderModule>, Error>
    create(VulkanDevice& device, const VulkanShaderModuleDesc& desc);
    
    /**
     * @brief Create a shader module from raw SPIR-V data
     * @param device Vulkan device
     * @param data Raw SPIR-V bytecode (must be 4-byte aligned)
     * @param size Size of bytecode in bytes
     * @param stage Shader stage
     * @param entryPoint Entry point name
     * @param name Debug name
     * @return Shader module or error
     */
    [[nodiscard]] static Result<std::unique_ptr<VulkanShaderModule>, Error>
    createFromBytes(VulkanDevice& device,
                    const void* data, usize size,
                    ShaderStageType stage,
                    const std::string& entryPoint = "main",
                    const std::string& name = "");
    
    /// Get Vulkan handle
    [[nodiscard]] VkShaderModule getHandle() const noexcept { return m_module; }
    
    /// Get shader stage
    [[nodiscard]] ShaderStageType getStage() const noexcept { return m_stage; }
    
    /// Get Vulkan shader stage flag
    [[nodiscard]] VkShaderStageFlagBits getVkStage() const noexcept { return toVkShaderStage(m_stage); }
    
    /// Get entry point name
    [[nodiscard]] const std::string& getEntryPoint() const noexcept { return m_entryPoint; }
    
    /// Get debug name
    [[nodiscard]] const std::string& getName() const noexcept { return m_name; }
    
    /// Get pipeline shader stage create info (convenience for pipeline creation)
    [[nodiscard]] VkPipelineShaderStageCreateInfo getStageInfo() const noexcept;

private:
    VulkanShaderModule(VulkanDevice& device);
    
    VulkanDevice* m_device = nullptr;
    VkShaderModule m_module = VK_NULL_HANDLE;
    ShaderStageType m_stage = ShaderStageType::Vertex;
    std::string m_entryPoint = "main";
    std::string m_name;
};

// ============================================================================
// Shader Program (Multi-Stage)
// ============================================================================

/**
 * @brief Shader program combining multiple shader stages
 */
class VulkanShaderProgram {
public:
    ~VulkanShaderProgram() = default;
    
    // Non-copyable but moveable
    VulkanShaderProgram(const VulkanShaderProgram&) = delete;
    VulkanShaderProgram& operator=(const VulkanShaderProgram&) = delete;
    VulkanShaderProgram(VulkanShaderProgram&&) = default;
    VulkanShaderProgram& operator=(VulkanShaderProgram&&) = default;
    
    /**
     * @brief Create a graphics shader program (vertex + fragment)
     * @param device Vulkan device
     * @param vertexSpirv Vertex shader SPIR-V
     * @param fragmentSpirv Fragment shader SPIR-V
     * @param name Debug name
     * @return Shader program or error
     */
    [[nodiscard]] static Result<VulkanShaderProgram, Error>
    createGraphics(VulkanDevice& device,
                   std::span<const u32> vertexSpirv,
                   std::span<const u32> fragmentSpirv,
                   const std::string& name = "GraphicsProgram");
    
    /**
     * @brief Create a compute shader program
     * @param device Vulkan device
     * @param computeSpirv Compute shader SPIR-V
     * @param name Debug name
     * @return Shader program or error
     */
    [[nodiscard]] static Result<VulkanShaderProgram, Error>
    createCompute(VulkanDevice& device,
                  std::span<const u32> computeSpirv,
                  const std::string& name = "ComputeProgram");
    
    /// Get all shader stage infos for pipeline creation
    [[nodiscard]] std::vector<VkPipelineShaderStageCreateInfo> getStageInfos() const;
    
    /// Check if program has a specific stage
    [[nodiscard]] bool hasStage(ShaderStageType stage) const noexcept;
    
    /// Get number of stages
    [[nodiscard]] usize getStageCount() const noexcept { return m_modules.size(); }

private:
    VulkanShaderProgram() = default;
    
    std::vector<std::unique_ptr<VulkanShaderModule>> m_modules;
    std::string m_name;
};

// ============================================================================
// Built-in Shaders
// ============================================================================

/**
 * @brief Provides built-in shader bytecode for common use cases
 */
namespace BuiltinShaders {

/**
 * @brief Get basic triangle vertex shader SPIR-V
 * 
 * Simple passthrough vertex shader for testing.
 * Input: vec2 inPosition, vec3 inColor at locations 0, 1
 * Output: vec3 fragColor at location 0
 */
[[nodiscard]] std::span<const u32> getTriangleVertexShader();

/**
 * @brief Get basic triangle fragment shader SPIR-V
 * 
 * Simple color output fragment shader.
 * Input: vec3 fragColor at location 0
 * Output: vec4 outColor at location 0
 */
[[nodiscard]] std::span<const u32> getTriangleFragmentShader();

/**
 * @brief Get fullscreen quad vertex shader SPIR-V
 * 
 * Generates fullscreen triangle without vertex input.
 * Output: vec2 texCoord at location 0
 */
[[nodiscard]] std::span<const u32> getFullscreenVertexShader();

/**
 * @brief Get basic texture sampling fragment shader SPIR-V
 * 
 * Samples texture at texCoord.
 * Input: vec2 texCoord at location 0
 * Binding: sampler2D tex at set 0, binding 0
 */
[[nodiscard]] std::span<const u32> getTextureFragmentShader();

} // namespace BuiltinShaders

} // namespace nova::render::vulkan
