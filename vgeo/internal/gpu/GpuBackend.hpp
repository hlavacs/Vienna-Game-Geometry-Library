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
#include "vgeo/internal/gpu/shapes/GpuContact.hpp"
#include "vgeo/internal/gpu/IntersectionType.hpp"

#include <TSConformal3D.h>
#include "vgeo/internal/cpu/shapes/Sphere.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"

#include <iostream>

#define BUFFEROBJECTS 64

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

struct PolytopeInfo{
    uint32_t offset;
    uint32_t size;
};

template <typename Bv>
class GpuBackend {

    //const uint32_t BUFFEROBJECTS = 64;
    const uint32_t SHADERCOUNT = 3;
    VulkanHandler                      m_vulkanHandler;
    GeometryRegistry<ShapeType::AaBox> m_aaBoxRegistry;
    GeometryRegistry<ShapeType::Capsule> m_capsuleRegistry;
    GeometryRegistry<ShapeType::Polytope> m_polytopeRegistry;
    GeometryRegistry<ShapeType::Sphere> m_sphereRegistry;

    std::vector<AaBox> m_aaBoxes;
    std::vector<vgeo::internal::cpu::Capsule> m_capsules;
    std::vector<vgeo::internal::cpu::Polytope> m_polytopes;
    std::vector<vgeo::internal::cpu::Sphere> m_spheres;

    std::vector<PolytopeInfo> m_polytopeInfos;

    GpuStorageBuffer m_aaBoxesBuffer;
    GpuStorageBuffer m_capsulesBuffer;
    GpuStorageBuffer m_polytopesBuffer;
    GpuStorageBuffer m_spheresBuffer;

    std::map<uint64_t, InstanceHandle> m_instanceHandles;
    std::vector<ComputePipeline> m_computePipelines;

public:
    int                test;
    Bv                 m_boundingVolume;
    std::vector<GpuContact> m_contacts;

    GpuBackend(VkPhysicalDevice& physicalDevice);

    CollisionResult query();

    void addCollisionTestPair(InstanceHandle instanceA, GeometryHandle geometryA, Terathon::Motor3D motorA, InstanceHandle instanceB, GeometryHandle geometryB, 
        Terathon::Motor3D motorB);

    void cleanup();

    GeometryHandle defineAaBox(Vec3 halfExtents);

    GeometryHandle defineSphere(real radius){
        GeometryHandle handle = addShape(m_sphereRegistry, m_spheres, vgeo::internal::cpu::Sphere{radius});
        
        if(m_spheres.size() * sizeof(vgeo::internal::cpu::Sphere) > m_spheresBuffer.m_bufferSize){
            m_vulkanHandler.growGpuStorageBuffer(m_spheresBuffer);
            updateComputePipelines(ShapeType::Sphere);
        }
            
        m_vulkanHandler.updateGpuStorageBuffer(m_spheresBuffer, m_spheres.data(), m_spheres.size() * sizeof(vgeo::internal::cpu::Sphere));

        return handle;
    }

    GeometryHandle defineCapsule(real halfLength, real radius){
        GeometryHandle handle = addShape(m_capsuleRegistry, m_capsules, vgeo::internal::cpu::Capsule{halfLength, radius});

        if(m_capsules.size() * sizeof(vgeo::internal::cpu::Capsule) > m_capsulesBuffer.m_bufferSize){
            m_vulkanHandler.growGpuStorageBuffer(m_capsulesBuffer);
            updateComputePipelines(ShapeType::Sphere);
        }

        m_vulkanHandler.updateGpuStorageBuffer(m_capsulesBuffer, m_capsules.data(), m_capsules.size() * sizeof(vgeo::internal::cpu::Capsule));

        return handle;
    }

