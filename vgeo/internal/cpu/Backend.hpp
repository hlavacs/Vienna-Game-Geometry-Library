#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/GeometryHandle.hpp"
#include "vgeo/InstanceHandle.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Shape.hpp"
#include "vgeo/ShapeType.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/ConvexHullBuilder.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"
#include "vgeo/internal/cpu/InstancePool.hpp"
#include "vgeo/internal/cpu/ShapeInstance.hpp"
#include "vgeo/internal/cpu/ShapePool.hpp"
#include "vgeo/internal/cpu/narrowphase/NarrowPhase.hpp"
#include "vgeo/internal/gpu/VulkanHandler.hpp"

#include <cassert>
#include <optional>
#include <span>
#include <variant>
#include <vector>

struct VkPhysicalDevice_T;

namespace vgeo::internal::cpu {

template <BroadPhase Bp>
class Backend {
public:
    Backend() = default;

    template <typename... Args>
    explicit Backend(Args&&... args) : m_broadphase(std::forward<Args>(args)...) {}

    explicit Backend(VkPhysicalDevice_T* physicalDevice) : m_vulkanHandler(std::in_place, physicalDevice) {}

    // Geometry

    GeometryHandle defineAaBox(Vec3 halfExtents) {
        return m_aaBoxes.add(AaBox{halfExtents});
    }

    GeometryHandle defineCapsule(float halfLength, float radius) {
        return m_capsules.add(Capsule{halfLength, radius});
    }

    GeometryHandle defineConvexHull(std::span<const Vec3> points) {
        return m_convexHulls.add(ConvexHull{ConvexHullBuilder::build(points)});
    }

    GeometryHandle defineSphere(float radius) {
        return m_spheres.add(Sphere{radius});
    }

    void removeGeometry(GeometryHandle geometry) {
        switch (geometry.getType()) {
            case ShapeType::AaBox:
                m_aaBoxes.remove(geometry);
                break;
            case ShapeType::Capsule:
                m_capsules.remove(geometry);
                break;
            case ShapeType::ConvexHull:
                m_convexHulls.remove(geometry);
                break;
            case ShapeType::Sphere:
                m_spheres.remove(geometry);
                break;
            default:
                assert(false && "unknown ShapeType in removeGeometry()");
                break;
        }
    }

    bool isValidGeometry(GeometryHandle geometry) const {
        switch (geometry.getType()) {
            case ShapeType::AaBox:
                return m_aaBoxes.isValid(geometry);
            case ShapeType::Capsule:
                return m_capsules.isValid(geometry);
            case ShapeType::ConvexHull:
                return m_convexHulls.isValid(geometry);
            case ShapeType::Sphere:
                return m_spheres.isValid(geometry);
            default:
                assert(false && "unknown ShapeType in isValidGeometry()");
                return false;
        }
    }

    // Instances

    InstanceHandle add(GeometryHandle geometry) {
        m_cachedResults.reset();
        InstanceHandle h = m_instances.add(ShapeInstance{geometry});
        m_broadphase.add(h, getShape(geometry));
        return h;
    }

    void remove(InstanceHandle h) {
        m_cachedResults.reset();
        m_broadphase.remove(h);
        m_instances.remove(h);
    }

    bool isValid(InstanceHandle h) const {
        return m_instances.isValid(h);
    }

    // Queries

    CollisionResults queryAll() const {
        if (m_cachedResults.has_value()) {
            return m_cachedResults.value();
        }

        m_cachedResults.emplace();
        std::vector<CandidatePair> candidates = m_broadphase.findCandidates();

        for (auto [handleA, handleB] : candidates) {
            GeometryHandle geometryA = m_instances[handleA].geometry;
            GeometryHandle geometryB = m_instances[handleB].geometry;

            if (geometryA.getType() > geometryB.getType()) {
                std::swap(handleA, handleB);
                std::swap(geometryA, geometryB);
            }

            Shape shapeA = getShape(geometryA);
            Shape shapeB = getShape(geometryB);

            std::optional<CollisionPair> result = std::visit(
                [&](const auto& a, const auto& b) { return collide(handleA, a, handleB, b); }, shapeA, shapeB);

            if (result) {
                m_cachedResults->pairs.push_back(std::move(*result));
            }
        }

        return m_cachedResults.value();
    }

    std::optional<CollisionPair> queryPair(InstanceHandle handleA, InstanceHandle handleB) const {
        Shape shapeA = getShape(m_instances[handleA].geometry);
        Shape shapeB = getShape(m_instances[handleB].geometry);

        return std::visit(
            [&](const auto& a, const auto& b) { return collide(handleA, a, handleB, b); }, shapeA, shapeB);
    }

    bool overlaps(InstanceHandle handleA, InstanceHandle handleB) const {
        return queryPair(handleA, handleB).has_value();
    }

    RayResult castRay(Vec3 origin, Vec3 dir) const {
        const Terathon::Point3D           terathonOrigin{origin.x, origin.y, origin.z};
        const Terathon::Vector3D          terathonDir{dir.x, dir.y, dir.z};
        const std::vector<InstanceHandle> candidates = m_broadphase.castRay(terathonOrigin, terathonDir);

        RayResult result;

        for (const InstanceHandle& handle : candidates) {
            Shape                 shape = getShape(m_instances[handle].geometry);
            std::optional<RayHit> hit =
                std::visit([&](const auto& s) { return s.intersectRay(handle, terathonOrigin, terathonDir); }, shape);

            if (hit) {
                result.hits.push_back(*hit);
            }
        }

        std::ranges::sort(result.hits, std::less{}, &RayHit::distance);
        return result;
    }

private:
    Shape getShape(GeometryHandle geometry) const {
        switch (geometry.getType()) {
            case ShapeType::AaBox:
                return m_aaBoxes[geometry];
            case ShapeType::Capsule:
                return m_capsules[geometry];
            case ShapeType::ConvexHull:
                return m_convexHulls[geometry];
            case ShapeType::Sphere:
                return m_spheres[geometry];
            default:
                assert(false && "unknown ShapeType in getShape()");
                return AaBox{};
        }
    }

    ShapePool<AaBox, ShapeType::AaBox>           m_aaBoxes;
    ShapePool<Capsule, ShapeType::Capsule>       m_capsules;
    ShapePool<ConvexHull, ShapeType::ConvexHull> m_convexHulls;
    ShapePool<Sphere, ShapeType::Sphere>         m_spheres;

    InstancePool<ShapeInstance>                       m_instances;
    std::optional<vgeo::internal::gpu::VulkanHandler> m_vulkanHandler;
    Bp                                                m_broadphase;
    mutable std::optional<CollisionResults>           m_cachedResults;
};

} // namespace vgeo::internal::cpu
