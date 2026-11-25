#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

namespace vgeo::internal::gpu{

class DescriptorSetLayoutCreator{
    VkDevice& m_device;
public:
    DescriptorSetLayoutCreator(VkDevice& device) : m_device{device} {}

    void createLayout(VkDescriptorSetLayout& descriptorSetLayout, uint32_t bindings) {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.resize(bindings);
        for(uint32_t i=0; i < bindings; i++){
            layoutBindings[i].binding = i;
            layoutBindings[i].descriptorCount = 1;
            layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBindings[i].pImmutableSamplers = nullptr;
            layoutBindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindings;
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

};

class DescriptorPoolCreator{
    VkDevice& m_device;
public:
    DescriptorPoolCreator(VkDevice& device) : m_device{device} {}

    void createDescriptorPool(VkDescriptorPool& descriptorPool, uint32_t descriptorCount, uint32_t maxSets){

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSize.descriptorCount = descriptorCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = maxSets;

        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
};

};