#pragma once

#include <set>

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
#include "vgeo/internal/gpu/shapes/ShapePair.hpp"
#include "vgeo/internal/gpu/IntersectionType.hpp"

#include <TSConformal3D.h>
#include "vgeo/internal/cpu/shapes/Sphere.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"

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

    const uint32_t BUFFEROBJECTS = 64;
    const uint32_t SHADERCOUNT = 3;
    VulkanHandler                      m_vulkanHandler;
    GeometryRegistry<ShapeType::AaBox> m_aaBoxRegistry;
    GeometryRegistry<ShapeType::Capsule> m_capsuleRegistry;
    // GeometryRegistry<ShapeType::Polytope> m_polytopeRegistry;
    GeometryRegistry<ShapeType::Sphere> m_sphereRegistry;

    std::vector<AaBox> m_aaBoxes;
    std::vector<GpuCapsule> m_capsules;
    // std::vector<Polytope> m_polytopes;
    std::vector<GpuSphere> m_spheres;

    std::map<uint64_t, InstanceHandle> m_instanceHandles;
    std::vector<ComputePipeline> m_computePipelines;
    std::vector<std::vector<ShapePair>> m_pairs;


public:
    int                test;
    Bv                 m_boundingVolume;
    std::vector<AaBox> m_testBoxes;
    std::vector<GpuContact> m_contacts;
    std::vector<GpuContact> m_contacts2;
    GpuBackend(VkPhysicalDevice& physicalDevice);
    void initPipeline();
    void queryBoxes();
    std::vector<GpuContact> querySpheres();
    void queryCapsules();
    CollisionResult query();

    void addPair(InstanceHandle handleA, const vgeo::internal::cpu::Sphere& sphereA, InstanceHandle handleB, const vgeo::internal::cpu::Sphere& sphereB);
    void addPair(InstanceHandle handleA, const vgeo::internal::cpu::Capsule& capsuleA, InstanceHandle handleB, const vgeo::internal::cpu::Capsule& capsuleB);
    void addPair(InstanceHandle handleA, const vgeo::internal::cpu::Capsule& capsule, InstanceHandle handleB, const vgeo::internal::cpu::Sphere& sphere);

    void cleanup();

    GeometryHandle defineAaBox(Vec3 halfExtents);
    GeometryHandle defineSphere(real radius, Vec3 center, InstanceHandle handle);
    GeometryHandle defineCapsule(real radius, Vec3 a, Vec3 b, InstanceHandle handle);
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

    std::string getShaderFile(IntersectionType intersection){
        switch (intersection)
        {
        case IntersectionType::SphereSphere:
            return "shaders/sphere_sphere.spv";    
        case IntersectionType::CapsuleCapsule:
            return "shaders/capsule_capsule.spv";
        case IntersectionType::CapsuleSphere:
            return "shaders/capsule_sphere.spv";
        default:
            throw std::runtime_error("Shaderfile does not exist");
        }
    }

    uint32_t getIndex(IntersectionType intersection){
        return std::to_underlying(intersection);
    }

    void initPipeline(IntersectionType intersection) {
        const uint32_t i = getIndex(intersection);
        const std::string file = getShaderFile(intersection);
        std::vector<VkDeviceSize> bufferSizes;

        switch(intersection)
        {
            case IntersectionType::SphereSphere:
                bufferSizes = {sizeof(SpherePair) * BUFFEROBJECTS, sizeof(GpuContact) * BUFFEROBJECTS};
                m_computePipelines[i] = m_vulkanHandler.createPipeline(file, bufferSizes);
                return;
            case IntersectionType::CapsuleCapsule:
                bufferSizes = {sizeof(CapsulePair) * BUFFEROBJECTS, sizeof(GpuContact) * BUFFEROBJECTS};
                m_computePipelines[i] = m_vulkanHandler.createPipeline(file, bufferSizes);
                return;
            case IntersectionType::CapsuleSphere:
                bufferSizes = {sizeof(CapsuleSpherePair) * BUFFEROBJECTS, sizeof(GpuContact) * BUFFEROBJECTS};
                m_computePipelines[i] = m_vulkanHandler.createPipeline(file, bufferSizes);
                return;
            default:
                throw std::runtime_error("Intersection does not exist");
        }
        
    }

};

template <typename Bv>
GpuBackend<Bv>::GpuBackend(VkPhysicalDevice& physicalDevice) : m_vulkanHandler(physicalDevice) {
    test = 1;

    m_vulkanHandler.createLogicalDevice();

    m_computePipelines.resize(SHADERCOUNT);
    initPipeline(IntersectionType::SphereSphere);
    initPipeline(IntersectionType::CapsuleCapsule);
    initPipeline(IntersectionType::CapsuleSphere);

    m_pairs.resize(SHADERCOUNT);

    m_vulkanHandler.createCommandPool();
}


template <typename Bv>
void GpuBackend<Bv>::queryBoxes() {
    VkDeviceSize bufferSize = sizeof(AaBox) * m_aaBoxes.size();
    m_testBoxes.resize(m_aaBoxes.size());

}

