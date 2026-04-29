#pragma once

#include "vgeo/Handle.hpp"
#include "vgeo/Point3D.hpp"
#include "vgeo/Vector3D.hpp"

#include <vector>

namespace vgeo {

struct RayHit {
    Handle shape;
    Point3D position;
    Vector3D normal;
    float distance;
};

struct RayResult {
    std::vector<RayHit> hits;
};

} // namespace vgeo
