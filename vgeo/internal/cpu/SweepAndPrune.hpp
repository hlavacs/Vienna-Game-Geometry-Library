#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/Shape.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"

#include <TSVector3D.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace vgeo::internal::cpu {

class SweepAndPrune {
public:
    void add(InstanceHandle handle, Shape shape) {
        Aabb aabb = std::visit([](const auto& shape) { return shape.template computeBv<Aabb>(); }, shape);
        m_aabbs.emplace(handle, aabb);

        insertSorted(handle, aabb.getMin().x, EndpointType::Min);
        insertSorted(handle, aabb.getMax().x, EndpointType::Max);
    }

    void update(InstanceHandle handle, Shape shape) {
        remove(handle);
        add(handle, shape);
    }

    void remove(InstanceHandle handle) {
        std::erase_if(m_axis, [&handle](const Endpoint& endpoint) { return handle == endpoint.handle; });
        m_aabbs.erase(handle);
    }

    std::vector<CandidatePair> findCandidates() const {
        std::vector<CandidatePair>                   result;
        std::vector<std::pair<InstanceHandle, Aabb>> active;

        for (const Endpoint& endpoint : m_axis) {
            if (endpoint.type == EndpointType::Min) {
                const Aabb& aabb = m_aabbs.at(endpoint.handle);
                for (const auto& [otherHandle, otherAabb] : active) {
                    if (aabb.overlaps(otherAabb)) {
                        result.push_back({otherHandle, endpoint.handle});
                    }
                }
                active.emplace_back(endpoint.handle, aabb);
            } else {
                std::erase_if(active, [&](const auto& entry) { return entry.first == endpoint.handle; });
            }
        }

        return result;
    }

    std::vector<InstanceHandle> castRay(Terathon::Point3D origin, Terathon::Vector3D dir) const {
        std::vector<InstanceHandle> hits;

        for (const auto& [handle, aabb] : m_aabbs) {
            if (aabb.intersectsRay(origin, dir)) {
                hits.emplace_back(handle);
            }
        }

        return hits;
    }

private:
    enum class EndpointType { Min, Max };

    struct Endpoint {
        InstanceHandle handle;
        real           value;
        EndpointType   type;
    };

    std::vector<Endpoint>                    m_axis;
    std::unordered_map<InstanceHandle, Aabb> m_aabbs;

    void insertSorted(InstanceHandle handle, real value, EndpointType type) {
        auto it = std::lower_bound(
            m_axis.begin(), m_axis.end(), Endpoint{handle, value, type}, [](const Endpoint& a, const Endpoint& b) {
                return a.value != b.value ? a.value < b.value : a.type < b.type;
            });
        m_axis.insert(it, Endpoint{handle, value, type});
    }
};

static_assert(BroadPhase<SweepAndPrune>);

} // namespace vgeo::internal::cpu
