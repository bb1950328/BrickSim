#include "rotation.h"
#include "../../editor.h"
#include "../../helpers/geometry.h"
#include "spdlog/spdlog.h"

namespace bricksim::graphical_transform {
    void Rotation::startImpl() {
        startLine = std::make_shared<overlay2d::LineElement>(overlay2d::coord_t(0, 0), overlay2d::coord_t(0, 0), 4, color::RED);
        scene->getOverlayCollection().addElement(startLine);
    }
    void Rotation::updateImpl() {
        const auto [axis, initialPoint] = findBestPointOnPlanes(initialCursorPos);

        startLine->setStart(scene->worldToScreenCoordinates(glm::vec4(initialNodeCenter, 1.f) * constants::LDU_TO_OPENGL));
        startLine->setEnd(scene->worldToScreenCoordinates(glm::vec4(initialPoint, 1.f) * constants::LDU_TO_OPENGL));
        const auto colors = std::to_array({color::RED, color::GREEN, color::BLUE});
        startLine->setColor(colors[axis]);

        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(currentCursorPos) * constants::OPENGL_TO_LDU;
        glm::vec3 axisVec(0.f);
        axisVec[axis] = 1;
        auto currentPoint = geometry::rayPlaneIntersection(worldRay, Ray3(initialNodeCenter, axisVec));
        if (!currentPoint.has_value()) {
            currentPoint = initialPoint;
        }
        const auto rotationAngle = geometry::getAngleBetweenThreePointsSigned(initialPoint, initialNodeCenter, *currentPoint, axisVec);

        const auto axisNames = "XYZ";
        spdlog::debug("axis={}, angle={}°", axisNames[axis], glm::degrees(rotationAngle));

        auto nodeIt = nodes.begin();
        auto transfIt = initialRelativeTransformations.begin();
        while (nodeIt != nodes.end()) {
            auto newTransf = glm::translate(glm::mat4(1.f), initialNodeCenter)
                             * glm::rotate(glm::mat4(1.f), rotationAngle, axisVec)
                             * glm::translate(glm::mat4(1.f), -initialNodeCenter)
                             * (*transfIt);
            (*nodeIt)->setRelativeTransformation(glm::transpose(newTransf));
            (*nodeIt)->incrementVersion();

            ++nodeIt;
            ++transfIt;
        }
    }
    void Rotation::endImpl() {
        scene->getOverlayCollection().removeElement(startLine);
    }
    Rotation::Rotation(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) :
        BaseAction(editor, nodes) {}
    constexpr GraphicalTransformationType Rotation::getType() const {
        return GraphicalTransformationType::ROTATE;
    }
    std::pair<uint8_t, glm::vec3> Rotation::findBestPointOnPlanes(glm::usvec2 cursorPos) {
        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(cursorPos) * constants::OPENGL_TO_LDU;

        std::array<float, 3> angles{};
        for (int i = 0; i < 3; ++i) {
            glm::vec3 axis(0.f);
            axis[i] = 1;
            auto angleToAxis = geometry::getAngleBetweenTwoVectors(axis, worldRay.direction);
            if (angleToAxis > M_PI_2) {
                angleToAxis = M_PI - angleToAxis;
            }
            angles[i] = lockedAxes[i] ? 0 : angleToAxis;
        }
        const auto minAxis = static_cast<uint8_t>(std::distance(angles.begin(), std::min_element(angles.begin(), angles.end())));
        glm::vec3 axis(0.f);
        axis[minAxis] = 1;

        const auto intersectionPoint = geometry::rayPlaneIntersection(worldRay, Ray3(initialNodeCenter, axis));
        return {minAxis, intersectionPoint.value_or(glm::vec3(0.f))};
    }
    Rotation::~Rotation() = default;
}
