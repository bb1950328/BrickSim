#include "base_action.h"
#include "../../config/read.h"
#include "../../controller.h"
#include "../../helpers/geometry.h"
#include "base_action.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include "translation.h"
#include <numeric>

namespace bricksim::graphical_transform {
    BaseAction::BaseAction(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes) :
        editor(editor), scene(editor.getScene()), nodes(nodes) {
        std::transform(nodes.cbegin(), nodes.cend(),
                       std::back_inserter(initialRelativeTransformations),
                       [](const auto& node) {
                           return glm::transpose(node->getRelativeTransformation());
                       });
        glm::vec4 center(0.f);
        for (const auto& item: nodes) {
            center += (glm::transpose(item->getAbsoluteTransformation()) * glm::vec4(0.f, 0.f, 0.f, 1.f));
        }
        initialNodeCenter = center / center.w;
    }

    BaseAction::~BaseAction() {
        if (state != State::FINISHED) {
            spdlog::error("graphical_transform::BaseAction destructed but not finished");
            cancel();
        }
    };

    void BaseAction::toggleAxisLock(bool x, bool y, bool z) {
        if (lockedAxes[0] == x && lockedAxes[1] == y && lockedAxes[2] == z) {
            lockedAxes = {false, false, false};
        } else {
            lockedAxes = {x, y, z};
        }
    }

    void BaseAction::cancel() {
        auto nodeIt = nodes.begin();
        auto transfIt = initialRelativeTransformations.begin();
        while (nodeIt != nodes.end()) {
            (*nodeIt)->setRelativeTransformation(glm::transpose(*transfIt));
            (*nodeIt)->incrementVersion();

            ++nodeIt;
            ++transfIt;
        }
        end();
    }

    BaseAction::State BaseAction::getState() const {
        return state;
    }

    void BaseAction::start(glm::svec2 initialCursorPos_) {
        if (state != State::READY) {
            throw std::invalid_argument("wrong state");
        }
        this->initialCursorPos = initialCursorPos_;
        this->currentCursorPos = initialCursorPos_;
        startImpl();
        state = State::ACTIVE;
    }

    void BaseAction::startImpl() {}

    void BaseAction::update(glm::svec2 currentCursorPos_) {
        if (state != State::ACTIVE) {
            throw std::invalid_argument("wrong state");
        }
        this->currentCursorPos = currentCursorPos_;
        updateImpl();
    }

    void BaseAction::end() {
        if (state != State::ACTIVE) {
            throw std::invalid_argument("wrong state");
        }
        endImpl();
        state = State::FINISHED;
    }

    void BaseAction::updateImpl() {}
    void BaseAction::endImpl() {}

    const std::array<bool, 3>& BaseAction::getLockedAxes() const {
        return lockedAxes;
    }

    const glm::svec2& BaseAction::getInitialCursorPos() const {
        return initialCursorPos;
    }

    const glm::svec2& BaseAction::getCurrentCursorPos() const {
        return currentCursorPos;
    }

    overlay2d::coord_t BaseAction::worldToO2DCoords(glm::vec3 worldCoords) const {
        return scene->worldToScreenCoordinates(glm::vec4(worldCoords, 1.f) * constants::LDU_TO_OPENGL);
    }

    void BaseAction::setAllNodeTransformations(const std::function<glm::mat4(const glm::mat4&)>& transformationProvider) {
        auto nodeIt = nodes.begin();
        auto transfIt = initialRelativeTransformations.begin();
        while (nodeIt != nodes.end()) {
            auto newTransf = transformationProvider(*transfIt);
            (*nodeIt)->setRelativeTransformation(glm::transpose(newTransf));
            (*nodeIt)->incrementVersion();

            ++nodeIt;
            ++transfIt;
        }
    }
}