template <typename Bv>
std::vector<GpuContact> GpuBackend<Bv>::querySpheres() {
    
    test = m_spheres.size();

    std::vector<GpuContact> results = {};

    m_vulkanHandler.startRecordingCommandBuffer();
    m_vulkanHandler.recordCommandBuffer(&m_spheres[0], m_spheres.size()/2, m_computePipelines[0]);
    m_vulkanHandler.recordCommandBuffer(&m_capsules[0], m_capsules.size()/2, m_computePipelines[1]);
    m_vulkanHandler.runComputeShader();

    std::vector<GpuContact> contacts;
    contacts.resize(BUFFEROBJECTS);
    m_vulkanHandler.getResults(&contacts[0], m_computePipelines[0]);
    contacts.resize(m_spheres.size()/2);
    results.insert(results.end(), contacts.begin(), contacts.end());

    contacts.resize(BUFFEROBJECTS);
    m_vulkanHandler.getResults(&contacts[0], m_computePipelines[1]);
    contacts.resize(m_capsules.size()/2);
    results.insert(results.end(), contacts.begin(), contacts.end());

    return results;

}

template <typename Bv>
CollisionResult GpuBackend<Bv>::query() {

    m_vulkanHandler.startRecordingCommandBuffer();
    
    std::vector<CollisionPair> results = {};

    //throw std::runtime_error(std::to_string(m_pairs[0].size()));

    for(uint32_t i=0; i < m_pairs.size(); i++){
        if(m_pairs[i].size() != 0){
            m_vulkanHandler.recordCommandBuffer(&m_pairs[i][0], m_pairs[i].size(), m_computePipelines[i]);
        }
    }

    m_vulkanHandler.runComputeShader();
    
    std::vector<GpuContact> contacts;
    for (uint32_t i=0; i < m_computePipelines.size(); i++) {
        if(m_pairs[i].size() != 0){
            contacts.resize(BUFFEROBJECTS);
            m_vulkanHandler.getResults(&contacts[0], m_computePipelines[i]);
            contacts.resize(m_pairs[i].size());
        }
        for(uint32_t j=0; j < contacts.size(); j++){
            if(contacts[j].hasContact) {
                Contact contact{contacts[j].normal, contacts[j].depth, contacts[j].witnessA, contacts[j].witnessB};
                InstanceHandle handleA = m_instanceHandles.find(contacts[j].idA)->second;
                InstanceHandle handleB = m_instanceHandles.find(contacts[j].idB)->second;
                results.push_back(CollisionPair{handleA, handleB, contact});
            }
        }
        
    }

    return CollisionResult{results};
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
GeometryHandle GpuBackend<Bv>::defineSphere(real radius, Vec3 center, InstanceHandle handle) {
    m_instanceHandles.insert_or_assign(handle.getId(), handle);
    return addShape(
        m_sphereRegistry,
        m_spheres,
        GpuSphere{handle.getId(), Terathon::Sphere3D{-1.0, center.x, center.y, center.z, -((center.x*center.x + center.y*center.y + center.z*center.z) - radius * radius) * 0.5f}});
}

template <typename Bv>
GeometryHandle GpuBackend<Bv>::defineCapsule(real radius, Vec3 a, Vec3 b, InstanceHandle handle) {
    m_instanceHandles.insert_or_assign(handle.getId(), handle);
    return addShape(
        m_capsuleRegistry,
        m_capsules,
        GpuCapsule{handle.getId(),Terathon::Point3D{a.x,a.y,a.z},Terathon::Point3D{b.x,b.y,b.z},radius});
}


template <typename Bv>
void GpuBackend<Bv>::addPair(InstanceHandle handleA, const vgeo::internal::cpu::Sphere& sphereA, InstanceHandle handleB, const vgeo::internal::cpu::Sphere& sphereB){
    m_instanceHandles.insert_or_assign(handleA.getId(), handleA);
    m_instanceHandles.insert_or_assign(handleB.getId(), handleB);

    uint32_t i = getIndex(IntersectionType::SphereSphere);
    m_pairs[i].push_back(SpherePair{GpuSphere{handleA.getId(), sphereA.getSphere()}, GpuSphere{handleB.getId(), sphereB.getSphere()}});
}

template <typename Bv>
void GpuBackend<Bv>::addPair(InstanceHandle handleA, const vgeo::internal::cpu::Capsule& capsuleA, InstanceHandle handleB, const vgeo::internal::cpu::Capsule& capsuleB){
    m_instanceHandles.insert_or_assign(handleA.getId(), handleA);
    m_instanceHandles.insert_or_assign(handleB.getId(), handleB);

    uint32_t i = getIndex(IntersectionType::CapsuleCapsule);
    m_pairs[i].push_back(CapsulePair{GpuCapsule{handleA.getId(), capsuleA.getA(), capsuleA.getB(), capsuleA.getRadius()}, 
                                            GpuCapsule{handleB.getId(), capsuleB.getA(), capsuleB.getB(), capsuleB.getRadius()}});

}

template <typename Bv>
void GpuBackend<Bv>::addPair(InstanceHandle handleA, const vgeo::internal::cpu::Capsule& capsule, InstanceHandle handleB, const vgeo::internal::cpu::Sphere& sphere){
    m_instanceHandles.insert_or_assign(handleA.getId(), handleA);
    m_instanceHandles.insert_or_assign(handleB.getId(), handleB);

    uint32_t i = getIndex(IntersectionType::CapsuleSphere);
    m_pairs[i].push_back(CapsuleSpherePair{GpuCapsule{handleA.getId(), capsule.getA(), capsule.getB(), capsule.getRadius()}, 
                                            GpuSphere{handleB.getId(), sphere.getSphere()}});

}

template <typename Bv>
void GpuBackend<Bv>::cleanup() {
    m_vulkanHandler.cleanup(m_computePipelines);
}

}
