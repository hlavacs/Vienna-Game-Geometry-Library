#pragma once

#include "vgeo/ShapeType.hpp"

#include <cstdint>

namespace vgeo::internal {
template <ShapeType>
class HandleRegistry;
}

namespace vgeo {

// Handle encodes three fields into a single uint64_t:
//
// bits  0-31 : index       32b - max 4,294,967,296
// bits 32-59 : generation  28b - max   268,435,456
// bits 60-63 : type         4b - max            16
class Handle {
public:
    Handle() = delete;

    uint32_t getIndex() const noexcept {
        return static_cast<uint32_t>(m_id & 0xFFFF'FFFFu);
    }

    uint32_t getGeneration() const noexcept {
        return static_cast<uint32_t>((m_id >> 32) & 0x0FFF'FFFFu);
    }

    ShapeType getType() const noexcept {
        return static_cast<ShapeType>((m_id >> 60) & 0x0Fu);
    }

    uint64_t getId() const noexcept {
        return m_id;
    }

    bool operator==(const Handle&) const noexcept = default;

private:
    template <ShapeType Type>
    friend class internal::HandleRegistry;

    explicit Handle(uint64_t id) noexcept : m_id(id) {}

    uint64_t m_id;
};

} // namespace vgeo
