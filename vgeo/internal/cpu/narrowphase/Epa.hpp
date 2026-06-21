#pragma once

// EPA collision response
// https://winter.dev/articles/epa-algorithm/

#include "vgeo/CollisionResults.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/internal/PolytopeUtils.hpp"
#include "vgeo/internal/cpu/narrowphase/Gjk.hpp"

#include <TSRigid3D.h>
#include <TSVector3D.h>

#include <algorithm>
#include <cmath>
#include <optional>
#include <span>
#include <tuple>
#include <vector>

namespace vgeo::internal::cpu {

inline constexpr int    maxEpaIterations = 64;
inline constexpr real   epaTolerance     = 1e-6f;
inline constexpr size_t maxEpaFaces      = 2048;

struct EpaFace {
    uint32_t           a;
    uint32_t           b;
    uint32_t           c;
    Terathon::Vector3D normal;
    real               distanceToOrigin;
};

inline std::optional<EpaFace>
makeFace(std::span<const Terathon::Point3D> vertices, uint32_t a, uint32_t b, uint32_t c) {
    Terathon::Vector3D ab     = vertices[b] - vertices[a];
    Terathon::Vector3D ac     = vertices[c] - vertices[a];
    Terathon::Vector3D normal = Terathon::Cross(ab, ac);

    if (Terathon::SquaredMag(normal) <= 0.0f) {
        return std::nullopt;
    }

    normal        = Terathon::Normalize(normal);
    real distance = Terathon::Dot(normal, vertices[a]);

    if (distance < 0.0f) {
        std::swap(b, c);
        normal   = -normal;
        distance = -distance;
    }

    return EpaFace{a, b, c, normal, distance};
}

inline std::optional<EpaFace> makeFace(const std::vector<Terathon::Point3D>& v, uint32_t a, uint32_t b, uint32_t c) {
    return makeFace(std::span<const Terathon::Point3D>(v.data(), v.size()), a, b, c);
}

inline void addIfUniqueEdge(std::vector<std::pair<uint32_t, uint32_t>>& edges, uint32_t a, uint32_t b) {
    auto reverse = std::find(edges.begin(), edges.end(), std::make_pair(b, a));
    if (reverse != edges.end()) {
        edges.erase(reverse);
    } else {
        edges.emplace_back(a, b);
    }
}

struct EpaPolytope {
    std::vector<Terathon::Point3D> vertices;
    std::vector<Terathon::Point3D> supportA;
    std::vector<Terathon::Point3D> supportB;
    std::vector<EpaFace>           faces;
    real                           tolerance;

    static std::optional<EpaPolytope> create(const Simplex& simplex) {
        EpaPolytope p;
        p.vertices.reserve(simplex.size);
        p.supportA.reserve(simplex.size);
        p.supportB.reserve(simplex.size);

        for (int i = 0; i < simplex.size; ++i) {
            p.vertices.emplace_back(simplex.points[i].x, simplex.points[i].y, simplex.points[i].z);
            p.supportA.push_back(simplex.supportPointsA[i]);
            p.supportB.push_back(simplex.supportPointsB[i]);
        }

        p.tolerance = std::max(epaTolerance, calculateRelativeEpsilon(p.vertices));

        for (auto [a, b, c] :
             {std::tuple{0u, 1u, 2u}, std::tuple{0u, 3u, 1u}, std::tuple{0u, 2u, 3u}, std::tuple{1u, 3u, 2u}}) {
            std::optional<EpaFace> face = makeFace(p.vertices, a, b, c);
            if (!face) {
                return std::nullopt;
            }
            p.faces.push_back(*face);
        }

        return p;
    }

    std::optional<size_t> closestFaceIndex() const {
        if (faces.empty()) {
            return std::nullopt;
        }

        size_t best = 0;
        for (size_t i = 1; i < faces.size(); ++i) {
            if (faces[i].distanceToOrigin < faces[best].distanceToOrigin) {
                best = i;
            }
        }

        return best;
    }

