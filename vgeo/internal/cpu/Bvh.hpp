#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/Shape.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"

#include <TSVector3D.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>
#include <vector>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv = Aabb>
class Bvh {
public:
    using BoundingVolumeType = Bv;

    void add(InstanceHandle handle, Shape shape) {
        Bv bv = std::visit([](const auto& shape) { return shape.template computeBv<Bv>(); }, shape);
        m_shapeEntries.emplace_back(handle, bv);
        m_indices.push_back(static_cast<uint32_t>(m_indices.size()));
        m_isDirty = true;
    }

    void update(InstanceHandle handle, Shape shape) {
        Bv   bv = std::visit([](const auto& s) { return s.template computeBv<Bv>(); }, shape);
        auto it = std::find_if(
            m_shapeEntries.begin(), m_shapeEntries.end(), [handle](const ShapeEntry& e) { return e.handle == handle; });
        if (it != m_shapeEntries.end()) {
            it->bv    = bv;
            m_isDirty = true;
        }
    }

    void remove(InstanceHandle handle) {
        auto it = std::find_if(m_shapeEntries.begin(), m_shapeEntries.end(), [handle](const ShapeEntry& entry) {
            return handle == entry.handle;
        });
        if (it == m_shapeEntries.end()) {
            return;
        }

        uint32_t index = static_cast<uint32_t>(std::distance(m_shapeEntries.begin(), it));
        m_shapeEntries.erase(it);
        std::erase(m_indices, index);
        m_isDirty = true;
    }

    std::vector<CandidatePair> findCandidates() const {
        if (m_isDirty) {
            rebuild();
        }

        std::vector<CandidatePair> pairs;
        if (!m_nodes.empty()) {
            collectPairs(0, pairs);
        }
        return pairs;
    }

    std::vector<InstanceHandle> castRay(Terathon::Point3D origin, Terathon::Vector3D dir) const {
        if (m_isDirty) {
            rebuild();
        }

        std::vector<InstanceHandle> hits;
        if (!m_nodes.empty()) {
            collectHits(0, origin, dir, hits);
        }
        return hits;
    }

private:
    static constexpr uint32_t Invalid = std::numeric_limits<uint32_t>::max();

    struct ShapeEntry {
        InstanceHandle handle;
        Bv             bv;
    };

    struct Node {
        Bv       bv;
        uint32_t left       = Invalid; // index into m_nodes
        uint32_t right      = Invalid; // index into m_nodes
        uint32_t entryIndex = Invalid; // index into m_shapeEntries

        bool isLeaf() const {
            return left == Invalid;
        }
    };

    std::vector<ShapeEntry>       m_shapeEntries;
    mutable std::vector<Node>     m_nodes;
    mutable std::vector<uint32_t> m_indices;
    mutable bool                  m_isDirty = true;

    void rebuild() const {
        m_nodes.clear();
        if (!m_shapeEntries.empty()) {
            buildNode(0, static_cast<uint32_t>(m_shapeEntries.size()));
        }
        m_isDirty = false;
    }

    uint32_t buildNode(uint32_t begin, uint32_t end) const {
        uint32_t nodeIndex = static_cast<uint32_t>(m_nodes.size());

        if (end - begin == 1) {
            Bv leafBv = m_shapeEntries[m_indices[begin]].bv;
            m_nodes.emplace_back(leafBv, Invalid, Invalid, m_indices[begin]);
            return nodeIndex;
        }

        Bv mergedBv = m_shapeEntries[m_indices[begin]].bv;
        for (uint32_t i = begin + 1; i < end; ++i) {
            mergedBv = Bv::merge(mergedBv, m_shapeEntries[m_indices[i]].bv);
        }

        m_nodes.emplace_back(mergedBv, Invalid, Invalid, Invalid);

        uint32_t mid             = splitMidpoint(begin, end);
        m_nodes[nodeIndex].left  = buildNode(begin, mid);
        m_nodes[nodeIndex].right = buildNode(mid, end);
        return nodeIndex;
    }

