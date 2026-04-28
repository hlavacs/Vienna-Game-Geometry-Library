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
#include "vgeo/internal/HandleRegistry.hpp"

#include <optional>

namespace vgeo::internal {

class CpuBackend {
public:
    Handle add(const AaBoxDesc&) {
        return m_aaBoxRegistry.allocate();
    }

    Handle add(const CapsuleDesc&) {
        return m_capsuleRegistry.allocate();
    }

    Handle add(const ConvexHullDesc&) {
        return m_convexHullRegistry.allocate();
    }

    Handle add(const SphereDesc&) {
        return m_sphereRegistry.allocate();
    }

    void remove(Handle h) {
        switch (h.getType()) {
            case ShapeType::AaBox:
                m_aaBoxRegistry.free(h);
                break;
            case ShapeType::Capsule:
                m_capsuleRegistry.free(h);
                break;
            case ShapeType::ConvexHull:
                m_convexHullRegistry.free(h);
                break;
            case ShapeType::Sphere:
                m_sphereRegistry.free(h);
                break;
        }
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
        }
        return false;
    }

    CollisionResults queryAll() const {
        return {};
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
    HandleRegistry<ShapeType::AaBox> m_aaBoxRegistry;
    HandleRegistry<ShapeType::Capsule> m_capsuleRegistry;
    HandleRegistry<ShapeType::ConvexHull> m_convexHullRegistry;
    HandleRegistry<ShapeType::Sphere> m_sphereRegistry;
};

} // namespace vgeo::internal
