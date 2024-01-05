#include "mesh_simple_classes.h"
#include "../../helpers/geometry.h"

namespace bricksim::mesh {
    TriangleVertex::TriangleVertex(const glm::vec3& position, const glm::vec3& normal) :
        position(position), normal(normal) {}


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
                ambientFactor = .6f;
                specularBrightness = .2f;
                break;
            case ldr::Color::RUBBER:
                ambientFactor = .75f;
                specularBrightness = 0;
                break;
            case ldr::Color::PURE:
                ambientFactor = 1;
                specularBrightness = 0;
                break;
            default:
                ambientFactor = .5f;
                specularBrightness = .5f;
                break;
        }
    }

    TexturedTriangleInstance::TexturedTriangleInstance(const glm::vec3& idColor, const glm::mat4& transformation) :
        idColor(idColor), transformation(transformation) {}
}
