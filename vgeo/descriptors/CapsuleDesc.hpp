#pragma once

#include "vgeo/Point3D.hpp"

namespace vgeo {

struct CapsuleDesc {
    Point3D a;
    Point3D b;
    float radius;
};

} // namespace vgeo
