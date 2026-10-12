#include "vgeo/CollisionResult.hpp"
#include "vgeo/CollisionSystem.hpp"
#include "vgeo/GeometryHandle.hpp"
#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Quat.hpp"
#include "vgeo/RayResult.hpp"
#include "vgeo/Vec3.hpp"
#include "vgeo/Vgeo.hpp"
#include "vgeo/internal/ConvexHullBuilder.hpp"
#include "vgeo/internal/ConvexHullData.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <span>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

constexpr Color clearColor{DARKGRAY};
constexpr Color gridColor{LIGHTGRAY};
constexpr Color baseColor{GRAY};
constexpr Color collisionColor{MAROON};
constexpr Color witnessColor{LIGHTGRAY};
constexpr Color normalColor{LIGHTGRAY};
constexpr Color crosshairColor{WHITE};
constexpr Color selectionColor{GOLD};
constexpr Color rayColor{ORANGE};

constexpr float pointRadius      = 0.05;
constexpr float translationSpeed = 2.0;
constexpr float rotationSpeed    = 1.0;
constexpr float scalingSpeedUp   = 1.2;
constexpr float scalingSpeedDown = 0.8;
constexpr float rayLength        = 50.0;
constexpr float arrowheadLength  = 0.1;

static Vector3 fromVgeo(vgeo::Vec3 v) {
    return {v.x, v.z, -v.y};
}

static vgeo::Vec3 toVgeo(Vector3 v) {
    return {v.x, -v.z, v.y};
}

static vgeo::Quat quatFromAxisAngle(vgeo::Vec3 axis, float angle) {
    const float halfSin = std::sin(angle * 0.5f);
    return {axis.x * halfSin, axis.y * halfSin, axis.z * halfSin, std::cos(angle * 0.5f)};
}

static vgeo::Quat quatMultiply(vgeo::Quat a, vgeo::Quat b) {
    return {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
    };
}

static Matrix toRaylib(const vgeo::Mat4& matrix) {
    Matrix result{};
    result.m0  = static_cast<float>(matrix.m[0][0]);
    result.m1  = static_cast<float>(matrix.m[0][2]);
    result.m2  = static_cast<float>(-matrix.m[0][1]);
    result.m3  = static_cast<float>(matrix.m[0][3]);
    result.m4  = static_cast<float>(matrix.m[2][0]);
    result.m5  = static_cast<float>(matrix.m[2][2]);
    result.m6  = static_cast<float>(-matrix.m[2][1]);
    result.m7  = static_cast<float>(matrix.m[2][3]);
    result.m8  = static_cast<float>(-matrix.m[1][0]);
    result.m9  = static_cast<float>(-matrix.m[1][2]);
    result.m10 = static_cast<float>(matrix.m[1][1]);
    result.m11 = static_cast<float>(matrix.m[1][3]);
    result.m12 = static_cast<float>(matrix.m[3][0]);
    result.m13 = static_cast<float>(matrix.m[3][2]);
    result.m14 = static_cast<float>(-matrix.m[3][1]);
    result.m15 = static_cast<float>(matrix.m[3][3]);
    return result;
}

struct ObjData {
    std::vector<vgeo::Vec3> vertices;
    std::vector<uint32_t>   indices;
};

static ObjData loadObj(const char* filePath) {
    ObjData       data;
    std::ifstream file(filePath);
    std::string   line;
    while (std::getline(file, line)) {
        if (line.rfind("v ", 0) == 0) {
            std::istringstream ss(line.substr(2));
            Vector3            p{};
            ss >> p.x >> p.y >> p.z;
            data.vertices.push_back(toVgeo(p));
        } else if (line.rfind("f ", 0) == 0) {
            std::istringstream ss(line.substr(2));
            std::string        token;
            while (ss >> token) {
                size_t slashPos    = token.find('/');
                int    vertexIndex = std::stoi(token.substr(0, slashPos));
                data.indices.push_back(static_cast<uint32_t>(vertexIndex - 1));
            }
        }
    }
    return data;
}

struct AaBox {
    vgeo::Vec3 halfExtents;
};

