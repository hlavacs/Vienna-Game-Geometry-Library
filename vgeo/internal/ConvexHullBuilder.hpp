#pragma once

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/ConvexHullData.hpp"

#include <span>

namespace vgeo::internal {

class ConvexHullBuilder {
public:
    static ConvexHullData build(std::span<const Point3D> points);
    ConvexHullBuilder() = delete;
};

} // namespace vgeo::internal
