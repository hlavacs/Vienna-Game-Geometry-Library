#pragma once

#include <vulkan/vulkan.h>

#include <TSMotor3D.h>

#include "vgeo/internal/gpu/StorageBuffer.hpp"
#include "vgeo/internal/gpu/VulkanHelpers.hpp"
#include "vgeo/ShapeType.hpp"

#include <unordered_set>
#include <array>
#include <stdexcept>

namespace vgeo::internal::gpu{

struct TransformData {
    uint32_t m_index;
    uint32_t m_size;
    Terathon::Motor3D m_motor;
    float m_scale;

    bool operator==(const TransformData& otherData) const noexcept {
        return m_index == otherData.m_index;
    }
};

}; // namespace vgeo::internal::gpu

namespace std {

template <>
struct hash<vgeo::internal::gpu::TransformData> {
    size_t operator()(const vgeo::internal::gpu::TransformData& h) const noexcept {
        return hash<uint32_t>{}(h.m_index);
    }
};

} // namespace std

namespace vgeo::internal::gpu{

class TransformPipeline {

    #define NUMBER_OF_BUFFERS 3
    VkDevice m_device;
    std::unordered_set<TransformData> m_transformData;
    std::array<VkDescriptorSetLayout, 2> m_descriptorSetLayouts{};
    VkDescriptorPool m_descriptorPool;
    std::array<VkDescriptorSet, 2> m_descriptorSets; 
    GpuStorageBuffer m_inputBuffer;
    GpuStorageBuffer m_outputBuffer;
    StorageBuffer m_transformDataBuffer;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_transformPipeline;

public:
    TransformPipeline() = default;
    TransformPipeline(VkDevice& device, VkShaderModule& shaderModule, const GpuStorageBuffer inputBuffer, const GpuStorageBuffer outputBuffer, 
            const StorageBuffer transformDataBuffer) : m_device{device}, m_inputBuffer{inputBuffer}, m_outputBuffer{outputBuffer}, m_transformDataBuffer{transformDataBuffer} 
    {
        DescriptorSetLayoutCreator layoutCreator{m_device};
        layoutCreator.createLayout(m_descriptorSetLayouts[0], 2);
        layoutCreator.createLayout(m_descriptorSetLayouts[1], 1);

        DescriptorPoolCreator poolCreator{m_device};
        poolCreator.createDescriptorPool(m_descriptorPool, NUMBER_OF_BUFFERS, 2);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = 2;
        allocInfo.pSetLayouts = m_descriptorSetLayouts.data();

        if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        updateDescriptorSets();

        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(uint32_t);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
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

        if (vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_transformPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline!");
        }
    }

    void updateInOutDescriptors() {
        updateSingleDescriptor(m_descriptorSets[0], m_inputBuffer.m_buffer, m_inputBuffer.m_bufferSize, 0);
        updateSingleDescriptor(m_descriptorSets[0], m_outputBuffer.m_buffer, m_outputBuffer.m_bufferSize, 1);
    }

    void updateTransformDataDescriptorSet(){
        updateSingleDescriptor(m_descriptorSets[1], m_transformDataBuffer.m_buffer, m_transformDataBuffer.m_bufferSize, 0);
    }

    GpuStorageBuffer& getInputBuffer(){return m_inputBuffer;}
    GpuStorageBuffer& getOutputBuffer(){return m_outputBuffer;}
    StorageBuffer& getTransformDataBuffer(){return m_transformDataBuffer;}
    uint32_t getTransformDataSize(){return m_transformData.size();}
    std::array<VkDescriptorSet, 2>& getDescriptorSets() {return m_descriptorSets;}
    VkPipelineLayout& getPipelineLayout() {return m_pipelineLayout;}
    VkPipeline& getPipeline() {return m_transformPipeline;}

    void addTransformData(TransformData data){    
        m_transformData.insert(data);
    }

    void resetTransformData(){
        m_transformData.clear();
    }

    bool checkTransformDataBufferSize(){
        return m_transformData.size() * sizeof(TransformData) <= m_transformDataBuffer.m_bufferSize;
    }

    void copyTransformDataToBuffer(){
        VkDeviceSize inputDataSize = m_transformData.size() * sizeof(TransformData);
        std::vector<TransformData> transformData;
        transformData.reserve(m_transformData.size());

        for(const TransformData& t : m_transformData){
            transformData.push_back(t);
        }

        memcpy(m_transformDataBuffer.m_data, transformData.data(), (size_t)inputDataSize);
    }

    void cleanup(){
        vkDestroyPipeline(m_device, m_transformPipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

        m_transformDataBuffer.cleanup(m_device);
        m_inputBuffer.cleanup(m_device);
        m_outputBuffer.cleanup(m_device);

        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayouts[0], nullptr);
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayouts[1], nullptr);
    }

private:

    void updateSingleDescriptor(VkDescriptorSet& descriptorSet, VkBuffer& buffer, VkDeviceSize bufferSize, uint32_t binding){
        VkWriteDescriptorSet descriptorWrite{};
        VkDescriptorBufferInfo bufferInfo{};

        bufferInfo.buffer = buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = bufferSize;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
    }

    void updateDescriptorSets() {

        std::vector<VkWriteDescriptorSet> descriptorWrites(NUMBER_OF_BUFFERS, VkWriteDescriptorSet{});

        std::vector<VkDescriptorBufferInfo> storageBufferInfos(NUMBER_OF_BUFFERS, VkDescriptorBufferInfo{});
        
        storageBufferInfos[0].buffer = m_inputBuffer.m_buffer;
        storageBufferInfos[0].offset = 0;
        storageBufferInfos[0].range = m_inputBuffer.m_bufferSize;

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[0];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &storageBufferInfos[0];

        storageBufferInfos[1].buffer = m_outputBuffer.m_buffer;
        storageBufferInfos[1].offset = 0;
        storageBufferInfos[1].range = m_outputBuffer.m_bufferSize;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[0];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &storageBufferInfos[1];

        storageBufferInfos[2].buffer = m_transformDataBuffer.m_buffer;
        storageBufferInfos[2].offset = 0;
        storageBufferInfos[2].range = m_transformDataBuffer.m_bufferSize;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = m_descriptorSets[1];
        descriptorWrites[2].dstBinding = 0;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &storageBufferInfos[2];

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

#undef NUMBER_OF_BUFFERS
};

};