    bool
    expand(const Terathon::Vector3D& supportPoint, const Terathon::Point3D& pointA, const Terathon::Point3D& pointB) {
        std::vector<EpaFace>                       visibleFaces;
        std::vector<std::pair<uint32_t, uint32_t>> boundary;

        for (const EpaFace& face : faces) {
            Terathon::Plane3D plane =
                Terathon::Unitize(Terathon::Plane3D{vertices[face.a], vertices[face.b], vertices[face.c]});
            real signedDist =
                Terathon::Antiwedge(Terathon::Point3D{supportPoint.x, supportPoint.y, supportPoint.z}, plane);
            if (signedDist > tolerance) {
                visibleFaces.push_back(face);
                addIfUniqueEdge(boundary, face.a, face.b);
                addIfUniqueEdge(boundary, face.b, face.c);
                addIfUniqueEdge(boundary, face.c, face.a);
            }
        }

        if (boundary.empty()) {
            return false;
        }

        std::vector<Terathon::Point3D> trialVertices = vertices;
        trialVertices.emplace_back(supportPoint.x, supportPoint.y, supportPoint.z);

        supportA.push_back(pointA);
        supportB.push_back(pointB);

        std::vector<EpaFace> newFaces;
        newFaces.reserve(boundary.size());
        for (auto [ea, eb] : boundary) {
            std::optional<EpaFace> face =
                makeFace(trialVertices, ea, eb, static_cast<uint32_t>(trialVertices.size() - 1));
            if (!face) {
                return false;
            }
            newFaces.push_back(*face);
        }

        vertices = std::move(trialVertices);

        std::vector<EpaFace> remainingFaces;
        remainingFaces.reserve(faces.size() - visibleFaces.size() + newFaces.size());
        for (const EpaFace& face : faces) {
            const bool isVisible = std::any_of(visibleFaces.begin(), visibleFaces.end(), [&](const EpaFace& visible) {
                return visible.a == face.a && visible.b == face.b && visible.c == face.c;
            });
            if (!isVisible) {
                remainingFaces.push_back(face);
            }
        }

        remainingFaces.insert(remainingFaces.end(), newFaces.begin(), newFaces.end());
        faces = std::move(remainingFaces);

        return true;
    }
};

inline std::tuple<real, real, real>
closestPointBarycentrics(const Terathon::Point3D& a, const Terathon::Point3D& b, const Terathon::Point3D& c) {
    Terathon::Vector3D ab = b - a;
    Terathon::Vector3D ac = c - a;
    Terathon::Vector3D ao = -a;

    real ab_ab = Terathon::Dot(ab, ab);
    real ab_ac = Terathon::Dot(ab, ac);
    real ac_ac = Terathon::Dot(ac, ac);
    real ao_ab = Terathon::Dot(ao, ab);
    real ao_ac = Terathon::Dot(ao, ac);

    real denominator = ab_ab * ac_ac - ab_ac * ab_ac;
    if (std::abs(denominator) < 1e-6f) {
        return {1.0f, 0.0f, 0.0f};
    }

    real v = (ac_ac * ao_ab - ab_ac * ao_ac) / denominator;
    real w = (ab_ab * ao_ac - ab_ac * ao_ab) / denominator;
    real u = 1.0f - v - w;

    if (u < 0.0f) {
        u = 0.0f;
    }
    if (v < 0.0f) {
        v = 0.0f;
    }
    if (w < 0.0f) {
        w = 0.0f;
    }

    real sum = u + v + w;
    if (sum > 0.0f) {
        u /= sum;
        v /= sum;
        w /= sum;
    } else {
        u = 1.0f;
        v = 0.0f;
        w = 0.0f;
    }

    return {u, v, w};
}

inline Contact buildCollisionContactFromFace(const EpaPolytope& polytope, const EpaFace& face, real depth) {
    const Terathon::Point3D& v0_mink = polytope.vertices[face.a];
    const Terathon::Point3D& v1_mink = polytope.vertices[face.b];
    const Terathon::Point3D& v2_mink = polytope.vertices[face.c];

    auto [u, v, w] = closestPointBarycentrics(v0_mink, v1_mink, v2_mink);

    const Terathon::Point3D& a0 = polytope.supportA[face.a];
    const Terathon::Point3D& a1 = polytope.supportA[face.b];
    const Terathon::Point3D& a2 = polytope.supportA[face.c];

    const Terathon::Point3D& b0 = polytope.supportB[face.a];
    const Terathon::Point3D& b1 = polytope.supportB[face.b];
    const Terathon::Point3D& b2 = polytope.supportB[face.c];

    Terathon::Point3D wA{
        u * a0.x + v * a1.x + w * a2.x, u * a0.y + v * a1.y + w * a2.y, u * a0.z + v * a1.z + w * a2.z};

    Terathon::Point3D wB{
        u * b0.x + v * b1.x + w * b2.x, u * b0.y + v * b1.y + w * b2.y, u * b0.z + v * b1.z + w * b2.z};

    Contact contact{};
    contact.normal   = {-face.normal.x, -face.normal.y, -face.normal.z};
    contact.depth    = depth;
    contact.witnessA = {wA.x, wA.y, wA.z};
    contact.witnessB = {wB.x, wB.y, wB.z};
    return contact;
}

inline std::optional<Contact> bestEffortContact(const EpaPolytope& polytope) {
    auto faceIdx = polytope.closestFaceIndex();
    if (!faceIdx) {
        return std::nullopt;
    }
    const EpaFace& closest = polytope.faces[*faceIdx];
    return buildCollisionContactFromFace(polytope, closest, closest.distanceToOrigin);
}

template <typename ShapeA, typename ShapeB>
std::optional<Contact> epa(const ShapeA& shapeA, const ShapeB& shapeB, const Simplex& simplex) {
    auto polytope = EpaPolytope::create(simplex);
    if (!polytope) {
        return std::nullopt;
    }

    for (int i = 0; i < maxEpaIterations; ++i) {
        if (polytope->faces.size() > maxEpaFaces) {
            return bestEffortContact(*polytope);
        }

        auto faceIdx = polytope->closestFaceIndex();
        if (!faceIdx) {
            return std::nullopt;
        }

        const EpaFace&     closest  = polytope->faces[*faceIdx];
        Terathon::Vector3D normal   = closest.normal;
        real               faceDist = closest.distanceToOrigin;

        auto supportA = shapeA.support(normal);
        auto supportB = shapeB.support(-normal);

        Terathon::Vector3D minkowskiDiff{supportA.x - supportB.x, supportA.y - supportB.y, supportA.z - supportB.z};
        real               supportDist = Terathon::Dot(normal, minkowskiDiff);

        if (std::abs(supportDist - faceDist) <= polytope->tolerance) {
            Contact contact = buildCollisionContactFromFace(*polytope, closest, faceDist);
            return contact;
        }

        if (!polytope->expand(minkowskiDiff, supportA, supportB)) {
            return bestEffortContact(*polytope);
        }
    }

    return bestEffortContact(*polytope);
}

} // namespace vgeo::internal::cpu
