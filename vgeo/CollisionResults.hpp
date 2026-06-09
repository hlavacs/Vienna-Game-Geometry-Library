#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Vec3.hpp"

#include <vector>

namespace vgeo {

struct Contact {
    Vec3  normal; // pointing from B to A
    float depth;
    Vec3  witnessA;
    Vec3  witnessB;
};

struct CollisionPair {
    InstanceHandle shapeA;
    InstanceHandle shapeB;
    Contact        contact;
};

struct CollisionResults {
    std::vector<CollisionPair> pairs;
};

} // namespace vgeo
