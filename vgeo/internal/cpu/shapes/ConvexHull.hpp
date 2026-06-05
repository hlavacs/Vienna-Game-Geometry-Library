#pragma once

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/ConvexHullData.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSVector3D.h>

#include <cfloat>
#include <cstdint>
#include <span>
#include <vector>

namespace vgeo::internal::cpu {

class ConvexHull {
public:
    ConvexHull() = default;

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

    template <BoundingVolume Bv>
    [[nodiscard]] Bv computeBv() const;

    [[nodiscard]] Terathon::Point3D centroid() const {
        Terathon::Vector3D sum{0.0f, 0.0f, 0.0f};
        for (const Terathon::Point3D& vertex : m_vertices) {
            sum += vertex;
        }
        float invCount = 1.0f / static_cast<float>(m_vertices.size());
        return {sum.x * invCount, sum.y * invCount, sum.z * invCount};
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        dir                        = Terathon::Normalize(dir);
        Terathon::Point3D furthest = m_vertices[0];
        float             maxDot   = Terathon::Dot(dir, furthest);
        for (const Terathon::Point3D& vertex : m_vertices) {
            float dot = Terathon::Dot(dir, vertex);
            if (dot > maxDot) {
                maxDot   = dot;
                furthest = vertex;
            }
        }
        return furthest;
    }

    std::optional<RayHit> intersectRay(Handle handle, Terathon::Point3D origin, Terathon::Vector3D dir) const {
        float              tNearest = FLT_MAX;
        Terathon::Vector3D normalNearest;

        for (size_t i = 0; i < m_indices.size(); i += 3) {
            const Terathon::Point3D& v0 = m_vertices[m_indices[i]];
            const Terathon::Point3D& v1 = m_vertices[m_indices[i + 1]];
            const Terathon::Point3D& v2 = m_vertices[m_indices[i + 2]];

            const Terathon::Vector3D e1          = v1 - v0;
            const Terathon::Vector3D e2          = v2 - v0;
            const Terathon::Vector3D h           = Terathon::Cross(dir, e2);
            const float              determinant = Terathon::Dot(e1, h);

            if (determinant < 1e-6f) {
                continue;
            }

            const float              f = 1.0f / determinant;
            const Terathon::Vector3D s = origin - v0;
            const float              u = f * Terathon::Dot(s, h);

            if (u < 0.0f || u > 1.0f) {
                continue;
            }

            const Terathon::Vector3D q = Terathon::Cross(s, e1);
            const float              v = f * Terathon::Dot(dir, q);

            if (v < 0.0f || u + v > 1.0f) {
                continue;
            }

            const float t = f * Terathon::Dot(e2, q);

            if (t < 0.0f || t >= tNearest) {
                continue;
            }

            tNearest      = t;
            normalNearest = Terathon::Normalize(Terathon::Cross(e1, e2));
        }

        if (tNearest == FLT_MAX) {
            return std::nullopt;
        }

        const Terathon::Point3D position = origin + dir * tNearest;

        return RayHit{handle,
                      Point3D{position.x, position.y, position.z},
                      Vector3D{normalNearest.x, normalNearest.y, normalNearest.z},
                      tNearest};
    }

private:
    std::vector<Terathon::Point3D> m_vertices;
    std::vector<uint32_t>          m_indices;
};

template <>
[[nodiscard]] inline Aabb ConvexHull::computeBv<Aabb>() const {
    Terathon::Point3D min = m_vertices[0];
    Terathon::Point3D max = m_vertices[0];
    for (const Terathon::Point3D& vertex : m_vertices) {
        min = Terathon::Point3D{std::min(min.x, vertex.x), std::min(min.y, vertex.y), std::min(min.z, vertex.z)};
        max = Terathon::Point3D{std::max(max.x, vertex.x), std::max(max.y, vertex.y), std::max(max.z, vertex.z)};
    }
    return {min, max};
}

static_assert(CollisionShape<ConvexHull, Aabb>);

} // namespace vgeo::internal::cpu
