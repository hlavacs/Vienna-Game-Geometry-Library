#pragma once

#include <print>
#include <fstream>
#include <filesystem>
#include <regex>
#include <vector>
#include <vulkan/vulkan.h>
#include <vgeo/internal/gpu/VulkanHandler.hpp>

namespace vgeo::internal::gpu {

template<typename T>
concept BoundingVolume = requires(T bv1, T bv2){
    {bv1.intersects(bv2)} -> std::same_as<bool>;
};

struct AABB{
    float minx;
    float miny;
    float minz;
    float padding1;
    float maxx;
    float maxy;
    float maxz;
    float padding2;
};

template <typename Bv>
class GpuBackend{
    
    VulkanHandler m_vulkanHandler;

    public:
        Bv m_boundingVolume;
        int test;
        GpuBackend(VkPhysicalDevice physicalDevice, Bv boundingVolume);
};

template <typename Bv>
GpuBackend<Bv>::GpuBackend(VkPhysicalDevice physicalDevice, Bv boundingVolume){
    test = 1;

    VulkanHandler m_vulkanHandler = VulkanHandler(physicalDevice);
    m_vulkanHandler.createLogicalDevice();
    m_vulkanHandler.createDescriptorSetLayouts();
    m_vulkanHandler.createPipelineLayout();
    m_vulkanHandler.createComputePipeline("shaders/comp.spv");
    m_vulkanHandler.createCommandPool();

    m_vulkanHandler.createInputStorageBuffers(&boundingVolume, sizeof(Bv));
    m_vulkanHandler.createOutputStorageBuffer(sizeof(Bv));

    m_vulkanHandler.createDescriptorPool();
    m_vulkanHandler.createDescriptorSets();
    m_vulkanHandler.runComputeShader(&m_boundingVolume);

}


}
