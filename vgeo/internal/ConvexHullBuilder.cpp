#include "vgeo/internal/ConvexHullBuilder.hpp"

#include "vgeo/Point3D.hpp"
#include "vgeo/internal/cpu/ConvexHull.hpp"

#include <TSRigid3D.h>
#include <TSVector3D.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

constexpr uint32_t invalid = std::numeric_limits<uint32_t>::max();

struct HalfEdge {
    uint32_t origin;
    uint32_t next;
    uint32_t twin;
    uint32_t face;
};

struct Face {
    uint32_t halfEdge;
    Terathon::Plane3D plane;
    std::vector<uint32_t> outsideSet;
    bool onHull = true;
};

struct HullMesh {
    std::vector<Terathon::Point3D> vertices;
    std::vector<HalfEdge> halfEdges;
    std::vector<Face> faces;
};

std::vector<Terathon::Point3D> convertPoints(std::span<const vgeo::Point3D> inputPoints) {
    std::vector<Terathon::Point3D> points;
    points.reserve(inputPoints.size());
    for (const vgeo::Point3D& p : inputPoints) {
        points.emplace_back(p.x, p.y, p.z);
    }
    return points;
}

// epsilon = 3 * (max|x| + max|y| + max|z|) * FLT_EPSILON  (Dirk Gregorius, Implementing QuickHull, GDC 2014)
float computeEpsilon(std::span<const Terathon::Point3D> points) {
    float maxX = 0.0f, maxY = 0.0f, maxZ = 0.0f;
    for (const auto& p : points) {
        maxX = std::max(maxX, std::abs(p.x));
        maxY = std::max(maxY, std::abs(p.y));
        maxZ = std::max(maxZ, std::abs(p.z));
    }
    return 3.0f * (maxX + maxY + maxZ) * std::numeric_limits<float>::epsilon();
}

std::array<uint32_t, 6> findExtremePoints(std::span<const Terathon::Point3D> points) {
    std::array<uint32_t, 6> extremePoints;
    extremePoints.fill(0);
    for (uint32_t i = 1; i < points.size(); ++i) {
        if (points[i].x < points[extremePoints[0]].x) {
            extremePoints[0] = i;
        }
        if (points[i].x > points[extremePoints[1]].x) {
            extremePoints[1] = i;
        }
        if (points[i].y < points[extremePoints[2]].y) {
            extremePoints[2] = i;
        }
        if (points[i].y > points[extremePoints[3]].y) {
            extremePoints[3] = i;
        }
        if (points[i].z < points[extremePoints[4]].z) {
            extremePoints[4] = i;
        }
        if (points[i].z > points[extremePoints[5]].z) {
            extremePoints[5] = i;
        }
    }
    return extremePoints;
}

std::pair<uint32_t, uint32_t> findMostDistantPair(std::span<const Terathon::Point3D> points) {
    const std::array<uint32_t, 6> extremePoints = findExtremePoints(points);
    uint32_t a = invalid, b = invalid;
    float maxDistSq = 0.0f;
    for (uint32_t i = 0; i < 6; ++i) {
        for (uint32_t j = i + 1; j < 6; ++j) {
            const float distSq =
                Terathon::SquaredWeightNorm(Terathon::Wedge(points[extremePoints[i]], points[extremePoints[j]]));
            if (distSq > maxDistSq) {
                maxDistSq = distSq;
                a = extremePoints[i];
                b = extremePoints[j];
            }
        }
    }
    return {a, b};
}

uint32_t findFurthestFromLine(std::span<const Terathon::Point3D> points, uint32_t a, uint32_t b) {
    Terathon::Line3D line =
        Terathon::Unitize(Terathon::Wedge(Terathon::Point3D{points[a].x, points[a].y, points[a].z},
                                          Terathon::Point3D{points[b].x, points[b].y, points[b].z}));
    uint32_t c = invalid;
    float maxDistSq = 0.0f;
    for (uint32_t i = 0; i < points.size(); ++i) {
        if (i == a || i == b) {
            continue;
        }
        Terathon::Vector3D perp =
            Terathon::Cross(line.v, Terathon::Point3D{points[i].x, points[i].y, points[i].z}) + line.m.xyz;
        float distSq = Terathon::SquaredMag(perp);
        if (distSq > maxDistSq) {
            maxDistSq = distSq;
            c = i;
        }
    }
    return c;
}

