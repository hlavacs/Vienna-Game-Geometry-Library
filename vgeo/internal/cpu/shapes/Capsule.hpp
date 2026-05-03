#pragma once
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSVector3D.h>

#include <algorithm>

namespace vgeo::internal::cpu {

class Capsule {
public:
    Capsule() = default;

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

    template <BoundingVolume Bv>
    [[nodiscard]] Bv computeBv() const;

    [[nodiscard]] Terathon::Point3D centroid() const {
        return {(m_a.x + m_b.x) * 0.5f, (m_a.y + m_b.y) * 0.5f, (m_a.z + m_b.z) * 0.5f};
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        Terathon::Point3D furthestEndpoint = Terathon::Dot(dir, m_a) >= Terathon::Dot(dir, m_b) ? m_a : m_b;
        return furthestEndpoint + m_radius * dir;
    }

private:
    Terathon::Point3D m_a;
    Terathon::Point3D m_b;
    float m_radius;
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
