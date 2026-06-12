#pragma once

#include "vgeo/GeometryHandle.hpp"

#include <TSMotor3D.h>

namespace vgeo::internal::cpu {

struct ShapeInstance {
    GeometryHandle    geometry;
    Terathon::Motor3D motor = Terathon::Motor3D::identity;
    float             scale{1.0f};
};

} // namespace vgeo::internal::cpu
