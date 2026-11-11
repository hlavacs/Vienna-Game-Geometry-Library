#pragma once

#include <vulkan/vulkan.h>

#include <TSMotor3D.h>

#include "StorageBuffer.hpp"
#include "vgeo/ShapeType.hpp"
#include "vgeo/internal/gpu/shapes/GpuContact.hpp"

#include <vector>
#include <stdexcept>
#include <array>
#include <functional>
#include <cassert>
#include <algorithm>
#include<iostream>

namespace vgeo::internal::gpu{

struct CollisionTestPairData {
    uint32_t typeA;
    uint32_t typeB;
    uint32_t indexA;
    uint32_t indexB;
    uint64_t instanceHandleA;
    uint64_t instanceHandleB;
    uint32_t sizeA;
    uint32_t sizeB;
    Terathon::Motor3D motorA;
    Terathon::Motor3D motorB;
};

class ComputePipeline {

    std::vector<ShapeType> m_structure;
    std::vector<CollisionTestPairData> m_calcData;
    std::array<VkDescriptorSetLayout, 2> m_descriptorSetLayouts{};
    VkDescriptorPool m_descriptorPool;
    std::array<VkDescriptorSet, 2> m_descriptorSets;
    std::vector<std::reference_wrapper<GpuStorageBuffer>> m_gpuBuffers;
    StorageBuffer m_calcDataBuffer;
    StorageBuffer m_outputBuffer;
    uint32_t m_bufferCount;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_computePipeline;

    public:
        ComputePipeline(std::vector<ShapeType> s) : m_structure{s}{}

