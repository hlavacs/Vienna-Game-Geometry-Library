#pragma once

#include "vgeo/CollisionResult.hpp"
#include "vgeo/GeometryHandle.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/ConvexHullBuilder.hpp"
#include "vgeo/internal/GeometryRegistry.hpp"
#include "vgeo/internal/gpu/VulkanHandler.hpp"
#include "vgeo/internal/gpu/shapes/AaBox.hpp"
#include "vgeo/internal/gpu/shapes/GpuSphere.hpp"
#include "vgeo/internal/gpu/shapes/GpuContact.hpp"

#include <TSConformal3D.h>
#include "vgeo/internal/cpu/shapes/Sphere.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"

namespace vgeo::internal::gpu {

struct SpherePair {
    GpuSphere sphereA;
    GpuSphere sphereB;
};

struct AABB {
    float minx;
    float miny;
    float minz;
    float padding1;
    float maxx;
    float maxy;
    float maxz;
    float padding2;
};

template <typename Bv>
class GpuBackend {

    VulkanHandler                      m_vulkanHandler;
    GeometryRegistry<ShapeType::AaBox> m_aaBoxRegistry;
    GeometryRegistry<ShapeType::Capsule> m_capsuleRegistry;
    // GeometryRegistry<ShapeType::Polytope> m_polytopeRegistry;
    GeometryRegistry<ShapeType::Sphere> m_sphereRegistry;

    std::vector<ShaderAaBox> m_aaBoxes;
    std::vector<vgeo::internal::cpu::Capsule> m_capsules;
    // std::vector<Polytope> m_polytopes;
    std::vector<GpuSphere> m_spheres;
    std::vector<SpherePair> m_spherePairs;


public:
    int                test;
    Bv                 m_boundingVolume;
    std::vector<ShaderAaBox> m_testBoxes;
    std::vector<GpuContact> m_contacts;
    GpuBackend(VkPhysicalDevice& physicalDevice);
    void queryBoxes();
    void querySpheres();
    void queryCapsules();

    uint32_t addPair(const vgeo::internal::cpu::Sphere& sphereA, const vgeo::internal::cpu::Sphere& sphereB);

