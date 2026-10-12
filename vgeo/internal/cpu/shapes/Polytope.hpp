#pragma once

#include "vgeo/Real.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/internal/ConvexHullData.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSMotor3D.h>
#include <TSVector3D.h>

#include <cstdint>
#include <span>
#include <vector>

namespace vgeo::internal::cpu {

class Polytope {
public:
    Polytope() = default;

    explicit Polytope(internal::ConvexHullData data) {
        for (const Vec3& v : data.vertices) {
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
        Terathon::Vector3D sum{0.0, 0.0, 0.0};
        for (const Terathon::Point3D& vertex : m_vertices) {
            sum += vertex;
        }
        real invCount = 1.0 / static_cast<real>(m_vertices.size());
        return {sum.x * invCount, sum.y * invCount, sum.z * invCount};
    }

    [[nodiscard]] Polytope applyTransform(const Terathon::Motor3D& motor, real scale) const {
        Polytope result = *this;
        for (auto& v : result.m_vertices) {
            v = Terathon::Transform(Terathon::Point3D{v.x * scale, v.y * scale, v.z * scale}, motor);
        }
        return result;
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        dir                        = Terathon::Normalize(dir);
        Terathon::Point3D furthest = m_vertices[0];
        real              maxDot   = Terathon::Dot(dir, furthest);
        for (const Terathon::Point3D& vertex : m_vertices) {
            real dot = Terathon::Dot(dir, vertex);
            if (dot > maxDot) {
                maxDot   = dot;
                furthest = vertex;
            }
        }
        return furthest;
    }

    [[nodiscard]] std::optional<RayHit>
    intersectRay(InstanceHandle handle, Terathon::Point3D origin, Terathon::Vector3D dir) const {
        real               tNearest = std::numeric_limits<real>::max();
        Terathon::Vector3D normalNearest;

        for (size_t i = 0; i < m_indices.size(); i += 3) {
            const Terathon::Point3D& v0 = m_vertices[m_indices[i]];
            const Terathon::Point3D& v1 = m_vertices[m_indices[i + 1]];
            const Terathon::Point3D& v2 = m_vertices[m_indices[i + 2]];

            const Terathon::Vector3D e1          = v1 - v0;
            const Terathon::Vector3D e2          = v2 - v0;
            const Terathon::Vector3D h           = Terathon::Cross(dir, e2);
            const real               determinant = Terathon::Dot(e1, h);

            if (determinant < 1e-6) {
                continue;
            }

            const real               f = 1.0 / determinant;
            const Terathon::Vector3D s = origin - v0;
            const real               u = f * Terathon::Dot(s, h);

            if (u < 0.0 || u > 1.0) {
                continue;
            }

            const Terathon::Vector3D q = Terathon::Cross(s, e1);
            const real               v = f * Terathon::Dot(dir, q);

            if (v < 0.0 || u + v > 1.0) {
                continue;
            }

            const real t = f * Terathon::Dot(e2, q);

            if (t < 0.0 || t >= tNearest) {
                continue;
            }

            tNearest      = t;
            normalNearest = Terathon::Normalize(Terathon::Cross(e1, e2));
        }

        if (tNearest == std::numeric_limits<real>::max()) {
            return std::nullopt;
        }

        const Terathon::Point3D position = origin + dir * tNearest;

        return RayHit{handle,
                      Vec3{position.x, position.y, position.z},
                      Vec3{normalNearest.x, normalNearest.y, normalNearest.z},
                      tNearest};
    }

private:
    std::vector<Terathon::Point3D> m_vertices;
    std::vector<uint32_t>          m_indices;
};

template <>
[[nodiscard]] inline Aabb Polytope::computeBv<Aabb>() const {
    Terathon::Point3D min = m_vertices[0];
    Terathon::Point3D max = m_vertices[0];
    for (const Terathon::Point3D& vertex : m_vertices) {
        min = Terathon::Point3D{std::min(min.x, vertex.x), std::min(min.y, vertex.y), std::min(min.z, vertex.z)};
        max = Terathon::Point3D{std::max(max.x, vertex.x), std::max(max.y, vertex.y), std::max(max.z, vertex.z)};
    }
    return {min, max};
}

static_assert(CollisionShape<Polytope, Aabb>);

} // namespace vgeo::internal::cpu
