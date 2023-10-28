#pragma once
#include "../connection/connector/cylindrical.h"
#include "../connection/connector_conversion.h"
#include "../editor/editor.h"
#include "../element_tree.h"
#include "../graphics/scene.h"

namespace bricksim::snap {
    class SnapToConnectorProcess {
        std::shared_ptr<etree::LdrNode> subjectNode;
        float subjectRadius;
        glm::mat4 initialRelativeTransformation;
        glm::vec3 initialAbsoluteCenter;
        std::shared_ptr<connection::connector_container_t> subjectConnectors;

        std::shared_ptr<Editor> editor;

        glm::vec2 lastCursorPos;
        glm::vec2 cursorOffset;
        std::vector<std::pair<float, glm::mat4>> bestResults;

    public:
        SnapToConnectorProcess(const std::shared_ptr<etree::LdrNode>& subjectNode, const std::shared_ptr<Editor>& editor, const glm::vec2& initialCursorPos);
        void updateCursorPos(const glm::vec2& currentCursorPos);
        void cancel();
        std::vector<float> getPossibleCylTranslations(const std::shared_ptr<connection::CylindricalConnector> fixed, const std::shared_ptr<connection::CylindricalConnector> moving, bool sameDir);
    };
}
