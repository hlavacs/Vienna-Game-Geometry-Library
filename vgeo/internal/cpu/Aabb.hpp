#pragma once

#include "BoundingVolume.hpp"

#include <TSVector3D.h>

#include <algorithm>
#include <cassert>

namespace vgeo::internal::cpu {

class Aabb {
public:
    Aabb(Terathon::Point3D min, Terathon::Point3D max) : m_min{min}, m_max{max} {
        assert(min.x <= max.x && min.y <= max.y && min.z <= max.z);
    }

    [[nodiscard]] Terathon::Point3D getMin() const {
        return m_min;
    }

    [[nodiscard]] Terathon::Point3D getMax() const {
        return m_max;
    }

    [[nodiscard]] bool intersects(const Aabb& other) const {
        return m_min.x <= other.m_max.x && m_max.x >= other.m_min.x && m_min.y <= other.m_max.y &&
               m_max.y >= other.m_min.y && m_min.z <= other.m_max.z && m_max.z >= other.m_min.z;
    }

    [[nodiscard]] Terathon::Point3D centroid() const {
        return {(m_min.x + m_max.x) * 0.5f, (m_min.y + m_max.y) * 0.5f, (m_min.z + m_max.z) * 0.5f};
    }

    [[nodiscard]] static Aabb merge(const Aabb& a, const Aabb& b) {
        return {
            {std::min(a.m_min.x, b.m_min.x), std::min(a.m_min.y, b.m_min.y), std::min(a.m_min.z, b.m_min.z)},
            {std::max(a.m_max.x, b.m_max.x), std::max(a.m_max.y, b.m_max.y), std::max(a.m_max.z, b.m_max.z)},
        };
    }

private:
    Terathon::Point3D m_min;
    Terathon::Point3D m_max;
};

static_assert(BoundingVolume<Aabb>);

} // namespace vgeo::internal::cpu
