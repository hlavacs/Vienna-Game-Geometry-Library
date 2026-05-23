#pragma once

#include "vgeo/Handle.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"
#include "vgeo/internal/cpu/ShapeVariant.hpp"

#include <TSVector3D.h>

#include <vector>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv = Aabb>
class Bvh {
public:
    using BoundingVolumeType = Bv;

    void add(Handle handle, ShapeVariant shape) {}

    void remove(Handle handle) {}

    std::vector<CandidatePair> findCandidates() const {
        return {};
    }

    std::vector<Handle> castRay(Terathon::Point3D origin, Terathon::Vector3D dir) const {
        return {};
    }

private:
};

static_assert(BvBroadPhase<Bvh<Aabb>>);

} // namespace vgeo::internal::cpu
