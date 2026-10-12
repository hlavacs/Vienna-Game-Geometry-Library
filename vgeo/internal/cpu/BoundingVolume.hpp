#pragma once

#include <TSVector3D.h>

#include <concepts>

namespace vgeo::internal::cpu {

template <typename Bv>
concept BoundingVolume = requires(const Bv bv, const Bv other, Terathon::Point3D origin, Terathon::Vector3D dir) {
    { Bv::merge(bv, other) } -> std::same_as<Bv>;
    { bv.centroid() } -> std::same_as<Terathon::Point3D>;
    { bv.overlaps(other) } -> std::same_as<bool>;
    { bv.intersectsRay(origin, dir) } -> std::same_as<bool>;
};

} // namespace vgeo::internal::cpu
