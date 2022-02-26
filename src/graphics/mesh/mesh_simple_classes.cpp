#include "mesh_simple_classes.h"
#include "../../helpers/util.h"
#include "../../helpers/geometry.h"
#include "../../constant_data/constants.h"
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

    AxisAlignedBoundingBox::AxisAlignedBoundingBox() :
        pMin(constants::pInf), pMax(constants::nInf) {
    }

    AxisAlignedBoundingBox::AxisAlignedBoundingBox(const glm::vec3& pMin, const glm::vec3& pMax) :
        pMin(pMin), pMax(pMax) {
    }

    AxisAlignedBoundingBox AxisAlignedBoundingBox::transform(const glm::mat4& transformation) const {
        const glm::vec3 p1 = glm::vec4(pMin, 1.0f) * transformation;
        const glm::vec3 p2 = glm::vec4(pMax, 1.0f) * transformation;
        return {util::cwiseMin(p1, p2),
                util::cwiseMax(p1, p2)};
    }

    bool AxisAlignedBoundingBox::isDefined() const {
        return std::isfinite(pMin.x);
    }

    glm::vec3 AxisAlignedBoundingBox::getCenter() const {
        return (pMin + pMax) / 2.f;
    }

    glm::vec3 AxisAlignedBoundingBox::getSize() const {
        return pMax - pMin;
    }

    void AxisAlignedBoundingBox::addPoint(const glm::vec3& p) {
        pMin = util::cwiseMin(pMin, p);
        pMax = util::cwiseMax(pMax, p);
    }

    void AxisAlignedBoundingBox::addAABB(const AxisAlignedBoundingBox& other) {
        pMin = util::cwiseMin(pMin, other.pMin);
        pMax = util::cwiseMax(pMax, other.pMax);
    }

    glm::mat4 RotatedBoundingBox::getUnitBoxTransformation() const {
        glm::mat4 transf = glm::mat4(1.f);
        transf = glm::toMat4(rotation) * transf;
        transf = glm::translate(glm::mat4(1.f), origin + centerOffset) * transf;
        transf = glm::scale(transf, size / 2.f);
        return transf;
    }
    RotatedBoundingBox RotatedBoundingBox::transform(const glm::mat4& transformation) const {
        RotatedBoundingBox result;
        const auto decomposedTransformation = util::decomposeTransformationToStruct(transformation);

        const glm::vec3 resCenter = transformation * glm::vec4(getCenter(), 1.f);
        result.origin = origin + decomposedTransformation.translation;
        result.centerOffset = resCenter - result.origin;
        result.rotation = rotation * decomposedTransformation.orientation;//geometry::quaternionRotationFromOneVectorToAnother(originalDirToCorner, resDirToCorner);
        result.size = size * decomposedTransformation.scale;
        return result;
    }
    glm::vec3 RotatedBoundingBox::getCenter() const {
        return origin + centerOffset;
    }
    RotatedBoundingBox::RotatedBoundingBox(const AxisAlignedBoundingBox& aabb) :
        RotatedBoundingBox(aabb, glm::vec3(0.f, 0.f, 0.f), glm::quat(1.f, 0.f, 0.f, 0.f)) {
    }
    RotatedBoundingBox::RotatedBoundingBox() = default;

}