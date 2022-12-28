#include "mesh_simple_classes.h"
#include "../../constant_data/constants.h"
#include "../../helpers/geometry.h"
#include "../../helpers/util.h"
#include <cstring>

namespace bricksim::mesh {
    bool MeshInstance::operator==(const MeshInstance& other) const = default;

    bool TriangleVertex::operator==(const TriangleVertex& other) const = default;
    TriangleVertex::TriangleVertex(const glm::vec3& position, const glm::vec3& normal) :
        position(position), normal(normal) {}

    bool TexturedTriangleVertex::operator==(const TexturedTriangleVertex& other) const = default;

    bool LineVertex::operator==(const LineVertex& other) const = default;

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

    AxisAlignedBoundingBox::AxisAlignedBoundingBox() :
        pMin(constants::pInf), pMax(constants::nInf) {
    }

    AxisAlignedBoundingBox::AxisAlignedBoundingBox(const glm::vec3& pMin, const glm::vec3& pMax) :
        pMin(pMin), pMax(pMax) {
    }

    AxisAlignedBoundingBox AxisAlignedBoundingBox::transform(const glm::mat4& transformation) const {
        const glm::vec3 p1 = transformation * glm::vec4(pMin, 1.0f);
        const glm::vec3 p2 = transformation * glm::vec4(pMax, 1.0f);
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

    void AxisAlignedBoundingBox::includePoint(const glm::vec3& p) {
        pMin = util::cwiseMin(pMin, p);
        pMax = util::cwiseMax(pMax, p);
    }

    void AxisAlignedBoundingBox::includeAABB(const AxisAlignedBoundingBox& other) {
        pMin = util::cwiseMin(pMin, other.pMin);
        pMax = util::cwiseMax(pMax, other.pMax);
    }

    void AxisAlignedBoundingBox::includeBBox(const RotatedBoundingBox& bbox) {
        const auto center = glm::vec4(bbox.getCenter(), 1.f);
        const auto transf = bbox.getUnitBoxTransformation();
        for (const auto pn: {glm::vec3(1.f, -1.f, 1.f), glm::vec3(-1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(-1.f, -1.f, 1.f)}) {
            const auto ptr = transf * glm::vec4(pn, 1.f);
            const auto oppositePtr = 2.f * center - ptr;
            includePoint(ptr);
            includePoint(oppositePtr);
        }
    }

    glm::mat4 RotatedBoundingBox::getUnitBoxTransformation() const {
        glm::mat4 transf(1.f);
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
        result.rotation = rotation * decomposedTransformation.orientation;
        result.size = size * decomposedTransformation.scale;
        return result;
    }

    glm::vec3 RotatedBoundingBox::getCenter() const {
        return origin + centerOffset;
    }

    RotatedBoundingBox::RotatedBoundingBox(const AxisAlignedBoundingBox& aabb) :
        RotatedBoundingBox(aabb, glm::vec3(0.f, 0.f, 0.f), glm::quat(1.f, 0.f, 0.f, 0.f)) {
    }

    RotatedBoundingBox::RotatedBoundingBox() :
        RotatedBoundingBox(AxisAlignedBoundingBox()) {
    }
    TexturedTriangleInstance::TexturedTriangleInstance(const glm::vec3& idColor, const glm::mat4& transformation) :
        idColor(idColor), transformation(transformation) {}
}
