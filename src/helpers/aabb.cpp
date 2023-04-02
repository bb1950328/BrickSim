#include "aabb.h"
#include "../../constant_data/constants.h"
#include "util.h"

namespace bricksim::aabb {
    AABB::AABB() :
        pMin(constants::pInf), pMax(constants::nInf) {
    }

    AABB::AABB(const glm::vec3& pMin, const glm::vec3& pMax) :
        pMin(pMin), pMax(pMax) {
    }

    AABB AABB::transform(const glm::mat4& transformation) const {
        const glm::vec3 p1 = transformation * glm::vec4(pMin, 1.0f);
        const glm::vec3 p2 = transformation * glm::vec4(pMax, 1.0f);
        return {util::cwiseMin(p1, p2),
                util::cwiseMax(p1, p2)};
    }

    bool AABB::isDefined() const {
        return std::isfinite(pMin.x);
    }

    glm::vec3 AABB::getCenter() const {
        return (pMin + pMax) / 2.f;
    }

    glm::vec3 AABB::getSize() const {
        return pMax - pMin;
    }

    void AABB::includePoint(const glm::vec3& p) {
        pMin = util::cwiseMin(pMin, p);
        pMax = util::cwiseMax(pMax, p);
    }

    void AABB::includeAABB(const AABB& other) {
        pMin = util::cwiseMin(pMin, other.pMin);
        pMax = util::cwiseMax(pMax, other.pMax);
    }

    void AABB::includeBBox(const RotatedBoundingBox& bbox) {
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

    RotatedBoundingBox::RotatedBoundingBox(const AABB& aabb) :
        RotatedBoundingBox(aabb, glm::vec3(0.f, 0.f, 0.f), glm::quat(1.f, 0.f, 0.f, 0.f)) {
    }

    RotatedBoundingBox::RotatedBoundingBox() :
        RotatedBoundingBox(AABB()) {
    }
}
