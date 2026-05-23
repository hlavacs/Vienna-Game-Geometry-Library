#include "vgeo/Vgeo.hpp"

int main() {
    vgeo::CollisionSystem<>                      cs1; // default
    vgeo::CollisionSystem<vgeo::Bvh<>>           cs2; // specify broadphase
    vgeo::CollisionSystem<vgeo::Bvh<vgeo::Aabb>> cs3; // specify broadphase and its bounding volume (if possible)

    return 0;
}
