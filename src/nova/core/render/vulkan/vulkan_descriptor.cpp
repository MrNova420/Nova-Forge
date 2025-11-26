/**
 * @file vulkan_descriptor.cpp
 * @brief Nova GraphicsCore™ - Vulkan Descriptor System Implementation
 * 
 * Part of the NovaCore Engine - World's Best Mobile-First Game Engine
 * Implements descriptor set layout, pool, and allocation functionality.
 * 
 * @copyright Copyright (c) 2025 WeNova Interactive (Kayden Shawn Massengill)
 */

#include "nova/core/render/vulkan/vulkan_descriptor.hpp"
#include "nova/core/render/vulkan/vulkan_device.hpp"
#include <algorithm>

namespace nova::render::vulkan {

// ============================================================================
// VulkanDescriptorSetLayout Implementation
// ============================================================================

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice& device)
    : m_device(&device) {
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    if (m_layout != VK_NULL_HANDLE && m_device) {
        const auto& funcs = m_device->getDeviceFuncs();
        funcs.vkDestroyDescriptorSetLayout(m_device->getDevice(), m_layout, nullptr);
        m_layout = VK_NULL_HANDLE;
    }
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept
    : m_device(other.m_device)
    , m_layout(other.m_layout)
    , m_bindings(std::move(other.m_bindings))
    , m_name(std::move(other.m_name)) {
    other.m_device = nullptr;
    other.m_layout = VK_NULL_HANDLE;
}

VulkanDescriptorSetLayout& VulkanDescriptorSetLayout::operator=(VulkanDescriptorSetLayout&& other) noexcept {
    if (this != &other) {
        // Destroy existing layout
        if (m_layout != VK_NULL_HANDLE && m_device) {
            const auto& funcs = m_device->getDeviceFuncs();
            funcs.vkDestroyDescriptorSetLayout(m_device->getDevice(), m_layout, nullptr);
        }
        
        m_device = other.m_device;
        m_layout = other.m_layout;
        m_bindings = std::move(other.m_bindings);
        m_name = std::move(other.m_name);
        
        other.m_device = nullptr;
        other.m_layout = VK_NULL_HANDLE;
    }
    return *this;
}

Result<std::unique_ptr<VulkanDescriptorSetLayout>, Error>
VulkanDescriptorSetLayout::create(VulkanDevice& device, const DescriptorSetLayoutDesc& desc) {
    auto layout = std::unique_ptr<VulkanDescriptorSetLayout>(
        new VulkanDescriptorSetLayout(device));
    
    layout->m_bindings = desc.bindings;
    layout->m_name = desc.name;
    
    // Convert bindings to Vulkan format
    std::vector<VkDescriptorSetLayoutBinding> vkBindings;
    vkBindings.reserve(desc.bindings.size());
    
    for (const auto& binding : desc.bindings) {
        VkDescriptorSetLayoutBinding vkBinding{};
        vkBinding.binding = binding.binding;
        vkBinding.descriptorType = toVkDescriptorType(binding.type);
        vkBinding.descriptorCount = binding.count;
        vkBinding.stageFlags = static_cast<VkShaderStageFlags>(binding.stages);
        vkBinding.pImmutableSamplers = nullptr;
        vkBindings.push_back(vkBinding);
    }
    
    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = static_cast<u32>(vkBindings.size());
    createInfo.pBindings = vkBindings.data();
    
    const auto& funcs = device.getDeviceFuncs();
    VkResult result = funcs.vkCreateDescriptorSetLayout(
        device.getDevice(), &createInfo, nullptr, &layout->m_layout);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            std::string("Failed to create descriptor set layout: ") + vkResultToString(result)));
    }
    
    // Set debug name if validation is enabled
    if (!desc.name.empty() && device.isValidationEnabled()) {
        device.setDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                           reinterpret_cast<u64>(layout->m_layout),
                           desc.name);
    }
    
    return layout;
}

// ============================================================================
// VulkanDescriptorPool Implementation
// ============================================================================

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice& device)
    : m_device(&device) {
}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    if (m_pool != VK_NULL_HANDLE && m_device) {
        const auto& funcs = m_device->getDeviceFuncs();
        funcs.vkDestroyDescriptorPool(m_device->getDevice(), m_pool, nullptr);
        m_pool = VK_NULL_HANDLE;
    }
}

VulkanDescriptorPool::VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept
    : m_device(other.m_device)
    , m_pool(other.m_pool)
    , m_maxSets(other.m_maxSets)
    , m_allocatedCount(other.m_allocatedCount)
    , m_freeIndividual(other.m_freeIndividual) {
    other.m_device = nullptr;
    other.m_pool = VK_NULL_HANDLE;
    other.m_allocatedCount = 0;
}

