#include "rotation.h"
#include "../../editor.h"
#include "../../helpers/geometry.h"

namespace bricksim::graphical_transform {
    void Rotation::startImpl() {
        BaseAction::startImpl();
    }
    void Rotation::updateImpl() {
        const auto [axis, initialPoint] = findBestPointOnPlanes(initialCursorPos);

        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(currentCursorPos) * constants::OPENGL_TO_LDU;
        glm::vec3 axisVec(0.f);
        axisVec[axis] = 1;
        auto currentPoint = geometry::rayPlaneIntersection(worldRay, Ray3(initialNodeCenter, axisVec));
        if (!currentPoint.has_value()) {
            currentPoint = initialPoint;
        }
        const auto rotationAngle = geometry::getAngleBetweenThreePointsSigned(initialPoint, initialNodeCenter, *currentPoint, axisVec);

        glm::mat4 rotation(1.f);
        rotation = glm::translate(rotation, -initialNodeCenter);
        rotation = glm::rotate(rotation, rotationAngle, axisVec);
        rotation = glm::translate(rotation, initialNodeCenter);

        auto nodeIt = nodes.begin();
        auto transfIt = initialRelativeTransformations.begin();
        while (nodeIt != nodes.end()) {
            auto newTransf = *transfIt;
            newTransf *= rotation;
            (*nodeIt)->setRelativeTransformation(glm::transpose(newTransf));
            (*nodeIt)->incrementVersion();

            ++nodeIt;
            ++transfIt;
        }
    }
    void Rotation::endImpl() {
        BaseAction::endImpl();
    }
    Rotation::Rotation(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) :
        BaseAction(editor, nodes) {}
    constexpr GraphicalTransformationType Rotation::getType() const {
        return GraphicalTransformationType::ROTATE;
    }
    std::pair<uint8_t, glm::vec3> Rotation::findBestPointOnPlanes(glm::usvec2 cursorPos) {
        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(cursorPos) * constants::OPENGL_TO_LDU;
        std::array<std::optional<glm::vec3>, 3> points;
        std::array<float, 3> dists{};
        for (int i = 0; i < 3; ++i) {
            glm::vec3 axis(0.f);
            axis[i] = 1;
            points[i] = lockedAxes[i]
                                ? geometry::rayPlaneIntersection(worldRay, Ray3(initialNodeCenter, axis))
                                : std::nullopt;
            dists[i] = points[i].has_value()
                               ? glm::length(*points[i] - initialNodeCenter)
                               : INFINITY;
        }
        const auto minAxis = std::distance(dists.begin(), std::min_element(dists.begin(), dists.end()));
        return {minAxis, *points[minAxis]};
    }
    Rotation::~Rotation() = default;
}
