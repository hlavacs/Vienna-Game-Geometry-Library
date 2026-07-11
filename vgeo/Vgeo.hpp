#pragma once

#include "vgeo/CollisionResult.hpp"            // IWYU pragma: export
#include "vgeo/CollisionSystem.hpp"            // IWYU pragma: export
#include "vgeo/GeometryHandle.hpp"             // IWYU pragma: export
#include "vgeo/InstanceHandle.hpp"             // IWYU pragma: export
#include "vgeo/Mat4.hpp"                       // IWYU pragma: export
#include "vgeo/Quat.hpp"                       // IWYU pragma: export
#include "vgeo/RayResult.hpp"                  // IWYU pragma: export
#include "vgeo/Vec3.hpp"                       // IWYU pragma: export
#include "vgeo/internal/cpu/Aabb.hpp"          // IWYU pragma: export
#include "vgeo/internal/cpu/BruteForce.hpp"    // IWYU pragma: export
#include "vgeo/internal/cpu/Bvh.hpp"           // IWYU pragma: export
#include "vgeo/internal/cpu/IndexedGrid.hpp"   // IWYU pragma: export
#include "vgeo/internal/cpu/SweepAndPrune.hpp" // IWYU pragma: export

namespace vgeo {

using Aabb = internal::cpu::Aabb;

template <typename Bv = Aabb>
using BruteForce = internal::cpu::BruteForce<Bv>;

template <typename Bv = Aabb>
using Bvh = internal::cpu::Bvh<Bv>;

using SweepAndPrune = internal::cpu::SweepAndPrune;

using IndexedGrid = internal::cpu::IndexedGrid;

} // namespace vgeo
