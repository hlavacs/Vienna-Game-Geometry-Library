#include "vgeo/Vec3.hpp"

namespace vgeo::internal::gpu{

struct GpuContact{
    bool hasContact;
    real depth;
    alignas(16) Vec3 normal;
    alignas(16) Vec3 witnessA;
    alignas(16) Vec3 witnessB;
};

}