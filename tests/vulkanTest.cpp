#include "vgeo/internal/gpu/GpuBackend.hpp"
#include "vgeo/internal/InstanceRegistry.hpp"

#include <catch2/catch_test_macros.hpp>
#include "catch2/catch_approx.hpp"

#include <vulkan/vulkan.h>
#include <iostream>


VkPhysicalDevice initVulkan(VkInstance& instance){
    

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
        std::cout << "GPU: " << deviceProperties.deviceName << '\n';
        std::cout << "Type: " << deviceProperties.deviceType << '\n';
        std::cout << "API Version: " << deviceProperties.apiVersion << '\n';
        std::cout << "maximum Workgroups: " << deviceProperties.limits.maxComputeWorkGroupCount[0] << '\n';
        break;
    }
    if(deviceProperties.deviceName == VK_NULL_HANDLE){
        throw std::runtime_error("Vulkan initialization failed!");
    }

    return physicalDevice;
}

TEST_CASE("Vulkan instance is created", "[vulkan]") {

    // Create a Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);
    
    VkPhysicalDevice physicalDevice = initVulkan(instance);

    vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB> gpuBackend =
        vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB>(physicalDevice);

    vgeo::internal::InstanceRegistry registry;
    std::vector<vgeo::InstanceHandle> handles;

    for(uint32_t i=0; i<10;i++){
        handles.push_back(registry.allocate());
    }

    vgeo::GeometryHandle handleA = gpuBackend.defineSphere(1.0);
    vgeo::GeometryHandle handleB = gpuBackend.defineSphere(1.0);
    vgeo::GeometryHandle handleC = gpuBackend.defineSphere(1.5);
    vgeo::GeometryHandle handleG = gpuBackend.defineSphere(1.5);
    vgeo::GeometryHandle handleD = gpuBackend.defineSphere(1.5);
    vgeo::GeometryHandle handleE = gpuBackend.defineCapsule(1.0, 1.0);
    vgeo::GeometryHandle handleF = gpuBackend.defineCapsule(1.0, 2.0);

    gpuBackend.remove(handleG);

    gpuBackend.addCollisionTestPair(handles[0], handleA, Terathon::Motor3D::identity, handles[1], handleB, Terathon::Motor3D::MakeTranslation({1.9, 0.0, 0.0}));
    gpuBackend.addCollisionTestPair(handles[2], handleC, Terathon::Motor3D::MakeTranslation({0.0, 0.0, 1.0}), 
        handles[3], handleD, Terathon::Motor3D::MakeTranslation({0.0, 0.0, 2.0}));
    gpuBackend.addCollisionTestPair(handles[4], handleE, Terathon::Motor3D::MakeTranslation({0.0, 0.0, 1.5}), 
        handles[5], handleF, Terathon::Motor3D::MakeTranslation({0.0, 0.0, -2.5}));
    gpuBackend.addCollisionTestPair(handles[0], handleA, Terathon::Motor3D::MakeTranslation({0.0, 0.0, 1.0}), 
        handles[5], handleF, Terathon::Motor3D::MakeTranslation({0.0, 0.0, -2.5}));
    gpuBackend.addCollisionTestPair(handles[3], handleD, Terathon::Motor3D::MakeTranslation({0.0, 0.0, 2.0}), 
        handles[0], handleA, Terathon::Motor3D::identity);
    gpuBackend.addCollisionTestPair(handles[0], handleA, Terathon::Motor3D::identity, 
        handles[5], handleF, Terathon::Motor3D::MakeTranslation({0.0, 0.0, -2.5}));

    vgeo::CollisionResult collisionResult = gpuBackend.query();

    CHECK(collisionResult.pairs.size() == 6);
    CHECK(collisionResult.pairs[0].shapeA.getId() == handles[0].getId());
    CHECK(collisionResult.pairs[0].shapeB.getId() == handles[1].getId());
    CHECK(collisionResult.pairs[0].contact.depth == Catch::Approx(0.1));
    CHECK(collisionResult.pairs[0].contact.normal.x == -1.0);
    CHECK(collisionResult.pairs[1].shapeA.getId() == handles[2].getId());
    CHECK(collisionResult.pairs[1].shapeB.getId() == handles[3].getId());
    CHECK(collisionResult.pairs[1].contact.depth == Catch::Approx(2.0));
    CHECK(collisionResult.pairs[1].contact.normal.z == -1.0);
    CHECK(collisionResult.pairs[2].shapeA.getId() == handles[3].getId());
    CHECK(collisionResult.pairs[2].shapeB.getId() == handles[0].getId());
    CHECK(collisionResult.pairs[2].contact.depth == Catch::Approx(0.5));
    CHECK(collisionResult.pairs[2].contact.normal.z == 1.0);
    CHECK(collisionResult.pairs[3].shapeA.getId() == handles[4].getId());
    CHECK(collisionResult.pairs[3].shapeB.getId() == handles[5].getId());
    CHECK(collisionResult.pairs[3].contact.depth == Catch::Approx(1.0));
    CHECK(collisionResult.pairs[3].contact.normal.z == 1.0);
    CHECK(collisionResult.pairs[4].shapeA.getId() == handles[0].getId());
    CHECK(collisionResult.pairs[4].shapeB.getId() == handles[5].getId());
    CHECK(collisionResult.pairs[4].contact.depth == Catch::Approx(1.5));
    CHECK(collisionResult.pairs[4].contact.normal.z == Catch::Approx(-1.0));

    gpuBackend.cleanup();

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);
}


