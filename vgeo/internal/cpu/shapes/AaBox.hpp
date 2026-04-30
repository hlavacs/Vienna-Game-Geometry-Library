#pragma once

#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSVector3D.h>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv>
class AaBox {
public:
    AaBox(Terathon::Point3D min, Terathon::Point3D max) : m_min{min}, m_max{max} {}

    [[nodiscard]] Terathon::Point3D getMin() const {
        return m_min;
    }

    [[nodiscard]] Terathon::Point3D getMax() const {
        return m_max;
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
    Terathon::Point3D m_min;
    Terathon::Point3D m_max;
};

static_assert(CollisionShape<AaBox<Aabb>>);

} // namespace vgeo::internal::cpu
