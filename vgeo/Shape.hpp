#pragma once

#include "vgeo/internal/cpu/shapes/AaBox.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"
#include "vgeo/internal/cpu/shapes/Polytope.hpp"
#include "vgeo/internal/cpu/shapes/Sphere.hpp"

#include <variant>

namespace vgeo::internal::cpu {

using Shape = std::variant<AaBox, Capsule, Sphere, Polytope>;

}
