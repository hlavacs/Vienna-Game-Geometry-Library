#include "vgeo/internal/cpu/shapes/AaBox.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace vgeo::internal::cpu;

TEST_CASE("AaBox centroid", "[AaBox]") {
    AaBox box{{0.0f, 0.0f, 0.0f}, {2.0f, 4.0f, 6.0f}};
    auto c = box.centroid();

    CHECK(c.x == 1.0f);
    CHECK(c.y == 2.0f);
    CHECK(c.z == 3.0f);
}

TEST_CASE("AaBox computeBv (Aabb)", "[AaBox]") {
    AaBox box{{-1.0f, -2.0f, -3.0f}, {4.0f, 5.0f, 6.0f}};
    auto bv = box.computeBv<Aabb>();

    CHECK(bv.getMin().x == -1.0f);
    CHECK(bv.getMin().y == -2.0f);
    CHECK(bv.getMin().z == -3.0f);
    CHECK(bv.getMax().x == 4.0f);
    CHECK(bv.getMax().y == 5.0f);
    CHECK(bv.getMax().z == 6.0f);
}

TEST_CASE("AaBox support along axes", "[AaBox]") {
    AaBox box{{-1.0f, -2.0f, -3.0f}, {4.0f, 5.0f, 6.0f}};

    auto a = box.support({1.0f, 0.0f, 0.0f});
    CHECK(a.x == 4.0f);

    auto b = box.support({-1.0f, 0.0f, 0.0f});
    CHECK(b.x == -1.0f);

    auto c = box.support({0.0f, 1.0f, 0.0f});
    CHECK(c.y == 5.0f);

    auto d = box.support({0.0f, -1.0f, 0.0f});
    CHECK(d.y == -2.0f);

    auto e = box.support({0.0f, 0.0f, 1.0f});
    CHECK(e.z == 6.0f);

    auto f = box.support({0.0f, 0.0f, -1.0f});
    CHECK(f.z == -3.0f);
}

TEST_CASE("AaBox support diagonal", "[AaBox]") {
    AaBox box{{-1.0f, -1.0f, -1.0f}, {2.0f, 2.0f, 2.0f}};

    auto a = box.support({1.0f, 1.0f, 1.0f});
    CHECK(a.x == 2.0f);
    CHECK(a.y == 2.0f);
    CHECK(a.z == 2.0f);

    auto b = box.support({-1.0f, -1.0f, -1.0f});
    CHECK(b.x == -1.0f);
    CHECK(b.y == -1.0f);
    CHECK(b.z == -1.0f);
}