VulkanDescriptorPool& VulkanDescriptorPool::operator=(VulkanDescriptorPool&& other) noexcept {
    if (this != &other) {
        // Destroy existing pool
        if (m_pool != VK_NULL_HANDLE && m_device) {
            const auto& funcs = m_device->getDeviceFuncs();
            funcs.vkDestroyDescriptorPool(m_device->getDevice(), m_pool, nullptr);
        }
        
        m_device = other.m_device;
        m_pool = other.m_pool;
        m_maxSets = other.m_maxSets;
        m_allocatedCount = other.m_allocatedCount;
        m_freeIndividual = other.m_freeIndividual;
        
        other.m_device = nullptr;
        other.m_pool = VK_NULL_HANDLE;
        other.m_allocatedCount = 0;
    }
    return *this;
}

Result<std::unique_ptr<VulkanDescriptorPool>, Error>
VulkanDescriptorPool::create(VulkanDevice& device, const DescriptorPoolDesc& desc) {
    auto pool = std::unique_ptr<VulkanDescriptorPool>(
        new VulkanDescriptorPool(device));
    
    pool->m_maxSets = desc.maxSets;
    pool->m_freeIndividual = desc.freeIndividual;
    
    // Convert pool sizes to Vulkan format
    std::vector<VkDescriptorPoolSize> vkSizes;
    vkSizes.reserve(desc.sizes.size());
    
    for (const auto& size : desc.sizes) {
        VkDescriptorPoolSize vkSize{};
        vkSize.type = toVkDescriptorType(size.type);
        vkSize.descriptorCount = size.count;
        vkSizes.push_back(vkSize);
    }
    
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.maxSets = desc.maxSets;
    createInfo.poolSizeCount = static_cast<u32>(vkSizes.size());
    createInfo.pPoolSizes = vkSizes.data();
    
    if (desc.freeIndividual) {
        createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    }
    
    const auto& funcs = device.getDeviceFuncs();
    VkResult result = funcs.vkCreateDescriptorPool(
        device.getDevice(), &createInfo, nullptr, &pool->m_pool);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            std::string("Failed to create descriptor pool: ") + vkResultToString(result)));
    }
    
    // Set debug name if validation is enabled
    if (!desc.name.empty() && device.isValidationEnabled()) {
        device.setDebugName(VK_OBJECT_TYPE_DESCRIPTOR_POOL,
                           reinterpret_cast<u64>(pool->m_pool),
                           desc.name);
    }
    
    return pool;
}

Result<std::vector<VkDescriptorSet>, Error>
VulkanDescriptorPool::allocateSets(const VulkanDescriptorSetLayout& layout, u32 count) {
    if (m_allocatedCount + count > m_maxSets) {
        return std::unexpected(errors::outOfRange(
            "Descriptor pool exhausted. Allocated: " + std::to_string(m_allocatedCount) +
            ", Requested: " + std::to_string(count) +
            ", Max: " + std::to_string(m_maxSets)));
    }
    
    std::vector<VkDescriptorSetLayout> layouts(count, layout.getHandle());
    std::vector<VkDescriptorSet> sets(count);
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pool;
    allocInfo.descriptorSetCount = count;
    allocInfo.pSetLayouts = layouts.data();
    
    const auto& funcs = m_device->getDeviceFuncs();
    VkResult result = funcs.vkAllocateDescriptorSets(
        m_device->getDevice(), &allocInfo, sets.data());
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            std::string("Failed to allocate descriptor sets: ") + vkResultToString(result)));
    }
    
    m_allocatedCount += count;
    return sets;
}

void VulkanDescriptorPool::freeSets(std::span<const VkDescriptorSet> sets) {
    if (!m_freeIndividual || sets.empty()) {
        return;
    }
    
    const auto& funcs = m_device->getDeviceFuncs();
    funcs.vkFreeDescriptorSets(m_device->getDevice(), m_pool,
                               static_cast<u32>(sets.size()), sets.data());
    
    m_allocatedCount -= static_cast<u32>(sets.size());
}

void VulkanDescriptorPool::reset() {
    const auto& funcs = m_device->getDeviceFuncs();
    funcs.vkResetDescriptorPool(m_device->getDevice(), m_pool, 0);
    m_allocatedCount = 0;
}

// ============================================================================
// VulkanDescriptorWriter Implementation
// ============================================================================

VulkanDescriptorWriter::VulkanDescriptorWriter(VulkanDevice& device)
    : m_device(&device) {
}

