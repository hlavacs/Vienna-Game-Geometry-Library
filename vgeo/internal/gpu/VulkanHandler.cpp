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

StorageBuffer VulkanHandler::createStorageBuffer(VkDeviceSize bufferSize) {

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
    
    void* data;
    vkMapMemory(m_logicalDevice, bufferMemory, 0, bufferSize, 0, &data);

    return StorageBuffer{buffer, bufferMemory, bufferSize, data};
}

GpuStorageBuffer VulkanHandler::createGpuStorageBuffer(VkDeviceSize bufferSize){
    VkBuffer gpuBuffer;
    VkDeviceMemory gpuBufferMemory;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, gpuBuffer, gpuBufferMemory);

    return GpuStorageBuffer{gpuBuffer, gpuBufferMemory, bufferSize};
}

void VulkanHandler::updateGpuStorageBuffer(GpuStorageBuffer& buffer, void* inputData, uint32_t inputDataSize) {
    assert(inputDataSize <= buffer.m_bufferSize);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(inputDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(m_logicalDevice, stagingBufferMemory, 0, inputDataSize, 0, &data);
    memcpy(data, inputData, (size_t)inputDataSize);
    vkUnmapMemory(m_logicalDevice, stagingBufferMemory);

    copyBuffer(stagingBuffer, buffer.m_buffer, inputDataSize);

    vkDestroyBuffer(m_logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_logicalDevice, stagingBufferMemory, nullptr);
}

void VulkanHandler::growGpuStorageBuffer(GpuStorageBuffer& buffer){

    VkBuffer oldBuffer = buffer.m_buffer;
    VkDeviceMemory oldBufferMemory = buffer.m_bufferMemory;
    VkDeviceSize oldBufferSize = buffer.m_bufferSize;

    VkBuffer newBuffer;
    VkDeviceMemory newBufferMemory;

    VkDeviceSize newBufferSize = oldBufferSize * 2;

    createBuffer(newBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, newBuffer, newBufferMemory);

    copyBuffer(oldBuffer, newBuffer, oldBufferSize);

    vkDestroyBuffer(m_logicalDevice, oldBuffer, nullptr);
    vkFreeMemory(m_logicalDevice, oldBufferMemory, nullptr);

    buffer.m_buffer = newBuffer;
    buffer.m_bufferMemory = newBufferMemory;
    buffer.m_bufferSize = newBufferSize;
}

ComputePipeline VulkanHandler::createPipeline(const std::string& filename, const std::vector<std::reference_wrapper<GpuStorageBuffer>>& gpuStorageBuffers, std::vector<ShapeType> structure){

    StorageBuffer calcDataStorageBuffer = createStorageBuffer(sizeof(CollisionTestPairData)*SHADER_LOCAL_SIZE_X);
    StorageBuffer outputStorageBuffer = createStorageBuffer(sizeof(GpuContact)*SHADER_LOCAL_SIZE_X);

    auto code = readFile(filename);
    if(code.empty()){
        throw std::runtime_error("Code is empty!");
    }
    VkShaderModule shaderModule = createShaderModule(code);

    ComputePipeline pipeline{structure};
    pipeline.initComputePipeline(m_logicalDevice, shaderModule, gpuStorageBuffers, calcDataStorageBuffer, outputStorageBuffer);
    
    vkDestroyShaderModule(m_logicalDevice, shaderModule, nullptr);

    return pipeline;
}

void VulkanHandler::updateDescriptorBinding(ComputePipeline& pipeline, ShapeType type){
    pipeline.updateDescriptor(m_logicalDevice, type);
}

void VulkanHandler::startRecordingCommandBuffer(){

    if (vkResetCommandBuffer(m_commandBuffer, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void VulkanHandler::recordCommandBuffer(ComputePipeline& pipeline) {

    pipeline.copyCalcDataToBuffer();

    uint32_t pushConstant = pipeline.getCalcDataSize();

    vkCmdPushConstants(m_commandBuffer, pipeline.getPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &pushConstant);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getPipeline());
    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getPipelineLayout(), 0, 2, pipeline.getDescriptorSets().data(), 0, 0);

    uint32_t groupCount = (pipeline.getCalcDataSize() + SHADER_LOCAL_SIZE_X - 1) / SHADER_LOCAL_SIZE_X;
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

void VulkanHandler::cleanup(GpuStorageBuffer& buffer){
    buffer.cleanup(m_logicalDevice);
}

void VulkanHandler::cleanup(std::vector<ComputePipeline>& pipelines) {

    for (ComputePipeline pipeline : pipelines){
        pipeline.cleanup(m_logicalDevice);
    }

    vkDeviceWaitIdle(m_logicalDevice);

    vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &m_commandBuffer);
    vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
    vkDestroyDevice(m_logicalDevice, nullptr);

}

}
