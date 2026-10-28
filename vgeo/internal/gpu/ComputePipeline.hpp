#pragma once

#include <vulkan/vulkan.h>
#include "StorageBuffer.hpp"
#include "IntersectionType.hpp"

namespace vgeo::internal::gpu{

//template<IntersectionTest iT>
struct ComputePipeline {
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_descriptorSet;
    StorageBuffer m_inputBuffers;
    StorageBuffer m_outputBuffer;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_computePipeline;
};


};