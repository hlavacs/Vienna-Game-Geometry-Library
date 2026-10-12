#include "vgeo/internal/cpu/shapes/Capsule.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace vgeo::internal::cpu;
using Catch::Approx;

TEST_CASE("Capsule centroid", "[Capsule]") {
    Capsule capsule{3.0f, 0.5f};
    auto    c = capsule.centroid();

    CHECK(c.x == 0.0f);
    CHECK(c.y == 0.0f);
    CHECK(c.z == 0.0f);
}

TEST_CASE("Capsule computeBv (Aabb)", "[Capsule]") {
    // Z-aligned: endpoints at {0,0,-1} and {0,0,1}, radius=1
    // AABB: min={-1,-1,-2}, max={1,1,2}
    Capsule capsule{1.0f, 1.0f};
    auto    bv = capsule.computeBv<Aabb>();

    CHECK(bv.getMin().x == -1.0f);
    CHECK(bv.getMin().y == -1.0f);
    CHECK(bv.getMin().z == -2.0f);
    CHECK(bv.getMax().x == 1.0f);
    CHECK(bv.getMax().y == 1.0f);
    CHECK(bv.getMax().z == 2.0f);
}

TEST_CASE("Capsule support along axis", "[Capsule]") {
    Capsule capsule{1.0f, 1.0f};

    auto a = capsule.support({0.0f, 0.0f, 1.0f});
    CHECK(a.z == Approx(2.0f));

    auto b = capsule.support({0.0f, 0.0f, -1.0f});
    CHECK(b.z == Approx(-2.0f));
}

TEST_CASE("Capsule support perpendicular to axis", "[Capsule]") {
    Capsule capsule{1.0f, 1.0f};

    auto a = capsule.support({1.0f, 0.0f, 0.0f});
    CHECK(a.x == Approx(1.0f));
}

TEST_CASE("Capsule support with zero direction stays finite", "[Capsule]") {
    // GJK can hand in a zero direction when its Minkowski support point lands on the origin;
    // Normalize() of a zero vector is undefined and must not be allowed to poison the result with NaN.
    Capsule capsule{1.0f, 1.0f};
    auto    p = capsule.support({0.0f, 0.0f, 0.0f});

    CHECK(std::isfinite(p.x));
    CHECK(std::isfinite(p.y));
    CHECK(std::isfinite(p.z));
}
