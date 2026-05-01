#pragma once

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/ConvexHullData.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSVector3D.h>

#include <cstdint>
#include <span>
#include <vector>

namespace vgeo::internal::cpu {

template <BoundingVolume Bv>
class ConvexHull {
public:
    explicit ConvexHull(internal::ConvexHullData data) {
        for (const Point3D& v : data.vertices) {
            m_vertices.emplace_back(v.x, v.y, v.z);
        }
        m_indices.assign(data.indices.begin(), data.indices.end());
    }

    [[nodiscard]] std::span<const Terathon::Point3D> getVertices() const {
        return m_vertices;
    }

    [[nodiscard]] std::span<const uint32_t> getIndices() const {
        return m_indices;
    }

    [[nodiscard]] Bv computeBv() const
        requires std::same_as<Bv, Aabb> {
        Terathon::Point3D min = m_vertices[0];
        Terathon::Point3D max = m_vertices[0];
        for (const Terathon::Point3D& vertex : m_vertices) {
            min = Terathon::Point3D{std::min(min.x, vertex.x), std::min(min.y, vertex.y), std::min(min.z, vertex.z)};
            max = Terathon::Point3D{std::max(max.x, vertex.x), std::max(max.y, vertex.y), std::max(max.z, vertex.z)};
        }
        return {min, max};
    }

    [[nodiscard]] Terathon::Point3D centroid() const {
        Terathon::Vector3D sum{0.0f, 0.0f, 0.0f};
        for (const Terathon::Point3D& vertex : m_vertices) {
            sum += vertex;
        }
        float invCount = 1.0f / static_cast<float>(m_vertices.size());
        return {sum.x * invCount, sum.y * invCount, sum.z * invCount};
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        Terathon::Point3D furthest = m_vertices[0];
        float maxDot = Terathon::Dot(dir, furthest);
        for (const Terathon::Point3D& vertex : m_vertices) {
            float dot = Terathon::Dot(dir, vertex);
            if (dot > maxDot) {
                maxDot = dot;
                furthest = vertex;
            }
        }
        return furthest;
    }

private:
    std::vector<Terathon::Point3D> m_vertices;
    std::vector<uint32_t> m_indices;
};

static_assert(CollisionShape<ConvexHull<Aabb>>);

} // namespace vgeo::internal::cpu
