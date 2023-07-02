#pragma once

#include "../../graphics/overlay2d/data.h"
#include "../../helpers/util.h"
namespace bricksim {
    class Editor;
}
namespace bricksim::etree {
    class Node;
}
namespace bricksim::graphics {
    class Scene;
}

namespace bricksim::graphical_transform {
    enum class GraphicalTransformationType {
        TRANSLATE,
        ROTATE,
        MOVE,
    };
    class BaseAction : public util::NoCopyNoMove {
    public:
        enum class State {
            READY,
            ACTIVE,
            FINISHED,
        };

        explicit BaseAction(Editor& editor, const std::vector<std::shared_ptr<etree::Node>>& nodes);
        virtual ~BaseAction();

        void start(glm::svec2 initialCursorPos_);
        void update(glm::svec2 currentCursorPos_);
        void end();
        void cancel();
        [[nodiscard]] State getState() const;

        /**
         * sets the axis locks to {\p x, \p y, \p z}. if the axis locks are already this value, set all axis locks to false
         * @param x @param y @param z
         */
        void toggleAxisLock(bool x, bool y, bool z);

        [[nodiscard]] constexpr virtual GraphicalTransformationType getType() const = 0;
        [[nodiscard]] const std::array<bool, 3>& getLockedAxes() const;
        [[nodiscard]] const glm::svec2& getInitialCursorPos() const;
        [[nodiscard]] const glm::svec2& getCurrentCursorPos() const;

    protected:
        Editor& editor;
        std::shared_ptr<graphics::Scene> scene;

        std::vector<std::shared_ptr<etree::Node>> nodes;
        std::vector<glm::mat4> initialRelativeTransformations;
        glm::vec3 initialNodeCenter;

        std::array<bool, 3> lockedAxes = {false, false, false};
        State state = State::READY;
        glm::svec2 initialCursorPos;
        glm::svec2 currentCursorPos;

        virtual void startImpl();
        virtual void updateImpl();
        virtual void endImpl();

        [[nodiscard]] overlay2d::coord_t worldToO2DCoords(glm::vec3 worldCoords) const;
        void setAllNodeTransformations(const std::function<glm::mat4(const glm::mat4&)>& transformationProvider);
    };
}