uint32_t findFurthestFromPlane(std::span<const Terathon::Point3D> points, uint32_t a, uint32_t b, uint32_t c) {
    Terathon::Plane3D plane = Terathon::Unitize(Terathon::Plane3D{points[a], points[b], points[c]});
    uint32_t d = invalid;
    float maxDistSq = 0.0f;
    for (uint32_t i = 0; i < points.size(); ++i) {
        if (i == a || i == b || i == c) {
            continue;
        }
        float dist = Terathon::Antiwedge(points[i], plane);
        if (dist * dist > maxDistSq) {
            maxDistSq = dist * dist;
            d = i;
        }
    }
    return d;
}

HullMesh buildInitialTetrahedron(std::span<const Terathon::Point3D> points) {
    auto [a, b] = findMostDistantPair(points);
    uint32_t c = findFurthestFromLine(points, a, b);
    uint32_t d = findFurthestFromPlane(points, a, b, c);

    if (Terathon::Antiwedge(points[d], Terathon::Unitize(Terathon::Plane3D{points[a], points[b], points[c]})) > 0.0f) {
        std::swap(b, c);
    }

    HullMesh mesh;
    mesh.vertices = {points[a], points[b], points[c], points[d]};

    mesh.halfEdges = {
        {0, 1, 5, 0},  // HE0:  0->1  face 0
        {1, 2, 11, 0}, // HE1:  1->2  face 0
        {2, 0, 6, 0},  // HE2:  2->0  face 0
        {0, 4, 8, 1},  // HE3:  0->3  face 1
        {3, 5, 9, 1},  // HE4:  3->1  face 1
        {1, 3, 0, 1},  // HE5:  1->0  face 1
        {0, 7, 2, 2},  // HE6:  0->2  face 2
        {2, 8, 10, 2}, // HE7:  2->3  face 2
        {3, 6, 3, 2},  // HE8:  3->0  face 2
        {1, 10, 4, 3}, // HE9:  1->3  face 3
        {3, 11, 7, 3}, // HE10: 3->2  face 3
        {2, 9, 1, 3},  // HE11: 2->1  face 3
    };

    mesh.faces.reserve(4);
    for (uint32_t f = 0; f < 4; ++f) {
        uint32_t he = f * 3;
        uint32_t v0 = mesh.halfEdges[he].origin;
        uint32_t v1 = mesh.halfEdges[mesh.halfEdges[he].next].origin;
        uint32_t v2 = mesh.halfEdges[mesh.halfEdges[mesh.halfEdges[he].next].next].origin;
        mesh.faces.emplace_back(
            he, Terathon::Unitize(Terathon::Plane3D{mesh.vertices[v0], mesh.vertices[v1], mesh.vertices[v2]}));
    }

    return mesh;
}

void assignOutsideSets(HullMesh& mesh, std::span<const Terathon::Point3D> points, float epsilon) {
    for (uint32_t i = 0; i < points.size(); ++i) {
        float maxDist = epsilon;
        uint32_t bestFace = invalid;

        for (uint32_t f = 0; f < mesh.faces.size(); ++f) {
            float dist = Terathon::Antiwedge(points[i], mesh.faces[f].plane);
            if (dist > maxDist) {
                maxDist = dist;
                bestFace = f;
            }
        }

        if (bestFace != invalid) {
            mesh.faces[bestFace].outsideSet.emplace_back(i);
        }
    }
}