static vgeo::GeometryHandle defineAaBox(vgeo::CollisionSystem<>&                         collisionSystem,
                                        std::unordered_map<vgeo::GeometryHandle, AaBox>& geometries,
                                        vgeo::Vec3                                       halfExtents) {
    vgeo::GeometryHandle geometry = collisionSystem.defineAaBox(halfExtents);
    geometries.emplace(geometry, AaBox{halfExtents});
    return geometry;
}

static vgeo::InstanceHandle addAaBox(vgeo::CollisionSystem<>&                               collisionSystem,
                                     const std::unordered_map<vgeo::GeometryHandle, AaBox>& geometries,
                                     std::unordered_map<vgeo::InstanceHandle, AaBox>&       boxes,
                                     vgeo::GeometryHandle                                   geometry,
                                     vgeo::Vec3                                             position) {
    vgeo::InstanceHandle instance = collisionSystem.add(geometry, position);
    boxes.emplace(instance, geometries.at(geometry));
    return instance;
}

struct Capsule {
    float halflength;
    float radius;
};

static vgeo::GeometryHandle defineCapsule(vgeo::CollisionSystem<>&                           collisionSystem,
                                          std::unordered_map<vgeo::GeometryHandle, Capsule>& geometries,
                                          float                                              halflength,
                                          float                                              radius) {
    vgeo::GeometryHandle geometry = collisionSystem.defineCapsule(halflength, radius);
    geometries.emplace(geometry, Capsule{halflength, radius});
    return geometry;
}

static vgeo::InstanceHandle addCapsule(vgeo::CollisionSystem<>&                                 collisionSystem,
                                       const std::unordered_map<vgeo::GeometryHandle, Capsule>& geometries,
                                       std::unordered_map<vgeo::InstanceHandle, Capsule>&       capsules,
                                       vgeo::GeometryHandle                                     geometry,
                                       vgeo::Vec3                                               position) {
    vgeo::InstanceHandle instance = collisionSystem.add(geometry, position);
    capsules.emplace(instance, geometries.at(geometry));
    return instance;
}

struct Polytope {
    Model model;
    Model originalModel;
};

static Model buildMeshModel(std::span<const vgeo::Vec3> vertices, std::span<const uint32_t> indices) {
    Mesh mesh          = {};
    mesh.vertexCount   = static_cast<int>(vertices.size());
    mesh.triangleCount = static_cast<int>(indices.size() / 3);
    mesh.vertices      = static_cast<float*>(MemAlloc(static_cast<unsigned int>(vertices.size() * 3 * sizeof(float))));
    mesh.indices =
        static_cast<unsigned short*>(MemAlloc(static_cast<unsigned int>(indices.size() * sizeof(unsigned short))));

    for (size_t i = 0; i < vertices.size(); ++i) {
        Vector3 v                = fromVgeo(vertices[i]);
        mesh.vertices[i * 3 + 0] = v.x;
        mesh.vertices[i * 3 + 1] = v.y;
        mesh.vertices[i * 3 + 2] = v.z;
    }
    for (size_t i = 0; i < indices.size(); ++i) {
        mesh.indices[i] = static_cast<unsigned short>(indices[i]);
    }

    UploadMesh(&mesh, false);
    return LoadModelFromMesh(mesh);
}

static vgeo::GeometryHandle definePolytope(vgeo::CollisionSystem<>&                            collisionSystem,
                                           std::unordered_map<vgeo::GeometryHandle, Polytope>& geometries,
                                           std::span<const vgeo::Vec3>                         points,
                                           std::span<const uint32_t>                           originalIndices) {
    vgeo::GeometryHandle geometry = collisionSystem.definePolytope(points);

    vgeo::internal::ConvexHullData hull = vgeo::internal::ConvexHullBuilder::build(points);

    Model hullModel     = buildMeshModel(hull.vertices, hull.indices);
    Model originalModel = buildMeshModel(points, originalIndices);

    geometries.emplace(geometry, Polytope{hullModel, originalModel});
    return geometry;
}

static vgeo::InstanceHandle addPolytope(vgeo::CollisionSystem<>&                                  collisionSystem,
                                        const std::unordered_map<vgeo::GeometryHandle, Polytope>& geometries,
                                        std::unordered_map<vgeo::InstanceHandle, Polytope>&       polytopes,
                                        vgeo::GeometryHandle                                      geometry,
                                        vgeo::Vec3                                                position) {
    vgeo::InstanceHandle instance = collisionSystem.add(geometry, position);
    polytopes.emplace(instance, geometries.at(geometry));
    return instance;
}

