#pragma once

#include "vgeo/Vec3.hpp"

#include <cstdint>
#include <vector>

namespace vgeo::internal {

struct ConvexHullData {
    std::vector<Vec3>     vertices;
    std::vector<uint32_t> indices;
};

} // namespace vgeo::internal
