#pragma once

#include "vgeo/Handle.hpp"
#include "vgeo/Point3D.hpp"
#include "vgeo/Vector3D.hpp"

#include <vector>

namespace vgeo {

struct Contact {
    Vector3D normal; // pointing from B to A
    float depth;
    Point3D witnessA;
    Point3D witnessB;
};

struct CollisionPair {
    Handle shapeA;
    Handle shapeB;
    Contact contact;
};

struct CollisionResults {
    std::vector<CollisionPair> pairs;
};

} // namespace vgeo
