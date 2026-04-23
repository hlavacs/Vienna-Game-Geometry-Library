#include <catch2/catch_test_macros.hpp>

#include<vulkan/vulkan.h>

TEST_CASE( "Vulkan instance is created", "[vulkan]" ) {
        // Create a Vulkan instance
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        VkInstance instance;
        vkCreateInstance(&createInfo, nullptr, &instance);

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            REQUIRE(deviceProperties.deviceName != nullptr);
        }


        // Destroy the Vulkan instance
        vkDestroyInstance(instance, nullptr);
}