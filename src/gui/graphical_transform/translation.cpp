#include "translation.h"
#include "../../config/read.h"
#include "../../controller.h"
#include "../../helpers/geometry.h"
#include "base_action.h"
#include <numeric>

namespace bricksim::graphical_transform {

    Translation::Translation(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) :
        BaseAction(editor, nodes) {
        currentNodeCenter = initialNodeCenter;
    }

    void Translation::updateAxisLines() {
        const auto& linearSnapPreset = controller::getSnapHandler().getLinear().getCurrentPreset();
        glm::vec3 pos = initialNodeCenter;
        GLM_CONSTEXPR std::array<glm::vec3, 3> axes = {
                glm::vec3(1.f, 0.f, 0.f),
                glm::vec3(0.f, 1.f, 0.f),
                glm::vec3(0.f, 0.f, 1.f),
        };
        for (int a = 0; a < 3; ++a) {
            glm::vec3 axisEndPos = pos;
            axisEndPos[a] = currentNodeCenter[a];
            if (controller::getSnapHandler().isEnabled()) {
                std::vector<o2d::coord_t> linePoints;
                const auto step = a == 1 ? linearSnapPreset.stepY : linearSnapPreset.stepXZ;
                const auto totalLineLength = currentPoint[a] - startPoint[a];
                const auto absTotalLinelength = static_cast<int>(std::abs(totalLineLength));
                if (absTotalLinelength >= step) {
                    for (int i = 0; i <= absTotalLinelength; i += step) {
                        const glm::vec3 worldCoord = pos + axes[a] * std::copysign(static_cast<float>(i), totalLineLength);
                        const glm::vec3 screenCoord = scene->worldToScreenCoordinates(glm::vec4(worldCoord, 1.f) * constants::LDU_TO_OPENGL);
                        if (-1 <= screenCoord.z && screenCoord.z <= 1) {
                            linePoints.emplace_back(screenCoord);
                        }
                    }
                }
                axisLines[a]->setPoints(linePoints);
            } else {
                axisLines[a]->setPoints({
                        scene->worldToScreenCoordinates(glm::vec4(pos, 1.f) * constants::LDU_TO_OPENGL),
                        scene->worldToScreenCoordinates(glm::vec4(axisEndPos, 1.f) * constants::LDU_TO_OPENGL),
                });
            }

            pos = axisEndPos;
        }
    }

    void Translation::addAxisLines() {
        static auto lineWidth = static_cast<o2d::length_t>(8 * config::get().gui.scale);
        auto it = axisLines.begin();
        for (const auto color: constants::AXIS_COLORS) {
            if (*it == nullptr) {
                *it = std::make_shared<o2d::DashedLineElement>(std::vector<o2d::coord_t>{o2d::coord_t(0, 0), o2d::coord_t(0, 0)}, lineWidth, lineWidth, color);
                scene->getOverlayCollection().addElement(*it);
            }
            ++it;
        }
    }
    void Translation::removeAxisLines() {
        for (auto& axisLine: axisLines) {
            if (axisLine != nullptr) {
                scene->getOverlayCollection().removeElement(axisLine);
                axisLine = nullptr;
            }
        }
    }

    void Translation::startImpl() {
        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(initialCursorPos) * constants::OPENGL_TO_LDU;
        const auto npResult = geometry::normalProjectionOnLine(worldRay, initialNodeCenter);
        distanceToCamera = npResult.projectionLength;
        startPoint = npResult.nearestPointOnLine;
        currentPoint = startPoint;

        addAxisLines();
    }
    void Translation::updateImpl() {
        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(currentCursorPos) * constants::OPENGL_TO_LDU;
        const auto lockedAxesCount = std::accumulate(lockedAxes.cbegin(), lockedAxes.cend(), 0);
        glm::vec3 projectedPoint;
        if (lockedAxesCount == 0) {
            projectedPoint = worldRay.origin + distanceToCamera * glm::normalize(worldRay.direction);
        } else if (lockedAxesCount == 1) {
            glm::vec3 planeNormal = {
                    lockedAxes[0] ? 1.f : 0.f,
                    lockedAxes[1] ? 1.f : 0.f,
                    lockedAxes[2] ? 1.f : 0.f,
            };
            projectedPoint = geometry::rayPlaneIntersection(worldRay, {startPoint, planeNormal}).value_or(startPoint);
        } else if (lockedAxesCount == 2) {
            glm::vec3 freeAxis = {
                    lockedAxes[0] ? 0.f : 1.f,
                    lockedAxes[1] ? 0.f : 1.f,
                    lockedAxes[2] ? 0.f : 1.f,
            };
            projectedPoint = geometry::closestLineBetweenTwoRays(worldRay, {startPoint, freeAxis}).pointOnB;
        } else {
            projectedPoint = startPoint;
        }
        auto translation = projectedPoint - startPoint;
        if (controller::getSnapHandler().isEnabled()) {
            const auto& linearSnapPreset = controller::getSnapHandler().getLinear().getCurrentPreset();
            translation = util::roundToNearestMultiple(translation, snap::LinearHandler::getStepXYZ(linearSnapPreset));
        }

        if (glm::length(lastTranslation - translation) < 0.1f) {
            return;
        }

        const auto oldNodeCenter = currentNodeCenter;
        currentNodeCenter = initialNodeCenter + translation;
        if (glm::length(currentNodeCenter - oldNodeCenter) > 1.f) {
            updateAxisLines();
        }
        currentPoint = startPoint + translation;

        setAllNodeTransformations([&translation](const glm::mat4& initialRelTransf) {
            auto newTransf = initialRelTransf;
            newTransf[3] += glm::vec4(translation, 0.f);
            return newTransf;
        });
        lastTranslation = translation;
    }
    void Translation::endImpl() {
        removeAxisLines();
    }
    constexpr GraphicalTransformationType Translation::getType() const {
        return GraphicalTransformationType::TRANSLATE;
    }
    Translation::~Translation() = default;
}
