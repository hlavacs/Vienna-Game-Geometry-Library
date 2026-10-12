#pragma once

#include "vgeo/CollisionResult.hpp"
#include "vgeo/GeometryHandle.hpp"
#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Mat4.hpp"
#include "vgeo/Quat.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/internal/cpu/Backend.hpp"
#include "vgeo/internal/cpu/Bvh.hpp"

#include <cassert>
#include <optional>
#include <span>

#ifdef VGEO_WITH_VULKAN
struct VkPhysicalDevice_T;
#endif

namespace vgeo {

template <internal::cpu::BroadPhase Bp = internal::cpu::Bvh<>>
class CollisionSystem {
public:
    CollisionSystem() = default;

    template <typename... Args>
    explicit CollisionSystem(Args&&... args) : m_backend(std::forward<Args>(args)...) {}

#ifdef VGEO_WITH_VULKAN
    explicit CollisionSystem(VkPhysicalDevice_T* physicalDevice) : m_backend(physicalDevice) {}
#endif

    // Geometry

    GeometryHandle defineAaBox(Vec3 halfExtents) {
        assert(halfExtents.x > 0 && halfExtents.y > 0 && halfExtents.z > 0);
        return m_backend.defineAaBox(halfExtents);
    }

    GeometryHandle defineCapsule(real halfLength, real radius) {
        assert(halfLength > 0);
        assert(radius > 0);
        return m_backend.defineCapsule(halfLength, radius);
    }

    GeometryHandle definePolytope(std::span<const Vec3> points) {
        assert(points.size() >= 4);
        return m_backend.definePolytope(points);
    }

    GeometryHandle defineSphere(real radius) {
        assert(radius > 0);
        return m_backend.defineSphere(radius);
    }

    void removeGeometry(GeometryHandle geometry) {
        assert(isValidGeometry(geometry));
        m_backend.removeGeometry(geometry);
    }

    bool isValidGeometry(GeometryHandle geometry) const {
        return m_backend.isValidGeometry(geometry);
    }

    // Instances

    InstanceHandle add(GeometryHandle geometry, Vec3 position = {}) {
        assert(isValidGeometry(geometry));
        return m_backend.add(geometry, position);
    }

    void setPosition(InstanceHandle handle, Vec3 position) {
        assert(isValid(handle));
        m_backend.setPosition(handle, position);
    }

    void setRotation(InstanceHandle handle, Quat rotation) {
        assert(isValid(handle));
        m_backend.setRotation(handle, rotation);
    }

    void setScale(InstanceHandle handle, real scale) {
        assert(isValid(handle));
        assert(scale > 0);
        m_backend.setScale(handle, scale);
    }

    Vec3 getPosition(InstanceHandle handle) const {
        assert(isValid(handle));
        return m_backend.getPosition(handle);
    }

    Quat getRotation(InstanceHandle handle) const {
        assert(isValid(handle));
        return m_backend.getRotation(handle);
    }

    real getScale(InstanceHandle handle) const {
        assert(isValid(handle));
        return m_backend.getScale(handle);
    }

    // Returns olumn-major 4x4 matrix combining rotation, scale, and translation.
    Mat4 getTransform(InstanceHandle handle) const {
        assert(isValid(handle));
        return m_backend.getTransform(handle);
    }

    void remove(InstanceHandle handle) {
        assert(isValid(handle));
        m_backend.remove(handle);
    }

    bool isValid(InstanceHandle handle) const {
        return m_backend.isValid(handle);
    }

    // Queries

    CollisionResult queryAll() const {
        return m_backend.queryAll();
    }

    std::optional<CollisionPair> queryPair(InstanceHandle shapeA, InstanceHandle shapeB) const {
        assert(isValid(shapeA));
        assert(isValid(shapeB));
        return m_backend.queryPair(shapeA, shapeB);
    }

    bool queryOverlap(InstanceHandle shapeA, InstanceHandle shapeB) const {
        assert(isValid(shapeA));
        assert(isValid(shapeB));
        return m_backend.overlaps(shapeA, shapeB);
    }

    RayResult castRay(Vec3 origin, Vec3 dir) const {
        assert(dir.x != 0 || dir.y != 0 || dir.z != 0);
        return m_backend.castRay(origin, dir);
    }

private:
    internal::cpu::Backend<Bp> m_backend;
};

} // namespace vgeo
