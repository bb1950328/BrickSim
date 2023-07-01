#include "rotation.h"
#include "../../config.h"
#include "../../controller.h"
#include "../../helpers/geometry.h"
#include "spdlog/fmt/ostr.h"
#include <glm/gtx/string_cast.hpp>

namespace bricksim::graphical_transform {
    void Rotation::startImpl() {
        static auto lineWidth = static_cast<overlay2d::length_t>(8 * config::get(config::GUI_SCALE));
        const auto nodeCenterOnScreen = worldToO2DCoords(initialNodeCenter);
        startLine = std::make_shared<overlay2d::LineElement>(nodeCenterOnScreen, overlay2d::coord_t(0, 0), lineWidth, color::BLACK);
        currentLine = std::make_shared<overlay2d::LineElement>(nodeCenterOnScreen, overlay2d::coord_t(0, 0), lineWidth, color::BLACK);
        arc = std::make_shared<overlay2d::DashedPolyLineElement>(lineWidth, lineWidth, color::BLACK);
        scene->getOverlayCollection().addElement(startLine);
        scene->getOverlayCollection().addElement(currentLine);
        scene->getOverlayCollection().addElement(arc);
    }
    void Rotation::updateImpl() {
        const auto initialWorldRay = editor.getScene()->screenCoordinatesToWorldRay(initialCursorPos) * constants::OPENGL_TO_LDU;
        const auto axis = findRotationAxis(initialWorldRay.direction);
        glm::vec3 axisVec(0.f);
        axisVec[axis] = 1;
        const auto initialPoint = geometry::rayPlaneIntersection(initialWorldRay, Ray3(initialNodeCenter, axisVec)).value_or(glm::vec3(0.f));

        const auto currentWorldRay = editor.getScene()->screenCoordinatesToWorldRay(currentCursorPos) * constants::OPENGL_TO_LDU;
        auto currentPoint = geometry::rayPlaneIntersection(currentWorldRay, Ray3(initialNodeCenter, axisVec)).value_or(initialPoint);

        const auto arcRadius = glm::length(currentPoint - initialNodeCenter);
        const auto initialPointScaled = initialNodeCenter + arcRadius * glm::normalize(initialPoint - initialNodeCenter);
        const auto relInitialPointScaled = initialPointScaled - initialNodeCenter;

        auto rotationAngle = geometry::getAngleBetweenThreePointsSigned(initialPoint, initialNodeCenter, currentPoint, axisVec);
        if (controller::getSnapHandler().isEnabled()) {
            const auto snapPreset = glm::radians(controller::getSnapHandler().getRotational().getCurrentPreset().stepDeg);
            rotationAngle = util::roundToNearestMultiple(rotationAngle, snapPreset);
            currentPoint = initialNodeCenter + glm::angleAxis(rotationAngle, axisVec) * relInitialPointScaled;
        }

        const auto color = constants::AXIS_COLORS[axis];
        startLine->setColor(color);
        currentLine->setColor(color);
        arc->setColor(color);

        startLine->setEnd(worldToO2DCoords(initialPointScaled));
        currentLine->setEnd(worldToO2DCoords(currentPoint));

        const auto arcAngles = getArcAngles(rotationAngle);
        arc->setPoints(getPointsForArc(arcAngles, axisVec, relInitialPointScaled));

        if (lastRotationAngle == rotationAngle) {
            return;
        }

        setAllNodeTransformations([this, rotationAngle, &axisVec](const glm::mat4& initialRelTransf) {
            return glm::translate(glm::mat4(1.f), initialNodeCenter)
                   * glm::rotate(glm::mat4(1.f), rotationAngle, axisVec)
                   * glm::translate(glm::mat4(1.f), -initialNodeCenter)
                   * initialRelTransf;
        });

        lastRotationAngle = rotationAngle;
    }
    std::vector<std::pair<float, float>> Rotation::getArcAngles(float& rotationAngle) {
        std::vector<std::pair<float, float>> result;
        const auto& snapHandler = controller::getSnapHandler();
        if (snapHandler.isEnabled()) {
            float snapPreset = glm::radians(snapHandler.getRotational().getCurrentPreset().stepDeg);
            const auto signedStep = std::copysign(snapPreset, rotationAngle);
            for (int is = 0; is < static_cast<int>(std::abs(rotationAngle) / snapPreset); ++is) {
                result.emplace_back(
                        static_cast<float>(is) * signedStep,
                        static_cast<float>(is + 1) * signedStep);
            }
        } else {
            result.emplace_back(0, rotationAngle);
        }
        return result;
    }
    void Rotation::endImpl() {
        scene->getOverlayCollection().removeElement(startLine);
        scene->getOverlayCollection().removeElement(currentLine);
        scene->getOverlayCollection().removeElement(arc);
    }
    Rotation::Rotation(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) :
        BaseAction(editor, nodes) {}
    constexpr GraphicalTransformationType Rotation::getType() const {
        return GraphicalTransformationType::ROTATE;
    }
    uint8_t Rotation::findRotationAxis(glm::vec3 worldRayDirection) const {
        std::array<float, 3> scores{};
        for (int i = 0; i < 3; ++i) {
            glm::vec3 axis(0.f);
            axis[i] = 1;
            auto angleToAxis = geometry::getAngleBetweenTwoVectors(axis, worldRayDirection);
            if (angleToAxis > M_PI_2) {
                angleToAxis = M_PI - angleToAxis;
            }
            float penalty = 0.f;
            if (angleToAxis > glm::radians(85.f)) {
                penalty += 20;
            }
            if (lockedAxes[i]) {
                penalty += 10;
            }
            scores[i] = penalty + angleToAxis;
        }

        return std::distance(scores.begin(), std::min_element(scores.begin(), scores.end()));
    }
    overlay2d::DashedPolyLineElement::points_t Rotation::getPointsForArc(const std::vector<std::pair<float, float>>& arcAngles, glm::vec3 axisVec, glm::vec3 relInitialPointScaled) const {
        std::vector<std::vector<overlay2d::coord_t>> coords2d;
        coords2d.reserve(arcAngles.size());

        const auto angleToPoint2d = [this, &axisVec, &relInitialPointScaled](float angle) {
            const auto relCurrentPoint = glm::angleAxis(angle, axisVec) * relInitialPointScaled;
            return worldToO2DCoords(initialNodeCenter + relCurrentPoint);
        };

        for (const auto& [beginAngle, endAngle]: arcAngles) {
            const auto pBegin = angleToPoint2d(beginAngle);
            const auto pEnd = angleToPoint2d(endAngle);
            const auto stepCount = static_cast<int>(glm::length(pEnd - pBegin) / 10);
            const auto stepAngle = (endAngle - beginAngle) / static_cast<float>((stepCount + 2));
            auto& line2d = coords2d.emplace_back();
            line2d.reserve(stepCount + 2);
            line2d.push_back(pBegin);
            for (int i = 1; i <= stepCount; ++i) {
                line2d.push_back(angleToPoint2d(beginAngle + static_cast<float>(i) * stepAngle));
            }
            line2d.push_back(pEnd);
        }
        return coords2d;
    }
    Rotation::~Rotation() = default;
}
