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
#include "vgeo/internal/gpu/shapes/GpuCapsule.hpp"

#include <TSConformal3D.h>
#include "vgeo/internal/cpu/shapes/Sphere.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"

namespace vgeo::internal::gpu {

struct SpherePair {
    GpuSphere sphereA;
    GpuSphere sphereB;
};

struct CapsulePair {
    GpuCapsule capsuleA;
    GpuCapsule capsuleB;
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

enum class IntersectionTest : uint32_t {
    SphereSphere    = 0,
    CapsuleCapsule  = 1,
};

template <typename Bv>
class GpuBackend {

    const std::map<IntersectionTest, std::string> SHADERFILES = { {IntersectionTest::SphereSphere,"shaders/sphere_sphere.spv"},
                                                            {IntersectionTest::CapsuleCapsule, "shaders/capsule_capsule.spv"}
                                                        };
    const uint32_t BUFFEROBJECTS = 64;
    VulkanHandler                      m_vulkanHandler;
    GeometryRegistry<ShapeType::AaBox> m_aaBoxRegistry;
    GeometryRegistry<ShapeType::Capsule> m_capsuleRegistry;
    // GeometryRegistry<ShapeType::Polytope> m_polytopeRegistry;
    GeometryRegistry<ShapeType::Sphere> m_sphereRegistry;

    std::vector<AaBox> m_aaBoxes;
    std::vector<GpuCapsule> m_capsules;
    // std::vector<Polytope> m_polytopes;
    std::vector<GpuSphere> m_spheres;
    std::vector<SpherePair> m_spherePairs;
    std::vector<CapsulePair> m_capsulePairs;

    std::vector<ComputePipeline> m_computePipelines;


public:
    int                test;
    Bv                 m_boundingVolume;
    std::vector<AaBox> m_testBoxes;
    std::vector<GpuContact> m_contacts;
    std::vector<GpuContact> m_contacts2;
    GpuBackend(VkPhysicalDevice& physicalDevice);
    void initPipeline();
    void queryBoxes();
    void querySpheres();
    void queryCapsules();
    void query();

    uint32_t addPair(const vgeo::internal::cpu::Sphere& sphereA, const vgeo::internal::cpu::Sphere& sphereB);
    uint32_t addPair(const vgeo::internal::cpu::Capsule& capsuleA, const vgeo::internal::cpu::Capsule& capsuleB);

    void cleanup();

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
}

template <typename Bv>
void GpuBackend<Bv>::initPipeline(){
    
    std::vector<VkDeviceSize> bufferSizes = {sizeof(GpuSphere) * BUFFEROBJECTS*2, sizeof(GpuContact) * BUFFEROBJECTS};
    std::vector<VkDeviceSize> bufferSizes2 = {sizeof(GpuCapsule) * BUFFEROBJECTS*2, sizeof(GpuContact) * BUFFEROBJECTS};
    
    m_computePipelines.push_back(m_vulkanHandler.createPipeline(SHADERFILES.at(IntersectionTest::SphereSphere), bufferSizes));
    m_computePipelines.push_back(m_vulkanHandler.createPipeline(SHADERFILES.at(IntersectionTest::CapsuleCapsule), bufferSizes2));
    
    m_vulkanHandler.createCommandPool();

}

template <typename Bv>
void GpuBackend<Bv>::queryBoxes() {
    VkDeviceSize bufferSize = sizeof(AaBox) * m_aaBoxes.size();
    m_testBoxes.resize(m_aaBoxes.size());

}

template <typename Bv>
void GpuBackend<Bv>::querySpheres() {
    
    test = m_spheres.size();
    VkDeviceSize bufferSizeIn = sizeof(GpuSphere) * BUFFEROBJECTS*2;
    
    m_contacts.resize(BUFFEROBJECTS);
    
    VkDeviceSize bufferSizeIn2 = sizeof(GpuCapsule) * BUFFEROBJECTS*2;

    m_contacts2.resize(BUFFEROBJECTS);

    m_vulkanHandler.startRecordingCommandBuffer();
    m_vulkanHandler.recordCommandBuffer(&m_spheres[0], bufferSizeIn, m_spheres.size()/2, m_computePipelines[0]);
    m_vulkanHandler.recordCommandBuffer(&m_capsules[0], bufferSizeIn2, m_capsules.size()/2, m_computePipelines[1]);
    m_vulkanHandler.runComputeShader();
    m_vulkanHandler.getResults(&m_contacts[0], m_computePipelines[0]);
    m_vulkanHandler.getResults(&m_contacts2[0], m_computePipelines[1]);

}

template <typename Bv>
void GpuBackend<Bv>::query() {

    VkDeviceSize bufferSizeIn = sizeof(GpuSphere) * m_spheres.size();
    m_vulkanHandler.recordCommandBuffer(&m_spheres[0], bufferSizeIn, m_spheres.size(), m_computePipelines[0]);

    VkDeviceSize bufferSizeOut = sizeof(GpuContact) * (m_spheres.size()/2);

    std::vector<GpuContact> contacts;
    contacts.resize(m_spheres.size()/2);
    /*
    for (const auto intersectiontest : IntersectionTest::All){
        uint32_t i = static_cast<uint32_t>(intersectiontest);


    }
    */
}

template <typename Bv>
void GpuBackend<Bv>::queryCapsules() {

    test = m_capsules.size();
    VkDeviceSize bufferSizeIn = sizeof(vgeo::internal::cpu::Capsule) * m_capsules.size();
    VkDeviceSize bufferSizeOut = sizeof(GpuContact) * (m_capsules.size()/2);


}

template <typename Bv>
GeometryHandle GpuBackend<Bv>::defineAaBox(Vec3 halfExtents) {
    return addShape(
        m_aaBoxRegistry,
        m_aaBoxes,
        AaBox{-halfExtents.x, -halfExtents.y, -halfExtents.z, 0.0f, halfExtents.x, halfExtents.y, halfExtents.z, 0.0f});
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
        GpuCapsule{a,b,radius});
}


template <typename Bv>
uint32_t GpuBackend<Bv>::addPair(const vgeo::internal::cpu::Sphere& sphereA, const vgeo::internal::cpu::Sphere& sphereB){
    uint32_t index = m_spherePairs.size();
    m_spherePairs.push_back(SpherePair{GpuSphere{sphereA.getSphere()}, GpuSphere{sphereB.getSphere()}});
    return index;
}

template <typename Bv>
uint32_t GpuBackend<Bv>::addPair(const vgeo::internal::cpu::Capsule& capsule1, const vgeo::internal::cpu::Capsule& capsule2){
    uint32_t index = m_capsulePairs.size();
    Terathon::Point3D a1 = capsule1.getA();
    Terathon::Point3D b1 = capsule1.getB();
    Terathon::Point3D a2 = capsule2.getA();
    Terathon::Point3D b2 = capsule2.getB();

    m_capsulePairs.push_back(CapsulePair{GpuCapsule{{a1.x,a1.y,a1.z}, {b1.x,b1.y,b1.z}, capsule1.getRadius()}, 
                                            GpuCapsule{{a2.x,a2.y,a2.z}, {b2.x,b2.y,b2.z}, capsule2.getRadius()}});
    return index;
}

template <typename Bv>
void GpuBackend<Bv>::cleanup() {
    m_vulkanHandler.cleanup(m_computePipelines);
}

}
