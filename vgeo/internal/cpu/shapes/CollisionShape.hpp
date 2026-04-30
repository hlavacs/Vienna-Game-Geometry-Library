#pragma once

#include "../BoundingVolume.hpp"

#include <TSVector3D.h>

#include <concepts>

namespace vgeo::internal::cpu {

template <typename S>
concept CollisionShape = requires(const S shape, Terathon::Vector3D dir) {
    { shape.computeBv() } -> BoundingVolume;
    { shape.centroid() } -> std::same_as<Terathon::Point3D>;
    { shape.support(dir) } -> std::same_as<Terathon::Point3D>;
};

} // namespace vgeo::internal::cpu
