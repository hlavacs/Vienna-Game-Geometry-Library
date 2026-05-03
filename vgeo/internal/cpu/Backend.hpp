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
#include "vgeo/internal/cpu/BruteForce.hpp"
#include "vgeo/internal/cpu/narrowphase/NarrowPhase.hpp"
#include "vgeo/internal/cpu/shapes/AaBox.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"
#include "vgeo/internal/cpu/shapes/ConvexHull.hpp"
#include "vgeo/internal/cpu/shapes/Sphere.hpp"

#include <optional>
#include <variant>
#include <vector>

namespace vgeo::internal::cpu {

template <typename Bv>
class Backend {
public:
    Handle add(const AaBoxDesc& desc) {
        Handle h = addShape(m_aaBoxRegistry, m_aaBoxes, AaBox{desc.min, desc.max});
        m_broadphase.add(h, m_aaBoxes[h.getIndex()].template computeBv<Bv>());
        return h;
    }

    Handle add(const CapsuleDesc& desc) {
        Handle h = addShape(m_capsuleRegistry, m_capsules, Capsule{desc.a, desc.b, desc.radius});
        m_broadphase.add(h, m_capsules[h.getIndex()].template computeBv<Bv>());
        return h;
    }

    Handle add(const ConvexHullDesc& desc) {
        Handle h = addShape(m_convexHullRegistry, m_convexHulls, ConvexHull{ConvexHullBuilder::build(desc.points)});
        m_broadphase.add(h, m_convexHulls[h.getIndex()].template computeBv<Bv>());
        return h;
    }

    Handle add(const SphereDesc& desc) {
        Handle h = addShape(m_sphereRegistry, m_spheres, Sphere{desc.center, desc.radius});
        m_broadphase.add(h, m_spheres[h.getIndex()].template computeBv<Bv>());
        return h;
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
        CollisionResults results;
        std::vector<CandidatePair> candidates = m_broadphase.findCandidates();

        for (auto [handleA, handleB] : candidates) {
            if (handleA.getType() > handleB.getType()) {
                std::swap(handleA, handleB);
            }

            Shape shapeA = getShape(handleA);
            Shape shapeB = getShape(handleB);

            std::optional<CollisionPair> result = std::visit(
                [&](const auto& shapeA, const auto& shapeB) { return collide(handleA, shapeA, handleB, shapeB); },
                shapeA,
                shapeB);

            if (result) {
                results.pairs.push_back(std::move(*result));
            }
        }

        return results;
    }

    std::optional<CollisionPair> queryPair(Handle, Handle) const {
        return {};
    }

    bool overlaps(Handle, Handle) const {
        return false;
    }

    RayResult castRay(Point3D, Vector3D) const {
        return {};
    }

private:
    using Shape = std::variant<AaBox, Capsule, Sphere, ConvexHull>;

    Shape getShape(Handle h) const {
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
        Handle h = registry.allocate();
        uint32_t index = h.getIndex();
        if (index >= storage.size()) {
            storage.resize(index + 1);
        }
        storage[index] = std::move(shape);
        return h;
    }

    HandleRegistry<ShapeType::AaBox> m_aaBoxRegistry;
    HandleRegistry<ShapeType::Capsule> m_capsuleRegistry;
    HandleRegistry<ShapeType::ConvexHull> m_convexHullRegistry;
    HandleRegistry<ShapeType::Sphere> m_sphereRegistry;

    std::vector<AaBox> m_aaBoxes;
    std::vector<Capsule> m_capsules;
    std::vector<ConvexHull> m_convexHulls;
    std::vector<Sphere> m_spheres;

    BruteForce<Bv> m_broadphase;
};

} // namespace vgeo::internal::cpu
