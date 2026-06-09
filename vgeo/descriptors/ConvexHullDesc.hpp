#pragma once

#include "vgeo/Vec3.hpp"

#include <vector>

namespace vgeo {

struct ConvexHullDesc {
    std::vector<Vec3> points;
};

} // namespace vgeo
