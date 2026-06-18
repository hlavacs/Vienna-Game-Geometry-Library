#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/GeometryHandle.hpp"
#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Mat4.hpp"
#include "vgeo/Quat.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/internal/cpu/Backend.hpp"
#include "vgeo/internal/cpu/Bvh.hpp"

#include <optional>
#include <span>

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

    GeometryHandle defineAaBox(Vec3 halfExtents) {
        return m_backend.defineAaBox(halfExtents);
    }

    GeometryHandle defineCapsule(real halfLength, real radius) {
        return m_backend.defineCapsule(halfLength, radius);
    }

    GeometryHandle definePolytope(std::span<const Vec3> points) {
        return m_backend.definePolytope(points);
    }

    GeometryHandle defineSphere(real radius) {
        return m_backend.defineSphere(radius);
    }

    void removeGeometry(GeometryHandle geometry) {
        m_backend.removeGeometry(geometry);
    }

    bool isValidGeometry(GeometryHandle geometry) const {
        return m_backend.isValidGeometry(geometry);
    }

    // Instances

    InstanceHandle add(GeometryHandle geometry, Vec3 position = {}) {
        return m_backend.add(geometry, position);
    }

    void setPosition(InstanceHandle handle, Vec3 position) {
        m_backend.setPosition(handle, position);
    }

    void setRotation(InstanceHandle handle, Quat rotation) {
        m_backend.setRotation(handle, rotation);
    }

    void setScale(InstanceHandle handle, real scale) {
        m_backend.setScale(handle, scale);
    }

    Vec3 getPosition(InstanceHandle handle) const {
        return m_backend.getPosition(handle);
    }

    Quat getRotation(InstanceHandle handle) const {
        return m_backend.getRotation(handle);
    }

    real getScale(InstanceHandle handle) const {
        return m_backend.getScale(handle);
    }

    // Returns olumn-major 4x4 matrix combining rotation, scale, and translation.
    Mat4 getTransform(InstanceHandle handle) const {
        return m_backend.getTransform(handle);
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
