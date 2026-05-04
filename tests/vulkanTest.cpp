#include <catch2/catch_test_macros.hpp>

#include<vulkan/vulkan.h>
#include <vgeo/internal/gpu/GpuBackend.hpp>
#include <vgeo/internal/gpu/VulkanHandler.hpp>

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
        
        vgeo::internal::gpu::AABB testAABB = vgeo::internal::gpu::AABB(0,0,0,99,1,1,1,99);

        vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB> gpuBackend = vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB>(physicalDevice, testAABB);

        std::vector<int> testvec1 = {1,1,1};
        std::vector<int> testvec2 = {2,2,2};

        REQUIRE(gpuBackend.test == 1);
        REQUIRE(gpuBackend.m_boundingVolume.minx == 1);
        REQUIRE(gpuBackend.m_boundingVolume.miny == 1);
        REQUIRE(gpuBackend.m_boundingVolume.minz == 1);
        REQUIRE(gpuBackend.m_boundingVolume.maxx == 2);
        REQUIRE(gpuBackend.m_boundingVolume.maxy == 2);
        REQUIRE(gpuBackend.m_boundingVolume.maxz == 2);

        // Destroy the Vulkan instance
        vkDestroyInstance(instance, nullptr);
}