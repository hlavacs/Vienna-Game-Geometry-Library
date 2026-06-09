#pragma once

#include "vgeo/Handle.hpp"
#include "vgeo/Vec3.hpp"

#include <vector>

namespace vgeo {

struct RayHit {
    Handle   shape;
    Vec3  position;
    Vec3  normal;
    float    distance;
};

struct RayResult {
    std::vector<RayHit> hits;
};

} // namespace vgeo
