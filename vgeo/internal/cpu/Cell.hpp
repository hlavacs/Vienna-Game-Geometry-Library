#pragma once

#include <functional>

namespace vgeo::internal::cpu {

struct Cell {
    int x;
    int y;
    int z;

    bool operator==(const Cell&) const noexcept = default;
};

} // namespace vgeo::internal::cpu

namespace std {

template <>
struct hash<vgeo::internal::cpu::Cell> {
    size_t operator()(const vgeo::internal::cpu::Cell& cell) const noexcept {
        // Spatial hash from Teschner et al. (2003), "Optimized Spatial Hashing
        // for Collision Detection of Deformable Objects", VMV 2003, Sec. 4.1
        return cell.x * 73856093 ^ cell.y * 19349663 ^ cell.z * 83492791;
    }
};

} // namespace std
