#pragma once

#include "vgeo/Handle.hpp"
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
    Handle  shapeA;
    Handle  shapeB;
    Contact contact;
};

struct CollisionResults {
    std::vector<CollisionPair> pairs;
};

} // namespace vgeo