    uint32_t splitMidpoint(uint32_t begin, uint32_t end) const {
        Terathon::Point3D min = m_shapeEntries[m_indices[begin]].bv.centroid();
        Terathon::Point3D max = min;

        for (uint32_t i = begin + 1; i < end; ++i) {
            Terathon::Point3D c = m_shapeEntries[m_indices[i]].bv.centroid();
            min.x               = std::min(min.x, c.x);
            max.x               = std::max(max.x, c.x);
            min.y               = std::min(min.y, c.y);
            max.y               = std::max(max.y, c.y);
            min.z               = std::min(min.z, c.z);
            max.z               = std::max(max.z, c.z);
        }

        real spreadX = max.x - min.x;
        real spreadY = max.y - min.y;
        real spreadZ = max.z - min.z;

        int axis = (spreadX > spreadY && spreadX > spreadZ) ? 0 : (spreadY > spreadZ) ? 1 : 2;

        real midpoint = (axis == 0)   ? (min.x + max.x) * 0.5
                        : (axis == 1) ? (min.y + max.y) * 0.5
                                      : (min.z + max.z) * 0.5;

        auto splitPoint = std::partition(m_indices.begin() + begin, m_indices.begin() + end, [&](uint32_t index) {
            auto centroid = m_shapeEntries[index].bv.centroid();
            return (axis == 0 ? centroid.x : axis == 1 ? centroid.y : centroid.z) < midpoint;
        });

        uint32_t mid = static_cast<uint32_t>(std::distance(m_indices.begin(), splitPoint));

        // split equally by count if all on one side
        if (mid == begin || mid == end) {
            mid = (begin + end) / 2;
        }

        return mid;
    }

    void collectPairs(uint32_t nodeIndex, std::vector<CandidatePair>& pairs) const {
        const Node& node = m_nodes[nodeIndex];
        if (node.isLeaf()) {
            return;
        }

        findPairs(node.left, node.right, pairs);

        collectPairs(node.left, pairs);
        collectPairs(node.right, pairs);
    }

    void findPairs(uint32_t a, uint32_t b, std::vector<CandidatePair>& pairs) const {
        const Node& nodeA = m_nodes[a];
        const Node& nodeB = m_nodes[b];

        if (!nodeA.bv.overlaps(nodeB.bv)) {
            return;
        }

        if (nodeA.isLeaf() && nodeB.isLeaf()) {
            pairs.push_back({m_shapeEntries[nodeA.entryIndex].handle, m_shapeEntries[nodeB.entryIndex].handle});
            return;
        }

        if (nodeA.isLeaf()) {
            findPairs(a, nodeB.left, pairs);
            findPairs(a, nodeB.right, pairs);
        } else if (nodeB.isLeaf()) {
            findPairs(nodeA.left, b, pairs);
            findPairs(nodeA.right, b, pairs);
        } else {
            findPairs(nodeA.left, nodeB.left, pairs);
            findPairs(nodeA.left, nodeB.right, pairs);
            findPairs(nodeA.right, nodeB.left, pairs);
            findPairs(nodeA.right, nodeB.right, pairs);
        }
    }

    void collectHits(uint32_t                     nodeIndex,
                     Terathon::Point3D            origin,
                     Terathon::Vector3D           dir,
                     std::vector<InstanceHandle>& hits) const {
        const Node& node = m_nodes[nodeIndex];

        if (!node.bv.intersectsRay(origin, dir)) {
            return;
        }

        if (node.isLeaf()) {
            hits.push_back(m_shapeEntries[node.entryIndex].handle);
            return;
        }

        collectHits(node.left, origin, dir, hits);
        collectHits(node.right, origin, dir, hits);
    }
};

static_assert(BvBroadPhase<Bvh<Aabb>>);

} // namespace vgeo::internal::cpu
