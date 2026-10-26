#include <cstring>
#include <vgeo/internal/gpu/VulkanHandler.hpp>

namespace vgeo::internal::gpu{

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


uint32_t VulkanHandler::findComputeQueueFamilyIndex(){
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

VulkanHandler::VulkanHandler(VkPhysicalDevice& physicalDevice){
    m_physicalDevice = physicalDevice;
    m_computeQueueFamilyIndex = findComputeQueueFamilyIndex();
    m_descriptorSetLayouts.resize(NR_OF_PIPELINES);
    m_pipelineLayouts.resize(NR_OF_PIPELINES);
    m_computePipelines.resize(NR_OF_PIPELINES);
    m_descriptorPools.resize(NR_OF_PIPELINES);
}

void VulkanHandler::createLogicalDevice(){
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = m_computeQueueFamilyIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;


    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    if (vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_logicalDevice, m_computeQueueFamilyIndex, 0, &m_computeQueue);
}


void VulkanHandler::createPushConstantRange(){
    m_pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    m_pushConstantRange.offset = 0;
    m_pushConstantRange.size = sizeof(uint32_t);
}

VkShaderModule VulkanHandler::createShaderModule(const std::vector<char>& code) {
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

void VulkanHandler::createCommandPool(){
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_computeQueueFamilyIndex;

    vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool);


    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &m_commandBuffer);
}

uint32_t VulkanHandler::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanHandler::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory){
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

VkCommandBuffer VulkanHandler::beginSingleTimeCommands() {
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

void VulkanHandler::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_computeQueue);

    vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
}

void VulkanHandler::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

ComputePipeline VulkanHandler::createPipeline(const std::string& filename, const std::vector<VkDeviceSize>& bufferSizes){

    VkDescriptorSetLayout descriptorSetLayout = createDescriptorSetLayout();
    
    std::vector<StorageBuffer> storageBuffers;
    
    for (VkDeviceSize bufferSize : bufferSizes){
        storageBuffers.push_back(createStorageBuffer(bufferSize));
    }

    VkDescriptorPool descriptorPool = createDescriptorPool(bufferSizes.size());

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

    VkDescriptorBufferInfo inputStoragebufferInfo{};
    inputStoragebufferInfo.buffer = storageBuffers[0].m_buffer;
    inputStoragebufferInfo.offset = 0;
    inputStoragebufferInfo.range = storageBuffers[0].m_bufferSize;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &inputStoragebufferInfo;

    VkDescriptorBufferInfo outputBufferInfo{};
    outputBufferInfo.buffer = storageBuffers[1].m_buffer;
    outputBufferInfo.offset = 0;
    outputBufferInfo.range = storageBuffers[1].m_bufferSize;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &outputBufferInfo;

    vkUpdateDescriptorSets(m_logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

    VkPipelineLayout pipelineLayout;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &m_pushConstantRange;

    if (vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkPipeline computePipeline;

    auto code = readFile(filename);
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
    pipelineInfo.layout = pipelineLayout;

    if (vkCreateComputePipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    return ComputePipeline{descriptorSetLayout, descriptorPool, descriptorSet, storageBuffers[0], storageBuffers[1], pipelineLayout, computePipeline};
}

VkDescriptorSetLayout VulkanHandler::createDescriptorSetLayout(){

    VkDescriptorSetLayout descriptorSetLayout;
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

    if (vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    return descriptorSetLayout;
}

StorageBuffer VulkanHandler::createStorageBuffer(VkDeviceSize bufferSize) {

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
    
    void* data;
    vkMapMemory(m_logicalDevice, bufferMemory, 0, bufferSize, 0, &data);

    return StorageBuffer{buffer, bufferMemory, bufferSize, data};
}

VkDescriptorPool VulkanHandler::createDescriptorPool(uint32_t bufferCount){

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorPoolSize> poolSizes;

    for(uint32_t i=0; i < bufferCount; i++){
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1
        });
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 2;

    if (vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    return descriptorPool;
}


void VulkanHandler::startRecordingCommandBuffer(){
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

}

void VulkanHandler::recordCommandBuffer(void* inputData, VkDeviceSize inputDataSize, ComputePipeline pipeline) {

    memcpy(pipeline.m_inputBuffers.m_data, inputData, (size_t)inputDataSize);

    vkCmdPushConstants(m_commandBuffer, pipeline.m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &pipeline.m_inputBuffers.m_bufferSize);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.m_computePipeline);
    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.m_pipelineLayout, 0, 1, &pipeline.m_descriptorSet, 0, 0);

    uint32_t groupCount = (pipeline.m_inputBuffers.m_bufferSize + SHADER_LOCAL_SIZE_X - 1) / SHADER_LOCAL_SIZE_X;
    vkCmdDispatch(m_commandBuffer, groupCount, 1, 1);

}

void VulkanHandler::runComputeShader(){
    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;


    if (vkQueueSubmit(m_computeQueue, 1, &submitInfo, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    };
    vkQueueWaitIdle(m_computeQueue);
}

void VulkanHandler::getResults(void* result, ComputePipeline pipeline){

    memcpy(result, pipeline.m_outputBuffer.m_data, pipeline.m_outputBuffer.m_bufferSize);
}


void VulkanHandler::cleanup(std::vector<ComputePipeline> pipelines) {

    for (ComputePipeline pipeline : pipelines){
        vkDestroyPipeline(m_logicalDevice, pipeline.m_computePipeline, nullptr);
        vkDestroyPipelineLayout(m_logicalDevice, pipeline.m_pipelineLayout, nullptr);

        pipeline.m_inputBuffers.cleanup(m_logicalDevice);
        pipeline.m_outputBuffer.cleanup(m_logicalDevice);

        vkDestroyDescriptorPool(m_logicalDevice, pipeline.m_descriptorPool, nullptr);

        vkDestroyDescriptorSetLayout(m_logicalDevice, pipeline.m_descriptorSetLayout, nullptr);
    }


    vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);

    vkDestroyDevice(m_logicalDevice, nullptr);

}

}
