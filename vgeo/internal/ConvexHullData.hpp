#pragma once

#include "vgeo/Point3D.hpp"

#include <cstdint>
#include <vector>

namespace vgeo::internal {

struct ConvexHullData {
    std::vector<Point3D> vertices;
    std::vector<uint32_t> indices;
};

} // namespace vgeo::internal
