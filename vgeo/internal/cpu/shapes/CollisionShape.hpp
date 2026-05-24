#pragma once

#include "vgeo/RayResult.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"

#include <TSVector3D.h>

#include <concepts>
#include <optional>

namespace vgeo::internal::cpu {

template <typename S, typename Bv>
concept CollisionShape =
    BoundingVolume<Bv> && requires(const S shape, Terathon::Vector3D dir, Terathon::Point3D origin) {
        { shape.template computeBv<Bv>() } -> std::same_as<Bv>;
        { shape.centroid() } -> std::same_as<Terathon::Point3D>;
        { shape.support(dir) } -> std::same_as<Terathon::Point3D>;
        { shape.intersectRay(origin, dir) } -> std::same_as<std::optional<RayHit>>;
    };

} // namespace vgeo::internal::cpu
