#include "transform_gizmo2.h"
#include "config.h"
#include "controller.h"
#include "editor.h"
#include "helpers/geometry.h"
#include "spdlog/spdlog.h"
#include <glm/gtx/io.hpp>
#include <spdlog/fmt/ostr.h>

namespace bricksim::transform_gizmo {

    TransformGizmo2::TransformGizmo2(Editor& editor) :
        editor(editor), scene(editor.getScene()) {
    }
    void TransformGizmo2::update() {
        if (isActive()) {
            updateAxisLines();
        }
    }
    void TransformGizmo2::updateAxisLines() {
        const auto& linearSnapPreset = controller::getSnapHandler().getLinear().getCurrentPreset();
        glm::vec3 pos = {0, 0, 0};
        constexpr std::array<glm::vec3, 3> axes = {glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)};
        for (int a = 0; a < 3; ++a) {
            std::vector<o2d::coord_t> linePoints;
            const auto step = a == 1 ? linearSnapPreset.stepY : linearSnapPreset.stepXZ;
            for (int i = -10; i < 11; ++i) {
                const glm::vec3 worldCoord = pos + axes[a] * static_cast<float>(i * step);
                const glm::vec3 screenCoord = scene->worldToScreenCoordinates(glm::vec4(worldCoord, 1.f) * constants::LDU_TO_OPENGL);
                if (-1 <= screenCoord.z && screenCoord.z <= 1) {
                    linePoints.emplace_back(screenCoord);
                }
            }
            axisLines[a]->setPoints(linePoints);
        }
    }
    void TransformGizmo2::start(const std::vector<std::shared_ptr<etree::Node>>& nodes) {
        addAxisLines();
        data = std::make_unique<TranslationData>();

        data->nodes = nodes;
        std::transform(nodes.cbegin(), nodes.cend(),
                       std::back_inserter(data->initialRelativeTransformations),
                       [](const auto& node) {
                           return glm::transpose(node->getRelativeTransformation());
                       });

        spdlog::debug("start translating {} nodes", data->nodes.size());
    }
    void TransformGizmo2::updateCursorPos(glm::svec2 currentCursorPos) {
        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(currentCursorPos) * constants::OPENGL_TO_LDU;
        const auto lockedAxesCount = std::accumulate(data->lockedAxes.cbegin(), data->lockedAxes.cend(), 0);
        if (lockedAxesCount == 0) {
            data->currentPoint = worldRay.origin + data->distanceToCamera * glm::normalize(worldRay.direction);
        } else if (lockedAxesCount == 1) {
            glm::vec3 planeNormal = {
                    data->lockedAxes[0] ? 1.f : 0.f,
                    data->lockedAxes[1] ? 1.f : 0.f,
                    data->lockedAxes[2] ? 1.f : 0.f,
            };
            data->currentPoint = geometry::rayPlaneIntersection(worldRay, {data->startPoint, planeNormal}).value_or(data->startPoint);
        } else if (lockedAxesCount == 2) {
            glm::vec3 freeAxis = {
                    data->lockedAxes[0] ? 0.f : 1.f,
                    data->lockedAxes[1] ? 0.f : 1.f,
                    data->lockedAxes[2] ? 0.f : 1.f,
            };
            data->currentPoint = geometry::closestLineBetweenTwoRays(worldRay, {data->startPoint, freeAxis}).pointOnB;
        } else {
            data->currentPoint = data->startPoint;
        }
        auto translation = data->currentPoint - data->startPoint;
        const auto& linearSnapPreset = controller::getSnapHandler().getLinear().getCurrentPreset();
        translation = util::roundToNearestMultiple(translation, linearSnapPreset.stepXYZ());
        spdlog::debug("current translation: {}", translation);

        auto nodeIt = data->nodes.begin();
        auto transfIt = data->initialRelativeTransformations.begin();
        while (nodeIt != data->nodes.end()) {
            (*nodeIt)->setRelativeTransformation(glm::transpose(glm::translate(*transfIt, translation)));
            (*nodeIt)->incrementVersion();

            ++nodeIt;
            ++transfIt;
        }
    }
    void TransformGizmo2::endDrag() {
        removeAxisLines();
        data = nullptr;
        spdlog::debug("end node drag");
    }
    void TransformGizmo2::addAxisLines() {
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
    void TransformGizmo2::removeAxisLines() {
        for (auto& axisLine: axisLines) {
            if (axisLine != nullptr) {
                scene->getOverlayCollection().removeElement(axisLine);
                axisLine = nullptr;
            }
        }
    }
    bool TransformGizmo2::isActive() {
        return data != nullptr;
    }
    void TransformGizmo2::toggleAxisLock(unsigned int axisIndex) {
        if (isActive()) {
            data->lockedAxes[axisIndex] ^= true;
        }
    }
    void TransformGizmo2::toggleAxisLock(bool x, bool y, bool z) {
        if (data->lockedAxes[0] == x && data->lockedAxes[1] == y && data->lockedAxes[2] == z) {
            data->lockedAxes = {false, false, false};
        } else {
            data->lockedAxes = {x, y, z};
        }
    }
    bool TransformGizmo2::isCursorDataInitialized() const {
        return data->cursorDataInitialized;
    }
    void TransformGizmo2::initCursorData(glm::svec2 initialCursorPos) {
        glm::vec4 center(0.f);
        for (const auto& item: data->nodes) {
            center += (glm::transpose(item->getAbsoluteTransformation()) * glm::vec4(0.f, 0.f, 0.f, 1.f));
        }
        const auto worldRay = editor.getScene()->screenCoordinatesToWorldRay(initialCursorPos) * constants::OPENGL_TO_LDU;
        const auto npResult = geometry::normalProjectionOnLine(worldRay, glm::vec3(center / center.w));
        data->distanceToCamera = npResult.projectionLength;
        data->initialCursorPos = initialCursorPos;
        data->startPoint = npResult.nearestPointOnLine;
        data->cursorDataInitialized = true;
    }
    TransformGizmo2::~TransformGizmo2() = default;
}
