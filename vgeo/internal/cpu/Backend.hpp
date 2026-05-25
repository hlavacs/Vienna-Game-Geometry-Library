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
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/ConvexHullBuilder.hpp"
#include "vgeo/internal/HandleRegistry.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"
#include "vgeo/internal/cpu/ShapeVariant.hpp"
#include "vgeo/internal/cpu/narrowphase/NarrowPhase.hpp"
#include "vgeo/internal/cpu/shapes/AaBox.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"
#include "vgeo/internal/cpu/shapes/ConvexHull.hpp"
#include "vgeo/internal/cpu/shapes/Sphere.hpp"
#include "vgeo/internal/gpu/VulkanHandler.hpp"

#include <cassert>
#include <optional>
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

    Handle add(const AaBoxDesc& desc) {
        m_cachedResults.reset();

        Handle h = addShape(m_aaBoxRegistry, m_aaBoxes, AaBox{desc.min, desc.max});
        m_broadphase.add(h, ShapeVariant{m_aaBoxes[h.getIndex()]});
        return h;
    }

    Handle add(const CapsuleDesc& desc) {
        m_cachedResults.reset();

        Handle h = addShape(m_capsuleRegistry, m_capsules, Capsule{desc.a, desc.b, desc.radius});
        m_broadphase.add(h, ShapeVariant{m_capsules[h.getIndex()]});
        return h;
    }

    Handle add(const ConvexHullDesc& desc) {
        m_cachedResults.reset();

        Handle h = addShape(m_convexHullRegistry, m_convexHulls, ConvexHull{ConvexHullBuilder::build(desc.points)});
        m_broadphase.add(h, ShapeVariant{m_convexHulls[h.getIndex()]});
        return h;
    }

    Handle add(const SphereDesc& desc) {
        m_cachedResults.reset();

        Handle h = addShape(m_sphereRegistry, m_spheres, Sphere{desc.center, desc.radius});
        m_broadphase.add(h, ShapeVariant{m_spheres[h.getIndex()]});
        return h;
    }

    void remove(Handle h) {
        m_cachedResults.reset();

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
            default:
                assert(false && "unknown ShapeType in remove()");
                break;
        }
        m_broadphase.remove(h);
    }

    bool isValid(Handle h) const {
        switch (h.getType()) {
            case ShapeType::AaBox:
                return m_aaBoxRegistry.isValid(h);
            case ShapeType::Capsule:
                return m_capsuleRegistry.isValid(h);
            case ShapeType::ConvexHull:
                return m_convexHullRegistry.isValid(h);
            case ShapeType::Sphere:
                return m_sphereRegistry.isValid(h);
            default:
                assert(false && "unknown ShapeType in isValid()");
                return false;
        }
    }

    CollisionResults queryAll() const {
        if (m_cachedResults.has_value()) {
            return m_cachedResults.value();
        }

        m_cachedResults.emplace();
        std::vector<CandidatePair> candidates = m_broadphase.findCandidates();

        for (auto [handleA, handleB] : candidates) {
            if (handleA.getType() > handleB.getType()) {
                std::swap(handleA, handleB);
            }

            ShapeVariant shapeA = getShape(handleA);
            ShapeVariant shapeB = getShape(handleB);

            std::optional<CollisionPair> result = std::visit(
                [&](const auto& shapeA, const auto& shapeB) { return collide(handleA, shapeA, handleB, shapeB); },
                shapeA,
                shapeB);

            if (result) {
                m_cachedResults->pairs.push_back(std::move(*result));
            }
        }

        return m_cachedResults.value();
    }

    std::optional<CollisionPair> queryPair(Handle handleA, Handle handleB) const {
        ShapeVariant shapeA = getShape(handleA);
        ShapeVariant shapeB = getShape(handleB);

        return std::visit(
            [&](const auto& shapeA, const auto& shapeB) { return collide(handleA, shapeA, handleB, shapeB); },
            shapeA,
            shapeB);
    }

    bool overlaps(Handle handleA, Handle handleB) const {
        return queryPair(handleA, handleB).has_value();
    }

    RayResult castRay(Point3D, Vector3D) const {
        return {};
    }

private:
    ShapeVariant getShape(Handle h) const {
        switch (h.getType()) {
            case ShapeType::AaBox:
                return m_aaBoxes[h.getIndex()];
            case ShapeType::Capsule:
                return m_capsules[h.getIndex()];
            case ShapeType::Sphere:
                return m_spheres[h.getIndex()];
            case ShapeType::ConvexHull:
                return m_convexHulls[h.getIndex()];
            default:
                assert(false && "unknown ShapeType in getShape()");
                return AaBox{};
        }
    }

    template <typename Shape, ShapeType Type>
    Handle addShape(HandleRegistry<Type>& registry, std::vector<Shape>& storage, Shape shape) {
        Handle   h     = registry.allocate();
        uint32_t index = h.getIndex();
        if (index >= storage.size()) {
            storage.resize(index + 1);
        }
        storage[index] = std::move(shape);
        return h;
    }

    HandleRegistry<ShapeType::AaBox>      m_aaBoxRegistry;
    HandleRegistry<ShapeType::Capsule>    m_capsuleRegistry;
    HandleRegistry<ShapeType::ConvexHull> m_convexHullRegistry;
    HandleRegistry<ShapeType::Sphere>     m_sphereRegistry;

    std::vector<AaBox>      m_aaBoxes;
    std::vector<Capsule>    m_capsules;
    std::vector<ConvexHull> m_convexHulls;
    std::vector<Sphere>     m_spheres;

    std::optional<vgeo::internal::gpu::VulkanHandler> m_vulkanHandler;
    Bp                                                m_broadphase;
    mutable std::optional<CollisionResults>           m_cachedResults;
};

} // namespace vgeo::internal::cpu
