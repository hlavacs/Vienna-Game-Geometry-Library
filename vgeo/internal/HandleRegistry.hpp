#pragma once

#include "vgeo/Handle.hpp"

#include <cassert>
#include <vector>

namespace vgeo::internal {

template <ShapeType Type> class HandleRegistry {
public:
    Handle allocate() {
        if (!m_freeList.empty()) {
            uint32_t index = m_freeList.back();
            m_freeList.pop_back();
            uint32_t id =
                index | (static_cast<uint32_t>(m_generations[index]) << 20) | (static_cast<uint32_t>(Type) << 28);
            return Handle{id};
        }
        uint32_t index = static_cast<uint32_t>(m_generations.size());
        m_generations.push_back(1);
        uint32_t id = index | (static_cast<uint32_t>(m_generations[index]) << 20) | (static_cast<uint32_t>(Type) << 28);
        return Handle{id};
    }

    void free(Handle handle) {
        assert(isValid(handle));
        uint32_t index = handle.getIndex();
        ++m_generations[index];
        if (m_generations[index] != 0) {
            m_freeList.push_back(index);
        }
    }

    bool isValid(Handle handle) const {
        uint32_t index = handle.getIndex();
        if (index >= m_generations.size()) {
            return false;
        }
        return m_generations[index] == handle.getGeneration();
    }

private:
    std::vector<uint8_t> m_generations;
    std::vector<uint32_t> m_freeList;
};

} // namespace vgeo::internal
