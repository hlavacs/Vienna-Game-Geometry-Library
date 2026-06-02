#pragma once

#include "TSRigid3D.h"
#include "vgeo/Point3D.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Vector3D.hpp"
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
        dir                = Terathon::Normalize(dir);
        const float radius = getRadius();
        return {m_sphere.x + radius * dir.x, m_sphere.y + radius * dir.y, m_sphere.z + radius * dir.z};
    }

    std::optional<RayHit> intersectRay(Handle handle, Terathon::Point3D origin, Terathon::Vector3D dir) const {
        const Terathon::Line3D   ray    = Terathon::Wedge(origin, dir);
        const Terathon::Dipole3D dipole = Terathon::Unitize(Terathon::Antiwedge(m_sphere, ray));

        if (Terathon::SquaredRadiusNorm(dipole) < 0.0f) {
            return std::nullopt;
        }

        const Terathon::FlatPoint3D dipoleFlatCenter = Terathon::FlatCenter(dipole);
        const Terathon::Point3D     dipoleCenter{dipoleFlatCenter.x, dipoleFlatCenter.y, dipoleFlatCenter.z};
        const float                 radius = std::sqrt(Terathon::SquaredRadiusNorm(dipole));

        const float tCenter = Terathon::Dot(dipoleCenter - origin, dir);
        const float tHit    = tCenter - radius;

        if (tHit < 0.0f) {
            return std::nullopt;
        }

        const Terathon::FlatPoint3D sphereFlatCenter = Terathon::FlatCenter(m_sphere);
        const Terathon::Point3D     sphereCenter{sphereFlatCenter.x, sphereFlatCenter.y, sphereFlatCenter.z};
        const Terathon::Point3D     position = origin + dir * tHit;
        const Terathon::Vector3D    normal   = Terathon::Normalize(position - sphereCenter);

        return RayHit{
            handle, Point3D{position.x, position.y, position.z}, Vector3D{normal.x, normal.y, normal.z}, tHit};
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
