#include "transform_gizmo.h"
#include "config.h"
#include "controller.h"
#include "editor.h"
#include "helpers/geometry.h"
#include "spdlog/spdlog.h"
#include <glm/gtx/io.hpp>
#include <numeric>
#include <spdlog/fmt/ostr.h>

namespace bricksim::transform_gizmo {

    TransformGizmo::TransformGizmo(Editor& editor) :
        editor(editor), scene(editor.getScene()) {
    }
    void TransformGizmo::update() {
        if (isActive()) {
            updateAxisLines();
        }
    }
    void TransformGizmo::updateAxisLines() {
        const auto& linearSnapPreset = controller::getSnapHandler().getLinear().getCurrentPreset();
        glm::vec3 pos = data->initialNodeCenter;
        constexpr std::array<glm::vec3, 3> axes = {
                glm::vec3(1.f, 0.f, 0.f),
                glm::vec3(0.f, 1.f, 0.f),
                glm::vec3(0.f, 0.f, 1.f),
        };
        for (int a = 0; a < 3; ++a) {
            glm::vec3 axisEndPos = pos;
            axisEndPos[a] = data->currentNodeCenter[a];
            if (controller::getSnapHandler().isEnabled()) {
                std::vector<o2d::coord_t> linePoints;
                const auto step = a == 1 ? linearSnapPreset.stepY : linearSnapPreset.stepXZ;
                const auto totalLineLength = data->currentPoint[a] - data->startPoint[a];
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
    void TransformGizmo::start(const std::vector<std::shared_ptr<etree::Node>>& nodes) {
        addAxisLines();
        data = std::make_unique<TranslationData>();

        data->nodes = nodes;
        std::transform(nodes.cbegin(), nodes.cend(),
                       std::back_inserter(data->initialRelativeTransformations),
                       [](const auto& node) {
                           return glm::transpose(node->getRelativeTransformation());
                       });

        glm::vec4 center(0.f);
        for (const auto& item: data->nodes) {
            center += (glm::transpose(item->getAbsoluteTransformation()) * glm::vec4(0.f, 0.f, 0.f, 1.f));
        }
        data->initialNodeCenter = center / center.w;
        data->currentNodeCenter = data->initialNodeCenter;
    }
    void TransformGizmo::updateCursorPos(glm::svec2 currentCursorPos) {
        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(currentCursorPos) * constants::OPENGL_TO_LDU;
        const auto lockedAxesCount = std::accumulate(data->lockedAxes.cbegin(), data->lockedAxes.cend(), 0);
        glm::vec3 projectedPoint;
        if (lockedAxesCount == 0) {
            projectedPoint = worldRay.origin + data->distanceToCamera * glm::normalize(worldRay.direction);
        } else if (lockedAxesCount == 1) {
            glm::vec3 planeNormal = {
                    data->lockedAxes[0] ? 1.f : 0.f,
                    data->lockedAxes[1] ? 1.f : 0.f,
                    data->lockedAxes[2] ? 1.f : 0.f,
            };
            projectedPoint = geometry::rayPlaneIntersection(worldRay, {data->startPoint, planeNormal}).value_or(data->startPoint);
        } else if (lockedAxesCount == 2) {
            glm::vec3 freeAxis = {
                    data->lockedAxes[0] ? 0.f : 1.f,
                    data->lockedAxes[1] ? 0.f : 1.f,
                    data->lockedAxes[2] ? 0.f : 1.f,
            };
            projectedPoint = geometry::closestLineBetweenTwoRays(worldRay, {data->startPoint, freeAxis}).pointOnB;
        } else {
            projectedPoint = data->startPoint;
        }
        auto translation = projectedPoint - data->startPoint;
        if (controller::getSnapHandler().isEnabled()) {
            const auto& linearSnapPreset = controller::getSnapHandler().getLinear().getCurrentPreset();
            translation = util::roundToNearestMultiple(translation, linearSnapPreset.stepXYZ());
        }

        const auto oldNodeCenter = data->currentNodeCenter;
        data->currentNodeCenter = data->initialNodeCenter + translation;
        if (glm::length(data->currentNodeCenter - oldNodeCenter) > 1.f) {
            updateAxisLines();
        }
        data->currentPoint = data->startPoint + translation;

        auto nodeIt = data->nodes.begin();
        auto transfIt = data->initialRelativeTransformations.begin();
        while (nodeIt != data->nodes.end()) {
            auto newTransf = *transfIt;
            newTransf[3] += glm::vec4(translation, 0.f);
            (*nodeIt)->setRelativeTransformation(glm::transpose(newTransf));
            (*nodeIt)->incrementVersion();

            ++nodeIt;
            ++transfIt;
        }
    }
    void TransformGizmo::end() {
        removeAxisLines();
        data = nullptr;
    }
    void TransformGizmo::addAxisLines() {
        static auto lineWidth = static_cast<o2d::length_t>(8 * config::get(config::GUI_SCALE));
        auto it = axisLines.begin();
        for (const auto color: {color::RED, color::GREEN, color::BLUE}) {
            if (*it == nullptr) {
                *it = std::make_shared<o2d::DashedLineElement>(std::vector<o2d::coord_t>{o2d::coord_t(0, 0), o2d::coord_t(0, 0)}, lineWidth, lineWidth, color);
                scene->getOverlayCollection().addElement(*it);
            }
            ++it;
        }
    }
    void TransformGizmo::removeAxisLines() {
        for (auto& axisLine: axisLines) {
            if (axisLine != nullptr) {
                scene->getOverlayCollection().removeElement(axisLine);
                axisLine = nullptr;
            }
        }
    }
    bool TransformGizmo::isActive() {
        return data != nullptr;
    }
    void TransformGizmo::toggleAxisLock(unsigned int axisIndex) {
        if (isActive()) {
            data->lockedAxes[axisIndex] ^= true;
        }
    }
    void TransformGizmo::toggleAxisLock(bool x, bool y, bool z) {
        if (data->lockedAxes[0] == x && data->lockedAxes[1] == y && data->lockedAxes[2] == z) {
            data->lockedAxes = {false, false, false};
        } else {
            data->lockedAxes = {x, y, z};
        }
    }
    bool TransformGizmo::isCursorDataInitialized() const {
        return data->cursorDataInitialized;
    }
    void TransformGizmo::initCursorData(glm::svec2 initialCursorPos) {
        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(initialCursorPos) * constants::OPENGL_TO_LDU;
        const auto npResult = geometry::normalProjectionOnLine(worldRay, data->initialNodeCenter);
        data->distanceToCamera = npResult.projectionLength;
        data->initialCursorPos = initialCursorPos;
        data->startPoint = npResult.nearestPointOnLine;
        data->currentPoint = data->startPoint;
        data->cursorDataInitialized = true;
    }
    void TransformGizmo::cancel() {
        auto nodeIt = data->nodes.begin();
        auto transfIt = data->initialRelativeTransformations.begin();
        while (nodeIt != data->nodes.end()) {
            (*nodeIt)->setRelativeTransformation(glm::transpose(*transfIt));
            (*nodeIt)->incrementVersion();

            ++nodeIt;
            ++transfIt;
        }
        end();
    }
    TransformGizmo::~TransformGizmo() = default;
}