    GeometryHandle definePolytope(std::span<const Vec3> points) {
        vgeo::internal::cpu::Polytope polytope = vgeo::internal::cpu::Polytope{ConvexHullBuilder::build(points)};
        GeometryHandle handle = addShape(m_polytopeRegistry, m_polytopes, polytope);


        uint32_t index = handle.getIndex();
        if(index >= m_polytopeInfos.size()){
            m_polytopeInfos.resize(index+1);
        }
        m_polytopeInfos[index] = PolytopeInfo{0, polytope.getVertices().size()};
        updateOffsets(m_polytopeInfos);
        return handle;
    }
    /*
    GeometryHandle define(const CapsuleDesc& desc) {
        return addShape(m_capsuleRegistry, m_capsules, Capsule{desc.a, desc.b, desc.radius});
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

    void initPipeline(const std::string file, std::vector<ShapeType> structure){

        std::vector<std::reference_wrapper<GpuStorageBuffer>> gpuBuffers;

        for(uint32_t i = 0; i < structure.size(); i++){
            switch(structure[i]){
                case(vgeo::ShapeType::Sphere):
                    gpuBuffers.push_back(std::ref(m_spheresBuffer));
                    continue;
                case(vgeo::ShapeType::Capsule):
                    gpuBuffers.push_back(std::ref(m_capsulesBuffer));
                    continue;
            }
        }

        m_computePipelines.push_back(m_vulkanHandler.createPipeline(file, gpuBuffers, structure));

    }

    void updateComputePipelines(ShapeType type){
        
        for(ComputePipeline& pipeline : m_computePipelines){
            if(pipeline.referencesBuffer(type)){
                m_vulkanHandler.updateDescriptorBinding(pipeline, type);
            }
        }
    }

    void updateOffsets(std::vector<PolytopeInfo> infos){
        uint32_t offset = 0;
        for(auto info : infos){
            info.offset = offset;
            offset += info.size;
        }
    }
    
};

template <typename Bv>
GpuBackend<Bv>::GpuBackend(VkPhysicalDevice& physicalDevice) : m_vulkanHandler(physicalDevice) {
    test = 1;

    m_vulkanHandler.createLogicalDevice();

    m_aaBoxesBuffer = m_vulkanHandler.createGpuStorageBuffer(sizeof(vgeo::internal::cpu::AaBox)*BUFFEROBJECTS);
    m_capsulesBuffer = m_vulkanHandler.createGpuStorageBuffer(sizeof(vgeo::internal::cpu::Capsule)*BUFFEROBJECTS);
    m_polytopesBuffer = m_vulkanHandler.createGpuStorageBuffer(sizeof(Terathon::Point3D)*BUFFEROBJECTS);
    m_spheresBuffer = m_vulkanHandler.createGpuStorageBuffer(sizeof(vgeo::internal::cpu::Sphere)*BUFFEROBJECTS);
    
    //m_computePipelines.resize(SHADERCOUNT);
    initPipeline("shaders/sphere_sphere.spv", {ShapeType::Sphere});
    initPipeline("shaders/capsule_capsule.spv", {ShapeType::Capsule});
    initPipeline("shaders/capsule_sphere.spv", {ShapeType::Capsule, ShapeType::Sphere});

    m_vulkanHandler.createCommandPool();
    
}


template <typename Bv>
CollisionResult GpuBackend<Bv>::query() {

    m_vulkanHandler.startRecordingCommandBuffer();
    
    std::vector<CollisionPair> results = {};

    for(ComputePipeline& pipeline : m_computePipelines){
        if(pipeline.getCalcDataSize() != 0){
            std::cout << "calcData: " << pipeline.getCalcDataSize() << "\n";
            m_vulkanHandler.recordCommandBuffer(pipeline);
        }
    }

    m_vulkanHandler.runComputeShader();
    
    for (ComputePipeline& pipeline : m_computePipelines) {
        std::vector<GpuContact> contacts;
        if(pipeline.getCalcDataSize() != 0){
            std::cout << "type: " << pipeline.getCalcData()[0].typeA << " and " << pipeline.getCalcData()[0].typeB << "\n";
            std::cout << "index: " << pipeline.getCalcData()[0].indexA << " and " << pipeline.getCalcData()[0].indexB << "\n";
            std::cout << "handle: " << pipeline.getCalcData()[0].instanceHandleA << " and " << pipeline.getCalcData()[0].instanceHandleB << "\n";
            std::cout << "buffers: " << pipeline.getGpuBuffers().size() << "\n";
            contacts.resize(BUFFEROBJECTS);
            pipeline.getResults(&contacts[0]);
            contacts.resize(pipeline.getCalcDataSize());
        }
        for(uint32_t j=0; j < contacts.size(); j++){
            std::cout << "contact: " << contacts[j].depth << "\n";
            if(contacts[j].hasContact) {
                Contact contact{contacts[j].normal, contacts[j].depth, contacts[j].witnessA, contacts[j].witnessB};
                InstanceHandle handleA = m_instanceHandles.find(contacts[j].idA)->second;
                InstanceHandle handleB = m_instanceHandles.find(contacts[j].idB)->second;
                results.push_back(CollisionPair{handleA, handleB, contact});
            }
        }
        
    }

    for(ComputePipeline& pipeline : m_computePipelines){
        pipeline.resetCalcData();
    }

    return CollisionResult{results};
}

template <typename Bv>
GeometryHandle GpuBackend<Bv>::defineAaBox(Vec3 halfExtents) {
    return addShape(
        m_aaBoxRegistry,
        m_aaBoxes,
        AaBox{-halfExtents.x, -halfExtents.y, -halfExtents.z, 0.0f, halfExtents.x, halfExtents.y, halfExtents.z, 0.0f});
}

template <typename Bv>
void GpuBackend<Bv>::addCollisionTestPair(InstanceHandle instanceA, GeometryHandle geometryA, Terathon::Motor3D motorA, InstanceHandle instanceB, GeometryHandle geometryB, 
        Terathon::Motor3D motorB) {

    m_instanceHandles.insert_or_assign(instanceA.getId(), instanceA);
    m_instanceHandles.insert_or_assign(instanceB.getId(), instanceB);
    
    std::vector<ShapeType> types;
    vgeo::ShapeType typeA = geometryA.getType();
    vgeo::ShapeType typeB = geometryB.getType();

    if(typeA == typeB){
        types.push_back(typeA);
    }
    else{
        types.push_back(typeA);
        types.push_back(typeB);
    }

    CollisionTestPairData collisionData = CollisionTestPairData{static_cast<uint32_t>(geometryA.getType()), static_cast<uint32_t>(geometryB.getType()), geometryA.getIndex(), 
        geometryB.getIndex(), instanceA.getId(), instanceB.getId(), 0, 0, motorA, motorB};
    
    bool added = false;
    for(ComputePipeline& pipeline : m_computePipelines){
        if(pipeline.checkStructure(types)){
            pipeline.addCollisionPair(collisionData);
            added = true;
            break;
        }
    }
    assert(added);

}

template <typename Bv>
void GpuBackend<Bv>::cleanup() {

    m_vulkanHandler.cleanup(m_aaBoxesBuffer);
    m_vulkanHandler.cleanup(m_capsulesBuffer);
    m_vulkanHandler.cleanup(m_polytopesBuffer);
    m_vulkanHandler.cleanup(m_spheresBuffer);
    m_vulkanHandler.cleanup(m_computePipelines);
}

}

#undef BUFFEROBJECTS