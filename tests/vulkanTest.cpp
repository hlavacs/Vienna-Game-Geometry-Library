#include "vgeo/internal/gpu/GpuBackend.hpp"

#include <catch2/catch_test_macros.hpp>
#include "catch2/catch_approx.hpp"

#include <vulkan/vulkan.h>

TEST_CASE("Vulkan instance is created", "[vulkan]") {
    // Create a Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    VkPhysicalDevice           physicalDevice;
    VkPhysicalDeviceProperties deviceProperties;

    for (const auto& device : devices) {
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        physicalDevice = device;
        break;
    }

    REQUIRE(deviceProperties.deviceName != VK_NULL_HANDLE);
    
    vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB> gpuBackend =
        vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB>(physicalDevice);

    
    
    gpuBackend.defineSphere(1.0, {0.0,0.0,0.0});
    gpuBackend.defineSphere(1.0, {1.9,0.0,0.0});
    gpuBackend.defineSphere(1.5, {1.0,0.0,0.0});
    gpuBackend.defineSphere(1.5, {2.0,0.0,0.0});
    gpuBackend.defineCapsule(1.0, {-1.0,0.0,0.0}, {1.0,0.0,0.0});
    gpuBackend.defineCapsule(1.0, {0.0,-1.0,0.0}, {0.0,1.0,0.0});

    gpuBackend.initPipeline();
    
    gpuBackend.querySpheres();

    CHECK(gpuBackend.test == 4);
    CHECK(gpuBackend.m_contacts[0].hasContact == true);
    CHECK(gpuBackend.m_contacts[0].depth == Catch::Approx(0.1));
    CHECK(gpuBackend.m_contacts[0].normal.x == -1.0);
    CHECK(gpuBackend.m_contacts[0].normal.y == 0.0);
    CHECK(gpuBackend.m_contacts[0].normal.z == 0.0);
    CHECK(gpuBackend.m_contacts[1].hasContact == true);
    CHECK(gpuBackend.m_contacts[1].normal.x == -1.0);
    
    CHECK(gpuBackend.m_contacts2[0].hasContact == true);
    CHECK(gpuBackend.m_contacts2[0].depth == 1.0);
    CHECK(gpuBackend.m_contacts2[0].normal.x  == -1.0);

    gpuBackend.defineSphere(0.1, {1.0, 1.0,1.0});
    gpuBackend.defineSphere(0.1, {2.0,2.0,2.0});
    gpuBackend.querySpheres();

    CHECK(gpuBackend.test == 6);
    CHECK(gpuBackend.m_contacts[0].hasContact == true);
    CHECK(gpuBackend.m_contacts[0].depth == Catch::Approx(0.1));
    CHECK(gpuBackend.m_contacts[0].normal.x == -1.0);
    CHECK(gpuBackend.m_contacts[0].normal.y == 0.0);
    CHECK(gpuBackend.m_contacts[0].normal.z == 0.0);
    CHECK(gpuBackend.m_contacts[1].hasContact == true);
    CHECK(gpuBackend.m_contacts[2].hasContact == false);
    CHECK(gpuBackend.m_contacts[2].depth == 0.0);



    gpuBackend.cleanup();

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);
}
