#pragma once

#include "vgeo/internal/gpu/shapes/GpuSphere.hpp"
#include "vgeo/internal/gpu/shapes/GpuCapsule.hpp"

#include <variant>

namespace vgeo::internal::gpu{

struct SpherePair {
    GpuSphere sphereA;
    GpuSphere sphereB;
};

struct CapsulePair {
    GpuCapsule capsuleA;
    GpuCapsule capsuleB;
};

struct CapsuleSpherePair {
    GpuCapsule capsule;
    GpuSphere sphere;
};

using ShapePair = std::variant<SpherePair, CapsulePair, CapsuleSpherePair>;

}