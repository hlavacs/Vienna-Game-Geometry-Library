#pragma once

#include "vgeo/Real.hpp"

namespace vgeo {

// Column-major 4x4 matrix: m[col][row]
struct Mat4 {
    real m[4][4];
};

} // namespace vgeo
