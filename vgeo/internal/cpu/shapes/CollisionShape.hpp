#pragma once
#include "../BoundingVolume.hpp"

#include <TSVector3D.h>

#include <concepts>

namespace vgeo::internal::cpu {

template <typename S, typename Bv>
concept CollisionShape = BoundingVolume<Bv> && requires(const S shape, Terathon::Vector3D dir) {
    { shape.template computeBv<Bv>() } -> std::same_as<Bv>;
    { shape.centroid() } -> std::same_as<Terathon::Point3D>;
    { shape.support(dir) } -> std::same_as<Terathon::Point3D>;
};

} // namespace vgeo::internal::cpu
