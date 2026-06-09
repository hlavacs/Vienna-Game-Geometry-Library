#pragma once

#include "vgeo/internal/gpu/VulkanHandler.hpp"
#include "vgeo/CollisionResults.hpp"
#include "vgeo/Handle.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/descriptors/AaBoxDesc.hpp"
#include "vgeo/descriptors/CapsuleDesc.hpp"
#include "vgeo/descriptors/ConvexHullDesc.hpp"
#include "vgeo/descriptors/SphereDesc.hpp"
#include "vgeo/internal/ConvexHullBuilder.hpp"
#include "vgeo/internal/HandleRegistry.hpp"
#include "vgeo/internal/gpu/shapes/AaBox.hpp"

namespace vgeo::internal::gpu {

struct AABB{
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
class GpuBackend{
    
    VulkanHandler m_vulkanHandler;
    HandleRegistry<ShapeType::AaBox> m_aaBoxRegistry;
    //HandleRegistry<ShapeType::Capsule> m_capsuleRegistry;
    //HandleRegistry<ShapeType::ConvexHull> m_convexHullRegistry;
    //HandleRegistry<ShapeType::Sphere> m_sphereRegistry;

    std::vector<AaBox> m_aaBoxes;
    //std::vector<Capsule> m_capsules;
    //std::vector<ConvexHull> m_convexHulls;
    //std::vector<Sphere> m_spheres;

    public:
        int test;
        Bv m_boundingVolume;
        std::vector<AaBox> m_testBoxes;
        GpuBackend(VkPhysicalDevice& physicalDevice);
        void queryBoxes();
        
        Handle add(const AaBoxDesc& desc);
        /*
        Handle add(const CapsuleDesc& desc) {
            return addShape(m_capsuleRegistry, m_capsules, Capsule{desc.a, desc.b, desc.radius});
        }

        Handle add(const ConvexHullDesc& desc) {
            return addShape(m_convexHullRegistry, m_convexHulls, ConvexHull{ConvexHullBuilder::build(desc.points)});
        }

        Handle add(const SphereDesc& desc) {
            return addShape(m_sphereRegistry, m_spheres, Sphere{desc.center, desc.radius});
        }

        void remove(Handle h) {
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
                case ShapeType::ConvexHull:
                    m_convexHullRegistry.free(h);
                    m_convexHulls[index] = ConvexHull{};
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
        Handle addShape(HandleRegistry<Type>& registry, std::vector<Shape>& storage, Shape shape) {
            Handle h = registry.allocate();
            uint32_t index = h.getIndex();

            if (index >= storage.size()) {
                storage.resize(index + 1);
            }

            storage[index] = std::move(shape);
            return h;
    }

};

template <typename Bv>
GpuBackend<Bv>::GpuBackend(VkPhysicalDevice& physicalDevice): m_vulkanHandler(physicalDevice){
    test = 1;

    m_vulkanHandler.createLogicalDevice();
    m_vulkanHandler.createDescriptorSetLayouts();
    m_vulkanHandler.createPipelineLayout();
    m_vulkanHandler.createComputePipeline("shaders/comp.spv");
    m_vulkanHandler.createCommandPool();

}

template <typename Bv>
void GpuBackend<Bv>::queryBoxes(){
    VkDeviceSize bufferSize = sizeof(AaBox)*m_aaBoxes.size();
    m_testBoxes.resize(m_aaBoxes.size());
    
    m_vulkanHandler.createInputStorageBuffers(&m_aaBoxes[0], bufferSize);
    
    m_vulkanHandler.createOutputStorageBuffer(bufferSize);
    
    m_vulkanHandler.createDescriptorPool();
    m_vulkanHandler.createDescriptorSets();
    m_vulkanHandler.runComputeShader(&m_testBoxes[0]);
}

template <typename Bv>
Handle GpuBackend<Bv>::add(const AaBoxDesc& desc){
    return addShape(m_aaBoxRegistry, m_aaBoxes, 
        AaBox{desc.min.x,desc.min.y,desc.min.z, 0.0, desc.max.x,desc.max.y,desc.max.z, 0.0});
}


}
