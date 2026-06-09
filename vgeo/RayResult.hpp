#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Vec3.hpp"

#include <vector>

namespace vgeo {

struct RayHit {
    InstanceHandle shape;
    Vec3           position;
    Vec3           normal;
    float          distance;
};

struct RayResult {
    std::vector<RayHit> hits;
};

} // namespace vgeo
