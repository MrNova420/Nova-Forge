/**
 * @file shader.hpp
 * @brief Shader module types and descriptors
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#pragma once

#include "render_types.hpp"
#include <string>
#include <vector>
#include <span>

namespace nova::render {

/**
 * @brief Shader stage
 */
enum class ShaderStage : u8 {
    Vertex,
    Fragment,
    Compute,
    Geometry,
    TessellationControl,
    TessellationEvaluation,
    Mesh,
    Task,
    RayGeneration,
    RayAnyHit,
    RayClosestHit,
    RayMiss,
    RayIntersection
};

/**
 * @brief Get shader stage name
 */
constexpr const char* getShaderStageName(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex: return "Vertex";
        case ShaderStage::Fragment: return "Fragment";
        case ShaderStage::Compute: return "Compute";
        case ShaderStage::Geometry: return "Geometry";
        case ShaderStage::TessellationControl: return "TessControl";
        case ShaderStage::TessellationEvaluation: return "TessEval";
        case ShaderStage::Mesh: return "Mesh";
        case ShaderStage::Task: return "Task";
        case ShaderStage::RayGeneration: return "RayGen";
        case ShaderStage::RayAnyHit: return "RayAnyHit";
        case ShaderStage::RayClosestHit: return "RayClosestHit";
        case ShaderStage::RayMiss: return "RayMiss";
        case ShaderStage::RayIntersection: return "RayIntersection";
    }
    return "Unknown";
}

/**
 * @brief Shader source type
 */
enum class ShaderSourceType : u8 {
    SPIRV,      ///< Pre-compiled SPIR-V bytecode
    GLSL,       ///< GLSL source code (will be compiled)
    HLSL,       ///< HLSL source code (will be compiled)
    MSL,        ///< Metal Shading Language source
    WGSL        ///< WebGPU Shading Language source
};

/**
 * @brief Shader creation descriptor
 */
struct ShaderDesc {
    /// Shader stage
    ShaderStage stage = ShaderStage::Vertex;
    
    /// Source type
    ShaderSourceType sourceType = ShaderSourceType::SPIRV;
    
    /// Shader code (binary for SPIRV, text for others)
    std::vector<u8> code;
    
    /// Entry point function name
    std::string entryPoint = "main";
    
    /// Debug name
    std::string name;
    
    // Factory methods
    static ShaderDesc spirv(ShaderStage stage, std::span<const u8> spirvCode,
                            const std::string& entryPoint = "main") {
        ShaderDesc desc;
        desc.stage = stage;
        desc.sourceType = ShaderSourceType::SPIRV;
        desc.code = std::vector<u8>(spirvCode.begin(), spirvCode.end());
        desc.entryPoint = entryPoint;
        return desc;
    }
    
    static ShaderDesc spirv(ShaderStage stage, std::span<const u32> spirvCode,
                            const std::string& entryPoint = "main") {
        ShaderDesc desc;
        desc.stage = stage;
        desc.sourceType = ShaderSourceType::SPIRV;
        const u8* data = reinterpret_cast<const u8*>(spirvCode.data());
        desc.code = std::vector<u8>(data, data + spirvCode.size() * sizeof(u32));
        desc.entryPoint = entryPoint;
        return desc;
    }
    
    static ShaderDesc glsl(ShaderStage stage, const std::string& source,
                           const std::string& entryPoint = "main") {
        ShaderDesc desc;
        desc.stage = stage;
        desc.sourceType = ShaderSourceType::GLSL;
        desc.code = std::vector<u8>(source.begin(), source.end());
        desc.code.push_back(0); // Null terminate
        desc.entryPoint = entryPoint;
        return desc;
    }
};

/**
 * @brief Shader stage info for pipeline creation
 */
struct ShaderStageInfo {
    ShaderHandle shader;
    ShaderStage stage = ShaderStage::Vertex;
    std::string entryPoint = "main";
    
    ShaderStageInfo() = default;
    ShaderStageInfo(ShaderHandle shaderHandle, ShaderStage shaderStage, 
                    const std::string& entry = "main")
        : shader(shaderHandle), stage(shaderStage), entryPoint(entry) {}
};

} // namespace nova::render
