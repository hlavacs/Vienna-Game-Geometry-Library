#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/Real.hpp"
#include "vgeo/Shape.hpp"
#include "vgeo/internal/CandidatePair.hpp"
#include "vgeo/internal/cpu/Aabb.hpp"
#include "vgeo/internal/cpu/BroadPhase.hpp"
#include "vgeo/internal/cpu/Cell.hpp"

#include <TSVector3D.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace vgeo::internal::cpu {

class IndexedGrid {
public:
    IndexedGrid(const real cellSize) : m_cellSize{cellSize}, m_boundsMin{INT_MAX}, m_boundsMax{INT_MIN} {}

    void add(InstanceHandle handle, Shape shape) {
        const Aabb aabb = std::visit([](const auto& shape) { return shape.template computeBv<Aabb>(); }, shape);

        const int cellMinX = static_cast<int>(std::floor(aabb.getMin().x / m_cellSize));
        const int cellMaxX = static_cast<int>(std::floor(aabb.getMax().x / m_cellSize));
        const int cellMinY = static_cast<int>(std::floor(aabb.getMin().y / m_cellSize));
        const int cellMaxY = static_cast<int>(std::floor(aabb.getMax().y / m_cellSize));
        const int cellMinZ = static_cast<int>(std::floor(aabb.getMin().z / m_cellSize));
        const int cellMaxZ = static_cast<int>(std::floor(aabb.getMax().z / m_cellSize));

        m_boundsMin.x = std::min(m_boundsMin.x, cellMinX);
        m_boundsMin.y = std::min(m_boundsMin.y, cellMinY);
        m_boundsMin.z = std::min(m_boundsMin.z, cellMinZ);
        m_boundsMax.x = std::max(m_boundsMax.x, cellMaxX);
        m_boundsMax.y = std::max(m_boundsMax.y, cellMaxY);
        m_boundsMax.z = std::max(m_boundsMax.z, cellMaxZ);

        std::vector<Cell>& handleCells = m_handleCells[handle];
        for (int x = cellMinX; x <= cellMaxX; ++x) {
            for (int y = cellMinY; y <= cellMaxY; ++y) {
                for (int z = cellMinZ; z <= cellMaxZ; ++z) {
                    m_grid[Cell{x, y, z}].push_back(handle);
                    handleCells.emplace_back(x, y, z);
                }
            }
        }
    }

    void update(InstanceHandle handle, Shape shape) {
        const Aabb aabb = std::visit([](const auto& shape) { return shape.template computeBv<Aabb>(); }, shape);

        const int cellMinX = static_cast<int>(std::floor(aabb.getMin().x / m_cellSize));
        const int cellMaxX = static_cast<int>(std::floor(aabb.getMax().x / m_cellSize));
        const int cellMinY = static_cast<int>(std::floor(aabb.getMin().y / m_cellSize));
        const int cellMaxY = static_cast<int>(std::floor(aabb.getMax().y / m_cellSize));
        const int cellMinZ = static_cast<int>(std::floor(aabb.getMin().z / m_cellSize));
        const int cellMaxZ = static_cast<int>(std::floor(aabb.getMax().z / m_cellSize));

        auto found = m_handleCells.find(handle);
        if (found != m_handleCells.end() &&
            occupiesBox(found->second, cellMinX, cellMaxX, cellMinY, cellMaxY, cellMinZ, cellMaxZ)) {
            return; // shape still in same cells
        }

        remove(handle);
        add(handle, shape);
    }

    void remove(InstanceHandle handle) {
        const std::vector<Cell> occupied = m_handleCells.at(handle);
        m_handleCells.erase(handle);

        for (const Cell& cell : occupied) {
            std::erase(m_grid[cell], handle);
        }
    }

    std::vector<CandidatePair> findCandidates() const {
        std::unordered_set<CandidatePair> pairs;

        for (const auto& [cell, handles] : m_grid) {
            for (size_t i = 0; i < handles.size(); ++i) {
                for (size_t j = i + 1; j < handles.size(); ++j) {
                    pairs.emplace(handles[i], handles[j]);
                }
            }
        }

        return std::vector<CandidatePair>(pairs.begin(), pairs.end());
    }

