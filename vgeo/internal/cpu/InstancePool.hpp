#pragma once

#include "vgeo/InstanceHandle.hpp"
#include "vgeo/internal/InstanceRegistry.hpp"

#include <optional>
#include <vector>

namespace vgeo::internal::cpu {

template <typename T>
struct InstancePool {
    InstanceHandle add(T instance) {
        InstanceHandle handle = registry.allocate();
        uint32_t       index  = handle.getIndex();
        if (index >= instances.size()) {
            instances.resize(index + 1);
        }
        instances[index] = std::move(instance);
        return handle;
    }

    void remove(InstanceHandle handle) {
        registry.free(handle);
        instances[handle.getIndex()] = std::nullopt;
    }

    bool isValid(InstanceHandle handle) const {
        return registry.isValid(handle);
    }

    T& operator[](InstanceHandle handle) {
        return *instances[handle.getIndex()];
    }

    const T& operator[](InstanceHandle handle) const {
        return *instances[handle.getIndex()];
    }

    internal::InstanceRegistry    registry;
    std::vector<std::optional<T>> instances;
};

} // namespace vgeo::internal::cpu
