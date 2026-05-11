#include <catch2/catch_test_macros.hpp>

#include<vulkan/vulkan.h>
#include "vgeo/internal/gpu/GpuBackend.hpp"

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
        VkPhysicalDevice physicalDevice;
        VkPhysicalDeviceProperties deviceProperties;

        for (const auto& device : devices) {
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            physicalDevice = device;
            break;
        }

        REQUIRE(deviceProperties.deviceName != VK_NULL_HANDLE);
        
        vgeo::AaBoxDesc testBox = vgeo::AaBoxDesc({0,0,0}, {1,1,1});
        vgeo::AaBoxDesc testBox2 = vgeo::AaBoxDesc({2,2,2}, {3,3,3});

        vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB> gpuBackend = vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB>(physicalDevice);
        gpuBackend.add(testBox);
        gpuBackend.add(testBox2);
        gpuBackend.queryBoxes();

        std::vector<int> testvec1 = {1,1,1};
        std::vector<int> testvec2 = {2,2,2};

        REQUIRE(gpuBackend.test == 1);
        REQUIRE(gpuBackend.m_testBoxes[0].min_x == 1);
        REQUIRE(gpuBackend.m_testBoxes[0].min_y == 1);
        REQUIRE(gpuBackend.m_testBoxes[0].min_z == 1);
        REQUIRE(gpuBackend.m_testBoxes[0].max_x == 2);
        REQUIRE(gpuBackend.m_testBoxes[0].max_y == 2);
        REQUIRE(gpuBackend.m_testBoxes[0].max_z == 2);
        
        REQUIRE(gpuBackend.m_testBoxes[1].min_x == 3);
        REQUIRE(gpuBackend.m_testBoxes[1].min_y == 3);
        REQUIRE(gpuBackend.m_testBoxes[1].min_z == 3);
        REQUIRE(gpuBackend.m_testBoxes[1].max_x == 4);
        REQUIRE(gpuBackend.m_testBoxes[1].max_y == 4);
        REQUIRE(gpuBackend.m_testBoxes[1].max_z == 4);
        
        // Destroy the Vulkan instance
        vkDestroyInstance(instance, nullptr);

}