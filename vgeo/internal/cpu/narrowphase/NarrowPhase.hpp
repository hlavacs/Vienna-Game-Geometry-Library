#pragma once

#include "TSRigid3D.h"
#include "vgeo/CollisionResults.hpp"
#include "vgeo/Handle.hpp"
#include "vgeo/internal/cpu/narrowphase/Epa.hpp"
#include "vgeo/internal/cpu/narrowphase/Gjk.hpp"
#include "vgeo/internal/cpu/shapes/Sphere.hpp"

#include <cmath>
#include <optional>

namespace vgeo::internal::cpu {

inline std::optional<CollisionPair> collide(Handle a, const Sphere& shapeA, Handle b, const Sphere& shapeB) {
    const Terathon::Sphere3D sphereA = Terathon::Unitize(shapeA.getSphere());
    const Terathon::Sphere3D sphereB = Terathon::Unitize(shapeB.getSphere());
    const Terathon::Circle3D intersectionCircle = Terathon::Antiwedge(sphereA, sphereB);
    const float intersectionRadiusSq = Terathon::SquaredRadiusNorm(intersectionCircle);

    if (intersectionRadiusSq < 0.0f) {
        return std::nullopt;
    }

    const Terathon::FlatPoint3D flatCenterA = Terathon::FlatCenter(sphereA);
    const Terathon::FlatPoint3D flatCenterB = Terathon::FlatCenter(sphereB);
    const Terathon::Point3D centerA{flatCenterA.x, flatCenterA.y, flatCenterA.z};
    const Terathon::Point3D centerB{flatCenterB.x, flatCenterB.y, flatCenterB.z};
    const float radiusA = std::sqrt(Terathon::SquaredRadiusNorm(sphereA));
    const float radiusB = std::sqrt(Terathon::SquaredRadiusNorm(sphereB));

    const Terathon::Vector3D delta = centerA - centerB;
    const float dist = Terathon::Magnitude(delta);

    Terathon::Vector3D contactNormal;
    if (dist > 0.0f) {
        contactNormal = Terathon::Normalize(delta);
    } else {
        contactNormal = Terathon::Vector3D{1.0f, 0.0f, 0.0f};
    }

    const Terathon::Point3D witnessA = centerA - contactNormal * radiusA;
    const Terathon::Point3D witnessB = centerB + contactNormal * radiusB;

    const Vector3D normal = {contactNormal.x, contactNormal.y, contactNormal.z};
    const float depth = (intersectionRadiusSq > 0.0f) ? ((radiusA + radiusB) - dist) : 0.0f;

    const Contact contact{normal, depth, {witnessA.x, witnessA.y, witnessA.z}, {witnessB.x, witnessB.y, witnessB.z}};
    return CollisionPair{a, b, contact};
}

// GJK+EPA fallback for any convex pair
template <typename ShapeA, typename ShapeB>
std::optional<CollisionPair> collide(Handle a, const ShapeA& shapeA, Handle b, const ShapeB& shapeB) {
    Simplex simplex;

    if (!gjk(shapeA, shapeB, simplex)) {
        return std::nullopt;
    }

    std::expected<Contact, EpaFailure> contact = epa(shapeA, shapeB, simplex);
    if (!contact) {
        return std::nullopt;
    }

    return CollisionPair{a, b, {*contact}};
}

} // namespace vgeo::internal::cpu
