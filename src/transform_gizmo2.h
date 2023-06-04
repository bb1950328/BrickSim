#pragma once
#include "element_tree.h"
#include "graphics/scene.h"
#include <memory>

namespace bricksim {
    class Editor;
}

namespace bricksim::transform_gizmo {
    namespace o2d = overlay2d;

    struct TranslationData {
        std::vector<std::shared_ptr<etree::Node>> nodes;
        std::vector<glm::mat4> initialRelativeTransformations;
        std::array<bool, 3> lockedAxes = {false, false, false};

        bool cursorDataInitialized = false;
        glm::vec3 startPoint;
        glm::vec3 currentPoint;
        float distanceToCamera;
        glm::svec2 initialCursorPos;
    };

    class TransformGizmo2 {
    public:
        explicit TransformGizmo2(Editor& editor);
        TransformGizmo2(const TransformGizmo2&) = delete;
        TransformGizmo2(TransformGizmo2&&) = delete;
        TransformGizmo2& operator=(const TransformGizmo2&) = delete;
        TransformGizmo2& operator=(TransformGizmo2&&) = delete;
        virtual ~TransformGizmo2();

        void update();
        void start(const std::vector<std::shared_ptr<etree::Node>>& nodes);
        void initCursorData(glm::svec2 initialCursorPos);
        void updateCursorPos(glm::svec2 currentCursorPos);
        void endDrag();

        bool isActive();
        [[nodiscard]] bool isCursorDataInitialized() const;
        void toggleAxisLock(unsigned int axisIndex);
        /**
         * sets the axis locks to {\p x, \p y, \p z}. if the axis locks are already this value, set all axis locks to false
         * @param x @param y @param z
         */
        void toggleAxisLock(bool x, bool y, bool z);

    private:
        std::shared_ptr<graphics::Scene> scene;
        Editor& editor;
        std::array<std::shared_ptr<o2d::DashedLineElement>, 3> axisLines;

        std::unique_ptr<TranslationData> data;

        void addAxisLines();
        void removeAxisLines();
        void updateAxisLines();
    };
}
