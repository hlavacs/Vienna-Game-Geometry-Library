#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/Handle.hpp"
#include "vgeo/Point3D.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Vector3D.hpp"
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

    explicit CollisionSystem(VkPhysicalDevice_T* physicalDevice) : m_backend(physicalDevice) {}

    Handle addAaBox(const AaBoxDesc& aaBox) {
        return m_backend.add(aaBox);
    }

    Handle addCapsule(const CapsuleDesc& capsule) {
        return m_backend.add(capsule);
    }

    Handle addConvexHull(const ConvexHullDesc& convexHull) {
        return m_backend.add(convexHull);
    }

    Handle addSphere(const SphereDesc& sphere) {
        return m_backend.add(sphere);
    }

    void remove(Handle handle) {
        m_backend.remove(handle);
    }

    bool isValid(Handle handle) const {
        return m_backend.isValid(handle);
    }

    CollisionResults queryAll() const {
        return m_backend.queryAll();
    }

    std::optional<CollisionPair> queryPair(Handle shapeA, Handle shapeB) const {
        return m_backend.queryPair(shapeA, shapeB);
    }

    bool queryOverlap(Handle shapeA, Handle shapeB) const {
        return m_backend.overlaps(shapeA, shapeB);
    }

    RayResult castRay(Point3D origin, Vector3D dir) const {
        return m_backend.castRay(origin, dir);
    }

private:
    internal::cpu::Backend<Bp> m_backend;
};

} // namespace vgeo
