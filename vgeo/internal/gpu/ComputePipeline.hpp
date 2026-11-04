#pragma once

#include <vulkan/vulkan.h>
#include "StorageBuffer.hpp"

namespace vgeo::internal::gpu{

class ComputePipeline {
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_descriptorSet;
    std::vector<StorageBuffer> m_storageBuffers;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_computePipeline;

    public:
        ComputePipeline() = default;
        ComputePipeline(VkDescriptorSetLayout dslayout, VkDescriptorPool pool, VkDescriptorSet set, std::vector<StorageBuffer> buffers, VkPipelineLayout pllayout, VkPipeline pipeline) 
            : m_descriptorSetLayout{dslayout}, m_descriptorPool{pool}, m_descriptorSet{set}, m_storageBuffers{buffers}, m_pipelineLayout{pllayout}, m_computePipeline{pipeline} {}

        void initComputePipeline(VkDevice& device, VkShaderModule& shaderModule, const std::vector<StorageBuffer>& buffers){
            
            VkDescriptorSetLayoutCreateInfo layoutInfo = createDescriptorSetLayoutInfo(buffers.size());
            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            m_storageBuffers = buffers;
            
            VkDescriptorPoolCreateInfo poolInfo = createDescriptorPoolInfo(buffers.size());

            if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &m_descriptorSetLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            std::vector<VkWriteDescriptorSet> descriptorWrites = writeDescriptorSet(m_descriptorSet, m_storageBuffers);

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

            VkPushConstantRange pushConstantRange;

            pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            pushConstantRange.offset = 0;
            pushConstantRange.size = sizeof(uint32_t);

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create compute pipeline layout!");
            }

            VkPipelineShaderStageCreateInfo shaderStage{};
            shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStage.module = shaderModule;
            shaderStage.pName = "main";

            VkComputePipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            pipelineInfo.stage = shaderStage;
            pipelineInfo.layout = m_pipelineLayout;

            if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_computePipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create compute pipeline!");
            }
        }
       
        
        VkDescriptorSetLayout& getDescriptorSetLayout() {return m_descriptorSetLayout;}
        VkDescriptorPool& getDescriptoPool() {return m_descriptorPool;}
        VkDescriptorSet& getDescriptorSet() {return m_descriptorSet;}
        StorageBuffer& getBuffer(const uint32_t index) {return m_storageBuffers[index];}
        StorageBuffer& getOutputBuffer() {return m_storageBuffers[m_storageBuffers.size()-1];}
        VkPipelineLayout& getPipelineLayout() {return m_pipelineLayout;}
        VkPipeline& getPipeline() {return m_computePipeline;}

        void getResults(void* result){
            memcpy(result, getOutputBuffer().m_data, getOutputBuffer().m_bufferSize);
        }

        void cleanup(VkDevice& device){
            vkDestroyPipeline(device, m_computePipeline, nullptr);
            vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
            
            for(StorageBuffer buffer : m_storageBuffers){
                buffer.cleanup(device);
            }

            vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
        }

    private:

    VkDescriptorSetLayoutCreateInfo createDescriptorSetLayoutInfo(const uint32_t bindingCount){

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.resize(bindingCount);
        for(uint32_t i=0; i<bindingCount; i++){
            layoutBindings[i].binding = i;
            layoutBindings[i].descriptorCount = 1;
            layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBindings[i].pImmutableSamplers = nullptr;
            layoutBindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindingCount;
        layoutInfo.pBindings = layoutBindings.data();

        return layoutInfo;
    }

    VkDescriptorPoolCreateInfo createDescriptorPoolInfo(uint32_t bufferCount){

        std::vector<VkDescriptorPoolSize> poolSizes;

        for(uint32_t i=0; i < bufferCount; i++){
            poolSizes.push_back(VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1
            });
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 2;

        return poolInfo;
    }
    
    std::vector<VkWriteDescriptorSet> writeDescriptorSet(VkDescriptorSet& descriptorSet, std::vector<StorageBuffer>& storageBuffers){
    
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.resize(storageBuffers.size());

        std::vector<VkDescriptorBufferInfo> storageBufferInfos;
        for(uint32_t i=0; i < storageBuffers.size(); i++){
            storageBufferInfos.push_back(VkDescriptorBufferInfo{});
        }

        for(uint32_t i=0; i < storageBuffers.size(); i++){
            
            storageBufferInfos[i].buffer = storageBuffers[i].m_buffer;
            storageBufferInfos[i].offset = 0;
            storageBufferInfos[i].range = storageBuffers[i].m_bufferSize;

            descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[i].dstSet = descriptorSet;
            descriptorWrites[i].dstBinding = i;
            descriptorWrites[i].dstArrayElement = 0;
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[i].descriptorCount = 1;
            descriptorWrites[i].pBufferInfo = &storageBufferInfos[i];
        }
        return descriptorWrites;
    }

        
};


};