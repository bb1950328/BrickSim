#include "mesh_simple_classes.h"
#include <cstring>

namespace bricksim::mesh {
    bool MeshInstance::operator==(const MeshInstance& other) const {
        return std::memcmp(this, &other, sizeof(*this)) == 0;
        //return transformation == other.transformation && color.get() == other.color.get() && elementId == other.elementId && selected == other.selected && layer == other.layer && scene == other.scene;
    }

    bool MeshInstance::operator!=(const MeshInstance& other) const {
        return std::memcmp(this, &other, sizeof(*this)) != 0;
        //return transformation != other.transformation || color.get() != other.color.get() || elementId != other.elementId || selected != other.selected || layer != other.layer || scene != other.scene;
    }

    bool TriangleVertex::operator==(const TriangleVertex& other) const {
        //return position == other.position && normal == other.normal;
        return std::memcmp(this, &other, sizeof(TriangleVertex)) == 0;
    }

    bool TexturedTriangleVertex::operator==(const TexturedTriangleVertex& other) const {
        return std::memcmp(this, &other, sizeof(*this)) == 0;
    }

    bool LineVertex::operator==(const LineVertex& other) const {
        //return position == other.position && color == other.color;
        return std::memcmp(this, &other, sizeof(LineVertex)) == 0;
    }
    void TriangleInstance::setColor(const ldr::ColorReference color) {
        const auto colorLocked = color.get();
        diffuseColor = colorLocked->value.asGlmVector();
        shininess = 32.0f;
        //useful tool: http://www.cs.toronto.edu/~jacobson/phong-demo/
        switch (colorLocked->finish) {
            case ldr::Color::METAL:
            case ldr::Color::CHROME:
            case ldr::Color::PEARLESCENT:
                //todo find out what's the difference
                shininess *= 2;
                ambientFactor = 1;
                specularBrightness = 1;
                break;
            case ldr::Color::MATTE_METALLIC:
                ambientFactor = 0.6;
                specularBrightness = 0.2;
                break;
            case ldr::Color::RUBBER:
                ambientFactor = 0.75;
                specularBrightness = 0;
                break;
            case ldr::Color::PURE:
                ambientFactor = 1;
                specularBrightness = 0;
                break;
            default:
                ambientFactor = 0.5;
                specularBrightness = 0.5;
                break;
        }
    }
}