struct Sphere {
    float radius;
};

static vgeo::GeometryHandle defineSphere(vgeo::CollisionSystem<>&                          collisionSystem,
                                         std::unordered_map<vgeo::GeometryHandle, Sphere>& geometries,
                                         float                                             radius) {
    vgeo::GeometryHandle geometry = collisionSystem.defineSphere(radius);
    geometries.emplace(geometry, Sphere{radius});
    return geometry;
}

static vgeo::InstanceHandle addSphere(vgeo::CollisionSystem<>&                                collisionSystem,
                                      const std::unordered_map<vgeo::GeometryHandle, Sphere>& geometries,
                                      std::unordered_map<vgeo::InstanceHandle, Sphere>&       spheres,
                                      vgeo::GeometryHandle                                    geometry,
                                      vgeo::Vec3                                              position) {
    vgeo::InstanceHandle instance = collisionSystem.add(geometry, position);
    spheres.emplace(instance, geometries.at(geometry));
    return instance;
}

enum class TransformType {
    Translation,
    Rotation,
    Scaling,
};

static const char* toString(TransformType type) {
    switch (type) {
        case TransformType::Translation:
            return "Translation";
        case TransformType::Rotation:
            return "Rotation";
        case TransformType::Scaling:
            return "Scaling";
    }
    return "Unknown";
}

static void handleTransformInput(vgeo::CollisionSystem<>& collisionSystem,
                                 vgeo::InstanceHandle     selectedShape,
                                 TransformType            transformType) {
    switch (transformType) {
        case TransformType::Translation: {
            if (IsKeyDown(KEY_L)) {
                vgeo::Vec3 position = collisionSystem.getPosition(selectedShape);
                position.x += translationSpeed * GetFrameTime();
                collisionSystem.setPosition(selectedShape, position);
            }
            if (IsKeyDown(KEY_J)) {
                vgeo::Vec3 position = collisionSystem.getPosition(selectedShape);
                position.x -= translationSpeed * GetFrameTime();
                collisionSystem.setPosition(selectedShape, position);
            }
            if (IsKeyDown(KEY_I)) {
                vgeo::Vec3 position = collisionSystem.getPosition(selectedShape);
                position.y += translationSpeed * GetFrameTime();
                collisionSystem.setPosition(selectedShape, position);
            }
            if (IsKeyDown(KEY_K)) {
                vgeo::Vec3 position = collisionSystem.getPosition(selectedShape);
                position.y -= translationSpeed * GetFrameTime();
                collisionSystem.setPosition(selectedShape, position);
            }
            if (IsKeyDown(KEY_O)) {
                vgeo::Vec3 position = collisionSystem.getPosition(selectedShape);
                position.z += translationSpeed * GetFrameTime();
                collisionSystem.setPosition(selectedShape, position);
            }
            if (IsKeyDown(KEY_U)) {
                vgeo::Vec3 position = collisionSystem.getPosition(selectedShape);
                position.z -= translationSpeed * GetFrameTime();
                collisionSystem.setPosition(selectedShape, position);
            }
            break;
        }
        case TransformType::Rotation: {
            if (IsKeyDown(KEY_L)) {
                vgeo::Quat rotation = collisionSystem.getRotation(selectedShape);
                vgeo::Quat delta    = quatFromAxisAngle({0, 1, 0}, rotationSpeed * GetFrameTime());
                collisionSystem.setRotation(selectedShape, quatMultiply(delta, rotation));
            }
            if (IsKeyDown(KEY_J)) {
                vgeo::Quat rotation = collisionSystem.getRotation(selectedShape);
                vgeo::Quat delta    = quatFromAxisAngle({0, 1, 0}, -rotationSpeed * GetFrameTime());
                collisionSystem.setRotation(selectedShape, quatMultiply(delta, rotation));
            }
            if (IsKeyDown(KEY_I)) {
                vgeo::Quat rotation = collisionSystem.getRotation(selectedShape);
                vgeo::Quat delta    = quatFromAxisAngle({1, 0, 0}, -rotationSpeed * GetFrameTime());
                collisionSystem.setRotation(selectedShape, quatMultiply(delta, rotation));
            }
            if (IsKeyDown(KEY_K)) {
                vgeo::Quat rotation = collisionSystem.getRotation(selectedShape);
                vgeo::Quat delta    = quatFromAxisAngle({1, 0, 0}, rotationSpeed * GetFrameTime());
                collisionSystem.setRotation(selectedShape, quatMultiply(delta, rotation));
            }
            if (IsKeyDown(KEY_O)) {
                vgeo::Quat rotation = collisionSystem.getRotation(selectedShape);
                vgeo::Quat delta    = quatFromAxisAngle({0, 0, 1}, -rotationSpeed * GetFrameTime());
                collisionSystem.setRotation(selectedShape, quatMultiply(delta, rotation));
            }
            if (IsKeyDown(KEY_U)) {
                vgeo::Quat rotation = collisionSystem.getRotation(selectedShape);
                vgeo::Quat delta    = quatFromAxisAngle({0, 0, 1}, +rotationSpeed * GetFrameTime());
                collisionSystem.setRotation(selectedShape, quatMultiply(delta, rotation));
            }
            break;
        }
        case TransformType::Scaling: {
            if (IsKeyDown(KEY_O)) {
                float scale = collisionSystem.getScale(selectedShape);
                scale *= std::pow(scalingSpeedUp, GetFrameTime());
                collisionSystem.setScale(selectedShape, scale);
            }
            if (IsKeyDown(KEY_U)) {
                float scale = collisionSystem.getScale(selectedShape);
                scale *= std::pow(scalingSpeedDown, GetFrameTime());
                collisionSystem.setScale(selectedShape, scale);
            }
            break;
        }
    }
}

