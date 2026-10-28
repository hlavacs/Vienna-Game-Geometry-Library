#pragma once

#include <TSVector3D.h>
#include "vgeo/Vec3.hpp"

namespace vgeo::internal::gpu{

struct GpuCapsule{
    uint64_t id;
    Terathon::Point3D m_a;
    Terathon::Point3D m_b;
    real m_radius;
};

}