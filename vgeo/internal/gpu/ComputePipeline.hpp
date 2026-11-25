#pragma once

#include <vulkan/vulkan.h>

#include <TSMotor3D.h>

#include "vgeo/internal/gpu/StorageBuffer.hpp"
#include "vgeo/internal/gpu/VulkanHelpers.hpp"
#include "vgeo/ShapeType.hpp"
#include "vgeo/internal/gpu/shapes/GpuContact.hpp"

#include <vector>
#include <stdexcept>
#include <array>
#include <functional>
#include <cassert>
#include <algorithm>
#include <iostream>

namespace vgeo::internal::gpu{

struct CollisionTestPairData {
    uint32_t typeA;
    uint32_t typeB;
    uint32_t indexA;
    uint32_t indexB;
    alignas(8) uint64_t instanceHandleA;
    alignas(8) uint64_t instanceHandleB;
    uint32_t sizeA;
    uint32_t sizeB;
};

class ComputePipeline {

    VkDevice m_device;
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
        ComputePipeline(VkDevice& device, std::vector<ShapeType> s, VkShaderModule& shaderModule, const std::vector<std::reference_wrapper<GpuStorageBuffer>>& gpuBuffers, 
                const StorageBuffer calcDataBuffer, const StorageBuffer outputBuffer) : m_device{device}, m_structure{s}, m_gpuBuffers{gpuBuffers}, 
                m_calcDataBuffer{calcDataBuffer}, m_outputBuffer{outputBuffer} 
            {
                assert(gpuBuffers.size() == m_structure.size());
                m_bufferCount = gpuBuffers.size()+2;

                DescriptorSetLayoutCreator layoutCreator{m_device};
                layoutCreator.createLayout(m_descriptorSetLayouts[0], gpuBuffers.size());
                layoutCreator.createLayout(m_descriptorSetLayouts[1], 2);

                DescriptorPoolCreator poolCreator{m_device};
                poolCreator.createDescriptorPool(m_descriptorPool, m_bufferCount, 2);

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = m_descriptorPool;
                allocInfo.descriptorSetCount = 2;
                allocInfo.pSetLayouts = m_descriptorSetLayouts.data();

                if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate descriptor sets!");
                }

                updateAllDescriptorSets();

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

                if (vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_computePipeline) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create compute pipeline!");
                }
            }

        void updateDescriptor(ShapeType type){
            for(uint32_t i = 0; i < m_structure.size(); i++){
                if(type == m_structure[i]){
                    updateSingleDescriptor(m_descriptorSets[0], m_gpuBuffers[i].get().m_buffer, m_gpuBuffers[i].get().m_bufferSize, i);
                }
            }
        }

        void updateCalcDataDescriptorSet(){
            updateSingleDescriptor(m_descriptorSets[1], m_calcDataBuffer.m_buffer, m_calcDataBuffer.m_bufferSize, 0);
        }

        void updateOutputDescriptorSet(){
            updateSingleDescriptor(m_descriptorSets[1], m_outputBuffer.m_buffer, m_outputBuffer.m_bufferSize, 1);
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

        bool checkCalcDataBufferSize(){
            return m_calcData.size() * sizeof(CollisionTestPairData) <= m_calcDataBuffer.m_bufferSize;
        }

        void copyCalcDataToBuffer(){
            VkDeviceSize inputDataSize = m_calcData.size() * sizeof(CollisionTestPairData);
            memcpy(m_calcDataBuffer.m_data, m_calcData.data(), (size_t)inputDataSize);
        }

        void getResults(void* result){
            VkDeviceSize outputDataSize = m_calcData.size() * sizeof(GpuContact);
            memcpy(result, m_outputBuffer.m_data, outputDataSize);
        }

        void cleanup(){
            vkDestroyPipeline(m_device, m_computePipeline, nullptr);
            vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

            m_calcDataBuffer.cleanup(m_device);
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
    
    void updateAllDescriptorSets(){
    
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

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

        
};


};