static void drawGrid(int slices, float spacing, Color color) {
    int halfSlices = slices / 2;

    rlBegin(RL_LINES);
    rlColor4ub(color.r, color.g, color.b, color.a);
    for (int i = -halfSlices; i <= halfSlices; i++) {
        rlVertex3f(static_cast<float>(i) * spacing, 0.0f, static_cast<float>(-halfSlices) * spacing);
        rlVertex3f(static_cast<float>(i) * spacing, 0.0f, static_cast<float>(halfSlices) * spacing);

        rlVertex3f(static_cast<float>(-halfSlices) * spacing, 0.0f, static_cast<float>(i) * spacing);
        rlVertex3f(static_cast<float>(halfSlices) * spacing, 0.0f, static_cast<float>(i) * spacing);
    }
    rlEnd();
}

static bool checkCollision(const std::vector<vgeo::CollisionPair>& pairs, vgeo::InstanceHandle instance) {
    return std::any_of(pairs.begin(), pairs.end(), [&](const vgeo::CollisionPair& pair) {
        return pair.shapeA == instance || pair.shapeB == instance;
    });
}

static void drawNormalVector(Vector3 startPosition, Vector3 endPosition) {
    Vector3 delta  = startPosition - endPosition;
    float   length = Vector3Length(delta);

    Vector3 backDirection = delta / length;
    Vector3 arrowheadBase = endPosition + backDirection * std::min(arrowheadLength, length);

    DrawCylinderEx(startPosition, arrowheadBase, 0.01, 0.01, 16, normalColor);
    DrawCylinderEx(endPosition, arrowheadBase, 0.0, 0.05, 16, normalColor);
}

struct PointLabel {
    Vector2     screenPosition;
    std::string text;
    Color       color;
};

static std::string formatPoint(const char* prefix, Vector3 p) {
    return std::string(prefix) + TextFormat(" [%.2f, %.2f, %.2f]", p.x, p.y, p.z);
}

static void drawPointLabels(const std::vector<PointLabel>& labels) {
    constexpr int   fontSize = 20;
    constexpr int   padding  = 5;
    constexpr Color bgColor{0, 0, 0, 150};

    for (const PointLabel& label : labels) {
        int x = static_cast<int>(label.screenPosition.x) + 20;
        int y = static_cast<int>(label.screenPosition.y) - fontSize / 2 - 20;
        int w = MeasureText(label.text.c_str(), fontSize) + padding * 2;

        DrawRectangle(x - padding, y - padding, w, fontSize + padding * 2, bgColor);
        DrawText(label.text.c_str(), x, y, fontSize, label.color);
    }
}

