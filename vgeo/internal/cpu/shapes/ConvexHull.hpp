#pragma once

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/ConvexHullData.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"

#include <TSVector3D.h>

#include <cstdint>
#include <span>
#include <vector>

namespace vgeo::internal::cpu {

class ConvexHull {
public:
    explicit ConvexHull(internal::ConvexHullData data) {
        for (const Point3D& v : data.vertices) {
            m_vertices.emplace_back(v.x, v.y, v.z);
        }
        m_indices.assign(data.indices.begin(), data.indices.end());
    }

    std::span<const Terathon::Point3D> getVertices() const {
        return m_vertices;
    }
    std::span<const uint32_t> getIndices() const {
        return m_indices;
    }

private:
    std::vector<Terathon::Point3D> m_vertices;
    std::vector<uint32_t> m_indices;
};

} // namespace vgeo::internal::cpu
