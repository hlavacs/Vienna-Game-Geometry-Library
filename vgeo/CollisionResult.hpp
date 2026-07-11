#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/Vec3.hpp"

#include <vector>

namespace vgeo {

struct Contact {
    Vec3 normal; // pointing from B to A
    real depth;
    Vec3 witnessA;
    Vec3 witnessB;
};

struct CollisionPair {
    InstanceHandle shapeA;
    InstanceHandle shapeB;
    Contact        contact;
};

struct CollisionResult {
    std::vector<CollisionPair> pairs;
};

} // namespace vgeo
