#pragma once

#include "vgeo/RayResult.hpp"
#include "vgeo/internal/cpu/BoundingVolume.hpp"

#include <TSMotor3D.h>
#include <TSVector3D.h>

#include <concepts>
#include <optional>

namespace vgeo::internal::cpu {

template <typename S, typename Bv>
concept CollisionShape = BoundingVolume<Bv> && requires(const S                  shape,
                                                        InstanceHandle           handle,
                                                        Terathon::Vector3D       dir,
                                                        Terathon::Point3D        origin,
                                                        const Terathon::Motor3D& motor,
                                                        real                     scale) {
    { shape.template computeBv<Bv>() } -> std::same_as<Bv>;
    { shape.centroid() } -> std::same_as<Terathon::Point3D>;
    { shape.support(dir) } -> std::same_as<Terathon::Point3D>;
    { shape.intersectRay(handle, origin, dir) } -> std::same_as<std::optional<RayHit>>;
    { shape.applyTransform(motor, scale) } -> std::same_as<S>;
};

} // namespace vgeo::internal::cpu
