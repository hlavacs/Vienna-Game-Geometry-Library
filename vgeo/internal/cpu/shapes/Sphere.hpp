#pragma once

#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSVector3D.h>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv>
class Sphere {
public:
    Sphere(Terathon::Point3D center, float radius) : m_center{center}, m_radius{radius} {}

    [[nodiscard]] Terathon::Point3D getCenter() const {
        return m_center;
    }

    [[nodiscard]] float getRadius() const {
        return m_radius;
    }

    [[nodiscard]] Bv computeBv() const {
        return {{0, 0, 0}, {0, 0, 0}};
    }

    [[nodiscard]] Terathon::Point3D centroid() const {
        return {0, 0, 0};
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D) const {
        return {0, 0, 0};
    }

private:
    Terathon::Point3D m_center;
    float m_radius;
};

static_assert(CollisionShape<Sphere<Aabb>>);

} // namespace vgeo::internal::cpu
