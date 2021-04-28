

#include "transform_gizmo.h"

#include <utility>
#include <algorithm>

const glm::vec4 ARROW_DIRECTIONS[3] = {
        {1.0f, 0.0f,  0.0f,  1.0f},
        {0.0f, 0.0f,  -1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f,  1.0f},
};

TransformGizmo::TransformGizmo(std::shared_ptr<Scene> scene) : scene(std::move(scene)) {}

void TransformGizmo::update() {
    const auto guiScale = config::getDouble(config::GUI_SCALE);

    glm::vec3 cameraPos = scene->getCamera()->getCameraPos();
    const glm::vec3 center(0.0f, 0.0f, 0.0f);//todo selected node center here

    auto cameraCenterDistance = glm::length(cameraPos - center);

    //todo update node transformation
}

TransformGizmo::~TransformGizmo() {
    //TODO
}

bool ArrowMeshNode::isTransformationUserEditable() const {
    return Node::isTransformationUserEditable();
}

bool ArrowMeshNode::isDisplayNameUserEditable() const {
    return false;
}

std::string ArrowMeshNode::getDescription() {
    return "Arrow of Transform Gizmo";
}

mesh_identifier_t ArrowMeshNode::getMeshIdentifier() const {
    return constants::MESH_ID_TRANSFORM_GIZMO_ARROW;
}

void ArrowMeshNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {
    constexpr uint16_t numCorners = 12;
    constexpr float lineRadius = 0.1f;
    constexpr float tipRadius = 0.2f;
    glm::vec3 backCenter(0.0f, 0.0f, 0.0f);
    glm::vec3 beforeTipCenter(0.75f, 0.0f, 0.0f);
    glm::vec3 tip(1.0f, 0.0f, 0.0f);

    TriangleVertex backCoverVertex{
            glm::vec4(backCenter.x, backCenter.y + lineRadius, backCenter.z, 1.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f)
    };

    TriangleVertex lineBackVertex{
            backCoverVertex.position,
            glm::vec3(0.0f, 1.0f, 0.0f)
    };
    TriangleVertex lineBeforeTipVertex{
            glm::vec4(beforeTipCenter.x, beforeTipCenter.y + lineRadius, beforeTipCenter.z, 1.0f),
            lineBackVertex.normal
    };

    TriangleVertex ringInnerVertex{
            lineBeforeTipVertex.position,
            backCoverVertex.normal
    };
    TriangleVertex ringOuterVertex{
            glm::vec4(beforeTipCenter.x, beforeTipCenter.y + tipRadius, beforeTipCenter.z, 1.0f),
            ringInnerVertex.normal
    };

    TriangleVertex tipEdgeVertex{
            ringOuterVertex.position,
            glm::normalize(glm::vec3(tipRadius, tip.x - beforeTipCenter.x, 0.0f))
    };

    TriangleVertex tipVertex{
            glm::vec4(tip, 1.0f),
            tipEdgeVertex.normal
    };

    unsigned int firstIndex = mesh->getNextVertexIndex(ldr_color_repo::getInstanceDummyColor());


    for (uint16_t i = 0; i < numCorners; ++i) {
        auto rotationMatrix = glm::rotate(2*M_PIf32*i/numCorners, glm::vec3(1.0f, 0.0f, 0.0f));
        //todo tipEdgeVertex.position *= rotationMatrix;
    }
}

bool ArrowMeshNode::isColorUserEditable() const {
    return MeshNode::isColorUserEditable();
}

ArrowMeshNode::ArrowMeshNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent) : MeshNode(color, parent) {}

bool TransformGizmoNode::isTransformationUserEditable() const {
    return false;
}

bool TransformGizmoNode::isDisplayNameUserEditable() const {
    return false;
}

std::string TransformGizmoNode::getDescription() {
    return "Transform Gizmo";
}

TransformGizmoNode::TransformGizmoNode(const std::shared_ptr<Node> &parent) : Node(parent) {
    visibleInElementTree = false;
    visible = true;
}

void TransformGizmoNode::initArrows() {
    uint8_t i = 0;
    for (const auto &color : {"#FF0000", "#00FF00", "#0000FF"}) {
        translateArrows[i] = std::make_shared<ArrowMeshNode>(ldr_color_repo::getPureColor(color), shared_from_this());
        addChild(translateArrows[i]);
        ++i;
    }
    translateArrows[1]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));
    translateArrows[1]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)));
}