uint32_t findActiveFace(const HullMesh& mesh) {
    for (uint32_t f = 0; f < mesh.faces.size(); ++f) {
        if (mesh.faces[f].onHull && !mesh.faces[f].outsideSet.empty()) {
            return f;
        }
    }
    return invalid;
}

uint32_t findFurthestInSet(const Face& face, std::span<const Terathon::Point3D> points) {
    uint32_t eyeIndex = invalid;
    float maxDist = 0.0f;
    for (uint32_t index : face.outsideSet) {
        float dist = Terathon::Antiwedge(points[index], face.plane);
        if (dist > maxDist) {
            maxDist = dist;
            eyeIndex = index;
        }
    }
    return eyeIndex;
}

struct HorizonResult {
    std::vector<uint32_t> visibleFaces;
    std::vector<uint32_t> horizon;
};

HorizonResult findHorizon(const HullMesh& mesh, uint32_t seedFace, const Terathon::Point3D& eye, float epsilon) {
    HorizonResult result;
    std::vector<bool> visited(mesh.faces.size(), false);
    std::vector<uint32_t> stack = {seedFace};
    visited[seedFace] = true;

    while (!stack.empty()) {
        uint32_t f = stack.back();
        stack.pop_back();
        result.visibleFaces.emplace_back(f);

        uint32_t he = mesh.faces[f].halfEdge;
        for (int i = 0; i < 3; ++i) {
            uint32_t twinHe = mesh.halfEdges[he].twin;
            uint32_t twinFace = mesh.halfEdges[twinHe].face;

            if (!visited[twinFace]) {
                visited[twinFace] = true;
                if (Terathon::Antiwedge(eye, mesh.faces[twinFace].plane) > epsilon) {
                    stack.emplace_back(twinFace);
                } else {
                    result.horizon.emplace_back(twinHe);
                }
            }

            he = mesh.halfEdges[he].next;
        }
    }

    return result;
}

uint32_t addFace(HullMesh& mesh, uint32_t v0, uint32_t v1, uint32_t v2) {
    uint32_t faceIndex = static_cast<uint32_t>(mesh.faces.size());
    uint32_t heBase = static_cast<uint32_t>(mesh.halfEdges.size());

    mesh.halfEdges.emplace_back(v0, heBase + 1, invalid, faceIndex);
    mesh.halfEdges.emplace_back(v1, heBase + 2, invalid, faceIndex);
    mesh.halfEdges.emplace_back(v2, heBase + 0, invalid, faceIndex);

    Terathon::Plane3D plane =
        Terathon::Unitize(Terathon::Plane3D{mesh.vertices[v0], mesh.vertices[v1], mesh.vertices[v2]});
    mesh.faces.emplace_back(heBase, plane);

    return faceIndex;
}

std::vector<uint32_t>
expandHull(HullMesh& mesh, const std::vector<uint32_t>& horizon, const Terathon::Point3D& eyePoint) {
    uint32_t eyeVertexIndex = static_cast<uint32_t>(mesh.vertices.size());
    mesh.vertices.emplace_back(eyePoint);

    uint32_t firstNewHe = static_cast<uint32_t>(mesh.halfEdges.size());
    std::vector<uint32_t> newFaces;
    newFaces.reserve(horizon.size());

    for (uint32_t h : horizon) {
        uint32_t u = mesh.halfEdges[h].origin;
        uint32_t v = mesh.halfEdges[mesh.halfEdges[h].next].origin;
        uint32_t heBase = static_cast<uint32_t>(mesh.halfEdges.size());

        newFaces.emplace_back(addFace(mesh, v, u, eyeVertexIndex));

        // base edge (v->u) twins immediately with the horizon edge (u->v)
        mesh.halfEdges[heBase].twin = h;
        mesh.halfEdges[h].twin = heBase;
    }

    uint32_t lastNewHe = static_cast<uint32_t>(mesh.halfEdges.size());
    std::unordered_map<uint64_t, uint32_t> edgeMap;
    edgeMap.reserve(lastNewHe - firstNewHe);

    for (uint32_t he = firstNewHe; he < lastNewHe; ++he) {
        if (mesh.halfEdges[he].twin != invalid) {
            continue;
        }
        uint32_t u = mesh.halfEdges[he].origin;
        uint32_t v = mesh.halfEdges[mesh.halfEdges[he].next].origin;
        edgeMap[static_cast<uint64_t>(u) << 32 | v] = he; // pack two uint32s into one uint64 key
    }

    for (uint32_t he = firstNewHe; he < lastNewHe; ++he) {
        if (mesh.halfEdges[he].twin != invalid) {
            continue;
        }
        uint32_t u = mesh.halfEdges[he].origin;
        uint32_t v = mesh.halfEdges[mesh.halfEdges[he].next].origin;
        mesh.halfEdges[he].twin = edgeMap[static_cast<uint64_t>(v) << 32 | u];
    }

    return newFaces;
}

