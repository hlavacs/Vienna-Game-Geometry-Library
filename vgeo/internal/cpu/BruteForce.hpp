// vgeo/internal/cpu/broadphase/BruteForce.hpp
#pragma once
#include "vgeo/Handle.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"

#include <TSVector3D.h>

#include <vector>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv>
class BruteForce {
public:
    void add(Handle handle, Bv bv) {
        m_shapeBvs.push_back({handle, bv});
    }

    void remove(Handle handle) {
        std::erase_if(m_shapeBvs, [handle](const auto& shapeBv) { return shapeBv.first == handle; });
    }

    std::vector<CandidatePair> findCandidates() const {
        std::vector<CandidatePair> candidates;
        for (size_t i = 0; i < m_shapeBvs.size(); ++i) {
            for (size_t j = i + 1; j < m_shapeBvs.size(); ++j) {
                if (m_shapeBvs[i].second.intersects(m_shapeBvs[j].second)) {
                    candidates.push_back({m_shapeBvs[i].first, m_shapeBvs[j].first});
                }
            }
        }
        return candidates;
    }

    std::vector<Handle> castRay(Terathon::Point3D origin, Terathon::Vector3D dir) const {
        return {}; // TODO: implement ray casting
    }

private:
    std::vector<std::pair<Handle, Bv>> m_shapeBvs;
};

static_assert(BroadPhase<BruteForce<Aabb>, Aabb>);

} // namespace vgeo::internal::cpu
