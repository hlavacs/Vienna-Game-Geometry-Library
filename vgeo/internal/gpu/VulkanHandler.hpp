#pragma once

#include <vulkan/vulkan.h>
#include <fstream>
#include <filesystem>
#include <regex>
#include <vector>

namespace vgeo::internal::gpu {

class VulkanHandler{

    VkPhysicalDevice m_physicalDevice;
    uint32_t m_computeQueueFamilyIndex;
    VkDevice m_logicalDevice;
    VkQueue m_computeQueue;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_computePipeline;
    VkCommandPool m_commandPool;
    std::vector<VkBuffer> m_inputStorageBuffers;
    std::vector<VkDeviceMemory> m_inputStorageBufferMemories;
    std::vector<uint32_t> m_inputStorageBufferSizes;
    VkBuffer m_outputBuffer;
    VkDeviceMemory m_outputBufferMemory;
    uint32_t m_outputBufferSize;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    public:
        VulkanHandler() = default;
        VulkanHandler(VkPhysicalDevice physicalDevice);
        void createLogicalDevice();
        void createDescriptorSetLayouts();
        void createPipelineLayout();
        void createComputePipeline(const std::string& filename);
        void createCommandPool();
        void createInputStorageBuffers(void* inputData, VkDeviceSize inputDataSize);
        void createOutputStorageBuffer(VkDeviceSize outputDataSize);
        void createDescriptorPool();
        void createDescriptorSets();
        void runComputeShader(void* result);


    private:
        uint32_t findComputeQueueFamilyIndex();
        VkShaderModule createShaderModule(const std::vector<char>& code);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


};

}

