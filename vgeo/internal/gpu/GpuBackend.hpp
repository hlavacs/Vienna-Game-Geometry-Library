#pragma once

#include "vgeo/CollisionResult.hpp"
#include "vgeo/GeometryHandle.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/internal/ConvexHullBuilder.hpp"
#include "vgeo/internal/GeometryRegistry.hpp"
#include "vgeo/internal/gpu/VulkanHandler.hpp"
#include "vgeo/internal/gpu/shapes/AaBox.hpp"

namespace vgeo::internal::gpu {

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
    // GeometryRegistry<ShapeType::Capsule> m_capsuleRegistry;
    // GeometryRegistry<ShapeType::Polytope> m_polytopeRegistry;
    // GeometryRegistry<ShapeType::Sphere> m_sphereRegistry;

    std::vector<AaBox> m_aaBoxes;
    // std::vector<Capsule> m_capsules;
    // std::vector<Polytope> m_polytopes;
    // std::vector<Sphere> m_spheres;

public:
    int                test;
    Bv                 m_boundingVolume;
    std::vector<AaBox> m_testBoxes;
    GpuBackend(VkPhysicalDevice& physicalDevice);
    void queryBoxes();

    GeometryHandle defineAaBox(Vec3 halfExtents);
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
    m_vulkanHandler.createComputePipeline("shaders/comp.spv");
    m_vulkanHandler.createCommandPool();
}

template <typename Bv>
void GpuBackend<Bv>::queryBoxes() {
    VkDeviceSize bufferSize = sizeof(AaBox) * m_aaBoxes.size();
    m_testBoxes.resize(m_aaBoxes.size());

    m_vulkanHandler.createInputStorageBuffers(&m_aaBoxes[0], bufferSize);

    m_vulkanHandler.createOutputStorageBuffer(bufferSize);

    m_vulkanHandler.createDescriptorPool();
    m_vulkanHandler.createDescriptorSets();
    m_vulkanHandler.runComputeShader(&m_testBoxes[0]);
}

template <typename Bv>
GeometryHandle GpuBackend<Bv>::defineAaBox(Vec3 halfExtents) {
    return addShape(
        m_aaBoxRegistry,
        m_aaBoxes,
        AaBox{-halfExtents.x, -halfExtents.y, -halfExtents.z, 0.0f, halfExtents.x, halfExtents.y, halfExtents.z, 0.0f});
}

}
