#pragma once

#include "vgeo/Handle.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"
#include "vgeo/internal/cpu/ShapeVariant.hpp"

#include <TSVector3D.h>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace vgeo::internal::cpu {

class SweepAndPrune {
public:
    void add(Handle handle, ShapeVariant shape) {
        Aabb aabb = std::visit([](const auto& shape) { return shape.template computeBv<Aabb>(); }, shape);
        m_aabbs.emplace(handle, aabb);

        m_axisX.emplace_back(handle, aabb.getMin().x, EndpointType::Min);
        m_axisX.emplace_back(handle, aabb.getMax().x, EndpointType::Max);

        m_axisY.emplace_back(handle, aabb.getMin().y, EndpointType::Min);
        m_axisY.emplace_back(handle, aabb.getMax().y, EndpointType::Max);

        m_axisZ.emplace_back(handle, aabb.getMin().z, EndpointType::Min);
        m_axisZ.emplace_back(handle, aabb.getMax().z, EndpointType::Max);

        std::inplace_merge(m_axisX.begin(), m_axisX.end() - 2, m_axisX.end(), [](const Endpoint& a, const Endpoint& b) {
            return a.value < b.value;
        });

        std::inplace_merge(m_axisY.begin(), m_axisY.end() - 2, m_axisY.end(), [](const Endpoint& a, const Endpoint& b) {
            return a.value < b.value;
        });

        std::inplace_merge(m_axisZ.begin(), m_axisZ.end() - 2, m_axisZ.end(), [](const Endpoint& a, const Endpoint& b) {
            return a.value < b.value;
        });
    }

    void remove(Handle handle) {
        std::erase_if(m_axisX, [&handle](const Endpoint& endpoint) { return handle == endpoint.handle; });
        std::erase_if(m_axisY, [&handle](const Endpoint& endpoint) { return handle == endpoint.handle; });
        std::erase_if(m_axisZ, [&handle](const Endpoint& endpoint) { return handle == endpoint.handle; });

        m_aabbs.erase(handle);
    }

    std::vector<CandidatePair> findCandidates() const {
        std::unordered_set<CandidatePair> candidatesX = collectPairs(m_axisX);
        std::unordered_set<CandidatePair> candidatesY = collectPairs(m_axisY);
        std::unordered_set<CandidatePair> candidatesZ = collectPairs(m_axisZ);

        std::vector<CandidatePair> result;
        for (const CandidatePair& pair : candidatesX) {
            if (candidatesY.contains(pair) && candidatesZ.contains(pair)) {
                result.push_back(pair);
            }
        }
        return result;
    }

    std::vector<Handle> castRay(Terathon::Point3D origin, Terathon::Vector3D dir) const {
        std::vector<Handle> hits;

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
        Handle       handle;
        float        value;
        EndpointType type;
    };

    std::vector<Endpoint>            m_axisX;
    std::vector<Endpoint>            m_axisY;
    std::vector<Endpoint>            m_axisZ;
    std::unordered_map<Handle, Aabb> m_aabbs;

    std::unordered_set<CandidatePair> collectPairs(const std::vector<Endpoint>& axis) const {
        std::unordered_set<Handle>        open;
        std::unordered_set<CandidatePair> pairs;

        for (const Endpoint& endpoint : axis) {
            if (endpoint.type == EndpointType::Min) {
                for (const Handle& other : open) {
                    pairs.emplace(other, endpoint.handle);
                }
                open.insert(endpoint.handle);
            } else {
                open.erase(endpoint.handle);
            }
        }

        return pairs;
    }
};

static_assert(BroadPhase<SweepAndPrune>);

} // namespace vgeo::internal::cpu
