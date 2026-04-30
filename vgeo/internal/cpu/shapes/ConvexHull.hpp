#pragma once

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/ConvexHullData.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSVector3D.h>

#include <cstdint>
#include <span>
#include <vector>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv>
class ConvexHull {
public:
    explicit ConvexHull(internal::ConvexHullData data) {
        for (const Point3D& v : data.vertices) {
            m_vertices.emplace_back(v.x, v.y, v.z);
        }
        m_indices.assign(data.indices.begin(), data.indices.end());
    }

    [[nodiscard]] std::span<const Terathon::Point3D> getVertices() const {
        return m_vertices;
    }

    [[nodiscard]] std::span<const uint32_t> getIndices() const {
        return m_indices;
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
    std::vector<Terathon::Point3D> m_vertices;
    std::vector<uint32_t> m_indices;
};

static_assert(CollisionShape<ConvexHull<Aabb>>);

} // namespace vgeo::internal::cpu
