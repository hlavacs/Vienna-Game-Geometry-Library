#pragma once

#include <cstdint>
#include <functional>

namespace vgeo::internal {

class InstanceRegistry;

} // namespace vgeo::internal

namespace vgeo {

// InstanceHandle identifies a shape instance.
//
// bits  0-31 : index
// bits 32-63 : generation
class InstanceHandle {
public:
    InstanceHandle() = delete;

    uint32_t getIndex() const noexcept {
        return static_cast<uint32_t>(m_id & 0xFFFF'FFFFu);
    }

    uint32_t getGeneration() const noexcept {
        return static_cast<uint32_t>(m_id >> 32);
    }

    uint64_t getId() const noexcept {
        return m_id;
    }

    bool operator==(const InstanceHandle&) const noexcept = default;

private:
    friend class internal::InstanceRegistry;

    explicit InstanceHandle(uint64_t id) noexcept : m_id(id) {}

    uint64_t m_id;
};

} // namespace vgeo

namespace std {

template <>
struct hash<vgeo::InstanceHandle> {
    size_t operator()(const vgeo::InstanceHandle& h) const noexcept {
        return hash<uint64_t>{}(h.getId());
    }
};

} // namespace std
