#pragma once

#include <span>
#include <vgeo/Point3D.hpp>
#include <vgeo/internal/cpu/ConvexHull.hpp>

namespace vgeo::internal {

class ConvexHullBuilder {
  public:
    static ConvexHull buildCpu(std::span<const vgeo::Point3D> points);
    static ConvexHull buildGpu(std::span<const vgeo::Point3D> points);
    ConvexHullBuilder() = delete;
};

} // namespace vgeo::internal
