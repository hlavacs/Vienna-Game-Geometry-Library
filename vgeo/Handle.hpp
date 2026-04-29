#pragma once

#include "vgeo/ShapeType.hpp"

#include <cstdint>

namespace vgeo::internal {
template <ShapeType>
class HandleRegistry;
}

namespace vgeo {

// Handle encodes three fields into a single uint32_t:
//
// bits  0-19 : index       20b - max 1,048,576
// bits 20-27 : generation   8b - starts at 1, wraps to 0 = retired
// bits 28-31 : type         4b - ShapeType enum value
class Handle {
public:
    Handle() = delete;

    uint32_t getIndex() const noexcept {
        return m_id & 0x000F'FFFFu;
    }

    uint8_t getGeneration() const noexcept {
        return (m_id >> 20) & 0xFFu;
    }

    ShapeType getType() const noexcept {
        return static_cast<ShapeType>((m_id >> 28) & 0xFu);
    }

    uint32_t getId() const noexcept {
        return m_id;
    }

    bool operator==(const Handle&) const noexcept = default;

private:
    template <ShapeType Type>
    friend class internal::HandleRegistry;

    explicit Handle(uint32_t id) noexcept : m_id(id) {}

    uint32_t m_id;
};

} // namespace vgeo