        void initComputePipeline(VkDevice& device, VkShaderModule& shaderModule, const std::vector<std::reference_wrapper<GpuStorageBuffer>>& gpuBuffers, 
            const StorageBuffer calcDataBuffer, const StorageBuffer outputBuffer)
            {
            assert(gpuBuffers.size() == m_structure.size());

            m_bufferCount = gpuBuffers.size()+2;

            createDescriptorSetLayout(device, gpuBuffers.size(), 0);

            createDescriptorSetLayout(device, 2, 1);

            m_gpuBuffers = gpuBuffers;
            m_calcDataBuffer = calcDataBuffer;
            m_outputBuffer = outputBuffer;
            
            createDescriptorPool(device);

            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = 2;
            allocInfo.pSetLayouts = m_descriptorSetLayouts.data();

            if (vkAllocateDescriptorSets(device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }

            updateAllDescriptorSets(device);

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

        void updateDescriptor(VkDevice& device, ShapeType type){
            uint32_t binding;
            for(uint32_t i = 0; i < m_structure.size(); i++){
                if(type == m_structure[i]){
                    binding = i;
                }
            }

            updateGpuDescriptor(device, m_gpuBuffers[binding].get(), binding);
        }

        std::vector<ShapeType> getStructure() {return m_structure;}
        std::vector<CollisionTestPairData> getCalcData() {return m_calcData;}
        uint32_t getCalcDataSize() {return m_calcData.size();}
        std::array<VkDescriptorSet, 2>& getDescriptorSets() {return m_descriptorSets;}
        std::vector<std::reference_wrapper<GpuStorageBuffer>> getGpuBuffers(){return m_gpuBuffers;}
        GpuStorageBuffer& getGpuBuffer(const uint32_t index) {return m_gpuBuffers[index];}
        StorageBuffer& getCalcDataBuffer() {return m_calcDataBuffer;}
        StorageBuffer& getOutputBuffer() {return m_outputBuffer;}
        VkPipelineLayout& getPipelineLayout() {return m_pipelineLayout;}
        VkPipeline& getPipeline() {return m_computePipeline;}

        bool referencesBuffer(ShapeType type){
            return std::any_of(m_structure.begin(), m_structure.end(), [type](ShapeType m_s) {return type == m_s;});
        }

        //checks if Pipeline contains the correct buffers
        bool checkStructure(std::vector<ShapeType>& structure){
            std::vector<bool> result;
            if(m_structure.size() == structure.size()){
                for(auto& s : structure){
                    if(referencesBuffer(s)){
                        result.push_back(true);
                    }
                    else{
                        result.push_back(false);
                    }
                }
            }
            else{
                return false;
            }
            return std::all_of(result.begin(), result.end(), [](bool b) {return b;});
        }

        void addCollisionPair(CollisionTestPairData pair){    
            m_calcData.push_back(pair);
        }

        void resetCalcData(){
            m_calcData.resize(0);
        }

        void copyCalcDataToBuffer(){
            VkDeviceSize inputDataSize = m_calcData.size() * sizeof(CollisionTestPairData);
            memcpy(m_calcDataBuffer.m_data, m_calcData.data(), (size_t)inputDataSize);
        }

        void getResults(void* result){
            VkDeviceSize outputDataSize = m_calcData.size() * sizeof(GpuContact);
            memcpy(result, m_outputBuffer.m_data, outputDataSize);
        }

        void cleanup(VkDevice& device){
            vkDestroyPipeline(device, m_computePipeline, nullptr);
            vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);

            m_calcDataBuffer.cleanup(device);
            m_outputBuffer.cleanup(device);

            vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(device, m_descriptorSetLayouts[0], nullptr);
            vkDestroyDescriptorSetLayout(device, m_descriptorSetLayouts[1], nullptr);
        }

    private:

    void createDescriptorSetLayout(VkDevice& device, uint32_t bindings, uint32_t layoutIndex){

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

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayouts[layoutIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createDescriptorPool(VkDevice device){

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSize.descriptorCount = m_bufferCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 2;

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void updateGpuDescriptor(VkDevice& device, const GpuStorageBuffer& buffer, uint32_t binding){
        VkWriteDescriptorSet descriptorWrite;
        VkDescriptorBufferInfo bufferInfo;

        bufferInfo.buffer = buffer.m_buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = buffer.m_bufferSize;

        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[0];
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
    
    void updateAllDescriptorSets(VkDevice& device){
    
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.resize(m_bufferCount);

        std::vector<VkDescriptorBufferInfo> storageBufferInfos;
        for(uint32_t i=0; i < m_bufferCount; i++){
            storageBufferInfos.push_back(VkDescriptorBufferInfo{});
        }

        for(uint32_t i=0; i < m_gpuBuffers.size(); i++){
            
            storageBufferInfos[i].buffer = m_gpuBuffers[i].get().m_buffer;
            storageBufferInfos[i].offset = 0;
            storageBufferInfos[i].range = m_gpuBuffers[i].get().m_bufferSize;

            descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[i].dstSet = m_descriptorSets[0];
            descriptorWrites[i].dstBinding = i;
            descriptorWrites[i].dstArrayElement = 0;
            descriptorWrites[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWrites[i].descriptorCount = 1;
            descriptorWrites[i].pBufferInfo = &storageBufferInfos[i];
        }

        uint32_t currIndex = m_gpuBuffers.size();

        storageBufferInfos[currIndex].buffer = m_calcDataBuffer.m_buffer;
        storageBufferInfos[currIndex].offset = 0;
        storageBufferInfos[currIndex].range = m_calcDataBuffer.m_bufferSize;

        descriptorWrites[currIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[currIndex].dstSet = m_descriptorSets[1];
        descriptorWrites[currIndex].dstBinding = 0;
        descriptorWrites[currIndex].dstArrayElement = 0;
        descriptorWrites[currIndex].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[currIndex].descriptorCount = 1;
        descriptorWrites[currIndex].pBufferInfo = &storageBufferInfos[currIndex];

        storageBufferInfos[currIndex+1].buffer = m_outputBuffer.m_buffer;
        storageBufferInfos[currIndex+1].offset = 0;
        storageBufferInfos[currIndex+1].range = m_outputBuffer.m_bufferSize;

        descriptorWrites[currIndex+1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[currIndex+1].dstSet = m_descriptorSets[1];
        descriptorWrites[currIndex+1].dstBinding = 1;
        descriptorWrites[currIndex+1].dstArrayElement = 0;
        descriptorWrites[currIndex+1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[currIndex+1].descriptorCount = 1;
        descriptorWrites[currIndex+1].pBufferInfo = &storageBufferInfos[currIndex+1];

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

        
};


};