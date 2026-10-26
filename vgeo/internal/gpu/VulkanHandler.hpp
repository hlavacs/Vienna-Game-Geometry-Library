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

    VkPhysicalDevice m_physicalDevice;
    uint32_t m_computeQueueFamilyIndex;
    VkDevice m_logicalDevice;
    VkQueue m_computeQueue;


    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;

    public:
        VulkanHandler(VkPhysicalDevice& physicalDevice);
        void createLogicalDevice();
        ComputePipeline createPipeline(const std::string& filename, const std::vector<VkDeviceSize>& bufferSizes);
        void createCommandPool();
        void startRecordingCommandBuffer();
        void recordCommandBuffer(void* inputData, VkDeviceSize inputDataSize, uint32_t inputDataEnd, ComputePipeline pipeline);
        void runComputeShader();
        void getResults(void* result, ComputePipeline pipeline);
        void cleanup(std::vector<ComputePipeline> pipelines);


    private:

        StorageBuffer createStorageBuffer(VkDeviceSize bufferSize);
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

