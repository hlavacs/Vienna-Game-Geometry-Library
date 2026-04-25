#pragma once

#include "vgeo/Point3D.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace vgeo::internal {

class ConvexHull {
  public:
    std::span<const Point3D> getVertices() const;
    std::span<const uint32_t> getIndices() const;

  private:
    friend class ConvexHullBuilder;
    ConvexHull() = default;
    std::vector<Point3D> m_vertices;
    std::vector<uint32_t> m_indices;
};

} // namespace vgeo::internal
