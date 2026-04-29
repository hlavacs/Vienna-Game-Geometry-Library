#pragma once

#include "vgeo/Handle.hpp"
#include "vgeo/Point3D.hpp"
#include "vgeo/Vector3D.hpp"

#include <vector>

namespace vgeo {

struct Contact {
    Point3D position;
    Vector3D normal;
    float depth;
};

struct CollisionPair {
    Handle shapeA;
    Handle shapeB;
    std::vector<Contact> contacts;
};

struct CollisionResults {
    std::vector<CollisionPair> pairs;
};

} // namespace vgeo
