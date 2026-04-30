#pragma once

#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSVector3D.h>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv>
class Capsule {
public:
    Capsule(Terathon::Point3D a, Terathon::Point3D b, float radius) : m_a{a}, m_b{b}, m_radius{radius} {}

    [[nodiscard]] Terathon::Point3D getA() const {
        return m_a;
    }

    [[nodiscard]] Terathon::Point3D getB() const {
        return m_b;
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
    Terathon::Point3D m_a;
    Terathon::Point3D m_b;
    float m_radius;
};

static_assert(CollisionShape<Capsule<Aabb>>);

} // namespace vgeo::internal::cpu
