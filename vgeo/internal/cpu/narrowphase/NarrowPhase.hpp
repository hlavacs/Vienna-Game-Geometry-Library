#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/Handle.hpp"
#include "vgeo/internal/cpu/narrowphase/Gjk.hpp"

#include <optional>

namespace vgeo::internal::cpu {

// GJK+EPA fallback for any convex pair
template <typename ShapeA, typename ShapeB>
std::optional<CollisionPair> collide(Handle a, const ShapeA& shapeA, Handle b, const ShapeB& shapeB) {
    Simplex simplex;

    if (!gjk(shapeA, shapeB, simplex)) {
        return {};
    }

    // TODO: EPA
    CollisionPair pair{a, b, {}};
    pair.contacts.emplace_back();

    return pair;
}

} // namespace vgeo::internal::cpu
