#pragma once

#include "vgeo/internal/cpu/shapes/AaBox.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"
#include "vgeo/internal/cpu/shapes/ConvexHull.hpp"
#include "vgeo/internal/cpu/shapes/Sphere.hpp"

#include <variant>

namespace vgeo::internal::cpu {

using ShapeVariant = std::variant<AaBox, Capsule, Sphere, ConvexHull>;

}
