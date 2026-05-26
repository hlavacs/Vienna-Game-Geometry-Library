#pragma once

#include "vgeo/Handle.hpp"
#include "vgeo/ShapeType.hpp"
#include "vgeo/internal/HandleRegistry.hpp"

#include <vector>

namespace vgeo::internal::cpu {

template <typename Shape, ShapeType Type>
struct ShapePool {
    Handle add(Shape shape) {
        Handle   h     = registry.allocate();
        uint32_t index = h.getIndex();
        if (index >= shapes.size()) {
            shapes.resize(index + 1);
        }
        shapes[index] = std::move(shape);
        return h;
    }

    void remove(Handle h) {
        registry.free(h);
        shapes[h.getIndex()] = Shape{};
    }

    bool isValid(Handle h) const {
        return registry.isValid(h);
    }

    const Shape& operator[](Handle h) const {
        return shapes[h.getIndex()];
    }

    HandleRegistry<Type> registry;
    std::vector<Shape>   shapes;
};

} // namespace vgeo::internal::cpu