TEST_CASE("CapsuleBuffer is extended", "[capsuleBuffer]") {
    // Create a Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);
    
    VkPhysicalDevice physicalDevice = initVulkan(instance);

    vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB> gpuBackend =
        vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB>(physicalDevice);

    vgeo::internal::InstanceRegistry registry;
    std::vector<vgeo::InstanceHandle> instanceHandles;

    for(uint32_t i=0; i < 100; i++){
        instanceHandles.push_back(registry.allocate());
    }
    
    std::vector<vgeo::GeometryHandle> geometryHandles;

    for(uint32_t i=0; i < 100; i++){
        geometryHandles.push_back(gpuBackend.defineCapsule(1.0, 1.0));
    }

    for(uint32_t i=0; i < 50; i++){
        gpuBackend.addCollisionTestPair(instanceHandles[2*i], geometryHandles[2*i], Terathon::Motor3D::identity, 
            instanceHandles[2*i+1], geometryHandles[2*i+1], Terathon::Motor3D::identity);
    }

    vgeo::CollisionResult collisionResult = gpuBackend.query();

    CHECK(collisionResult.pairs.size() == 50);

    for(uint32_t i=0; i < 50; i++){
        CHECK(collisionResult.pairs[i].shapeA.getId() == instanceHandles[2*i].getId());
        CHECK(collisionResult.pairs[i].shapeB.getId() == instanceHandles[2*i+1].getId());
        CHECK(collisionResult.pairs[i].contact.depth == Catch::Approx(2));
        CHECK(collisionResult.pairs[i].contact.normal.y == 1.0);
    }

    gpuBackend.cleanup();

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);
}

TEST_CASE("SphereBuffer is extended", "[sphereBuffer]") {
    // Create a Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);
    
    VkPhysicalDevice physicalDevice = initVulkan(instance);

    vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB> gpuBackend =
        vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB>(physicalDevice);

    vgeo::internal::InstanceRegistry registry;
    std::vector<vgeo::InstanceHandle> instanceHandles;

    for(uint32_t i=0; i < 1000; i++){
        instanceHandles.push_back(registry.allocate());
    }
    
    std::vector<vgeo::GeometryHandle> geometryHandles;

    for(uint32_t i=0; i < 1000; i++){
        geometryHandles.push_back(gpuBackend.defineSphere(1.0));
    }

    for(uint32_t i=0; i < 500; i++){
        gpuBackend.addCollisionTestPair(instanceHandles[2*i], geometryHandles[2*i], Terathon::Motor3D::identity, 
            instanceHandles[2*i+1], geometryHandles[2*i+1], Terathon::Motor3D::MakeTranslation({1.9, 0.0, 0.0}));
    }

    vgeo::CollisionResult collisionResult = gpuBackend.query();

    CHECK(collisionResult.pairs.size() == 500);

    for(uint32_t i=0; i < 500; i++){
        CHECK(collisionResult.pairs[i].shapeA.getId() == instanceHandles[2*i].getId());
        CHECK(collisionResult.pairs[i].shapeB.getId() == instanceHandles[2*i+1].getId());
        CHECK(collisionResult.pairs[i].contact.depth == Catch::Approx(0.1));
        CHECK(collisionResult.pairs[i].contact.normal.x == -1.0);
    }

    gpuBackend.cleanup();

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);
}

