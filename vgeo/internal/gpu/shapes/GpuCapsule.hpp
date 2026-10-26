#include "vgeo/Vec3.hpp"

namespace vgeo::internal::gpu{

struct GpuCapsule{

    alignas(16) Vec3 m_a;
    alignas(16) Vec3 m_b;
    float m_radius;
};

}