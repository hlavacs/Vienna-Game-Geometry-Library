#pragma once

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSConformal3D.h>
#include <TSVector3D.h>

#include <cmath>

namespace vgeo::internal::cpu {

class Sphere {
public:
    Sphere() = default;

    Sphere(Point3D center, float radius)
        : m_sphere{-1.0f,
                   center.x,
                   center.y,
                   center.z,
                   -(Terathon::SquaredMag(Terathon::Point3D{center.x, center.y, center.z}) - radius * radius) * 0.5f} {}

    [[nodiscard]] Terathon::Sphere3D getSphere() const {
        return m_sphere;
    }

    [[nodiscard]] Terathon::Point3D getCenter() const {
        return {m_sphere.x, m_sphere.y, m_sphere.z};
    }

    [[nodiscard]] float getRadius() const {
        return std::sqrt(Terathon::SquaredRadiusNorm(m_sphere));
    }

    template <BoundingVolume Bv>
    [[nodiscard]] Bv computeBv() const;

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

template <>
[[nodiscard]] inline Aabb Sphere::computeBv<Aabb>() const {
    const float radius = getRadius();
    return {
        {m_sphere.x - radius, m_sphere.y - radius, m_sphere.z - radius},
        {m_sphere.x + radius, m_sphere.y + radius, m_sphere.z + radius},
    };
}

static_assert(CollisionShape<Sphere, Aabb>);

} // namespace vgeo::internal::cpu
