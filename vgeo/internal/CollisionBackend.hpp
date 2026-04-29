#pragma once

#include "vgeo/CollisionResults.hpp"
#include "vgeo/Handle.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/descriptors/AaBoxDesc.hpp"
#include "vgeo/descriptors/CapsuleDesc.hpp"
#include "vgeo/descriptors/ConvexHullDesc.hpp"
#include "vgeo/descriptors/SphereDesc.hpp"

#include <concepts>
#include <optional>

namespace vgeo::internal {

template <typename Backend>
concept CollisionBackend = requires(Backend backend,
                                    Handle handle,
                                    AaBoxDesc aaBox,
                                    CapsuleDesc capsule,
                                    ConvexHullDesc convexHull,
                                    SphereDesc sphere,
                                    Point3D origin,
                                    Vector3D dir) {
    { backend.add(aaBox) } -> std::same_as<Handle>;
    { backend.add(capsule) } -> std::same_as<Handle>;
    { backend.add(convexHull) } -> std::same_as<Handle>;
    { backend.add(sphere) } -> std::same_as<Handle>;

    { backend.remove(handle) } -> std::same_as<void>;
    { backend.isValid(handle) } -> std::same_as<bool>;

    { backend.queryAll() } -> std::same_as<CollisionResults>;
    { backend.queryPair(handle, handle) } -> std::same_as<std::optional<CollisionPair>>;
    { backend.overlaps(handle, handle) } -> std::same_as<bool>;
    { backend.castRay(origin, dir) } -> std::same_as<RayResult>;
};

} // namespace vgeo::internal
