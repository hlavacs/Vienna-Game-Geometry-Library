#pragma once

#include <TSConformal3D.h>
#include "vgeo/internal/cpu/shapes/Sphere.hpp"

namespace vgeo::internal::gpu{

struct GpuSphere {
    uint64_t id;
    Terathon::Sphere3D m_sphere;
};


}