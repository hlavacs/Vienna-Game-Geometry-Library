#include "vgeo/internal/cpu/shapes/ConvexHull.hpp"

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/ConvexHullBuilder.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <vector>

using namespace vgeo::internal;
using namespace vgeo::internal::cpu;
using Catch::Approx;

static ConvexHullData buildTetrahedron() {
    std::vector<vgeo::Point3D> points{
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
        {0.0f, 2.0f, 0.0f},
        {0.0f, 0.0f, 2.0f}
    };
    return ConvexHullBuilder::build(points);
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

    CHECK(bv.getMin().x == Approx(0.0f));
    CHECK(bv.getMin().y == Approx(0.0f));
    CHECK(bv.getMin().z == Approx(0.0f));
    CHECK(bv.getMax().x == Approx(2.0f));
    CHECK(bv.getMax().y == Approx(2.0f));
    CHECK(bv.getMax().z == Approx(2.0f));
}

TEST_CASE("ConvexHull support", "[ConvexHull]") {
    ConvexHull hull{buildTetrahedron()};

    auto a = hull.support({1.0f, 0.0f, 0.0f});
    CHECK(a.x == Approx(2.0f));

    auto b = hull.support({0.0f, 1.0f, 0.0f});
    CHECK(b.y == Approx(2.0f));

    auto c = hull.support({0.0f, 0.0f, 1.0f});
    CHECK(c.z == Approx(2.0f));

    auto d = hull.support({-1.0f, -1.0f, -1.0f});
    CHECK(d.x == Approx(0.0f));
    CHECK(d.y == Approx(0.0f));
    CHECK(d.z == Approx(0.0f));
}
