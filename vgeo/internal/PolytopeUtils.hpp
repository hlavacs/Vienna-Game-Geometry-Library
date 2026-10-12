#pragma once

#include "vgeo/Real.hpp"

#include <TSVector3D.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace vgeo::internal {

struct DirectedEdge {
    uint32_t a;
    uint32_t b;
};

inline void addBoundaryEdge(std::vector<DirectedEdge>& boundary, uint32_t a, uint32_t b) {
    auto reversed = std::find_if(
        boundary.begin(), boundary.end(), [&](const DirectedEdge& edge) { return edge.a == b && edge.b == a; });
    if (reversed != boundary.end()) {
        boundary.erase(reversed);
        return;
    }

    boundary.emplace_back(a, b);
}

inline real calculateRelativeEpsilon(std::span<const Terathon::Point3D> points) {
    real maxX = 0.0;
    real maxY = 0.0;
    real maxZ = 0.0;
    for (const Terathon::Point3D& p : points) {
        maxX = std::max(maxX, std::abs(p.x));
        maxY = std::max(maxY, std::abs(p.y));
        maxZ = std::max(maxZ, std::abs(p.z));
    }

    // epsilon = 3 * (max|x| + max|y| + max|z|) * FLT_EPSILON
    // (Dirk Gregorius, Implementing QuickHull, GDC 2014)
    return 3.0 * (maxX + maxY + maxZ) * std::numeric_limits<real>::epsilon();
}

inline constexpr uint32_t invalidIndex = std::numeric_limits<uint32_t>::max();

struct HalfEdge {
    uint32_t origin;
    uint32_t next;
    uint32_t twin;
    uint32_t face;
};

inline uint64_t edgeKey(uint32_t a, uint32_t b) {
    return (static_cast<uint64_t>(a) << 32) | b;
}

} // namespace vgeo::internal
