#include "vgeo/internal/cpu/shapes/Capsule.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace vgeo::internal::cpu;
using Catch::Approx;

TEST_CASE("Capsule centroid", "[Capsule]") {
    Capsule capsule{{0.0f, 0.0f, 0.0f}, {2.0f, 4.0f, 6.0f}, 0.5f};
    auto c = capsule.centroid();

    CHECK(c.x == 1.0f);
    CHECK(c.y == 2.0f);
    CHECK(c.z == 3.0f);
}

TEST_CASE("Capsule computeBv (Aabb)", "[Capsule]") {
    Capsule capsule{{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 1.0f};
    auto bv = capsule.computeBv<Aabb>();

    CHECK(bv.getMin().x == -2.0f);
    CHECK(bv.getMin().y == -1.0f);
    CHECK(bv.getMin().z == -1.0f);
    CHECK(bv.getMax().x == 2.0f);
    CHECK(bv.getMax().y == 1.0f);
    CHECK(bv.getMax().z == 1.0f);
}

TEST_CASE("Capsule support along axes", "[Capsule]") {
    Capsule capsule{{0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 1.0f};

    auto a = capsule.support({0.0f, 1.0f, 0.0f});
    CHECK(a.y == Approx(2.0f)); // top endpoint + radius

    auto b = capsule.support({0.0f, -1.0f, 0.0f});
    CHECK(b.y == Approx(-2.0f)); // bottom endpoint - radius
}

TEST_CASE("Capsule support perpendicular to axis", "[Capsule]") {
    Capsule capsule{{0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 1.0f};

    auto a = capsule.support({1.0f, 0.0f, 0.0f});
    CHECK(a.x == Approx(1.0f)); // radius in x direction
}
