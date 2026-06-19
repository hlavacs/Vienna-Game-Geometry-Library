#pragma once

// GJK collision detection
// https://winter.dev/articles/gjk-algorithm/

#include <TSVector3D.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>

namespace vgeo::internal::cpu {

inline constexpr int maxGjkIterations = 64;

inline bool isSameDirection(const Terathon::Vector3D& a, const Terathon::Vector3D& b) {
    return Terathon::Dot(a, b) > 0.0f;
}

inline Terathon::Vector3D arbitraryPerpendicular(const Terathon::Vector3D& v) {
    const Terathon::Vector3D axis = (std::abs(v.x) < std::abs(v.y))
                                        ? ((std::abs(v.x) < std::abs(v.z)) ? Terathon::Vector3D{1.0f, 0.0f, 0.0f}
                                                                           : Terathon::Vector3D{0.0f, 0.0f, 1.0f})
                                        : ((std::abs(v.y) < std::abs(v.z)) ? Terathon::Vector3D{0.0f, 1.0f, 0.0f}
                                                                           : Terathon::Vector3D{0.0f, 0.0f, 1.0f});
    return Terathon::Cross(v, axis);
}

inline Terathon::Vector3D perpendicularTowards(const Terathon::Vector3D& edge, const Terathon::Vector3D& towards) {
    const Terathon::Vector3D dir = Terathon::Cross(Terathon::Cross(edge, towards), edge);
    return (Terathon::SquaredMag(dir) > 1e-12f) ? dir : arbitraryPerpendicular(edge);
}

template <typename ShapeA, typename ShapeB>
inline Terathon::Vector3D minkowskiSupport(const ShapeA& shapeA, const ShapeB& shapeB, Terathon::Vector3D dir) {
    auto supportA = shapeA.support(dir);
    auto supportB = shapeB.support(-dir);
    return {supportA.x - supportB.x, supportA.y - supportB.y, supportA.z - supportB.z};
}

struct Simplex {
    std::array<Terathon::Vector3D, 4> points;
    std::array<Terathon::Point3D, 4>  supportPointsA; // Support points on shape A
    std::array<Terathon::Point3D, 4>  supportPointsB; // Support points on shape B
    int                               size = 0;

    void pushFront(Terathon::Vector3D p, Terathon::Point3D pA, Terathon::Point3D pB) {
        points[3] = points[2];
        points[2] = points[1];
        points[1] = points[0];
        points[0] = p;

        supportPointsA[3] = supportPointsA[2];
        supportPointsA[2] = supportPointsA[1];
        supportPointsA[1] = supportPointsA[0];
        supportPointsA[0] = pA;

        supportPointsB[3] = supportPointsB[2];
        supportPointsB[2] = supportPointsB[1];
        supportPointsB[1] = supportPointsB[0];
        supportPointsB[0] = pB;

        size = std::min(size + 1, 4);
    }

    void set(std::initializer_list<Terathon::Vector3D> list) {
        size = 0;
        for (const auto& p : list) {
            points[size++] = p;
        }
    }

    bool lineCase(Terathon::Vector3D& dir) {
        Terathon::Vector3D a  = points[0];
        Terathon::Vector3D b  = points[1];
        Terathon::Vector3D ab = b - a;
        Terathon::Vector3D ao = -a;

        if (isSameDirection(ab, ao)) {
            dir = perpendicularTowards(ab, ao);
        } else {
            set({points[0]});
            dir = ao;
        }
        return false;
    }

    bool triangleCase(Terathon::Vector3D& dir) {
        Terathon::Vector3D a   = points[0];
        Terathon::Vector3D b   = points[1];
        Terathon::Vector3D c   = points[2];
        Terathon::Vector3D ab  = b - a;
        Terathon::Vector3D ac  = c - a;
        Terathon::Vector3D ao  = -a;
        Terathon::Vector3D abc = Terathon::Cross(ab, ac);

        if (isSameDirection(Terathon::Cross(abc, ac), ao)) {
            if (isSameDirection(ac, ao)) {
                set({points[0], points[2]});
                dir = perpendicularTowards(ac, ao);
            } else {
                set({points[0], points[1]});
                return lineCase(dir);
            }
        } else if (isSameDirection(Terathon::Cross(ab, abc), ao)) {
            set({points[0], points[1]});
            return lineCase(dir);
        } else if (isSameDirection(abc, ao)) {
            dir = abc;
        } else {
            // flip winding so normal points toward origin
            set({points[0], points[2], points[1]});
            dir = (Terathon::SquaredMag(abc) > 1e-12f) ? -abc : arbitraryPerpendicular(ab);
        }

        return false;
    }

    bool tetrahedronCase(Terathon::Vector3D& dir) {
        Terathon::Vector3D a   = points[0];
        Terathon::Vector3D b   = points[1];
        Terathon::Vector3D c   = points[2];
        Terathon::Vector3D d   = points[3];
        Terathon::Vector3D ab  = b - a;
        Terathon::Vector3D ac  = c - a;
        Terathon::Vector3D ad  = d - a;
        Terathon::Vector3D ao  = -a;
        Terathon::Vector3D abc = Terathon::Cross(ab, ac);
        Terathon::Vector3D acd = Terathon::Cross(ac, ad);
        Terathon::Vector3D adb = Terathon::Cross(ad, ab);

        if (isSameDirection(abc, ao)) {
            set({points[0], points[1], points[2]});
            return triangleCase(dir);
        }
        if (isSameDirection(acd, ao)) {
            set({points[0], points[2], points[3]});
            return triangleCase(dir);
        }
        if (isSameDirection(adb, ao)) {
            set({points[0], points[3], points[1]});
            return triangleCase(dir);
        }

        return true;
    }

    bool nextSimplex(Terathon::Vector3D& dir) {
        switch (size) {
            case 2:
                return lineCase(dir);
            case 3:
                return triangleCase(dir);
            case 4:
                return tetrahedronCase(dir);
            default:
                return false;
        }
    }
};

template <typename ShapeA, typename ShapeB>
bool gjk(const ShapeA& shapeA, const ShapeB& shapeB, Simplex& simplex) {
    Terathon::Vector3D dir = shapeB.centroid() - shapeA.centroid();
    if (Terathon::Dot(dir, dir) <= 0.0f) {
        dir = {1.0f, 0.0f, 0.0f};
    }

    auto               supportA = shapeA.support(dir);
    auto               supportB = shapeB.support(-dir);
    Terathon::Vector3D sv       = minkowskiSupport(shapeA, shapeB, dir);
    simplex.pushFront(sv, supportA, supportB);
    dir = -simplex.points[0];

    for (int i = 0; i < maxGjkIterations; ++i) {
        supportA                        = shapeA.support(dir);
        supportB                        = shapeB.support(-dir);
        Terathon::Vector3D supportPoint = minkowskiSupport(shapeA, shapeB, dir);

        if (Terathon::Dot(supportPoint, dir) <= 0.0f) {
            return false;
        }

        simplex.pushFront(supportPoint, supportA, supportB);
        if (simplex.nextSimplex(dir)) {
            return true;
        }
    }
    return false;
}

} // namespace vgeo::internal::cpu
