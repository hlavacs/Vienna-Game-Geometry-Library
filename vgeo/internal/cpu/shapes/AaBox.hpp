#pragma once

#include "vgeo/Vec3.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"
#include "vgeo/internal/cpu/shapes/CollisionShape.hpp"

#include <TSMotor3D.h>
#include <TSVector3D.h>

#include <cfloat>
#include <optional>

namespace vgeo::internal::cpu {

class AaBox {
public:
    AaBox() = default;

    explicit AaBox(Vec3 halfExtents)
        : m_min{-halfExtents.x, -halfExtents.y, -halfExtents.z}, m_max{halfExtents.x, halfExtents.y, halfExtents.z} {}

    [[nodiscard]] Terathon::Point3D getMin() const {
        return m_min;
    }

    [[nodiscard]] Terathon::Point3D getMax() const {
        return m_max;
    }

    template <BoundingVolume Bv>
    [[nodiscard]] Bv computeBv() const;

    [[nodiscard]] Terathon::Point3D centroid() const {
        return {(m_min.x + m_max.x) * 0.5f, (m_min.y + m_max.y) * 0.5f, (m_min.z + m_max.z) * 0.5f};
    }

    // AaBox has no rotation to stay axis aligned, only translation and scale
    [[nodiscard]] AaBox applyTransform(const Terathon::Motor3D& motor, float scale) const {
        const Terathon::Point3D pos = motor.GetPosition();
        AaBox                   result;
        result.m_min = Terathon::Point3D(m_min.x * scale + pos.x, m_min.y * scale + pos.y, m_min.z * scale + pos.z);
        result.m_max = Terathon::Point3D(m_max.x * scale + pos.x, m_max.y * scale + pos.y, m_max.z * scale + pos.z);
        return result;
    }

    [[nodiscard]] Terathon::Point3D support(Terathon::Vector3D dir) const {
        dir = Terathon::Normalize(dir);
        return {
            dir.x >= 0.0f ? m_max.x : m_min.x, dir.y >= 0.0f ? m_max.y : m_min.y, dir.z >= 0.0f ? m_max.z : m_min.z};
    }

    [[nodiscard]] std::optional<RayHit>
    intersectRay(InstanceHandle handle, Terathon::Point3D origin, Terathon::Vector3D dir) const {
        float tEntry  = 0.0f;
        float tExit   = FLT_MAX;
        int   hitAxis = 0;

        for (int axis = 0; axis < 3; ++axis) {
            const float originAxis = axis == 0 ? origin.x : axis == 1 ? origin.y : origin.z;
            const float dirAxis    = axis == 0 ? dir.x : axis == 1 ? dir.y : dir.z;
            const float slabMin    = axis == 0 ? m_min.x : axis == 1 ? m_min.y : m_min.z;
            const float slabMax    = axis == 0 ? m_max.x : axis == 1 ? m_max.y : m_max.z;

            if (std::abs(dirAxis) < 1e-6f) {
                if (originAxis < slabMin || originAxis > slabMax) {
                    return std::nullopt;
                }
                continue;
            }

            float t0 = (slabMin - originAxis) / dirAxis;
            float t1 = (slabMax - originAxis) / dirAxis;

            if (t0 > t1) {
                std::swap(t0, t1);
            }

            if (t0 > tEntry) {
                tEntry  = t0;
                hitAxis = axis;
            }
            tExit = std::min(tExit, t1);

            if (tEntry > tExit) {
                return std::nullopt;
            }
        }

        if (tEntry < 0.0f) {
            return std::nullopt;
        }

        const Terathon::Point3D position = origin + dir * tEntry;

        Terathon::Vector3D normal{0.0f, 0.0f, 0.0f};
        switch (hitAxis) {
            case 0:
                normal.x = dir.x > 0.0f ? -1.0f : 1.0f;
                break;
            case 1:
                normal.y = dir.y > 0.0f ? -1.0f : 1.0f;
                break;
            case 2:
                normal.z = dir.z > 0.0f ? -1.0f : 1.0f;
                break;
        }

        return RayHit{handle, Vec3{position.x, position.y, position.z}, Vec3{normal.x, normal.y, normal.z}, tEntry};
    }

private:
    Terathon::Point3D m_min;
    Terathon::Point3D m_max;
};

template <>
[[nodiscard]] inline Aabb AaBox::computeBv<Aabb>() const {
    return {m_min, m_max};
}

static_assert(CollisionShape<AaBox, Aabb>);

} // namespace vgeo::internal::cpu
