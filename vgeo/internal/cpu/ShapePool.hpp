#pragma once

#include "vgeo/GeometryHandle.hpp"
#include "vgeo/ShapeType.hpp"
#include "vgeo/internal/GeometryRegistry.hpp"

#include <vector>

namespace vgeo::internal::cpu {

template <typename Shape, ShapeType Type>
struct ShapePool {
    GeometryHandle add(Shape shape) {
        GeometryHandle handle = registry.allocate();
        uint32_t       index  = handle.getIndex();
        if (index >= shapes.size()) {
            shapes.resize(index + 1);
        }
        shapes[index] = std::move(shape);
        return handle;
    }

    void remove(GeometryHandle handle) {
        registry.free(handle);
        shapes[handle.getIndex()] = Shape{};
    }

    bool isValid(GeometryHandle handle) const {
        return registry.isValid(handle);
    }

    const Shape& operator[](GeometryHandle handle) const {
        return shapes[handle.getIndex()];
    }

    internal::GeometryRegistry<Type> registry;
    std::vector<Shape>               shapes;
};

} // namespace vgeo::internal::cpu
