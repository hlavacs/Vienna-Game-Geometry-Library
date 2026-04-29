#pragma once

#include <TSVector3D.h>

#include <concepts>

namespace vgeo::internal::cpu {

template <typename Bv>
concept BoundingVolume = requires(const Bv bv, const Bv other) {
    { bv.intersects(other) } -> std::same_as<bool>;
    { bv.centroid() } -> std::same_as<Terathon::Point3D>;
    { Bv::merge(bv, other) } -> std::same_as<Bv>;
};

} // namespace vgeo::internal::cpu
