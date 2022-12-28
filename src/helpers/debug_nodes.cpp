#include <glm/gtx/transform.hpp>
#include "debug_nodes.h"

namespace bricksim::etree {
    PointDebugNode::PointDebugNode(const std::shared_ptr<Node>& parent) :
        UVSphereNode(ldr::color_repo::getPureColor(color::getRandom()), parent) {
        visibleInElementTree = false;
        layer = constants::DEBUG_NODES_LAYER;
    }

    std::string PointDebugNode::getDescription() {
        return "Point Debug Node";
    }

    const glm::vec3 &PointDebugNode::getPosition() const {
        return position;
    }

    void PointDebugNode::setPosition(const glm::vec3 &newPosition) {
        setRelativeTransformation(glm::transpose(glm::scale(glm::translate(newPosition), {10, 10, 10})));
        position = newPosition;
    }

    bool PointDebugNode::isDisplayNameUserEditable() const {
        return false;
    }

    bool PointDebugNode::isTransformationUserEditable() const {
        return false;
    }

    bool PointDebugNode::isColorUserEditable() const {
        return false;
    }
}
