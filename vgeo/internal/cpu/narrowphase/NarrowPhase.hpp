#pragma once

#include "TSRigid3D.h"
#include "TSVector3D.h"
#include "vgeo/CollisionResult.hpp"
#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/internal/cpu/narrowphase/Epa.hpp"
#include "vgeo/internal/cpu/narrowphase/Gjk.hpp"
#include "vgeo/internal/cpu/shapes/Capsule.hpp"
#include "vgeo/internal/cpu/shapes/Sphere.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace vgeo::internal::cpu {

// GJK+EPA fallback for any convex pair
template <typename ShapeA, typename ShapeB>
std::optional<CollisionPair>
collide(InstanceHandle handleA, const ShapeA& shapeA, InstanceHandle handleB, const ShapeB& shapeB) {
    Simplex simplex;

    if (!gjk(shapeA, shapeB, simplex)) {
        return std::nullopt;
    }

    std::optional<Contact> contact = epa(shapeA, shapeB, simplex);
    if (!contact) {
        return std::nullopt;
    }

    return CollisionPair{handleA, handleB, {*contact}};
}

inline std::optional<CollisionPair>
collide(InstanceHandle handleA, const Capsule& capsuleA, InstanceHandle handleB, const Capsule& capsuleB) {
    const Terathon::Point3D  a1     = capsuleA.getA();
    const Terathon::Point3D  a2     = capsuleA.getB();
    const Terathon::Point3D  b1     = capsuleB.getA();
    const Terathon::Point3D  b2     = capsuleB.getB();
    const Terathon::Vector3D d1     = a2 - a1;
    const Terathon::Vector3D d2     = b2 - b1;
    const real               lenASq = Terathon::SquaredMag(d1);
    const real               lenBSq = Terathon::SquaredMag(d2);

    real              s, t;
    Terathon::Point3D closestA, closestB;

    if (lenASq <= 1e-6 && lenBSq <= 1e-6) {
        s        = 0.0;
        t        = 0.0;
        closestA = a1;
        closestB = b1;
    } else if (lenASq <= 1e-6) {
        s        = 0.0;
        t        = std::clamp(Terathon::Dot(a1 - b1, d2) / lenBSq, real(0.0), real(1.0));
        closestA = a1;
        closestB = b1 + t * d2;
    } else if (lenBSq <= 1e-6) {
        const real c = Terathon::Dot(d1, a1 - b1);
        t            = 0.0;
        s            = std::clamp(-c / lenASq, real(0.0), real(1.0));
        closestA     = a1 + s * d1;
        closestB     = b1;
    } else {
        const real b     = Terathon::Dot(d1, d2);
        const real c     = Terathon::Dot(d1, a1 - b1);
        const real f     = Terathon::Dot(d2, a1 - b1);
        const real denom = lenASq * lenBSq - b * b;

        s = (denom != 0.0) ? std::clamp((b * f - c * lenBSq) / denom, real(0.0), real(1.0)) : 0.0;
        t = (b * s + f) / lenBSq;

        if (t < 0.0) {
            t = 0.0;
            s = std::clamp(-c / lenASq, real(0.0), real(1.0));
        } else if (t > 1.0) {
            t = 1.0;
            s = std::clamp((b - c) / lenASq, real(0.0), real(1.0));
        }

        closestA = a1 + s * d1;
        closestB = b1 + t * d2;
    }

    const Terathon::Vector3D delta  = closestA - closestB;
    const real               distSq = Terathon::SquaredMag(delta);
    const real               rSum   = capsuleA.getRadius() + capsuleB.getRadius();

    if (distSq >= rSum * rSum) {
        return std::nullopt;
    }

    const real dist  = std::sqrt(distSq);
    const real depth = rSum - dist;

    if (depth <= 0.0) {
        return std::nullopt;
    }

    const Terathon::Vector3D normal   = (dist > 1e-6) ? delta / dist : Terathon::Vector3D{0.0, 1.0, 0.0};
    const Terathon::Point3D  witnessA = closestA - normal * capsuleA.getRadius();
    const Terathon::Point3D  witnessB = closestB + normal * capsuleB.getRadius();

    return CollisionPair{handleA,
                         handleB,
                         Contact{{normal.x, normal.y, normal.z},
                                 depth,
                                 {witnessA.x, witnessA.y, witnessA.z},
                                 {witnessB.x, witnessB.y, witnessB.z}}};
}

