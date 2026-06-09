#include "vgeo/internal/cpu/shapes/Sphere.hpp"

#include "catch2/catch_approx.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace vgeo::internal::cpu;
using Catch::Approx;

TEST_CASE("Sphere centroid", "[Sphere]") {
    Sphere sphere{1.0f};
    auto   c = sphere.centroid();

    CHECK(c.x == 0.0f);
    CHECK(c.y == 0.0f);
    CHECK(c.z == 0.0f);
}

TEST_CASE("Sphere computeBv (Aabb)", "[Sphere]") {
    Sphere sphere{1.0f};
    auto   bv = sphere.computeBv<Aabb>();

    CHECK(bv.getMin().x == -1.0f);
    CHECK(bv.getMin().y == -1.0f);
    CHECK(bv.getMin().z == -1.0f);
    CHECK(bv.getMax().x == 1.0f);
    CHECK(bv.getMax().y == 1.0f);
    CHECK(bv.getMax().z == 1.0f);
}

TEST_CASE("Sphere support", "[Sphere]") {
    Sphere sphere{1.0f};

    auto a = sphere.support({1.0f, 0.0f, 0.0f});
    CHECK(a.x == Approx(1.0f));

    auto b = sphere.support({-1.0f, 0.0f, 0.0f});
    CHECK(b.x == Approx(-1.0f));

    auto c = sphere.support({0.0f, 1.0f, 0.0f});
    CHECK(c.y == Approx(1.0f));

    auto d = sphere.support({0.0f, -1.0f, 0.0f});
    CHECK(d.y == Approx(-1.0f));
}
