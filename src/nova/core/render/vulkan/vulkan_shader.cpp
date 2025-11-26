/**
 * @file vulkan_shader.cpp
 * @brief Nova GraphicsCore™ - Vulkan Shader Module Implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Implements shader module creation and built-in shaders.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_shader.hpp"
#include "nova/core/render/vulkan/vulkan_device.hpp"

namespace nova::render::vulkan {

// ============================================================================
// VulkanShaderModule Implementation
// ============================================================================

VulkanShaderModule::VulkanShaderModule(VulkanDevice& device)
    : m_device(&device) {
}

VulkanShaderModule::~VulkanShaderModule() {
    if (m_module != VK_NULL_HANDLE && m_device) {
        const auto& funcs = m_device->getDeviceFuncs();
        funcs.vkDestroyShaderModule(m_device->getDevice(), m_module, nullptr);
        m_module = VK_NULL_HANDLE;
    }
}

VulkanShaderModule::VulkanShaderModule(VulkanShaderModule&& other) noexcept
    : m_device(other.m_device)
    , m_module(other.m_module)
    , m_stage(other.m_stage)
    , m_entryPoint(std::move(other.m_entryPoint))
    , m_name(std::move(other.m_name)) {
    other.m_device = nullptr;
    other.m_module = VK_NULL_HANDLE;
}

VulkanShaderModule& VulkanShaderModule::operator=(VulkanShaderModule&& other) noexcept {
    if (this != &other) {
        if (m_module != VK_NULL_HANDLE && m_device) {
            const auto& funcs = m_device->getDeviceFuncs();
            funcs.vkDestroyShaderModule(m_device->getDevice(), m_module, nullptr);
        }
        
        m_device = other.m_device;
        m_module = other.m_module;
        m_stage = other.m_stage;
        m_entryPoint = std::move(other.m_entryPoint);
        m_name = std::move(other.m_name);
        
        other.m_device = nullptr;
        other.m_module = VK_NULL_HANDLE;
    }
    return *this;
}

Result<std::unique_ptr<VulkanShaderModule>, Error>
VulkanShaderModule::create(VulkanDevice& device, const VulkanShaderModuleDesc& desc) {
    return createFromBytes(device, 
                           desc.spirvCode.data(), 
                           desc.spirvCode.size() * sizeof(u32),
                           desc.stage,
                           desc.entryPoint,
                           desc.name);
}

Result<std::unique_ptr<VulkanShaderModule>, Error>
VulkanShaderModule::createFromBytes(VulkanDevice& device,
                                    const void* data, usize size,
                                    ShaderStageType stage,
                                    const std::string& entryPoint,
                                    const std::string& name) {
    // Validate SPIR-V data
    if (!data || size < 4) {
        return std::unexpected(errors::invalidArgument("Invalid SPIR-V data"));
    }
    
    if (size % 4 != 0) {
        return std::unexpected(errors::invalidArgument("SPIR-V size must be 4-byte aligned"));
    }
    
    // Check SPIR-V magic number
    const u32* spirv = static_cast<const u32*>(data);
    if (spirv[0] != 0x07230203) {
        return std::unexpected(errors::invalidArgument("Invalid SPIR-V magic number"));
    }
    
    auto shader = std::unique_ptr<VulkanShaderModule>(new VulkanShaderModule(device));
    shader->m_stage = stage;
    shader->m_entryPoint = entryPoint;
    shader->m_name = name;
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = spirv;
    
    const auto& funcs = device.getDeviceFuncs();
    VkResult result = funcs.vkCreateShaderModule(device.getDevice(), &createInfo, 
                                                  nullptr, &shader->m_module);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            std::string("Failed to create shader module: ") + vkResultToString(result)));
    }
    
    // Set debug name
    if (!name.empty() && device.isValidationEnabled()) {
        device.setDebugName(VK_OBJECT_TYPE_SHADER_MODULE,
                           reinterpret_cast<u64>(shader->m_module),
                           name);
    }
    
    return shader;
}

VkPipelineShaderStageCreateInfo VulkanShaderModule::getStageInfo() const noexcept {
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = getVkStage();
    stageInfo.module = m_module;
    stageInfo.pName = m_entryPoint.c_str();
    stageInfo.pSpecializationInfo = nullptr;
    return stageInfo;
}

// ============================================================================
// VulkanShaderProgram Implementation
// ============================================================================

Result<VulkanShaderProgram, Error>
VulkanShaderProgram::createGraphics(VulkanDevice& device,
                                    std::span<const u32> vertexSpirv,
                                    std::span<const u32> fragmentSpirv,
                                    const std::string& name) {
    VulkanShaderProgram program;
    program.m_name = name;
    
    // Create vertex shader
    VulkanShaderModuleDesc vertexDesc;
    vertexDesc.spirvCode = vertexSpirv;
    vertexDesc.stage = ShaderStageType::Vertex;
    vertexDesc.entryPoint = "main";
    vertexDesc.name = name + "_VS";
    
    auto vertexResult = VulkanShaderModule::create(device, vertexDesc);
    if (!vertexResult) {
        return std::unexpected(vertexResult.error());
    }
    program.m_modules.push_back(std::move(*vertexResult));
    
    // Create fragment shader
    VulkanShaderModuleDesc fragmentDesc;
    fragmentDesc.spirvCode = fragmentSpirv;
    fragmentDesc.stage = ShaderStageType::Fragment;
    fragmentDesc.entryPoint = "main";
    fragmentDesc.name = name + "_FS";
    
    auto fragmentResult = VulkanShaderModule::create(device, fragmentDesc);
    if (!fragmentResult) {
        return std::unexpected(fragmentResult.error());
    }
    program.m_modules.push_back(std::move(*fragmentResult));
    
    return program;
}

Result<VulkanShaderProgram, Error>
VulkanShaderProgram::createCompute(VulkanDevice& device,
                                   std::span<const u32> computeSpirv,
                                   const std::string& name) {
    VulkanShaderProgram program;
    program.m_name = name;
    
    VulkanShaderModuleDesc computeDesc;
    computeDesc.spirvCode = computeSpirv;
    computeDesc.stage = ShaderStageType::Compute;
    computeDesc.entryPoint = "main";
    computeDesc.name = name + "_CS";
    
    auto computeResult = VulkanShaderModule::create(device, computeDesc);
    if (!computeResult) {
        return std::unexpected(computeResult.error());
    }
    program.m_modules.push_back(std::move(*computeResult));
    
    return program;
}

std::vector<VkPipelineShaderStageCreateInfo> VulkanShaderProgram::getStageInfos() const {
    std::vector<VkPipelineShaderStageCreateInfo> infos;
    infos.reserve(m_modules.size());
    
    for (const auto& module : m_modules) {
        infos.push_back(module->getStageInfo());
    }
    
    return infos;
}

bool VulkanShaderProgram::hasStage(ShaderStageType stage) const noexcept {
    for (const auto& module : m_modules) {
        if (module->getStage() == stage) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Built-in Shader SPIR-V Bytecode
// ============================================================================

// These are pre-compiled SPIR-V bytecode for basic shaders.
// Generated from GLSL using glslangValidator.

namespace {

// Basic triangle vertex shader SPIR-V
// GLSL source:
// #version 450
// layout(location = 0) in vec2 inPosition;
// layout(location = 1) in vec3 inColor;
// layout(location = 0) out vec3 fragColor;
// void main() {
//     gl_Position = vec4(inPosition, 0.0, 1.0);
//     fragColor = inColor;
// }
const u32 g_triangleVertexShader[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x0000002a, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0009000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x0000001c,
    0x00000020, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004,
    0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67, 0x65567265,
    0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000, 0x505f6c67,
    0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001, 0x505f6c67,
    0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002,
    0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b,
    0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e, 0x00030005,
    0x0000000d, 0x00000000, 0x00050005, 0x00000012, 0x6f506e69, 0x69746973,
    0x00006e6f, 0x00050005, 0x0000001c, 0x67617266, 0x6f6c6f43, 0x00000072,
    0x00040005, 0x00000020, 0x6f436e69, 0x00726f6c, 0x00050048, 0x0000000b,
    0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b, 0x00000001,
    0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b,
    0x00000003, 0x00050048, 0x0000000b, 0x00000003, 0x0000000b, 0x00000004,
    0x00030047, 0x0000000b, 0x00000002, 0x00040047, 0x00000012, 0x0000001e,
    0x00000000, 0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00040047,
    0x00000020, 0x0000001e, 0x00000001, 0x00020013, 0x00000002, 0x00030021,
    0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
    0x00000007, 0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020,
    0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001, 0x0004001c,
    0x0000000a, 0x00000006, 0x00000009, 0x0006001e, 0x0000000b, 0x00000007,
    0x00000006, 0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003,
    0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003, 0x00040015,
    0x0000000e, 0x00000020, 0x00000001, 0x0004002b, 0x0000000e, 0x0000000f,
    0x00000000, 0x00040017, 0x00000010, 0x00000006, 0x00000002, 0x00040020,
    0x00000011, 0x00000001, 0x00000010, 0x0004003b, 0x00000011, 0x00000012,
    0x00000001, 0x0004002b, 0x00000006, 0x00000014, 0x00000000, 0x0004002b,
    0x00000006, 0x00000015, 0x3f800000, 0x00040020, 0x0000001a, 0x00000003,
    0x00000007, 0x00040017, 0x0000001b, 0x00000006, 0x00000003, 0x00040020,
    0x0000001d, 0x00000003, 0x0000001b, 0x0004003b, 0x0000001d, 0x0000001c,
    0x00000003, 0x00040020, 0x0000001f, 0x00000001, 0x0000001b, 0x0004003b,
    0x0000001f, 0x00000020, 0x00000001, 0x00050036, 0x00000002, 0x00000004,
    0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000010,
    0x00000013, 0x00000012, 0x00050051, 0x00000006, 0x00000016, 0x00000013,
    0x00000000, 0x00050051, 0x00000006, 0x00000017, 0x00000013, 0x00000001,
    0x00070050, 0x00000007, 0x00000018, 0x00000016, 0x00000017, 0x00000014,
    0x00000015, 0x00050041, 0x0000001a, 0x00000019, 0x0000000d, 0x0000000f,
    0x0003003e, 0x00000019, 0x00000018, 0x0004003d, 0x0000001b, 0x00000021,
    0x00000020, 0x0003003e, 0x0000001c, 0x00000021, 0x000100fd, 0x00010038
};

// Basic triangle fragment shader SPIR-V
// GLSL source:
// #version 450
// layout(location = 0) in vec3 fragColor;
// layout(location = 0) out vec4 outColor;
// void main() {
//     outColor = vec4(fragColor, 1.0);
// }
const u32 g_triangleFragmentShader[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000014, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000c, 0x00030010,
    0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x4374756f,
    0x726f6c6f, 0x00000000, 0x00050005, 0x0000000c, 0x67617266, 0x6f6c6f43,
    0x00000072, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047,
    0x0000000c, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021,
    0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
    0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
    0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040017,
    0x0000000a, 0x00000006, 0x00000003, 0x00040020, 0x0000000b, 0x00000001,
    0x0000000a, 0x0004003b, 0x0000000b, 0x0000000c, 0x00000001, 0x0004002b,
    0x00000006, 0x0000000e, 0x3f800000, 0x00050036, 0x00000002, 0x00000004,
    0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000a,
    0x0000000d, 0x0000000c, 0x00050051, 0x00000006, 0x0000000f, 0x0000000d,
    0x00000000, 0x00050051, 0x00000006, 0x00000010, 0x0000000d, 0x00000001,
    0x00050051, 0x00000006, 0x00000011, 0x0000000d, 0x00000002, 0x00070050,
    0x00000007, 0x00000012, 0x0000000f, 0x00000010, 0x00000011, 0x0000000e,
    0x0003003e, 0x00000009, 0x00000012, 0x000100fd, 0x00010038
};

// Fullscreen quad vertex shader SPIR-V
// GLSL source:
// #version 450
// layout(location = 0) out vec2 texCoord;
// void main() {
//     texCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
//     gl_Position = vec4(texCoord * 2.0 - 1.0, 0.0, 1.0);
// }
const u32 g_fullscreenVertexShader[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x0000002f, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000008, 0x0000000c, 0x0000001d,
    0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
    0x00000000, 0x00050005, 0x00000008, 0x43786574, 0x64726f6f, 0x00000000,
    0x00060005, 0x0000000c, 0x565f6c67, 0x65747265, 0x646e4978, 0x00007865,
    0x00060005, 0x0000001b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000,
    0x00060006, 0x0000001b, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69,
    0x00070006, 0x0000001b, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953,
    0x00000000, 0x00070006, 0x0000001b, 0x00000002, 0x435f6c67, 0x4470696c,
    0x61747369, 0x0065636e, 0x00070006, 0x0000001b, 0x00000003, 0x435f6c67,
    0x446c6c75, 0x61747369, 0x0065636e, 0x00030005, 0x0000001d, 0x00000000,
    0x00040047, 0x00000008, 0x0000001e, 0x00000000, 0x00040047, 0x0000000c,
    0x0000000b, 0x0000002a, 0x00050048, 0x0000001b, 0x00000000, 0x0000000b,
    0x00000000, 0x00050048, 0x0000001b, 0x00000001, 0x0000000b, 0x00000001,
    0x00050048, 0x0000001b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048,
    0x0000001b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000001b,
    0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000002, 0x00040020, 0x00000009, 0x00000003, 0x00000007, 0x0004003b,
    0x00000009, 0x00000008, 0x00000003, 0x00040015, 0x0000000a, 0x00000020,
    0x00000001, 0x00040020, 0x0000000b, 0x00000001, 0x0000000a, 0x0004003b,
    0x0000000b, 0x0000000c, 0x00000001, 0x0004002b, 0x0000000a, 0x0000000e,
    0x00000001, 0x0004002b, 0x0000000a, 0x00000010, 0x00000002, 0x00040017,
    0x00000018, 0x00000006, 0x00000004, 0x00040015, 0x00000019, 0x00000020,
    0x00000000, 0x0004002b, 0x00000019, 0x0000001a, 0x00000001, 0x0004001c,
    0x0000001c, 0x00000006, 0x0000001a, 0x0006001e, 0x0000001b, 0x00000018,
    0x00000006, 0x0000001c, 0x0000001c, 0x00040020, 0x0000001e, 0x00000003,
    0x0000001b, 0x0004003b, 0x0000001e, 0x0000001d, 0x00000003, 0x0004002b,
    0x0000000a, 0x0000001f, 0x00000000, 0x0004002b, 0x00000006, 0x00000021,
    0x40000000, 0x0004002b, 0x00000006, 0x00000023, 0x3f800000, 0x0004002b,
    0x00000006, 0x00000026, 0x00000000, 0x00040020, 0x0000002d, 0x00000003,
    0x00000018, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x0004003d, 0x0000000a, 0x0000000d, 0x0000000c,
    0x000500c4, 0x0000000a, 0x0000000f, 0x0000000d, 0x0000000e, 0x000500c7,
    0x0000000a, 0x00000011, 0x0000000f, 0x00000010, 0x0004006f, 0x00000006,
    0x00000012, 0x00000011, 0x0004003d, 0x0000000a, 0x00000013, 0x0000000c,
    0x000500c7, 0x0000000a, 0x00000014, 0x00000013, 0x00000010, 0x0004006f,
    0x00000006, 0x00000015, 0x00000014, 0x00050050, 0x00000007, 0x00000016,
    0x00000012, 0x00000015, 0x0003003e, 0x00000008, 0x00000016, 0x0004003d,
    0x00000007, 0x00000020, 0x00000008, 0x0005008e, 0x00000007, 0x00000022,
    0x00000020, 0x00000021, 0x00050050, 0x00000007, 0x00000024, 0x00000023,
    0x00000023, 0x00050083, 0x00000007, 0x00000025, 0x00000022, 0x00000024,
    0x00050051, 0x00000006, 0x00000027, 0x00000025, 0x00000000, 0x00050051,
    0x00000006, 0x00000028, 0x00000025, 0x00000001, 0x00070050, 0x00000018,
    0x00000029, 0x00000027, 0x00000028, 0x00000026, 0x00000023, 0x00050041,
    0x0000002d, 0x0000002e, 0x0000001d, 0x0000001f, 0x0003003e, 0x0000002e,
    0x00000029, 0x000100fd, 0x00010038
};

// Basic texture sampling fragment shader SPIR-V
// GLSL source:
// #version 450
// layout(location = 0) in vec2 texCoord;
// layout(binding = 0) uniform sampler2D tex;
// layout(location = 0) out vec4 outColor;
// void main() {
//     outColor = texture(tex, texCoord);
// }
const u32 g_textureFragmentShader[] = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000014, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00000011,
    0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2,
    0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009,
    0x4374756f, 0x726f6c6f, 0x00000000, 0x00030005, 0x0000000d, 0x00786574,
    0x00050005, 0x00000011, 0x43786574, 0x64726f6f, 0x00000000, 0x00040047,
    0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000d, 0x00000022,
    0x00000000, 0x00040047, 0x0000000d, 0x00000021, 0x00000000, 0x00040047,
    0x00000011, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021,
    0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
    0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003,
    0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00090019,
    0x0000000a, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
    0x00000001, 0x00000000, 0x0003001b, 0x0000000b, 0x0000000a, 0x00040020,
    0x0000000c, 0x00000000, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
    0x00000000, 0x00040017, 0x0000000f, 0x00000006, 0x00000002, 0x00040020,
    0x00000010, 0x00000001, 0x0000000f, 0x0004003b, 0x00000010, 0x00000011,
    0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003,
    0x000200f8, 0x00000005, 0x0004003d, 0x0000000b, 0x0000000e, 0x0000000d,
    0x0004003d, 0x0000000f, 0x00000012, 0x00000011, 0x00050057, 0x00000007,
    0x00000013, 0x0000000e, 0x00000012, 0x0003003e, 0x00000009, 0x00000013,
    0x000100fd, 0x00010038
};

} // anonymous namespace

// ============================================================================
// Built-in Shader Accessors
// ============================================================================

namespace BuiltinShaders {

std::span<const u32> getTriangleVertexShader() {
    return {g_triangleVertexShader, sizeof(g_triangleVertexShader) / sizeof(u32)};
}

std::span<const u32> getTriangleFragmentShader() {
    return {g_triangleFragmentShader, sizeof(g_triangleFragmentShader) / sizeof(u32)};
}

std::span<const u32> getFullscreenVertexShader() {
    return {g_fullscreenVertexShader, sizeof(g_fullscreenVertexShader) / sizeof(u32)};
}

std::span<const u32> getTextureFragmentShader() {
    return {g_textureFragmentShader, sizeof(g_textureFragmentShader) / sizeof(u32)};
}

} // namespace BuiltinShaders

} // namespace nova::render::vulkan
