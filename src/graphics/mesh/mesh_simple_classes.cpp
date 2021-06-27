#include <cstring>
#include "mesh_simple_classes.h"

namespace bricksim::mesh {
    bool MeshInstance::operator==(const MeshInstance &other) const {
        return std::memcmp(this, &other, sizeof(*this)) == 0;
        //return transformation == other.transformation && color.get() == other.color.get() && elementId == other.elementId && selected == other.selected && layer == other.layer && scene == other.scene;
    }

    bool MeshInstance::operator!=(const MeshInstance &other) const {
        return std::memcmp(this, &other, sizeof(*this)) != 0;
        //return transformation != other.transformation || color.get() != other.color.get() || elementId != other.elementId || selected != other.selected || layer != other.layer || scene != other.scene;
    }

    bool TriangleVertex::operator==(const TriangleVertex &other) const {
        //return position == other.position && normal == other.normal;
        return std::memcmp(this, &other, sizeof(TriangleVertex)) == 0;
    }

    bool TexturedTriangleVertex::operator==(const TexturedTriangleVertex &other) const {
        return std::memcmp(this, &other, sizeof(*this)) == 0;
    }

    bool LineVertex::operator==(const LineVertex &other) const {
        //return position == other.position && color == other.color;
        return std::memcmp(this, &other, sizeof(LineVertex)) == 0;
    }
}