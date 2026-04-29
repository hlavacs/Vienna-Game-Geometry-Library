#pragma once

#include "vgeo/Point3D.hpp"

#include <vector>

namespace vgeo {

struct ConvexHullDesc {
    std::vector<Point3D> points;
};

} // namespace vgeo
