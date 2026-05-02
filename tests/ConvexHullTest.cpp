#include "vgeo/internal/cpu/shapes/ConvexHull.hpp"

#include "vgeo/internal/ConvexHullBuilder.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace vgeo::internal;
using namespace vgeo::internal::cpu;
using Catch::Approx;

static ConvexHullData buildTetrahedron() {
    return ConvexHullBuilder::build({{0.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}, {0.0f, 2.0f, 0.0f}, {0.0f, 0.0f, 2.0f}});
}

TEST_CASE("ConvexHull centroid", "[ConvexHull]") {
    ConvexHull hull{buildTetrahedron()};
    auto c = hull.centroid();

    CHECK(c.x == Approx(0.5f));
    CHECK(c.y == Approx(0.5f));
    CHECK(c.z == Approx(0.5f));
}

TEST_CASE("ConvexHull computeBv (Aabb)", "[ConvexHull]") {
    ConvexHull hull{buildTetrahedron()};
    auto bv = hull.computeBv<Aabb>();

    CHECK(bv.getMin().x == 0.0f);
    CHECK(bv.getMin().y == 0.0f);
    CHECK(bv.getMin().z == 0.0f);
    CHECK(bv.getMax().x == 2.0f);
    CHECK(bv.getMax().y == 2.0f);
    CHECK(bv.getMax().z == 2.0f);
}

TEST_CASE("ConvexHull support", "[ConvexHull]") {
    ConvexHull hull{buildTetrahedron()};

    auto a = hull.support({1.0f, 0.0f, 0.0f});
    CHECK(a.x == 2.0f);

    auto b = hull.support({0.0f, 1.0f, 0.0f});
    CHECK(b.y == 2.0f);

    auto c = hull.support({0.0f, 0.0f, 1.0f});
    CHECK(c.z == 2.0f);

    auto d = hull.support({-1.0f, -1.0f, -1.0f});
    CHECK(d.x == 0.0f);
    CHECK(d.y == 0.0f);
    CHECK(d.z == 0.0f);
}