TEST_CASE("PolytopeBufferTest", "[polytopes]") {

    // Create a Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);
    
    VkPhysicalDevice physicalDevice = initVulkan(instance);

    vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB> gpuBackend =
        vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB>(physicalDevice);

    vgeo::internal::InstanceRegistry registry;
    std::vector<vgeo::InstanceHandle> handles;

    for(uint32_t i=0; i<10;i++){
        handles.push_back(registry.allocate());
    }

    std::span<const vgeo::Vec3> boxA = {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 1.0, 0.0}, {1.0, 0.0, 1.0}, {0.0, 1.0, 1.0}, {1.0, 1.0, 1.0}};
    std::span<const vgeo::Vec3> boxB = {{0.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}, {0.0, -1.0, 0.0}, {0.0, 0.0, -1.0}, {-1.0, 1.0, 0.0}, {-1.0, 0.0, -1.0}, {0.0, -1.0, -1.0}, {-1.0, -1.0, -1.0}};

    vgeo::GeometryHandle handleA = gpuBackend.definePolytope(boxA);
    vgeo::GeometryHandle handleB = gpuBackend.definePolytope(boxB);

    gpuBackend.addCollisionTestPair(handles[0], handleA, Terathon::Motor3D::identity, handles[1], handleB, Terathon::Motor3D::identity);


    vgeo::CollisionResult collisionResult = gpuBackend.query();

    CHECK(collisionResult.pairs.size() == 1);
    CHECK(collisionResult.pairs[0].shapeA.getId() == handles[0].getId());
    CHECK(collisionResult.pairs[0].shapeB.getId() == handles[1].getId());
    CHECK(collisionResult.pairs[0].contact.depth == Catch::Approx(10.0));
    CHECK(collisionResult.pairs[0].contact.normal.x == 1.0);
    CHECK(collisionResult.pairs[0].contact.normal.y == 0.0);
    CHECK(collisionResult.pairs[0].contact.normal.z == 0.0);
    CHECK(collisionResult.pairs[0].contact.witnessA.x == 0.0);
    CHECK(collisionResult.pairs[0].contact.witnessA.y == -1.0);
    CHECK(collisionResult.pairs[0].contact.witnessA.z == 0.0);
    CHECK(collisionResult.pairs[0].contact.witnessB.x == 0.0);
    CHECK(collisionResult.pairs[0].contact.witnessB.y == 1.0);
    CHECK(collisionResult.pairs[0].contact.witnessB.z == 1.0);

    gpuBackend.cleanup();

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);
}

TEST_CASE("SphereCollision", "[spheres]"){

    // Create a Vulkan instance
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkInstance instance;
    vkCreateInstance(&createInfo, nullptr, &instance);
    
    VkPhysicalDevice physicalDevice = initVulkan(instance);

    vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB> gpuBackend =
        vgeo::internal::gpu::GpuBackend<vgeo::internal::gpu::AABB>(physicalDevice);

    vgeo::internal::InstanceRegistry registry;
    std::vector<vgeo::InstanceHandle> handles;

    for(uint32_t i=0; i<10;i++){
        handles.push_back(registry.allocate());
    }

    vgeo::GeometryHandle handleA = gpuBackend.defineSphere(1.0);
    vgeo::GeometryHandle handleB = gpuBackend.defineSphere(1.0);

    gpuBackend.addCollisionTestPair(handles[0], handleA, Terathon::Motor3D::identity, handles[1], handleB, Terathon::Motor3D::MakeTranslation({1.9, 0.0, 0.0}));

    vgeo::CollisionResult collisionResult = gpuBackend.query();

    CHECK(collisionResult.pairs.size() == 1);
    CHECK(collisionResult.pairs[0].shapeA.getId() == handles[0].getId());
    CHECK(collisionResult.pairs[0].shapeB.getId() == handles[1].getId());
    CHECK(collisionResult.pairs[0].contact.depth == Catch::Approx(0.1));
    CHECK(collisionResult.pairs[0].contact.normal.x == -1.0);


    gpuBackend.cleanup();

    // Destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);
}