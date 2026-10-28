#pragma once

namespace vgeo::internal::gpu{
    
enum class IntersectionType : uint32_t {
    SphereSphere    = 0,
    CapsuleCapsule  = 1,
    CapsuleSphere = 2,
};

};