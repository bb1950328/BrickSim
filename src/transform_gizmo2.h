#pragma once
#include "element_tree.h"
#include "graphics/scene.h"
#include <memory>

namespace bricksim {
    class Editor;
}

namespace bricksim::transform_gizmo {
    namespace o2d = overlay2d;
    class TransformGizmo2 {
    public:
        explicit TransformGizmo2(Editor& editor);
        TransformGizmo2(const TransformGizmo2&) = delete;
        TransformGizmo2(TransformGizmo2&&) = delete;
        TransformGizmo2& operator=(const TransformGizmo2&) = delete;
        TransformGizmo2& operator=(TransformGizmo2&&) = delete;
        virtual ~TransformGizmo2();

        void update();
        bool ownsNode(const std::shared_ptr<etree::Node>& node_);
        void startDrag(std::shared_ptr<etree::Node>& draggedNode, glm::svec2 initialCursorPos);
        void updateCurrentDragDelta(glm::svec2 totalDragDelta);
        void endDrag();

    private:
        std::shared_ptr<graphics::Scene> scene;
        Editor& editor;
        std::array<std::shared_ptr<o2d::DashedLineElement>, 3> axisLines;
    };
}