void redistributePoints(HullMesh& mesh,
                        const std::vector<uint32_t>& visibleFaces,
                        const std::vector<uint32_t>& newFaces,
                        std::span<const Terathon::Point3D> points,
                        float epsilon) {
    std::vector<uint32_t> orphaned;
    for (uint32_t f : visibleFaces) {
        orphaned.insert(orphaned.end(), mesh.faces[f].outsideSet.begin(), mesh.faces[f].outsideSet.end());
        mesh.faces[f].outsideSet.clear();
        mesh.faces[f].onHull = false;
    }

    for (uint32_t index : orphaned) {
        float maxDist = epsilon;
        uint32_t bestFace = invalid;
        for (uint32_t f : newFaces) {
            float dist = Terathon::Antiwedge(points[index], mesh.faces[f].plane);
            if (dist > maxDist) {
                maxDist = dist;
                bestFace = f;
            }
        }
        if (bestFace != invalid) {
            mesh.faces[bestFace].outsideSet.emplace_back(index);
        }
    }
}

HullMesh computeHull(std::span<const vgeo::Point3D> inputPoints) {
    std::vector<Terathon::Point3D> points = convertPoints(inputPoints);
    float epsilon = computeEpsilon(points);
    HullMesh mesh = buildInitialTetrahedron(points);
    assignOutsideSets(mesh, points, epsilon);

    for (uint32_t face = findActiveFace(mesh); face != invalid; face = findActiveFace(mesh)) {
        uint32_t eye = findFurthestInSet(mesh.faces[face], points);
        auto [visibleFaces, horizon] = findHorizon(mesh, face, points[eye], epsilon);
        std::vector<uint32_t> newFaces = expandHull(mesh, horizon, points[eye]);
        redistributePoints(mesh, visibleFaces, newFaces, points, epsilon);
    }

    return mesh;
}

}

namespace vgeo::internal {

cpu::ConvexHull ConvexHullBuilder::buildCpu(std::span<const vgeo::Point3D> inputPoints) {
    assert(inputPoints.size() >= 4);

    HullMesh mesh = computeHull(inputPoints);

    cpu::ConvexHull hull;
    for (const Terathon::Point3D& v : mesh.vertices) {
        hull.m_vertices.emplace_back(v.x, v.y, v.z);
    }
    for (const Face& f : mesh.faces) {
        if (!f.onHull) {
            continue;
        }
        uint32_t he = f.halfEdge;
        hull.m_indices.emplace_back(mesh.halfEdges[he].origin);
        he = mesh.halfEdges[he].next;
        hull.m_indices.emplace_back(mesh.halfEdges[he].origin);
        he = mesh.halfEdges[he].next;
        hull.m_indices.emplace_back(mesh.halfEdges[he].origin);
    }
    return hull;
}

cpu::ConvexHull ConvexHullBuilder::buildGpu(std::span<const vgeo::Point3D> inputPoints) {
    // TODO: convert to GPU sided convex hull
}

} // namespace vgeo::internal
