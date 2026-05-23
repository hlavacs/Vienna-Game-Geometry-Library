#pragma once

#include "vgeo/Handle.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/ShapeVariant.hpp"

#include <concepts>
#include <vector>

namespace vgeo::internal::cpu {

template <typename Bp>
concept BroadPhase =
    requires(Bp bp, Handle handle, ShapeVariant shape, Terathon::Point3D origin, Terathon::Vector3D dir) {
        { bp.add(handle, shape) } -> std::same_as<void>;
        { bp.remove(handle) } -> std::same_as<void>;
        { bp.findCandidates() } -> std::same_as<std::vector<CandidatePair>>;
        { bp.castRay(origin, dir) } -> std::same_as<std::vector<Handle>>;
    };

template <typename Bp>
concept BvBroadPhase =
    BroadPhase<Bp> && requires { typename Bp::BoundingVolumeType; } && BoundingVolume<typename Bp::BoundingVolumeType>;

} // namespace vgeo::internal::cpu
