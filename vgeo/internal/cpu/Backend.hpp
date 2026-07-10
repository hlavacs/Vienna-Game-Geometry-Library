#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/GeometryHandle.hpp"
#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Mat4.hpp"
#include "vgeo/Quat.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Real.hpp"
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
#ifdef VGEO_WITH_VULKAN
#include "vgeo/internal/gpu/VulkanHandler.hpp"
#endif

#include <TSMatrix4D.h>

#include <cassert>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#ifdef VGEO_WITH_VULKAN
struct VkPhysicalDevice_T;
#endif

namespace vgeo::internal::cpu {

template <BroadPhase Bp>
class Backend {
public:
    Backend() = default;

    template <typename... Args>
    explicit Backend(Args&&... args) : m_broadphase(std::forward<Args>(args)...) {}

#ifdef VGEO_WITH_VULKAN
    explicit Backend(VkPhysicalDevice_T* physicalDevice) : m_vulkanHandler(std::in_place, physicalDevice) {}
#endif

    // Geometry

    GeometryHandle defineAaBox(Vec3 halfExtents) {
        return m_aaBoxes.add(AaBox{halfExtents});
    }

    GeometryHandle defineCapsule(real halfLength, real radius) {
        return m_capsules.add(Capsule{halfLength, radius});
    }

    GeometryHandle definePolytope(std::span<const Vec3> points) {
        return m_polytopes.add(Polytope{ConvexHullBuilder::build(points)});
    }

    GeometryHandle defineSphere(real radius) {
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
            case ShapeType::Polytope:
                m_polytopes.remove(geometry);
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
            case ShapeType::Polytope:
                return m_polytopes.isValid(geometry);
            case ShapeType::Sphere:
                return m_spheres.isValid(geometry);
            default:
                assert(false && "unknown ShapeType in isValidGeometry()");
                return false;
        }
    }

    // Instances

    InstanceHandle add(GeometryHandle geometry, Vec3 position = {}) {
        InstanceHandle h = m_instances.add(ShapeInstance{
            geometry, Terathon::Motor3D::MakeTranslation(Terathon::Vector3D{position.x, position.y, position.z})});
        m_broadphase.add(h, getWorldShape(h));
        return h;
    }

    void setPosition(InstanceHandle h, Vec3 position) {
        const Terathon::Quaternion rotation = m_instances[h].motor.v;
        m_instances[h].motor =
            Terathon::Motor3D::MakeTranslation({position.x, position.y, position.z}) * Terathon::Motor3D{rotation};
        m_dirtyInstances.insert(h);
        m_broadphase.update(h, getWorldShape(h));
    }

    void setRotation(InstanceHandle h, vgeo::Quat rotation) {
        const Terathon::Point3D position = m_instances[h].motor.GetPosition();
        m_instances[h].motor = Terathon::Motor3D::MakeTranslation({position.x, position.y, position.z}) *
                               Terathon::Motor3D{Terathon::Quaternion{rotation.x, rotation.y, rotation.z, rotation.w}};
        m_dirtyInstances.insert(h);
        m_broadphase.update(h, getWorldShape(h));
    }

    void setScale(InstanceHandle h, real scale) {
        m_instances[h].scale = scale;
        m_dirtyInstances.insert(h);
        m_broadphase.update(h, getWorldShape(h));
    }

    // Column-major 4x4 matrix combining rotation, scale, and translation
    vgeo::Mat4 getTransform(InstanceHandle h) const {
        const Terathon::Motor3D&    motor     = m_instances[h].motor;
        const real                  scale     = m_instances[h].scale;
        const Terathon::Transform3D transform = motor.GetTransformMatrix();
        vgeo::Mat4                  result{};
        for (int col = 0; col < 3; ++col) {
            for (int row = 0; row < 3; ++row) {
                result.m[col][row] = transform(row, col) * scale;
            }
            result.m[col][3] = 0.0;
        }
        result.m[3][0] = transform(0, 3);
        result.m[3][1] = transform(1, 3);
        result.m[3][2] = transform(2, 3);
        result.m[3][3] = 1.0;
        return result;
    }

    void remove(InstanceHandle h) {
        m_broadphase.remove(h);
        m_instances.remove(h);
        m_dirtyInstances.erase(h);
        m_pairCacheNeedsPrune = true;
    }

    bool isValid(InstanceHandle h) const {
        return m_instances.isValid(h);
    }

    Vec3 getPosition(InstanceHandle h) const {
        const Terathon::Point3D p = m_instances[h].motor.GetPosition();
        return {p.x, p.y, p.z};
    }

    vgeo::Quat getRotation(InstanceHandle h) const {
        const Terathon::Quaternion& q = m_instances[h].motor.v;
        return {q.x, q.y, q.z, q.w};
    }

