#include <vgeo/CollisionSystem.hpp>

int main() {
    auto cs = vgeo::CollisionSystem<>::cpu();
    auto cs2 = vgeo::CollisionSystem<vgeo::Aabb>::cpu(); // set bounding volume type explicitly

    return 0;
}
