#include "vgeo/internal/cpu/shapes/AaBox.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace vgeo::internal::cpu;
using Catch::Approx;

TEST_CASE("AaBox centroid", "[AaBox]") {
    AaBox box{{0.0f, 0.0f, 0.0f}, {2.0f, 4.0f, 6.0f}};
    auto  c = box.centroid();

    CHECK(c.x == Approx(1.0f));
    CHECK(c.y == Approx(2.0f));
    CHECK(c.z == Approx(3.0f));
}

TEST_CASE("AaBox computeBv (Aabb)", "[AaBox]") {
    AaBox box{{-1.0f, -2.0f, -3.0f}, {4.0f, 5.0f, 6.0f}};
    auto  bv = box.computeBv<Aabb>();

    CHECK(bv.getMin().x == Approx(-1.0f));
    CHECK(bv.getMin().y == Approx(-2.0f));
    CHECK(bv.getMin().z == Approx(-3.0f));
    CHECK(bv.getMax().x == Approx(4.0f));
    CHECK(bv.getMax().y == Approx(5.0f));
    CHECK(bv.getMax().z == Approx(6.0f));
}

TEST_CASE("AaBox support along axes", "[AaBox]") {
    AaBox box{{-1.0f, -2.0f, -3.0f}, {4.0f, 5.0f, 6.0f}};

    auto a = box.support({1.0f, 0.0f, 0.0f});
    CHECK(a.x == Approx(4.0f));

    auto b = box.support({-1.0f, 0.0f, 0.0f});
    CHECK(b.x == Approx(-1.0f));

    auto c = box.support({0.0f, 1.0f, 0.0f});
    CHECK(c.y == Approx(5.0f));

    auto d = box.support({0.0f, -1.0f, 0.0f});
    CHECK(d.y == Approx(-2.0f));

    auto e = box.support({0.0f, 0.0f, 1.0f});
    CHECK(e.z == Approx(6.0f));

    auto f = box.support({0.0f, 0.0f, -1.0f});
    CHECK(f.z == Approx(-3.0f));
}

TEST_CASE("AaBox support diagonal", "[AaBox]") {
    AaBox box{{-1.0f, -1.0f, -1.0f}, {2.0f, 2.0f, 2.0f}};

    auto a = box.support({1.0f, 1.0f, 1.0f});
    CHECK(a.x == Approx(2.0f));
    CHECK(a.y == Approx(2.0f));
    CHECK(a.z == Approx(2.0f));

    auto b = box.support({-1.0f, -1.0f, -1.0f});
    CHECK(b.x == Approx(-1.0f));
    CHECK(b.y == Approx(-1.0f));
    CHECK(b.z == Approx(-1.0f));
}