int main() {
    const int screenWidth  = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "VGEO Visualizer");

    Camera camera     = {0};
    camera.position   = {0.0, 10.0, 10.0};
    camera.target     = {0.0, 0.0, 0.0};
    camera.up         = {0.0, 1.0, 0.0};
    camera.fovy       = 45.0;
    camera.projection = CAMERA_PERSPECTIVE;

    bool isWireframeMode = false;
    bool showConvexHull  = true;
    bool showPointLabels = true;
    bool showHud         = true;

    DisableCursor();

    SetTargetFPS(60);

    vgeo::CollisionSystem<> collisionSystem;

    std::unordered_map<vgeo::GeometryHandle, AaBox> aaBoxGeometries;
    std::unordered_map<vgeo::InstanceHandle, AaBox> aaBoxes;

    std::unordered_map<vgeo::GeometryHandle, Capsule> capsuleGeometries;
    std::unordered_map<vgeo::InstanceHandle, Capsule> capsules;

    std::unordered_map<vgeo::GeometryHandle, Polytope> polytopeGeometries;
    std::unordered_map<vgeo::InstanceHandle, Polytope> polytopes;

    std::unordered_map<vgeo::GeometryHandle, Sphere> sphereGeometries;
    std::unordered_map<vgeo::InstanceHandle, Sphere> spheres;

    std::optional<vgeo::InstanceHandle> selectedShape;
    std::optional<vgeo::RayResult>      rayResult;
    vgeo::CollisionResult               collisionResult;

    std::optional<Vector3> rayOrigin;
    std::optional<Vector3> rayDirection;
    TransformType          transformType{TransformType::Translation};

    vgeo::GeometryHandle aaBoxGeometry = defineAaBox(collisionSystem, aaBoxGeometries, {1.0, 1.0, 1.0});
    addAaBox(collisionSystem, aaBoxGeometries, aaBoxes, aaBoxGeometry, {-6.0, 0.0, 0.0});

    vgeo::GeometryHandle capsuleGeometry = defineCapsule(collisionSystem, capsuleGeometries, 2.0, 1.0);
    addCapsule(collisionSystem, capsuleGeometries, capsules, capsuleGeometry, {-2.0, 0.0, 0.0});

    ObjData              teapot = loadObj("resources/utah_teapot.obj");
    vgeo::GeometryHandle teapotGeometry =
        definePolytope(collisionSystem, polytopeGeometries, teapot.vertices, teapot.indices);
    vgeo::InstanceHandle teapotInstance =
        addPolytope(collisionSystem, polytopeGeometries, polytopes, teapotGeometry, {3.5, 0.0, -1.0});
    collisionSystem.setRotation(teapotInstance, quatFromAxisAngle({1.0, 0.0, 0.0}, -PI / 2));

    vgeo::GeometryHandle sphereGeometry = defineSphere(collisionSystem, sphereGeometries, 1.0);
    addSphere(collisionSystem, sphereGeometries, spheres, sphereGeometry, {8.0, 0.0, 0.0});

    bool wasWindowFocused = true;

    while (!WindowShouldClose()) {
        //----------------------------------------------------------------------------------------------------------------------
        // Update
        //----------------------------------------------------------------------------------------------------------------------
        collisionResult = collisionSystem.queryAll();
        if (rayOrigin && rayDirection) {
            rayResult = collisionSystem.castRay(toVgeo(*rayOrigin), toVgeo(*rayDirection));
        }

        bool isWindowFocused  = IsWindowFocused();
        bool hasRegainedFocus = isWindowFocused && !wasWindowFocused;
        wasWindowFocused      = isWindowFocused;

        if (isWindowFocused && !hasRegainedFocus) {
            UpdateCamera(&camera, CAMERA_FREE);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const vgeo::RayResult rayResult =
                collisionSystem.castRay(toVgeo(camera.position), toVgeo(camera.target - camera.position));
            if (rayResult.hits.empty() || selectedShape == rayResult.hits.front().shape) {
                selectedShape.reset();
            } else {
                selectedShape = rayResult.hits.front().shape;
            }
        }

        if (IsKeyPressed(KEY_G)) {
            isWireframeMode = !isWireframeMode;
        }

        if (IsKeyPressed(KEY_M)) {
            showConvexHull = !showConvexHull;
        }

        if (IsKeyPressed(KEY_T)) {
            showPointLabels = !showPointLabels;
        }

        if (IsKeyPressed(KEY_H)) {
            showHud = !showHud;
        }

        if (IsKeyPressed(KEY_F)) {
            switch (transformType) {
                case TransformType::Translation: {
                    transformType = TransformType::Rotation;
                    break;
                }
                case TransformType::Rotation: {
                    transformType = TransformType::Scaling;
                    break;
                }
                case TransformType::Scaling: {
                    transformType = TransformType::Translation;
                    break;
                }
            }
        }

        if (IsKeyPressed(KEY_R)) {
            rayOrigin    = camera.position;
            rayDirection = Vector3Normalize(camera.target - camera.position);
        }

        if (selectedShape) {
            handleTransformInput(collisionSystem, *selectedShape, transformType);
        }

        //----------------------------------------------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(clearColor);
        BeginMode3D(camera);

        rlSetLineWidth(1.0);
        drawGrid(50, 1.0, gridColor);
        rlDrawRenderBatchActive();

        rlSetLineWidth(2.0);
        DrawLine3D({-25.0, 0.0, 0.0}, {25.0, 0.0, 0.0}, MAROON);
        DrawLine3D({0.0, -25.0, 0.0}, {0.0, 25.0, 0.0}, DARKBLUE);
        DrawLine3D({0.0, 0.0, -25.0}, {0.0, 0.0, 25.0}, DARKGREEN);

        for (const auto& [instance, box] : aaBoxes) {
            vgeo::Vec3 position    = collisionSystem.getPosition(instance);
            float      scale       = collisionSystem.getScale(instance);
            bool       isColliding = checkCollision(collisionResult.pairs, instance);
            Color      color       = isColliding ? collisionColor : baseColor;

            Vector3 size = {static_cast<float>(box.halfExtents.x) * 2.0f * scale,
                            static_cast<float>(box.halfExtents.z) * 2.0f * scale,
                            static_cast<float>(box.halfExtents.y) * 2.0f * scale};

            Vector3 worldPosition = fromVgeo(position);

            if (isWireframeMode) {
                DrawCubeWiresV(worldPosition, size, color);
            } else {
                DrawCubeV(worldPosition, size, color);
            }
            if (selectedShape && instance == *selectedShape) {
                DrawCubeWiresV(worldPosition, size, selectionColor);
            }
        }

        for (const auto& [instance, capsule] : capsules) {
            Matrix transform   = toRaylib(collisionSystem.getTransform(instance));
            bool   isColliding = checkCollision(collisionResult.pairs, instance);
            Color  color       = isColliding ? collisionColor : baseColor;

            Vector3 localStart = {0.0, -capsule.halflength, 0.0};
            Vector3 localEnd   = {0.0, capsule.halflength, 0.0};

            rlPushMatrix();
            rlMultMatrixf(MatrixToFloat(transform));
            if (isWireframeMode) {
                DrawCapsuleWires(localStart, localEnd, capsule.radius, 16, 16, color);
            } else {
                DrawCapsule(localStart, localEnd, capsule.radius, 16, 16, color);
            }
            if (selectedShape && instance == *selectedShape) {
                DrawCapsuleWires(localStart, localEnd, capsule.radius, 16, 16, selectionColor);
            }
            rlPopMatrix();
        }

        for (const auto& [instance, polytope] : polytopes) {
            Matrix transform   = toRaylib(collisionSystem.getTransform(instance));
            bool   isColliding = checkCollision(collisionResult.pairs, instance);
            Color  color       = isColliding ? collisionColor : baseColor;

            const Model& activeModel = showConvexHull ? polytope.model : polytope.originalModel;

            rlPushMatrix();
            rlMultMatrixf(MatrixToFloat(transform));
            rlDisableBackfaceCulling();
            if (isWireframeMode) {
                DrawModelWires(activeModel, {0.0, 0.0, 0.0}, 1.0, color);
            } else {
                DrawModel(activeModel, {0.0, 0.0, 0.0}, 1.0, color);
            }
            if (selectedShape && instance == *selectedShape) {
                DrawModelWires(activeModel, {0.0, 0.0, 0.0}, 1.0, selectionColor);
            }
            rlEnableBackfaceCulling();
            rlPopMatrix();
        }

        for (const auto& [instance, sphere] : spheres) {
            Matrix transform   = toRaylib(collisionSystem.getTransform(instance));
            bool   isColliding = checkCollision(collisionResult.pairs, instance);
            Color  color       = isColliding ? collisionColor : baseColor;

            rlPushMatrix();
            rlMultMatrixf(MatrixToFloat(transform));
            if (isWireframeMode) {
                DrawSphereWires({0.0, 0.0, 0.0}, sphere.radius, 16, 16, color);
            } else {
                DrawSphereEx({0.0, 0.0, 0.0}, sphere.radius, 16, 16, color);
            }
            if (selectedShape && instance == *selectedShape) {
                DrawSphereWires({0.0, 0.0, 0.0}, sphere.radius, 16, 16, selectionColor);
            }
            rlPopMatrix();
        }

        // Collision result
        std::vector<PointLabel> pointLabels;
        for (const vgeo::CollisionPair& pair : collisionResult.pairs) {
            Vector3 witnessA = fromVgeo(pair.contact.witnessA);
            Vector3 witnessB = fromVgeo(pair.contact.witnessB);

            DrawSphere(witnessA, pointRadius, witnessColor);
            DrawSphere(witnessB, pointRadius, witnessColor);
            drawNormalVector(witnessA,
                             witnessA + (fromVgeo(pair.contact.normal) * pair.contact.depth) -
                                 (fromVgeo(pair.contact.normal) * pointRadius));

            pointLabels.push_back({GetWorldToScreen(witnessA, camera), formatPoint("A", witnessA), witnessColor});
            pointLabels.push_back({GetWorldToScreen(witnessB, camera), formatPoint("B", witnessB), witnessColor});
        }

        // Ray result
        if (rayResult) {
            DrawSphere(*rayOrigin, pointRadius, rayColor);
            DrawCylinderEx(*rayOrigin, *rayOrigin + *rayDirection * rayLength, 0.01, 0.01, 16, rayColor);
            for (const vgeo::RayHit& hit : rayResult->hits) {
                Vector3 hitPosition = fromVgeo(hit.position);

                DrawSphere(hitPosition, pointRadius, rayColor);
                drawNormalVector(hitPosition, hitPosition + fromVgeo(hit.normal));

                pointLabels.push_back(
                    {GetWorldToScreen(hitPosition, camera), formatPoint("Hit", hitPosition), rayColor});
            }
        }

        EndMode3D();

        if (showPointLabels) {
            drawPointLabels(pointLabels);
        }

        if (showHud) {
            std::string modeText  = std::string("Mode: ") + toString(transformType);
            int         fontSize  = 30;
            int         padding   = 10;
            int         textWidth = MeasureText(modeText.c_str(), fontSize);

            DrawRectangle(30 - padding, 30 - padding, textWidth + padding * 2, fontSize + padding * 2, {0, 0, 0, 150});
            DrawRectangleLines(30 - padding, 30 - padding, textWidth + padding * 2, fontSize + padding * 2, WHITE);
            DrawText(modeText.c_str(), 30, 30, fontSize, WHITE);

            DrawLineEx({static_cast<float>(screenWidth) / 2 - 10, static_cast<float>(screenHeight) / 2},
                       {static_cast<float>(screenWidth) / 2 + 10, static_cast<float>(screenHeight) / 2},
                       2.0,
                       crosshairColor);
            DrawLineEx({static_cast<float>(screenWidth) / 2, static_cast<float>(screenHeight) / 2 - 10},
                       {static_cast<float>(screenWidth) / 2, static_cast<float>(screenHeight) / 2 + 10},
                       2.0,
                       crosshairColor);
        }

        EndDrawing();
    }

    for (auto& [geometry, polytope] : polytopeGeometries) {
        UnloadModel(polytope.model);
        UnloadModel(polytope.originalModel);
    }

    CloseWindow();

    return 0;
}
