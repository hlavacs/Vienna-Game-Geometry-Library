#pragma once

#include <vulkan/vulkan.h>

namespace vgeo::internal::gpu{

struct StorageBuffer {
    VkBuffer m_buffer;
    VkDeviceMemory m_bufferMemory;
    VkDeviceSize m_bufferSize;
    void* m_data;

    void cleanup(VkDevice logicalDevice){
        vkDestroyBuffer(logicalDevice, m_buffer, nullptr);
        vkFreeMemory(logicalDevice, m_bufferMemory, nullptr);
    }

};

}