inline std::optional<CollisionPair>
collide(InstanceHandle handleA, const Capsule& capsule, InstanceHandle handleB, const Sphere& sphere) {
    const Terathon::Sphere3D    s          = Terathon::Unitize(sphere.getSphere());
    const Terathon::FlatPoint3D flatCenter = Terathon::FlatCenter(s);
    const Terathon::Point3D     center{flatCenter.x, flatCenter.y, flatCenter.z};
    const real                  radius = std::sqrt(Terathon::SquaredRadiusNorm(s));

    const Terathon::Point3D  b1    = capsule.getA();
    const Terathon::Point3D  b2    = capsule.getB();
    const Terathon::Vector3D d     = b2 - b1;
    const real               lenSq = Terathon::SquaredMag(d);

    const real              t       = std::clamp(Terathon::Dot(center - b1, d) / lenSq, real(0.0), real(1.0));
    const Terathon::Point3D closest = b1 + t * d;

    const Terathon::Vector3D delta  = center - closest;
    const real               distSq = Terathon::SquaredMag(delta);
    const real               rSum   = radius + capsule.getRadius();

    if (distSq >= rSum * rSum) {
        return std::nullopt;
    }

    const real dist  = std::sqrt(distSq);
    const real depth = rSum - dist;

    if (depth <= 0.0) {
        return std::nullopt;
    }

    const Terathon::Vector3D normal   = (dist > 1e-6) ? delta / dist : Terathon::Vector3D{0.0, 1.0, 0.0};
    const Terathon::Point3D  witnessA = center - normal * radius;
    const Terathon::Point3D  witnessB = closest + normal * capsule.getRadius();

    return CollisionPair{handleA,
                         handleB,
                         Contact{{normal.x, normal.y, normal.z},
                                 depth,
                                 {witnessA.x, witnessA.y, witnessA.z},
                                 {witnessB.x, witnessB.y, witnessB.z}}};
}

inline std::optional<CollisionPair>
collide(InstanceHandle handleA, const Sphere& sphereA, InstanceHandle handleB, const Sphere& sphereB) {
    const Terathon::Sphere3D s1                 = Terathon::Unitize(sphereA.getSphere());
    const Terathon::Sphere3D s2                 = Terathon::Unitize(sphereB.getSphere());
    const Terathon::Circle3D intersectionCircle = Terathon::Antiwedge(s1, s2);

    if (Terathon::SquaredRadiusNorm(intersectionCircle) < 0.0) {
        return std::nullopt;
    }

    const Terathon::FlatPoint3D fp1 = Terathon::FlatCenter(s1);
    const Terathon::FlatPoint3D fp2 = Terathon::FlatCenter(s2);
    const Terathon::Point3D     center1{fp1.x, fp1.y, fp1.z};
    const Terathon::Point3D     center2{fp2.x, fp2.y, fp2.z};
    const real                  radius1 = std::sqrt(Terathon::SquaredRadiusNorm(s1));
    const real                  radius2 = std::sqrt(Terathon::SquaredRadiusNorm(s2));

    const Terathon::Vector3D delta = center1 - center2;
    const real               dist  = Terathon::Magnitude(delta);
    const real               depth = (radius1 + radius2) - dist;

    if (depth <= 0.0) {
        return std::nullopt;
    }

    const Terathon::Vector3D normal   = (dist > 1e-6) ? delta / dist : Terathon::Vector3D{1.0, 0.0, 0.0};
    const Terathon::Point3D  witnessA = center1 - normal * radius1;
    const Terathon::Point3D  witnessB = center2 + normal * radius2;

    return CollisionPair{handleA,
                         handleB,
                         Contact{{normal.x, normal.y, normal.z},
                                 depth,
                                 {witnessA.x, witnessA.y, witnessA.z},
                                 {witnessB.x, witnessB.y, witnessB.z}}};
}

} // namespace vgeo::internal::cpu
