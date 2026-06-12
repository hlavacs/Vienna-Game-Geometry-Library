#pragma once

#include "TSMotor3D.h"
#include "TSRigid3D.h"
#include "vgeo/RayResult.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/Vec3.hpp"
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

    explicit Sphere(real radius) : m_sphere{-1.0f, 0.0f, 0.0f, 0.0f, radius * radius * 0.5f} {}

    [[nodiscard]] Terathon::Sphere3D getSphere() const {
        return m_sphere;
    }

    [[nodiscard]] Terathon::Point3D getCenter() const {
        return {m_sphere.x, m_sphere.y, m_sphere.z};
    }

    [[nodiscard]] real getRadius() const {
        return std::sqrt(Terathon::SquaredRadiusNorm(m_sphere));
    }

    template <BoundingVolume Bv>
    [[nodiscard]] Bv computeBv() const;

    [[nodiscard]] Terathon::Point3D centroid() const {
        return getCenter();
    }

    [[nodiscard]] Sphere applyTransform(const Terathon::Motor3D& motor, real scale) const {
        const real              radius = getRadius() * scale;
        const Terathon::Point3D center = motor.GetPosition();
        Sphere                  result;
        result.m_sphere = Terathon::Sphere3D{
            -1.0f, center.x, center.y, center.z, -(Terathon::SquaredMag(center) - radius * radius) * 0.5f};
        return result;
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        dir               = Terathon::Normalize(dir);
        const real radius = getRadius();
        return {m_sphere.x + radius * dir.x, m_sphere.y + radius * dir.y, m_sphere.z + radius * dir.z};
    }

    [[nodiscard]] std::optional<RayHit>
    intersectRay(InstanceHandle handle, Terathon::Point3D origin, Terathon::Vector3D dir) const {
        const Terathon::Line3D   ray    = Terathon::Wedge(origin, dir);
        const Terathon::Dipole3D dipole = Terathon::Unitize(Terathon::Antiwedge(m_sphere, ray));

        if (Terathon::SquaredRadiusNorm(dipole) < 0.0f) {
            return std::nullopt;
        }

        const Terathon::FlatPoint3D dipoleFlatCenter = Terathon::FlatCenter(dipole);
        const Terathon::Point3D     dipoleCenter{dipoleFlatCenter.x, dipoleFlatCenter.y, dipoleFlatCenter.z};
        const real                  radius = std::sqrt(Terathon::SquaredRadiusNorm(dipole));

        const real tCenter = Terathon::Dot(dipoleCenter - origin, dir);
        const real tHit    = tCenter - radius;

        if (tHit < 0.0f) {
            return std::nullopt;
        }

        const Terathon::FlatPoint3D sphereFlatCenter = Terathon::FlatCenter(m_sphere);
        const Terathon::Point3D     sphereCenter{sphereFlatCenter.x, sphereFlatCenter.y, sphereFlatCenter.z};
        const Terathon::Point3D     position = origin + dir * tHit;
        const Terathon::Vector3D    normal   = Terathon::Normalize(position - sphereCenter);

        return RayHit{handle, Vec3{position.x, position.y, position.z}, Vec3{normal.x, normal.y, normal.z}, tHit};
    }

private:
    Terathon::Sphere3D m_sphere;
};

template <>
[[nodiscard]] inline Aabb Sphere::computeBv<Aabb>() const {
    const real radius = getRadius();
    return {
        {m_sphere.x - radius, m_sphere.y - radius, m_sphere.z - radius},
        {m_sphere.x + radius, m_sphere.y + radius, m_sphere.z + radius},
    };
}

static_assert(CollisionShape<Sphere, Aabb>);

} // namespace vgeo::internal::cpu
