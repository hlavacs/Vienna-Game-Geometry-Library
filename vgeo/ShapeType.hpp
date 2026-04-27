#include <cstdint>

namespace vgeo {

enum class ShapeType : uint32_t {
    Sphere = 0,
    Capsule = 1,
    AaBox = 2,
    ConvexHull = 3,
};

} // namespace vgeo
