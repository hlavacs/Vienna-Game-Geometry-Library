#pragma once

#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSConformal3D.h>
#include <TSVector3D.h>

#include <cmath>
#include <concepts>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv>
class Sphere {
public:
    Sphere(Terathon::Point3D center, float radius)
        : m_sphere{-1.0f, center.x, center.y, center.z, -(Terathon::SquaredMag(center) - radius * radius) * 0.5f} {}

    [[nodiscard]] Terathon::Sphere3D getSphere() const {
        return m_sphere;
    }

    [[nodiscard]] Terathon::Point3D getCenter() const {
        return {m_sphere.x, m_sphere.y, m_sphere.z};
    }

    [[nodiscard]] float getRadius() const {
        return std::sqrt(Terathon::SquaredRadiusNorm(m_sphere));
    }

    [[nodiscard]] Bv computeBv() const
        requires std::same_as<Bv, Aabb> {
        const float radius = getRadius();
        return {
            {m_sphere.x - radius, m_sphere.y - radius, m_sphere.z - radius},
            {m_sphere.x + radius, m_sphere.y + radius, m_sphere.z + radius},
        };
    }

    [[nodiscard]] Terathon::Point3D centroid() const {
        return getCenter();
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        const float radius = getRadius();
        return {m_sphere.x + radius * dir.x, m_sphere.y + radius * dir.y, m_sphere.z + radius * dir.z};
    }

private:
    Terathon::Sphere3D m_sphere;
};

static_assert(CollisionShape<Sphere<Aabb>>);

} // namespace vgeo::internal::cpu
