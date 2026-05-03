#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/Handle.hpp"

#include <optional>

namespace vgeo::internal::cpu {

// GJK+EPA fallback for any convex pair
template <typename ShapeA, typename ShapeB>
std::optional<CollisionPair> collide(Handle a, const ShapeA& shapeA, Handle b, const ShapeB& shapeB) {
    // TODO: GJK + EPA
    return {};
}

} // namespace vgeo::internal::cpu
