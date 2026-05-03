#pragma once
#include "vgeo/Handle.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"

#include <concepts>
#include <vector>

namespace vgeo::internal::cpu {

template <typename Bp, typename Bv>
concept BroadPhase =
    BoundingVolume<Bv> && requires(Bp bp, Handle handle, Bv bv, Terathon::Point3D origin, Terathon::Vector3D dir) {
        { bp.add(handle, bv) } -> std::same_as<void>;
        { bp.remove(handle) } -> std::same_as<void>;
        { bp.findCandidates() } -> std::same_as<std::vector<CandidatePair>>;
        { bp.castRay(origin, dir) } -> std::same_as<std::vector<Handle>>;
    };

} // namespace vgeo::internal::cpu
