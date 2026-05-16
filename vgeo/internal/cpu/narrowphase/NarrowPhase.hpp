#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/Handle.hpp"
#include "vgeo/internal/cpu/narrowphase/Epa.hpp"
#include "vgeo/internal/cpu/narrowphase/Gjk.hpp"

#include <optional>

namespace vgeo::internal::cpu {

// GJK+EPA fallback for any convex pair
template <typename ShapeA, typename ShapeB>
std::optional<CollisionPair> collide(Handle a, const ShapeA& shapeA, Handle b, const ShapeB& shapeB) {
    Simplex simplex;

    if (!gjk(shapeA, shapeB, simplex)) {
        return std::nullopt;
    }

    std::expected<Contact, EpaFailure> contact = epa(shapeA, shapeB, simplex);
    if (!contact) {
        return std::nullopt;
    }

    return CollisionPair{a, b, {*contact}};
}

} // namespace vgeo::internal::cpu