    std::vector<InstanceHandle> castRay(Terathon::Point3D origin, Terathon::Vector3D dir) const {
        std::vector<InstanceHandle>        hits;
        std::unordered_set<InstanceHandle> seen;
        if (m_grid.empty()) {
            return hits;
        }

        // Clip the ray to it is inside the whole grid
        const real gridMinX = m_boundsMin.x * m_cellSize;
        const real gridMinY = m_boundsMin.y * m_cellSize;
        const real gridMinZ = m_boundsMin.z * m_cellSize;
        const real gridMaxX = (m_boundsMax.x + 1) * m_cellSize;
        const real gridMaxY = (m_boundsMax.y + 1) * m_cellSize;
        const real gridMaxZ = (m_boundsMax.z + 1) * m_cellSize;

        real tEntry = 0.0;
        real tExit  = std::numeric_limits<real>::max();

        for (int axis = 0; axis < 3; ++axis) {
            const real originAxis = axis == 0 ? origin.x : axis == 1 ? origin.y : origin.z;
            const real dirAxis    = axis == 0 ? dir.x : axis == 1 ? dir.y : dir.z;
            const real slabMin    = axis == 0 ? gridMinX : axis == 1 ? gridMinY : gridMinZ;
            const real slabMax    = axis == 0 ? gridMaxX : axis == 1 ? gridMaxY : gridMaxZ;

            if (std::abs(dirAxis) < 1e-6) {
                if (originAxis < slabMin || originAxis > slabMax) {
                    return hits;
                }
                continue;
            }

            real t0 = (slabMin - originAxis) / dirAxis;
            real t1 = (slabMax - originAxis) / dirAxis;

            if (t0 > t1) {
                std::swap(t0, t1);
            }

            tEntry = std::max(tEntry, t0);
            tExit  = std::min(tExit, t1);

            if (tEntry > tExit) {
                return hits;
            }
        }

        const bool isOriginOutsideGrid = origin.x < gridMinX || origin.x >= gridMaxX || origin.y < gridMinY ||
                                         origin.y >= gridMaxY || origin.z < gridMinZ || origin.z >= gridMaxZ;

        if (isOriginOutsideGrid) {
            constexpr real eps = 1e-6;
            origin.x += dir.x * (tEntry + eps);
            origin.y += dir.y * (tEntry + eps);
            origin.z += dir.z * (tEntry + eps);
        }

        const int originCellX = static_cast<int>(std::floor(origin.x / m_cellSize));
        const int originCellY = static_cast<int>(std::floor(origin.y / m_cellSize));
        const int originCellZ = static_cast<int>(std::floor(origin.z / m_cellSize));

        const int stepX = dir.x > 0 ? 1 : dir.x < 0 ? -1 : 0;
        const int stepY = dir.y > 0 ? 1 : dir.y < 0 ? -1 : 0;
        const int stepZ = dir.z > 0 ? 1 : dir.z < 0 ? -1 : 0;

        const real tDeltaX = std::abs(dir.x) > 1e-6 ? std::abs(m_cellSize / dir.x) : std::numeric_limits<real>::max();
        const real tDeltaY = std::abs(dir.y) > 1e-6 ? std::abs(m_cellSize / dir.y) : std::numeric_limits<real>::max();
        const real tDeltaZ = std::abs(dir.z) > 1e-6 ? std::abs(m_cellSize / dir.z) : std::numeric_limits<real>::max();

        real tMaxX = std::abs(dir.x) > 1e-6 ? ((originCellX + (stepX > 0 ? 1 : 0)) * m_cellSize - origin.x) / dir.x
                                            : std::numeric_limits<real>::max();
        real tMaxY = std::abs(dir.y) > 1e-6 ? ((originCellY + (stepY > 0 ? 1 : 0)) * m_cellSize - origin.y) / dir.y
                                            : std::numeric_limits<real>::max();
        real tMaxZ = std::abs(dir.z) > 1e-6 ? ((originCellZ + (stepZ > 0 ? 1 : 0)) * m_cellSize - origin.z) / dir.z
                                            : std::numeric_limits<real>::max();

        int currentX = originCellX;
        int currentY = originCellY;
        int currentZ = originCellZ;

        while (currentX >= m_boundsMin.x && currentX <= m_boundsMax.x && currentY >= m_boundsMin.y &&
               currentY <= m_boundsMax.y && currentZ >= m_boundsMin.z && currentZ <= m_boundsMax.z) {
            // add handles of current cell
            if (m_grid.contains({currentX, currentY, currentZ})) {
                for (const InstanceHandle& handle : m_grid.at({currentX, currentY, currentZ})) {
                    if (!seen.contains(handle)) {
                        seen.insert(handle);
                        hits.push_back(handle);
                    }
                }
            }

            // step into next cell
            if (tMaxX < tMaxY && tMaxX < tMaxZ) {
                currentX += stepX;
                tMaxX += tDeltaX;
            } else if (tMaxY < tMaxZ) {
                currentY += stepY;
                tMaxY += tDeltaY;
            } else {
                currentZ += stepZ;
                tMaxZ += tDeltaZ;
            }
        }

        return hits;
    }

private:
    const real m_cellSize;
    Cell       m_boundsMin;
    Cell       m_boundsMax;

    std::unordered_map<Cell, std::vector<InstanceHandle>> m_grid;        // which handles are in a cell
    std::unordered_map<InstanceHandle, std::vector<Cell>> m_handleCells; // which cells does a handle occupy

    static bool
    occupiesBox(const std::vector<Cell>& cells, int minX, int maxX, int minY, int maxY, int minZ, int maxZ) {
        Cell lo = cells.front();
        Cell hi = lo;
        for (const Cell& cell : cells) {
            lo.x = std::min(lo.x, cell.x);
            hi.x = std::max(hi.x, cell.x);
            lo.y = std::min(lo.y, cell.y);
            hi.y = std::max(hi.y, cell.y);
            lo.z = std::min(lo.z, cell.z);
            hi.z = std::max(hi.z, cell.z);
        }
        return lo.x == minX && hi.x == maxX && lo.y == minY && hi.y == maxY && lo.z == minZ && hi.z == maxZ;
    }
};

static_assert(BroadPhase<IndexedGrid>);

} // namespace vgeo::internal::cpu
