#pragma once

#include "../element_tree.h"
#include "../graphics/mesh/mesh_generated.h"

namespace bricksim::etree {
    class PointDebugNode : public mesh::generated::UVSphereNode {
    private:
        glm::vec3 position{0.f};

    public:
        explicit PointDebugNode(const std::shared_ptr<Node> &parent);
        std::string getDescription() override;
        const glm::vec3 &getPosition() const;
        void setPosition(const glm::vec3 &newPosition);

        bool isDisplayNameUserEditable() const override;
        bool isTransformationUserEditable() const override;
        bool isColorUserEditable() const override;
    };
}
