#include "vgeo/internal/cpu/ConvexHull.hpp"

#include "vgeo/Point3D.hpp"

namespace vgeo::internal::cpu {

std::span<const Point3D> ConvexHull::getVertices() const {
    return m_vertices;
}

std::span<const uint32_t> ConvexHull::getIndices() const {
    return m_indices;
}

} // namespace vgeo::internal::cpu
