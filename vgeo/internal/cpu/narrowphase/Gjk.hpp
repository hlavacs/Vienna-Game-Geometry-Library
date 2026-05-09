#pragma once

// GJK collision detection
// https://winter.dev/articles/gjk-algorithm/

#include <TSVector3D.h>

#include <algorithm>
#include <array>
#include <initializer_list>

namespace vgeo::internal::cpu {

inline constexpr int maxGjkIterations = 64;

inline bool isSameDirection(const Terathon::Vector3D& a, const Terathon::Vector3D& b) {
    return Terathon::Dot(a, b) > 0.0f;
}

struct Simplex {
    std::array<Terathon::Vector3D, 4> points;
    int size = 0;

    void pushFront(Terathon::Vector3D p) {
        points[3] = points[2];
        points[2] = points[1];
        points[1] = points[0];
        points[0] = p;
        size = std::min(size + 1, 4);
    }

    void set(std::initializer_list<Terathon::Vector3D> list) {
        size = 0;
        for (const auto& p : list) {
            points[size++] = p;
        }
    }

    bool lineCase(Terathon::Vector3D& dir) {
        Terathon::Vector3D a = points[0];
        Terathon::Vector3D b = points[1];
        Terathon::Vector3D ab = b - a;
        Terathon::Vector3D ao = -a;

        if (isSameDirection(ab, ao)) {
            dir = Terathon::Cross(Terathon::Cross(ab, ao), ab);
        } else {
            set({a});
            dir = ao;
        }
        return false;
    }

    bool triangleCase(Terathon::Vector3D& dir) {
        Terathon::Vector3D a = points[0];
        Terathon::Vector3D b = points[1];
        Terathon::Vector3D c = points[2];
        Terathon::Vector3D ab = b - a;
        Terathon::Vector3D ac = c - a;
        Terathon::Vector3D ao = -a;
        Terathon::Vector3D abc = Terathon::Cross(ab, ac);

        if (isSameDirection(Terathon::Cross(abc, ac), ao)) {
            if (isSameDirection(ac, ao)) {
                set({a, c});
                dir = Terathon::Cross(Terathon::Cross(ac, ao), ac);
            } else {
                set({a, b});
                return lineCase(dir);
            }
        } else if (isSameDirection(Terathon::Cross(ab, abc), ao)) {
            set({a, b});
            return lineCase(dir);
        } else if (isSameDirection(abc, ao)) {
            dir = abc;
        } else {
            // flip winding so normal points toward origin
            set({a, c, b});
            dir = -abc;
        }

        return false;
    }

    bool tetrahedronCase(Terathon::Vector3D& dir) {
        Terathon::Vector3D a = points[0];
        Terathon::Vector3D b = points[1];
        Terathon::Vector3D c = points[2];
        Terathon::Vector3D d = points[3];
        Terathon::Vector3D ab = b - a;
        Terathon::Vector3D ac = c - a;
        Terathon::Vector3D ad = d - a;
        Terathon::Vector3D ao = -a;
        Terathon::Vector3D abc = Terathon::Cross(ab, ac);
        Terathon::Vector3D acd = Terathon::Cross(ac, ad);
        Terathon::Vector3D adb = Terathon::Cross(ad, ab);

        if (isSameDirection(abc, ao)) {
            set({a, b, c});
            return triangleCase(dir);
        }
        if (isSameDirection(acd, ao)) {
            set({a, c, d});
            return triangleCase(dir);
        }
        if (isSameDirection(adb, ao)) {
            set({a, d, b});
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
    auto minkowskiSupport = [&](Terathon::Vector3D dir) { return shapeA.support(dir) - shapeB.support(-dir); };

    Terathon::Vector3D dir = shapeB.centroid() - shapeA.centroid();
    if (Terathon::Dot(dir, dir) <= 0.0f) {
        dir = {1.0f, 0.0f, 0.0f};
    }

    simplex.pushFront(minkowskiSupport(dir));
    dir = -simplex.points[0];

    for (int i = 0; i < maxGjkIterations; ++i) {
        Terathon::Vector3D supportPoint = minkowskiSupport(dir);

        if (Terathon::Dot(supportPoint, dir) <= 0.0f) {
            return false;
        }

        simplex.pushFront(supportPoint);
        if (simplex.nextSimplex(dir)) {
            return true;
        }
    }
    return false;
}

} // namespace vgeo::internal::cpu