    GeometryHandle defineAaBox(Vec3 halfExtents);
    GeometryHandle defineSphere(real radius, Vec3 center);
    GeometryHandle defineCapsule(real radius, Vec3 a, Vec3 b);
    /*
    GeometryHandle define(const CapsuleDesc& desc) {
        return addShape(m_capsuleRegistry, m_capsules, Capsule{desc.a, desc.b, desc.radius});
    }

    GeometryHandle define(const PolytopeDesc& desc) {
        return addShape(m_polytopeRegistry, m_polytopes, Polytope{ConvexHullBuilder::build(desc.points)});
    }

    GeometryHandle define(const SphereDesc& desc) {
        return addShape(m_sphereRegistry, m_spheres, Sphere{desc.center, desc.radius});
    }

    void remove(InstanceHandle h) {
        uint32_t index = h.getIndex();
        switch (h.getType()) {
            case ShapeType::AaBox:
                m_aaBoxRegistry.free(h);
                m_aaBoxes[index] = AaBox{};
                break;
            case ShapeType::Capsule:
                m_capsuleRegistry.free(h);
                m_capsules[index] = Capsule{};
                break;
            case ShapeType::Polytope:
                m_polytopeRegistry.free(h);
                m_polytopes[index] = Polytope{};
                break;
            case ShapeType::Sphere:
                m_sphereRegistry.free(h);
                m_spheres[index] = Sphere{};
                break;
        }
    }
    */

private:
    template <typename Shape, ShapeType Type>
    GeometryHandle addShape(GeometryRegistry<Type>& registry, std::vector<Shape>& storage, Shape shape) {
        GeometryHandle h     = registry.allocate();
        uint32_t       index = h.getIndex();

        if (index >= storage.size()) {
            storage.resize(index + 1);
        }

        storage[index] = std::move(shape);
        return h;
    }

};

template <typename Bv>
GpuBackend<Bv>::GpuBackend(VkPhysicalDevice& physicalDevice) : m_vulkanHandler(physicalDevice) {
    test = 1;

    m_vulkanHandler.createLogicalDevice();
    m_vulkanHandler.createDescriptorSetLayouts();
    m_vulkanHandler.createPipelineLayout();
    m_vulkanHandler.createComputePipeline("shaders/sphere_sphere.spv");
    m_vulkanHandler.createCommandPool();
}

template <typename Bv>
void GpuBackend<Bv>::queryBoxes() {
    VkDeviceSize bufferSize = sizeof(ShaderAaBox) * m_aaBoxes.size();
    m_testBoxes.resize(m_aaBoxes.size());

    m_vulkanHandler.createInputStorageBuffers(&m_aaBoxes[0], bufferSize);

    m_vulkanHandler.createOutputStorageBuffer(bufferSize);

    m_vulkanHandler.createDescriptorPool();
    m_vulkanHandler.createDescriptorSets();
    m_vulkanHandler.runComputeShader(&m_testBoxes[0]);
}

template <typename Bv>
void GpuBackend<Bv>::querySpheres() {

    test = m_spheres.size();
    VkDeviceSize bufferSizeIn = sizeof(GpuSphere) * m_spheres.size();
    VkDeviceSize bufferSizeOut = sizeof(GpuContact) * (m_spheres.size()/2);

    m_contacts.resize(m_spheres.size()/2);

    m_vulkanHandler.resetInputStorageBuffers();
    m_vulkanHandler.createInputStorageBuffers(&m_spheres[0], bufferSizeIn);

    m_vulkanHandler.createOutputStorageBuffer(bufferSizeOut);
    m_vulkanHandler.createDescriptorPool();
    m_vulkanHandler.createDescriptorSets();   
    m_vulkanHandler.runComputeShader(&m_contacts[0]);

}

template <typename Bv>
void GpuBackend<Bv>::queryCapsules() {

    test = m_capsules.size();
    VkDeviceSize bufferSizeIn = sizeof(vgeo::internal::cpu::Capsule) * m_capsules.size();
    VkDeviceSize bufferSizeOut = sizeof(GpuContact) * (m_capsules.size()/2);

    m_contacts.resize(m_capsules.size()/2);

    m_vulkanHandler.resetInputStorageBuffers();
    m_vulkanHandler.createInputStorageBuffers(&m_capsules[0], bufferSizeIn);

    m_vulkanHandler.createOutputStorageBuffer(bufferSizeOut);
    m_vulkanHandler.createDescriptorPool();
    m_vulkanHandler.createDescriptorSets();   
    m_vulkanHandler.runComputeShader(&m_contacts[0]);

}

template <typename Bv>
GeometryHandle GpuBackend<Bv>::defineAaBox(Vec3 halfExtents) {
    return addShape(
        m_aaBoxRegistry,
        m_aaBoxes,
        ShaderAaBox{-halfExtents.x, -halfExtents.y, -halfExtents.z, 0.0f, halfExtents.x, halfExtents.y, halfExtents.z, 0.0f});
}

template <typename Bv>
GeometryHandle GpuBackend<Bv>::defineSphere(real radius, Vec3 center) {
    return addShape(
        m_sphereRegistry,
        m_spheres,
        GpuSphere{Terathon::Sphere3D{-1.0, center.x, center.y, center.z, -((center.x*center.x + center.y*center.y + center.z*center.z) - radius * radius) * 0.5f}});
}

template <typename Bv>
GeometryHandle GpuBackend<Bv>::defineCapsule(real radius, Vec3 a, Vec3 b) {
    return addShape(
        m_capsuleRegistry,
        m_capsules,
        vgeo::internal::cpu::Capsule{a,b,radius});
}


template <typename Bv>
uint32_t GpuBackend<Bv>::addPair(const vgeo::internal::cpu::Sphere& sphereA, const vgeo::internal::cpu::Sphere& sphereB){
    uint32_t index = m_spherePairs.size();
    m_spherePairs.push_back(SpherePair{GpuSphere{sphereA.getSphere()}, GpuSphere{sphereB.getSphere()}});
    return index;
}

}
