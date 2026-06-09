#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/GeometryHandle.hpp"
#include "vgeo/InstanceHandle.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/descriptors/AaBoxDesc.hpp"
#include "vgeo/descriptors/CapsuleDesc.hpp"
#include "vgeo/descriptors/ConvexHullDesc.hpp"
#include "vgeo/descriptors/SphereDesc.hpp"
#include "vgeo/internal/cpu/Backend.hpp"
#include "vgeo/internal/cpu/Bvh.hpp"

#include <optional>

struct VkPhysicalDevice_T;

namespace vgeo {

template <internal::cpu::BroadPhase Bp = internal::cpu::Bvh<>>
class CollisionSystem {
public:
    CollisionSystem() = default;

    template <typename... Args>
    explicit CollisionSystem(Args&&... args) : m_backend(std::forward<Args>(args)...) {}

    explicit CollisionSystem(VkPhysicalDevice_T* physicalDevice) : m_backend(physicalDevice) {}

    // Geometry

    GeometryHandle defineAaBox(const AaBoxDesc& desc) {
        return m_backend.defineAaBox(desc);
    }

    GeometryHandle defineCapsule(const CapsuleDesc& desc) {
        return m_backend.defineCapsule(desc);
    }

    GeometryHandle defineConvexHull(const ConvexHullDesc& desc) {
        return m_backend.defineConvexHull(desc);
    }

    GeometryHandle defineSphere(const SphereDesc& desc) {
        return m_backend.defineSphere(desc);
    }

    void removeGeometry(GeometryHandle geometry) {
        m_backend.removeGeometry(geometry);
    }

    bool isValidGeometry(GeometryHandle geometry) const {
        return m_backend.isValidGeometry(geometry);
    }

    // Instances

    InstanceHandle add(GeometryHandle geometry) {
        return m_backend.add(geometry);
    }

    void remove(InstanceHandle handle) {
        m_backend.remove(handle);
    }

    bool isValid(InstanceHandle handle) const {
        return m_backend.isValid(handle);
    }

    // Queries

    CollisionResults queryAll() const {
        return m_backend.queryAll();
    }

    std::optional<CollisionPair> queryPair(InstanceHandle shapeA, InstanceHandle shapeB) const {
        return m_backend.queryPair(shapeA, shapeB);
    }

    bool queryOverlap(InstanceHandle shapeA, InstanceHandle shapeB) const {
        return m_backend.overlaps(shapeA, shapeB);
    }

    RayResult castRay(Vec3 origin, Vec3 dir) const {
        return m_backend.castRay(origin, dir);
    }

private:
    internal::cpu::Backend<Bp> m_backend;
};

} // namespace vgeo
