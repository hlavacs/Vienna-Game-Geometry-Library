#include <vgeo/internal/gpu/GpuBackend.hpp>

<<<<<<< HEAD
namespace vgeo {

    
    /*
    GpuBackend::GpuBackend(){
        // Create a Vulkan instance
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
=======
namespace vgeo::internal {
GpuBackend::GpuBackend() {
    // Create a Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
>>>>>>> 00f3bb6c0fb0b95af1a1844b4076260fc0232159

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

<<<<<<< HEAD
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            

            std::println("GPU: {}", deviceProperties.deviceName);
        }

        // Destroy the Vulkan instance
        vkDestroyInstance(instance, nullptr);
    }
    */

}
=======
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        std::print("GPU: {}\n", deviceProperties.deviceName);
    }

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);
}
} // namespace vgeo::internal
>>>>>>> 00f3bb6c0fb0b95af1a1844b4076260fc0232159
