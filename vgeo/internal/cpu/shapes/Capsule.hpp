#pragma once
#include "vgeo/Real.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSMotor3D.h>
#include <TSVector3D.h>

#include <algorithm>
#include <cmath>

namespace vgeo::internal::cpu {

class Capsule {
public:
    Capsule() = default;

    Capsule(real halfLength, real radius)
        : m_a{0.0f, -halfLength, 0.0f}, m_b{0.0f, halfLength, 0.0f}, m_radius{radius} {}

    [[nodiscard]] Terathon::Point3D getA() const {
        return m_a;
    }

    [[nodiscard]] Terathon::Point3D getB() const {
        return m_b;
    }

    [[nodiscard]] real getRadius() const {
        return m_radius;
    }

    template <BoundingVolume Bv>
    [[nodiscard]] Bv computeBv() const;

    [[nodiscard]] Terathon::Point3D centroid() const {
        return {(m_a.x + m_b.x) * 0.5f, (m_a.y + m_b.y) * 0.5f, (m_a.z + m_b.z) * 0.5f};
    }

    [[nodiscard]] Capsule applyTransform(const Terathon::Motor3D& motor, real scale) const {
        Capsule result  = *this;
        result.m_a      = Terathon::Transform(Terathon::Point3D{m_a.x * scale, m_a.y * scale, m_a.z * scale}, motor);
        result.m_b      = Terathon::Transform(Terathon::Point3D{m_b.x * scale, m_b.y * scale, m_b.z * scale}, motor);
        result.m_radius = m_radius * scale;
        return result;
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        dir                                = Terathon::Normalize(dir);
        Terathon::Point3D furthestEndpoint = Terathon::Dot(dir, m_a) >= Terathon::Dot(dir, m_b) ? m_a : m_b;
        return furthestEndpoint + m_radius * dir;
    }

    [[nodiscard]] std::optional<RayHit>
    intersectRay(InstanceHandle handle, Terathon::Point3D origin, Terathon::Vector3D dir) const {
        const Terathon::Vector3D axis    = m_b - m_a;
        const real               axisLen = Terathon::Magnitude(axis);
        const Terathon::Vector3D axisDir = Terathon::Normalize(axis);

        const std::optional<real> tCylinder = intersectCylinder(origin, dir, axisDir, axisLen);
        const std::optional<real> tCapA     = intersectCap(origin, dir, m_a, -axisDir);
        const std::optional<real> tCapB     = intersectCap(origin, dir, m_b, axisDir);

        enum class HitPart { Cylinder, CapA, CapB };
        HitPart hitPart = HitPart::Cylinder;

        real tNearest = std::numeric_limits<real>::max();
        if (tCylinder && *tCylinder < tNearest) {
            tNearest = *tCylinder;
            hitPart  = HitPart::Cylinder;
        }
        if (tCapA && *tCapA < tNearest) {
            tNearest = *tCapA;
            hitPart  = HitPart::CapA;
        }
        if (tCapB && *tCapB < tNearest) {
            tNearest = *tCapB;
            hitPart  = HitPart::CapB;
        }

        if (tNearest == std::numeric_limits<real>::max()) {
            return std::nullopt;
        }

        const Terathon::Point3D position = origin + dir * tNearest;
        Terathon::Vector3D      normal;

        switch (hitPart) {
            case HitPart::Cylinder: {
                const real              proj          = Terathon::Dot(position - m_a, axisDir);
                const Terathon::Point3D closestOnAxis = m_a + axisDir * proj;
                normal                                = Terathon::Normalize(position - closestOnAxis);
                break;
            }
            case HitPart::CapA:
                normal = Terathon::Normalize(position - m_a);
                break;
            case HitPart::CapB:
                normal = Terathon::Normalize(position - m_b);
                break;
        }

        return RayHit{handle, Vec3{position.x, position.y, position.z}, Vec3{normal.x, normal.y, normal.z}, tNearest};
    }

private:
    Terathon::Point3D m_a;
    Terathon::Point3D m_b;
    real              m_radius;

    [[nodiscard]] std::optional<real> intersectCylinder(Terathon::Point3D  origin,
                                                        Terathon::Vector3D dir,
                                                        Terathon::Vector3D axisDir,
                                                        real               axisLen) const {
        const Terathon::Vector3D oc           = origin - m_a;
        const Terathon::Vector3D d            = Terathon::Reject(dir, axisDir);
        const Terathon::Vector3D f            = Terathon::Reject(oc, axisDir);
        const real               a            = Terathon::Dot(d, d);
        const real               b            = 2.0f * Terathon::Dot(d, f);
        const real               c            = Terathon::Dot(f, f) - m_radius * m_radius;
        const real               discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0.0f || a <= 1e-6f) {
            return std::nullopt;
        }

        const real sqrtDiscriminant = std::sqrt(discriminant);
        const real t0               = (-b - sqrtDiscriminant) / (2.0f * a);
        const real t1               = (-b + sqrtDiscriminant) / (2.0f * a);
        const real t                = t0 >= 0.0f ? t0 : t1;

        if (t < 0.0f) {
            return std::nullopt;
        }

        const Terathon::Point3D  hit   = origin + dir * t;
        const Terathon::Vector3D toHit = hit - m_a;
        const real               proj  = Terathon::Dot(toHit, axisDir);

        if (proj < 0.0f || proj > axisLen) {
            return std::nullopt;
        }

        return t;
    }

    [[nodiscard]] std::optional<real> intersectCap(Terathon::Point3D  origin,
                                                   Terathon::Vector3D dir,
                                                   Terathon::Point3D  capCenter,
                                                   Terathon::Vector3D hemisphereDir) const {
        const Terathon::Vector3D oc   = origin - capCenter;
        const real               a    = Terathon::Dot(dir, dir);
        const real               b    = 2.0f * Terathon::Dot(dir, oc);
        const real               c    = Terathon::Dot(oc, oc) - m_radius * m_radius;
        const real               disc = b * b - 4.0f * a * c;

        if (disc < 0.0f) {
            return std::nullopt;
        }

        const real sqrtDisc = std::sqrt(disc);
        const real t0       = (-b - sqrtDisc) / (2.0f * a);
        const real t1       = (-b + sqrtDisc) / (2.0f * a);
        const real t        = t0 >= 0.0f ? t0 : t1;

        if (t < 0.0f) {
            return std::nullopt;
        }

        const Terathon::Point3D  hit    = origin + dir * t;
        const Terathon::Vector3D normal = Terathon::Normalize(hit - capCenter);

        if (Terathon::Dot(normal, hemisphereDir) < 0.0f) {
            return std::nullopt;
        }

        return t;
    }
};

template <>
[[nodiscard]] inline Aabb Capsule::computeBv<Aabb>() const {
    return {
        {std::min(m_a.x, m_b.x) - m_radius, std::min(m_a.y, m_b.y) - m_radius, std::min(m_a.z, m_b.z) - m_radius},
        {std::max(m_a.x, m_b.x) + m_radius, std::max(m_a.y, m_b.y) + m_radius, std::max(m_a.z, m_b.z) + m_radius},
    };
}

static_assert(CollisionShape<Capsule, Aabb>);

} // namespace vgeo::internal::cpu
