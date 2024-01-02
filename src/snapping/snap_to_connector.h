#pragma once
#include "../connection/connector/cylindrical.h"
#include "../connection/connector_conversion.h"
#include "../editor/editor.h"
#include "../element_tree.h"
#include "../graphics/scene.h"

namespace bricksim::snap {
    class SnapToConnectorProcess {
        std::vector<std::shared_ptr<etree::Node>> subjectNodes;
        float subjectRadius;
        std::vector<glm::mat4> initialRelativeTransformations;
        glm::vec3 initialAbsoluteCenter;

        glm::mat4 userTransformation;

        ///already transformed to initial absolute transformation
        std::shared_ptr<connection::connector_container_t> subjectConnectors;

        std::shared_ptr<Editor> editor;

        glm::vec2 initialCursorPos;
        glm::vec2 lastCursorPos;
        glm::vec2 cursorOffset;
        ///pair.first is score, pair.second is transformation relative to initial transformation of node
        std::vector<std::pair<float, glm::mat4>> bestResults;

    public:
        SnapToConnectorProcess(const std::vector<std::shared_ptr<etree::Node>>& subjectNodes, const std::shared_ptr<Editor>& editor, const glm::vec2& initialCursorPos);
        void updateCursorPos(const glm::vec2& currentCursorPos);
        void applyInitialTransformations();
        void applyResultTransformation(std::size_t index);
        [[nodiscard]] std::size_t getResultCount() const;
        std::vector<float> getPossibleCylTranslations(const std::shared_ptr<connection::CylindricalConnector>& fixed, const std::shared_ptr<connection::CylindricalConnector>& moving, bool sameDir);
        void setUserTransformation(const glm::mat4& value);

    private:
        void setRelativeTransformationIfDifferent(int subjectNodeIndex, const glm::mat4& newRelTransf);
    };
}
