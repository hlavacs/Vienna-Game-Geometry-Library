#pragma once

#include <cstdint>

namespace vgeo {

enum class ShapeType : uint32_t {
    AaBox    = 0,
    Capsule  = 1,
    Polytope = 2,
    Sphere   = 3,
};

} // namespace vgeo
