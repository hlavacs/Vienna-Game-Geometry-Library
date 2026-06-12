#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"

#include <TSVector3D.h>

#include <vector>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv = Aabb>
class BruteForce {
public:
    using BoundingVolumeType = Bv;

    void add(InstanceHandle handle, Shape shape) {
        Bv bv = std::visit([](const auto& shape) { return shape.template computeBv<Bv>(); }, shape);
        m_shapeBvs.push_back({handle, bv});
    }

    void remove(InstanceHandle handle) {
        std::erase_if(m_shapeBvs, [handle](const auto& shapeBv) { return shapeBv.first == handle; });
    }

    void update(InstanceHandle handle, Shape shape) {
        Bv   bv = std::visit([](const auto& s) { return s.template computeBv<Bv>(); }, shape);
        auto it =
            std::find_if(m_shapeBvs.begin(), m_shapeBvs.end(), [handle](const auto& p) { return p.first == handle; });
        if (it != m_shapeBvs.end()) {
            it->second = bv;
        }
    }

    std::vector<CandidatePair> findCandidates() const {
        std::vector<CandidatePair> candidates;
        for (size_t i = 0; i < m_shapeBvs.size(); ++i) {
            for (size_t j = i + 1; j < m_shapeBvs.size(); ++j) {
                if (m_shapeBvs[i].second.overlaps(m_shapeBvs[j].second)) {
                    candidates.push_back({m_shapeBvs[i].first, m_shapeBvs[j].first});
                }
            }
        }
        return candidates;
    }

    std::vector<InstanceHandle> castRay(Terathon::Point3D origin, Terathon::Vector3D dir) const {
        std::vector<InstanceHandle> hits;
        for (const auto& [handle, bv] : m_shapeBvs) {
            if (bv.intersectsRay(origin, dir)) {
                hits.push_back(handle);
            }
        }
        return hits;
    }

private:
    std::vector<std::pair<InstanceHandle, Bv>> m_shapeBvs;
};

static_assert(BvBroadPhase<BruteForce<Aabb>>);

} // namespace vgeo::internal::cpu
