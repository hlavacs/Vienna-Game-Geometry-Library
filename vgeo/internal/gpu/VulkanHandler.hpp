#pragma once

#include <vulkan/vulkan.h>
#include <fstream>
#include <filesystem>
#include <regex>
#include <vector>

#include "helpers/StorageBuffer.hpp"

namespace vgeo::internal::gpu {

struct ComputePipeline {
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_descriptorSet;
    StorageBuffer m_inputBuffers;
    StorageBuffer m_outputBuffer;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_computePipeline;
};

class VulkanHandler{
    const uint32_t SHADER_LOCAL_SIZE_X = 64;
    const uint32_t NR_OF_PIPELINES = 2;

    VkPhysicalDevice m_physicalDevice;
    uint32_t m_computeQueueFamilyIndex;
    VkDevice m_logicalDevice;
    VkQueue m_computeQueue;
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    VkPushConstantRange m_pushConstantRange;
    std::vector<VkPipelineLayout> m_pipelineLayouts;
    std::vector<VkPipeline> m_computePipelines;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    std::vector<VkBuffer> m_inputStorageBuffers;
    std::vector<VkDeviceMemory> m_inputStorageBufferMemories;
    std::vector<uint32_t> m_inputStorageBufferSizes;
    VkBuffer m_outputBuffer;
    VkDeviceMemory m_outputBufferMemory;
    uint32_t m_outputBufferSize;
    std::vector<VkDescriptorPool> m_descriptorPools;
    std::vector<VkDescriptorSet> m_descriptorSets;

    public:
        VulkanHandler(VkPhysicalDevice& physicalDevice);
        void createLogicalDevice();
        ComputePipeline createPipeline(const std::string& filename, const std::vector<VkDeviceSize>& bufferSizes);

        void createPushConstantRange();

        void createCommandPool();

        StorageBuffer createStorageBuffer(VkDeviceSize bufferSize);


        void startRecordingCommandBuffer();
        void recordCommandBuffer(void* inputData, VkDeviceSize inputDataSize, ComputePipeline pipeline);
        void runComputeShader();
        void getResults(void* result, ComputePipeline pipeline);
        void cleanup(std::vector<ComputePipeline> pipelines);


    private:

        VkDescriptorSetLayout createDescriptorSetLayout();
        VkDescriptorPool createDescriptorPool(uint32_t bufferCount);

        uint32_t findComputeQueueFamilyIndex();
        VkShaderModule createShaderModule(const std::vector<char>& code);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


};

}

