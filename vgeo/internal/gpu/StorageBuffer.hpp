#pragma once

#include <vulkan/vulkan.h>

namespace vgeo::internal::gpu{

struct StorageBuffer {
    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;
    VkDeviceSize m_bufferSize;
    void* m_data;

    void cleanup(VkDevice& device){
        vkUnmapMemory(device, m_bufferMemory);
        vkDestroyBuffer(device, m_buffer, nullptr);
        vkFreeMemory(device, m_bufferMemory, nullptr);
    }
};

struct GpuStorageBuffer{
    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;
    VkDeviceSize m_bufferSize;

    void cleanup(VkDevice& device){
        vkDestroyBuffer(device, m_buffer, nullptr);
        vkFreeMemory(device, m_bufferMemory, nullptr);
    }
};

}