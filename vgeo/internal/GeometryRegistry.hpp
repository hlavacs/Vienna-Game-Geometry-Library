#pragma once

#include "vgeo/GeometryHandle.hpp"
#include "vgeo/ShapeType.hpp"

#include <cassert>
#include <vector>

namespace vgeo::internal {

template <ShapeType Type>
class GeometryRegistry {
public:
    GeometryHandle allocate() {
        if (!m_freeList.empty()) {
            uint32_t index = m_freeList.back();
            m_freeList.pop_back();
            uint64_t id = static_cast<uint64_t>(index) | (static_cast<uint64_t>(m_generations[index]) << 32) |
                          (static_cast<uint64_t>(Type) << 60);
            return GeometryHandle{id};
        }
        uint32_t index = static_cast<uint32_t>(m_generations.size());
        m_generations.push_back(1);
        uint64_t id = static_cast<uint64_t>(index) | (static_cast<uint64_t>(m_generations[index]) << 32) |
                      (static_cast<uint64_t>(Type) << 60);
        return GeometryHandle{id};
    }

    void free(GeometryHandle handle) {
        assert(isValid(handle));
        uint32_t index = handle.getIndex();
        ++m_generations[index];
        m_freeList.push_back(index);
    }

    bool isValid(GeometryHandle handle) const {
        uint32_t index = handle.getIndex();
        if (index >= m_generations.size()) {
            return false;
        }
        return m_generations[index] == handle.getGeneration();
    }

private:
    std::vector<uint32_t> m_generations;
    std::vector<uint32_t> m_freeList;
};

} // namespace vgeo::internal
