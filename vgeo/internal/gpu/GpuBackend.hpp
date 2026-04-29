#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <print>
#include <fstream>
#include <filesystem>
#include <regex>

namespace vgeo {

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

static std::vector<char> readFile(const std::string& filename) {

    std::string localPath = "Vienna-Game-Geometry-Library/vgeo/internal/gpu/" + filename;

    std::string input = std::filesystem::current_path().generic_string();
    std::regex re(R"(^.*?(?=Vienna-Game-Geometry-Library))");

    std::smatch match;
    std::string globalPath;
    if (std::regex_search(input, match, re)) {
        globalPath = match.str() + localPath;
    }
    
    std::ifstream file(globalPath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file! current path: " + std::filesystem::current_path().generic_string());
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


template <typename Bv>
class GpuBackend{
    
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_logicalDevice;
    VkQueue m_computeQueue;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_computePipeline;
    VkCommandPool m_commandPool;
    VkBuffer m_shaderStorageBuffer;
    VkDeviceMemory m_shaderStorageDeviceMemory;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    

    public:
        Bv m_boundingVolume;
        int test;
        GpuBackend(VkPhysicalDevice physicalDevice, Bv boundingVolume);

    private:

    uint32_t findComputeQueueFamilyIndex(){
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

        uint32_t result = 0;
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                result = i;
            }
        }
        return result;
    }

    void createLogicalDevice(uint32_t queueFamilyIndex){
        float priority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamilyIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;


        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueInfo;

        if (vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_logicalDevice, queueFamilyIndex, 0, &m_computeQueue);
    }

    void createDescriptorSetLayouts(){
        std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings{};
        layoutBindings[0].binding = 0;
        layoutBindings[0].descriptorCount = 1;
        layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[0].pImmutableSamplers = nullptr;
        layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

                
        layoutBindings[1].binding = 1;
        layoutBindings[1].descriptorCount = 1;
        layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBindings[1].pImmutableSamplers = nullptr;
        layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 2;
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createPipelineLayout(){
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

        if (vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline layout!");
        }
        ;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    void createComputePipeline(){
        auto code = readFile("comp.spv");
        if(code.empty()){
            throw std::runtime_error("Code is empty!");
        }
        VkShaderModule shaderModule = createShaderModule(code);

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.module = shaderModule;
        shaderStage.pName = "main";

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = shaderStage;
        pipelineInfo.layout = m_pipelineLayout;

        if (vkCreateComputePipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_computePipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline!");
        }

    }

    void createCommandPool(uint32_t queueFamilyIndex){
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool);

        VkCommandBuffer commandBuffer;
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0);
    }

    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_computeQueue);

        vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 3> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes[0].descriptorCount = 1;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes[1].descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = 2;

        if (vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void createDescriptorSets(VkBuffer shaderStorageBuffer, VkBuffer outputBuffer) {

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_descriptorSetLayout;
        
        m_descriptorSets.resize(1);
        if (vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        
        VkDescriptorBufferInfo storagebufferInfo{};
        storagebufferInfo.buffer = shaderStorageBuffer;
        storagebufferInfo.offset = 0;
        storagebufferInfo.range = sizeof(AABB);

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[0];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &storagebufferInfo;

        VkDescriptorBufferInfo outputbufferInfo{};
        outputbufferInfo.buffer = outputBuffer;
        outputbufferInfo.offset = 0;
        outputbufferInfo.range = sizeof(AABB);

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[0];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &outputbufferInfo;

        vkUpdateDescriptorSets(m_logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
};

template <typename Bv>
GpuBackend<Bv>::GpuBackend(VkPhysicalDevice physicalDevice, Bv boundingVolume){
    test = 1;
    m_boundingVolume = AABB(2,2,2,3,3,3);
    m_physicalDevice = physicalDevice;
    uint32_t queueFamilyIndex = findComputeQueueFamilyIndex();

    createLogicalDevice(queueFamilyIndex);
    createDescriptorSetLayouts();
    createPipelineLayout();
    createComputePipeline();
    createCommandPool(queueFamilyIndex);
    
    VkDeviceSize bufferSize = sizeof(Bv);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, &boundingVolume, (size_t)bufferSize);
    vkUnmapMemory(m_logicalDevice, stagingBufferMemory);
    
    VkBuffer shaderStorageBuffer;
    VkDeviceMemory shaderStorageBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
         VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorageBuffer, 
         shaderStorageBufferMemory);
    
    copyBuffer(stagingBuffer, shaderStorageBuffer, bufferSize);

    vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
    
    VkBuffer outputBuffer;
    VkDeviceMemory outputBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
         VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , outputBuffer, 
         outputBufferMemory);

    createDescriptorPool();
    createDescriptorSets(shaderStorageBuffer, outputBuffer);
    
    VkCommandBuffer commandBuffer;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorSets[0], 0, 0);
    
    vkCmdDispatch(commandBuffer, 1, 1, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;


    if (vkQueueSubmit(m_computeQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    };
    vkQueueWaitIdle(m_computeQueue);
    
    void* mapped;
    vkMapMemory(m_logicalDevice, outputBufferMemory, 0, bufferSize, 0, &mapped);

    memcpy(&m_boundingVolume, mapped, bufferSize);

    vkUnmapMemory(m_logicalDevice, outputBufferMemory);
}


}