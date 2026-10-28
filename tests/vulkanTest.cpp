#include "vgeo/internal/gpu/GpuBackend.hpp"
#include "vgeo/internal/InstanceRegistry.hpp"

#include <catch2/catch_test_macros.hpp>
#include "catch2/catch_approx.hpp"

#include <vulkan/vulkan.h>


vgeo::internal::cpu::Sphere testSphereBuilder(vgeo::Vec3 center, vgeo::real radius) {
    vgeo::internal::cpu::Sphere sphere{radius};
    return sphere.applyTransform(Terathon::Motor3D::MakeTranslation({center.x, center.y, center.z}), 1.0);   
}

vgeo::internal::cpu::Capsule testCapsuleBuilder(vgeo::Vec3 center, vgeo::real halfLength, vgeo::real radius) {
    vgeo::internal::cpu::Capsule capsule{halfLength, radius};
    return capsule.applyTransform(Terathon::Motor3D::MakeTranslation({center.x, center.y, center.z}), 1.0);    
}

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

    vgeo::internal::InstanceRegistry registry;
    std::vector<vgeo::InstanceHandle> handles;

    for(uint32_t i=0; i<10;i++){
        handles.push_back(registry.allocate());
    }

    gpuBackend.defineSphere(1.0, {0.0,0.0,0.0}, handles[0]);
    gpuBackend.defineSphere(1.0, {1.9,0.0,0.0}, handles[1]);
    gpuBackend.defineSphere(1.5, {1.0,0.0,0.0}, handles[2]);
    gpuBackend.defineSphere(1.5, {2.0,0.0,0.0}, handles[3]);
    gpuBackend.defineCapsule(1.0, {-1.0,0.0,0.0}, {1.0,0.0,0.0}, handles[6]);
    gpuBackend.defineCapsule(2.0, {0.0,-1.0,0.0}, {0.0,1.0,0.0}, handles[7]);

    
    
    std::vector<vgeo::internal::gpu::GpuContact> result = gpuBackend.querySpheres();

    CHECK(gpuBackend.test == 4);
    CHECK(result[0].hasContact == true);
    CHECK(result[0].idA == handles[0].getId());
    CHECK(result[0].depth == Catch::Approx(0.1));
    CHECK(result[0].idA == handles[0].getId());
    CHECK(result[0].idB == handles[1].getId());
    CHECK(result[0].normal.x == -1.0);
    CHECK(result[0].normal.y == 0.0);
    CHECK(result[0].normal.z == 0.0);
    CHECK(result[1].hasContact == true);
    CHECK(result[1].normal.x == -1.0);
    
    CHECK(result[2].hasContact == true);
    CHECK(result[2].depth == 3.0);
    CHECK(result[2].idA == handles[6].getId());
    CHECK(result[2].idB == handles[7].getId());
    CHECK(result[2].normal.y  == 1.0);


    gpuBackend.defineSphere(0.1, {1.0, 1.0,1.0}, handles[4]);
    gpuBackend.defineSphere(0.1, {2.0,2.0,2.0}, handles[5]);
    result = gpuBackend.querySpheres();

    CHECK(gpuBackend.test == 6);
    CHECK(result[0].hasContact == true);
    CHECK(result[0].depth == Catch::Approx(0.1));
    CHECK(result[0].normal.x == -1.0);
    CHECK(result[0].normal.y == 0.0);
    CHECK(result[0].normal.z == 0.0);
    CHECK(result[1].hasContact == true);
    CHECK(result[2].hasContact == false);
    CHECK(result[2].depth == 0.0);


    vgeo::internal::cpu::Sphere s1 = testSphereBuilder({0.0,0.0,0.0}, 1.0);
    vgeo::internal::cpu::Sphere s2 = testSphereBuilder({1.9,0.0,0.0}, 1.0);
    gpuBackend.addPair(handles[4], s1, handles[5], s2);

    vgeo::internal::cpu::Capsule c1 = testCapsuleBuilder({0.0,0.0,0.0}, 1.0, 1.0);
    vgeo::internal::cpu::Capsule c2 = testCapsuleBuilder({0.0,1.0,2.0}, 2.0, 1.0);
    gpuBackend.addPair(handles[2], c1, handles[3], c2);
    gpuBackend.addPair(handles[3], c2, handles[4], s1);

    vgeo::CollisionResult collisionResult = gpuBackend.query();

    CHECK(collisionResult.pairs[0].shapeA.getId() == handles[4].getId());
    CHECK(collisionResult.pairs[0].shapeB.getId() == handles[5].getId());
    CHECK(collisionResult.pairs[0].contact.depth == Catch::Approx(0.1));
    CHECK(collisionResult.pairs[0].contact.normal.x == -1.0);
    CHECK(collisionResult.pairs[1].shapeA.getId() == handles[2].getId());
    CHECK(collisionResult.pairs[1].shapeB.getId() == handles[3].getId());
    CHECK(collisionResult.pairs[1].contact.depth == Catch::Approx(1.0));
    CHECK(collisionResult.pairs[1].contact.normal.y == -1.0);
    CHECK(collisionResult.pairs[2].shapeA.getId() == handles[3].getId());
    CHECK(collisionResult.pairs[2].shapeB.getId() == handles[4].getId());
    CHECK(collisionResult.pairs[2].contact.depth == Catch::Approx(1.0));
    CHECK(collisionResult.pairs[2].contact.normal.y == 1.0);


    gpuBackend.cleanup();

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);
}
