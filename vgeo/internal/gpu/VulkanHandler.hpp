#pragma once

#include <vulkan/vulkan.h>
#include <fstream>
#include <filesystem>
#include <regex>
#include <vector>

#include "vgeo/Shape.hpp"

#include "vgeo/internal/gpu/StorageBuffer.hpp"
#include "vgeo/internal/gpu/ComputePipeline.hpp"
#include "vgeo/internal/gpu/shapes/GpuContact.hpp"

namespace vgeo::internal::gpu {

struct VulkanContext {
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_logicalDevice;
    VkQueue m_computeQueue;
    uint32_t m_computeQueueFamilyIndex;
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
        StorageBuffer createShapeStorageBuffer(vgeo::internal::cpu::Shape shape);
        ComputePipeline createPipeline(const std::string& filename, const std::vector<std::reference_wrapper<GpuStorageBuffer>>& gpuStorageBuffers, std::vector<ShapeType> structure);
        void createCommandPool();
        GpuStorageBuffer createGpuStorageBuffer(VkDeviceSize bufferSize);
        void updateGpuStorageBuffer(GpuStorageBuffer& buffer, void* inputData, uint32_t inputDataSize);
        void growGpuStorageBuffer(GpuStorageBuffer& buffer);
        void updateDescriptorBinding(ComputePipeline& pipeline, ShapeType type);
        void startRecordingCommandBuffer();
        void recordCommandBuffer(ComputePipeline& pipeline);
        void runComputeShader();
        void cleanup(GpuStorageBuffer& buffer);
        void cleanup(std::vector<ComputePipeline>& pipelines);


    private:

        StorageBuffer createStorageBuffer(VkDeviceSize bufferSize);
        VkDescriptorSetLayout createDescriptorSetLayout(const uint32_t bindingCount);
        VkDescriptorPool createDescriptorPool(uint32_t bufferCount);
        void writeDescriptorSet(VkDescriptorSet& descriptorSet, std::vector<StorageBuffer>& storageBuffers);

        uint32_t findComputeQueueFamilyIndex();
        VkShaderModule createShaderModule(const std::vector<char>& code);
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


};

}

