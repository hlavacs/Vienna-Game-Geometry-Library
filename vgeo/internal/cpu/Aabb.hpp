#pragma once

#include "BoundingVolume.hpp"
#include "vgeo/Real.hpp"

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

    [[nodiscard]] static Aabb merge(const Aabb& a, const Aabb& b) {
        return {
            {std::min(a.m_min.x, b.m_min.x), std::min(a.m_min.y, b.m_min.y), std::min(a.m_min.z, b.m_min.z)},
            {std::max(a.m_max.x, b.m_max.x), std::max(a.m_max.y, b.m_max.y), std::max(a.m_max.z, b.m_max.z)},
        };
    }

    [[nodiscard]] Terathon::Point3D centroid() const {
        return {(m_min.x + m_max.x) * 0.5f, (m_min.y + m_max.y) * 0.5f, (m_min.z + m_max.z) * 0.5f};
    }

    [[nodiscard]] bool overlaps(const Aabb& other) const {
        return m_min.x <= other.m_max.x && m_max.x >= other.m_min.x && m_min.y <= other.m_max.y &&
               m_max.y >= other.m_min.y && m_min.z <= other.m_max.z && m_max.z >= other.m_min.z;
    }

    [[nodiscard]] bool intersectsRay(Terathon::Point3D origin, Terathon::Vector3D dir) const {
        real       t1    = (m_min.x - origin.x) / dir.x;
        real       t2    = (m_max.x - origin.x) / dir.x;
        const real tMinX = std::min(t1, t2);
        const real tMaxX = std::max(t1, t2);

        t1               = (m_min.y - origin.y) / dir.y;
        t2               = (m_max.y - origin.y) / dir.y;
        const real tMinY = std::min(t1, t2);
        const real tMaxY = std::max(t1, t2);

        t1               = (m_min.z - origin.z) / dir.z;
        t2               = (m_max.z - origin.z) / dir.z;
        const real tMinZ = std::min(t1, t2);
        const real tMaxZ = std::max(t1, t2);

        const real entry = std::max({tMinX, tMinY, tMinZ});
        const real exit  = std::min({tMaxX, tMaxY, tMaxZ});

        return entry <= exit && exit >= 0;
    }

private:
    Terathon::Point3D m_min;
    Terathon::Point3D m_max;
};

static_assert(BoundingVolume<Aabb>);

} // namespace vgeo::internal::cpu