VulkanDescriptorWriter& VulkanDescriptorWriter::clear() {
    m_writes.clear();
    m_bufferInfos.clear();
    m_imageInfos.clear();
    return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeBuffer(
    VkDescriptorSet set,
    u32 binding,
    DescriptorType type,
    const DescriptorBufferInfo& bufferInfo,
    u32 arrayElement) {
    
    // Store buffer info
    VkDescriptorBufferInfo vkBufferInfo{};
    vkBufferInfo.buffer = bufferInfo.buffer;
    vkBufferInfo.offset = bufferInfo.offset;
    vkBufferInfo.range = bufferInfo.range;
    m_bufferInfos.push_back(vkBufferInfo);
    
    // Create write descriptor
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = 1;
    write.descriptorType = toVkDescriptorType(type);
    write.pBufferInfo = &m_bufferInfos.back();
    
    m_writes.push_back(write);
    return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeBuffers(
    VkDescriptorSet set,
    u32 binding,
    DescriptorType type,
    std::span<const DescriptorBufferInfo> bufferInfos,
    u32 arrayElement) {
    
    if (bufferInfos.empty()) return *this;
    
    // Store buffer infos
    size_t firstIndex = m_bufferInfos.size();
    for (const auto& info : bufferInfos) {
        VkDescriptorBufferInfo vkInfo{};
        vkInfo.buffer = info.buffer;
        vkInfo.offset = info.offset;
        vkInfo.range = info.range;
        m_bufferInfos.push_back(vkInfo);
    }
    
    // Create write descriptor
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = static_cast<u32>(bufferInfos.size());
    write.descriptorType = toVkDescriptorType(type);
    write.pBufferInfo = &m_bufferInfos[firstIndex];
    
    m_writes.push_back(write);
    return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeImage(
    VkDescriptorSet set,
    u32 binding,
    DescriptorType type,
    const DescriptorImageInfo& imageInfo,
    u32 arrayElement) {
    
    // Store image info
    VkDescriptorImageInfo vkImageInfo{};
    vkImageInfo.sampler = imageInfo.sampler;
    vkImageInfo.imageView = imageInfo.imageView;
    vkImageInfo.imageLayout = imageInfo.imageLayout;
    m_imageInfos.push_back(vkImageInfo);
    
    // Create write descriptor
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = 1;
    write.descriptorType = toVkDescriptorType(type);
    write.pImageInfo = &m_imageInfos.back();
    
    m_writes.push_back(write);
    return *this;
}

VulkanDescriptorWriter& VulkanDescriptorWriter::writeImages(
    VkDescriptorSet set,
    u32 binding,
    DescriptorType type,
    std::span<const DescriptorImageInfo> imageInfos,
    u32 arrayElement) {
    
    if (imageInfos.empty()) return *this;
    
    // Store image infos
    size_t firstIndex = m_imageInfos.size();
    for (const auto& info : imageInfos) {
        VkDescriptorImageInfo vkInfo{};
        vkInfo.sampler = info.sampler;
        vkInfo.imageView = info.imageView;
        vkInfo.imageLayout = info.imageLayout;
        m_imageInfos.push_back(vkInfo);
    }
    
    // Create write descriptor
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = static_cast<u32>(imageInfos.size());
    write.descriptorType = toVkDescriptorType(type);
    write.pImageInfo = &m_imageInfos[firstIndex];
    
    m_writes.push_back(write);
    return *this;
}

void VulkanDescriptorWriter::update() {
    if (m_writes.empty()) return;
    
    const auto& funcs = m_device->getDeviceFuncs();
    funcs.vkUpdateDescriptorSets(
        m_device->getDevice(),
        static_cast<u32>(m_writes.size()),
        m_writes.data(),
        0, nullptr);
    
    // Clear after update
    clear();
}

// ============================================================================
// VulkanBindlessDescriptor Implementation
// ============================================================================

VulkanBindlessDescriptor::VulkanBindlessDescriptor(VulkanDevice& device)
    : m_device(&device) {
}

VulkanBindlessDescriptor::~VulkanBindlessDescriptor() {
    if (m_device) {
        const auto& funcs = m_device->getDeviceFuncs();
        
        // Note: Descriptor sets are freed when pool is destroyed
        if (m_pool != VK_NULL_HANDLE) {
            funcs.vkDestroyDescriptorPool(m_device->getDevice(), m_pool, nullptr);
        }
        if (m_layout != VK_NULL_HANDLE) {
            funcs.vkDestroyDescriptorSetLayout(m_device->getDevice(), m_layout, nullptr);
        }
    }
}

Result<std::unique_ptr<VulkanBindlessDescriptor>, Error>
VulkanBindlessDescriptor::create(VulkanDevice& device, u32 maxTextures) {
    auto bindless = std::unique_ptr<VulkanBindlessDescriptor>(
        new VulkanBindlessDescriptor(device));
    
    bindless->m_maxTextures = maxTextures;
    
    // Check if device supports descriptor indexing
    // For now, we assume it's supported if Vulkan 1.2+ or extension is enabled
    
    const auto& funcs = device.getDeviceFuncs();
    
    // Create descriptor set layout with variable descriptor count
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = maxTextures;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;
    
    VkDescriptorBindingFlags bindingFlags = 
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsInfo.bindingCount = 1;
    bindingFlagsInfo.pBindingFlags = &bindingFlags;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = &bindingFlagsInfo;
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;
    
    VkResult result = funcs.vkCreateDescriptorSetLayout(
        device.getDevice(), &layoutInfo, nullptr, &bindless->m_layout);
    
    if (result != VK_SUCCESS) {
        return std::unexpected(errors::graphics(
            std::string("Failed to create bindless descriptor set layout: ") + vkResultToString(result)));
    }
    
    // Create descriptor pool
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = maxTextures;
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    
    result = funcs.vkCreateDescriptorPool(
        device.getDevice(), &poolInfo, nullptr, &bindless->m_pool);
    
    if (result != VK_SUCCESS) {
        funcs.vkDestroyDescriptorSetLayout(device.getDevice(), bindless->m_layout, nullptr);
        return std::unexpected(errors::graphics(
            std::string("Failed to create bindless descriptor pool: ") + vkResultToString(result)));
    }
    
    // Allocate the single bindless descriptor set
    u32 variableCount = maxTextures;
    
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableInfo{};
    variableInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    variableInfo.descriptorSetCount = 1;
    variableInfo.pDescriptorCounts = &variableCount;
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &variableInfo;
    allocInfo.descriptorPool = bindless->m_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &bindless->m_layout;
    
    result = funcs.vkAllocateDescriptorSets(
        device.getDevice(), &allocInfo, &bindless->m_set);
    
    if (result != VK_SUCCESS) {
        funcs.vkDestroyDescriptorPool(device.getDevice(), bindless->m_pool, nullptr);
        funcs.vkDestroyDescriptorSetLayout(device.getDevice(), bindless->m_layout, nullptr);
        return std::unexpected(errors::graphics(
            std::string("Failed to allocate bindless descriptor set: ") + vkResultToString(result)));
    }
    
    // Initialize free indices
    bindless->m_freeIndices.reserve(maxTextures);
    for (u32 i = maxTextures; i > 0; --i) {
        bindless->m_freeIndices.push_back(i - 1);
    }
    
    // Set debug names
    if (device.isValidationEnabled()) {
        device.setDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                           reinterpret_cast<u64>(bindless->m_layout),
                           "BindlessTextureLayout");
        device.setDebugName(VK_OBJECT_TYPE_DESCRIPTOR_POOL,
                           reinterpret_cast<u64>(bindless->m_pool),
                           "BindlessTexturePool");
        device.setDebugName(VK_OBJECT_TYPE_DESCRIPTOR_SET,
                           reinterpret_cast<u64>(bindless->m_set),
                           "BindlessTextureSet");
    }
    
    return bindless;
}

Result<u32, Error> VulkanBindlessDescriptor::addTexture(VkImageView imageView, VkSampler sampler) {
    if (m_freeIndices.empty()) {
        return std::unexpected(errors::outOfRange(
            "Bindless texture array is full (max: " + std::to_string(m_maxTextures) + ")"));
    }
    
    u32 index = m_freeIndices.back();
    m_freeIndices.pop_back();
    
    // Update the descriptor
    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = sampler;
    imageInfo.imageView = imageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_set;
    write.dstBinding = 0;
    write.dstArrayElement = index;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;
    
    const auto& funcs = m_device->getDeviceFuncs();
    funcs.vkUpdateDescriptorSets(m_device->getDevice(), 1, &write, 0, nullptr);
    
    ++m_textureCount;
    return index;
}

void VulkanBindlessDescriptor::removeTexture(u32 index) {
    if (index >= m_maxTextures) return;
    
    m_freeIndices.push_back(index);
    --m_textureCount;
    
    // Note: We don't need to update the descriptor - it will just be unused
    // The shader should check validity via a separate bitmask or count
}

} // namespace nova::render::vulkan
