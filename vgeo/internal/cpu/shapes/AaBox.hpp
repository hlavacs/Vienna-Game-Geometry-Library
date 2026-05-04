#pragma once

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSVector3D.h>

namespace vgeo::internal::cpu {

class AaBox {
public:
    AaBox() = default;

    AaBox(Point3D min, Point3D max) : m_min{min.x, min.y, min.z}, m_max{max.x, max.y, max.z} {}

    [[nodiscard]] Terathon::Point3D getMin() const {
        return m_min;
    }

    [[nodiscard]] Terathon::Point3D getMax() const {
        return m_max;
    }

    template <BoundingVolume Bv>
    [[nodiscard]] Bv computeBv() const;

    [[nodiscard]] Terathon::Point3D centroid() const {
        return {(m_min.x + m_max.x) * 0.5f, (m_min.y + m_max.y) * 0.5f, (m_min.z + m_max.z) * 0.5f};
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        return {
            dir.x >= 0.0f ? m_max.x : m_min.x, dir.y >= 0.0f ? m_max.y : m_min.y, dir.z >= 0.0f ? m_max.z : m_min.z};
    }

private:
    Terathon::Point3D m_min;
    Terathon::Point3D m_max;
};

template <>
[[nodiscard]] inline Aabb AaBox::computeBv<Aabb>() const {
    return {m_min, m_max};
}

static_assert(CollisionShape<AaBox, Aabb>);

} // namespace vgeo::internal::cpu
