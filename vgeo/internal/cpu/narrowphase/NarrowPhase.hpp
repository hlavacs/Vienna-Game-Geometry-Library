#pragma once

#include "TSRigid3D.h"
#include "TSVector3D.h"
#include "vgeo/CollisionResults.hpp"
#include "vgeo/Handle.hpp"
#include "vgeo/internal/cpu/narrowphase/Epa.hpp"
#include "vgeo/internal/cpu/narrowphase/Gjk.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"
#include "vgeo/internal/cpu/shapes/Sphere.hpp"

#include <cmath>
#include <optional>

namespace vgeo::internal::cpu {

inline std::optional<CollisionPair>
collide(Handle handleA, const Capsule& capsuleA, Handle handleB, const Capsule& capsuleB) {
    const Terathon::Point3D  a1 = capsuleA.getA();
    const Terathon::Point3D  a2 = capsuleA.getB();
    const Terathon::Point3D  b1 = capsuleB.getA();
    const Terathon::Point3D  b2 = capsuleB.getB();
    const Terathon::Vector3D d1 = a2 - a1;
    const Terathon::Vector3D d2 = b2 - b1;

    const float lenASq = Terathon::SquaredMag(d1);
    const float lenBSq = Terathon::SquaredMag(d2);

    float s;
    float t;

    Terathon::Point3D closestA;
    Terathon::Point3D closestB;

    if (lenASq <= 1e-6f && lenBSq <= 1e-6f) {
        s = 0.0f;
        t = 0.0f;

        closestA = a1;
        closestB = b1;
    } else if (lenASq <= 1e-6f) {
        s = 0.0f;
        t = std::clamp(Terathon::Dot(a1 - b1, d2) / lenBSq, 0.0f, 1.0f);

        closestA = a1;
        closestB = b1 + t * d2;
    } else if (lenBSq <= 1e-6f) {
        const float c = Terathon::Dot(d1, a1 - b1);
        t             = 0.0f;
        s             = std::clamp(-c / lenASq, 0.0f, 1.0f);

        closestA = a1 + s * d1;
        closestB = b1;
    } else {
        const float b     = Terathon::Dot(d1, d2);
        const float c     = Terathon::Dot(d1, a1 - b1);
        const float f     = Terathon::Dot(d2, a1 - b1);
        const float denom = lenASq * lenBSq - b * b;

        s = (denom != 0.0f) ? std::clamp((b * f - c * lenBSq) / denom, 0.0f, 1.0f) : 0.0f;
        t = (b * s + f) / lenBSq;

        if (t < 0.0f) {
            t = 0.0f;
            s = std::clamp(-c / lenASq, 0.0f, 1.0f);
        } else if (t > 1.0f) {
            t = 1.0f;
            s = std::clamp((b - c) / lenASq, 0.0f, 1.0f);
        }

        closestA = a1 + s * d1;
        closestB = b1 + t * d2;
    }

    const Terathon::Vector3D delta  = closestA - closestB;
    const float              distSq = Terathon::SquaredMag(delta);
    const float              rSum   = capsuleA.getRadius() + capsuleB.getRadius();

    if (distSq >= rSum * rSum) {
        return std::nullopt;
    }

    const float dist = std::sqrt(distSq);

    const Terathon::Vector3D normal = (dist > 1e-6f) ? delta / dist : Terathon::Vector3D{0.0f, 1.0f, 0.0f};

    const float             depth    = rSum - dist;
    const Terathon::Point3D witnessA = closestA - normal * capsuleA.getRadius();
    const Terathon::Point3D witnessB = closestB + normal * capsuleB.getRadius();

    return CollisionPair{handleA,
                         handleB,
                         Contact{{normal.x, normal.y, normal.z},
                                 depth,
                                 {witnessA.x, witnessA.y, witnessA.z},
                                 {witnessB.x, witnessB.y, witnessB.z}}};
}

inline std::optional<CollisionPair>
collide(Handle handleA, const Sphere& shapeA, Handle handleB, const Sphere& shapeB) {
    const Terathon::Sphere3D sphereA              = Terathon::Unitize(shapeA.getSphere());
    const Terathon::Sphere3D sphereB              = Terathon::Unitize(shapeB.getSphere());
    const Terathon::Circle3D intersectionCircle   = Terathon::Antiwedge(sphereA, sphereB);
    const float              intersectionRadiusSq = Terathon::SquaredRadiusNorm(intersectionCircle);

    if (intersectionRadiusSq < 0.0f) {
        return std::nullopt;
    }

    const Terathon::FlatPoint3D flatCenterA = Terathon::FlatCenter(sphereA);
    const Terathon::FlatPoint3D flatCenterB = Terathon::FlatCenter(sphereB);
    const Terathon::Point3D     centerA{flatCenterA.x, flatCenterA.y, flatCenterA.z};
    const Terathon::Point3D     centerB{flatCenterB.x, flatCenterB.y, flatCenterB.z};
    const float                 radiusA = std::sqrt(Terathon::SquaredRadiusNorm(sphereA));
    const float                 radiusB = std::sqrt(Terathon::SquaredRadiusNorm(sphereB));

    const Terathon::Vector3D delta = centerA - centerB;
    const float              dist  = Terathon::Magnitude(delta);

    Terathon::Vector3D contactNormal;
    if (dist > 0.0f) {
        contactNormal = Terathon::Normalize(delta);
    } else {
        contactNormal = Terathon::Vector3D{1.0f, 0.0f, 0.0f};
    }

    const Terathon::Point3D witnessA = centerA - contactNormal * radiusA;
    const Terathon::Point3D witnessB = centerB + contactNormal * radiusB;

    const Vector3D normal = {contactNormal.x, contactNormal.y, contactNormal.z};
    const float    depth  = (intersectionRadiusSq > 0.0f) ? ((radiusA + radiusB) - dist) : 0.0f;

    const Contact contact{normal, depth, {witnessA.x, witnessA.y, witnessA.z}, {witnessB.x, witnessB.y, witnessB.z}};
    return CollisionPair{handleA, handleB, contact};
}

// GJK+EPA fallback for any convex pair
template <typename ShapeA, typename ShapeB>
std::optional<CollisionPair> collide(Handle handleA, const ShapeA& shapeA, Handle handleB, const ShapeB& shapeB) {
    Simplex simplex;

    if (!gjk(shapeA, shapeB, simplex)) {
        return std::nullopt;
    }

    std::expected<Contact, EpaFailure> contact = epa(shapeA, shapeB, simplex);
    if (!contact) {
        return std::nullopt;
    }

    return CollisionPair{handleA, handleB, {*contact}};
}

} // namespace vgeo::internal::cpu