    real getScale(InstanceHandle h) const {
        return m_instances[h].scale;
    }

    // Queries

    CollisionResults queryAll() const {
        if (m_pairCacheNeedsPrune) {
            std::erase_if(m_pairCache, [&](const auto& cachedPair) {
                return !m_instances.isValid(cachedPair.second.a) || !m_instances.isValid(cachedPair.second.b);
            });
            m_pairCacheNeedsPrune = false;
        }

        if (!m_dirtyInstances.empty()) {
            std::erase_if(m_pairCache, [&](const auto& cachedPair) {
                return m_dirtyInstances.contains(cachedPair.second.a) || m_dirtyInstances.contains(cachedPair.second.b);
            });
            m_dirtyInstances.clear();
        }

        CollisionResults           results;
        std::vector<CandidatePair> candidates = m_broadphase.findCandidates();

        for (auto [handleA, handleB] : candidates) {
            GeometryHandle geometryA = m_instances[handleA].geometry;
            GeometryHandle geometryB = m_instances[handleB].geometry;

            if (geometryA.getType() > geometryB.getType()) {
                std::swap(handleA, handleB);
            }

            if (std::optional<CollisionPair> result = queryPair(handleA, handleB)) {
                results.pairs.push_back(std::move(*result));
            }
        }

        return results;
    }

    std::optional<CollisionPair> queryPair(InstanceHandle handleA, InstanceHandle handleB) const {
        const bool dirty = m_dirtyInstances.contains(handleA) || m_dirtyInstances.contains(handleB);

        const CandidatePair key{handleA, handleB};

        if (!dirty) {
            auto it = m_pairCache.find(key);
            if (it != m_pairCache.end()) {
                const CachedPairResult& cached = it->second;
                if (cached.a == handleA && cached.b == handleB) {
                    return cached.contact ? std::optional(CollisionPair{handleA, handleB, *cached.contact})
                                          : std::nullopt;
                }
                if (cached.a == handleB && cached.b == handleA) {
                    if (!cached.contact) {
                        return std::nullopt;
                    }
                    Contact mirrored = *cached.contact;
                    mirrored.normal  = {-mirrored.normal.x, -mirrored.normal.y, -mirrored.normal.z};
                    std::swap(mirrored.witnessA, mirrored.witnessB);
                    return CollisionPair{handleA, handleB, mirrored};
                }
            }
        }

        Shape shapeA = getWorldShape(handleA);
        Shape shapeB = getWorldShape(handleB);

        std::optional<CollisionPair> result =
            std::visit([&](const auto& a, const auto& b) { return collide(handleA, a, handleB, b); }, shapeA, shapeB);

        m_pairCache.insert_or_assign(
            key, CachedPairResult{handleA, handleB, result ? std::optional(result->contact) : std::nullopt});

        return result;
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
            Shape                 shape = getWorldShape(handle);
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
    Shape getLocalShape(GeometryHandle geometry) const {
        switch (geometry.getType()) {
            case ShapeType::AaBox:
                return m_aaBoxes[geometry];
            case ShapeType::Capsule:
                return m_capsules[geometry];
            case ShapeType::Polytope:
                return m_polytopes[geometry];
            case ShapeType::Sphere:
                return m_spheres[geometry];
            default:
                assert(false && "unknown ShapeType in getLocalShape()");
                return AaBox{};
        }
    }

    Shape getWorldShape(InstanceHandle h) const {
        const ShapeInstance& instance = m_instances[h];
        return std::visit(
            [&](const auto& shape) -> Shape { return shape.applyTransform(instance.motor, instance.scale); },
            getLocalShape(instance.geometry));
    }

    struct CachedPairResult {
        InstanceHandle         a;
        InstanceHandle         b;
        std::optional<Contact> contact;
    };

    ShapePool<AaBox, ShapeType::AaBox>       m_aaBoxes;
    ShapePool<Capsule, ShapeType::Capsule>   m_capsules;
    ShapePool<Polytope, ShapeType::Polytope> m_polytopes;
    ShapePool<Sphere, ShapeType::Sphere>     m_spheres;

    InstancePool<ShapeInstance>                                 m_instances;
    Bp                                                          m_broadphase;
    mutable std::unordered_map<CandidatePair, CachedPairResult> m_pairCache;
    mutable std::unordered_set<InstanceHandle>                  m_dirtyInstances;
    mutable bool                                                m_pairCacheNeedsPrune{false};

#ifdef VGEO_WITH_VULKAN
    std::optional<vgeo::internal::gpu::VulkanHandler> m_vulkanHandler;
#endif
};

} // namespace vgeo::internal::cpu
