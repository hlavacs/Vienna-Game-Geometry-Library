#pragma once

#include "vgeo/GeometryHandle.hpp"
#include "vgeo/Real.hpp"

#include <TSMotor3D.h>

namespace vgeo::internal::cpu {

struct ShapeInstance {
    GeometryHandle    geometry;
    Terathon::Motor3D motor = Terathon::Motor3D::identity;
    real              scale{1.0};
};

} // namespace vgeo::internal::cpu
