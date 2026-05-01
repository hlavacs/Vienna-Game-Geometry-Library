#include "vgeo/internal/cpu/shapes/Sphere.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace vgeo::internal::cpu;

TEST_CASE("Sphere centroid", "[Sphere]") {
    Sphere<Aabb> sphere{{2.0f, 3.0f, 4.0f}, 1.0f};
    auto c = sphere.centroid();

    CHECK(c.x == 2.0f);
    CHECK(c.y == 3.0f);
    CHECK(c.z == 4.0f);
}

TEST_CASE("Sphere computeBv (Aabb)", "[Sphere]") {
    Sphere<Aabb> sphere{{2.0f, 3.0f, 4.0f}, 1.0f};
    auto bv = sphere.computeBv();

    CHECK(bv.getMin().x == 1.0f);
    CHECK(bv.getMin().y == 2.0f);
    CHECK(bv.getMin().z == 3.0f);
    CHECK(bv.getMax().x == 3.0f);
    CHECK(bv.getMax().y == 4.0f);
    CHECK(bv.getMax().z == 5.0f);
}

TEST_CASE("Sphere support", "[Sphere]") {
    Sphere<Aabb> sphere{{2.0f, 3.0f, 4.0f}, 1.0f};

    auto a = sphere.support({1.0f, 0.0f, 0.0f});
    CHECK(a.x == 3.0f);

    auto b = sphere.support({-1.0f, 0.0f, 0.0f});
    CHECK(b.x == 1.0f);

    auto c = sphere.support({0.0f, 1.0f, 0.0f});
    CHECK(c.y == 4.0f);

    auto d = sphere.support({0.0f, -1.0f, 0.0f});
    CHECK(d.y == 2.0f);
}
