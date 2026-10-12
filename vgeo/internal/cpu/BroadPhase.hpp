#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Shape.hpp"
#include "vgeo/internal/CandidatePair.hpp"

#include <concepts>
#include <vector>

namespace vgeo::internal::cpu {

template <typename Bp>
concept BroadPhase =
    requires(Bp bp, InstanceHandle handle, Shape shape, Terathon::Point3D origin, Terathon::Vector3D dir) {
        { bp.add(handle, shape) } -> std::same_as<void>;
        { bp.remove(handle) } -> std::same_as<void>;
        { bp.update(handle, shape) } -> std::same_as<void>;
        { bp.findCandidates() } -> std::same_as<std::vector<CandidatePair>>;
        { bp.castRay(origin, dir) } -> std::same_as<std::vector<InstanceHandle>>;
    };

template <typename Bp>
concept BvBroadPhase =
    BroadPhase<Bp> && requires { typename Bp::BoundingVolumeType; } && BoundingVolume<typename Bp::BoundingVolumeType>;

} // namespace vgeo::internal::cpu
