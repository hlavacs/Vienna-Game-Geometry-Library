#pragma once

#include "vgeo/Vec3.hpp"
#include "vgeo/internal/ConvexHullData.hpp"

#include <span>

namespace vgeo::internal {

class ConvexHullBuilder {
public:
    static ConvexHullData build(std::span<const Vec3> points);
    ConvexHullBuilder() = delete;
};

} // namespace vgeo::internal
