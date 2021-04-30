

#include "transform_gizmo.h"

#include <utility>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>

const glm::vec4 ARROW_DIRECTIONS[3] = {
        {1.0f, 0.0f,  0.0f,  1.0f},
        {0.0f, 0.0f,  -1.0f, 1.0f},
        {0.0f, -1.0f, 0.0f,  1.0f},
};

TransformGizmo::TransformGizmo(std::shared_ptr<Scene> scene) : scene(std::move(scene)) {
    transformGizmoNode = std::make_shared<TransformGizmoNode>(this->scene->getRootNode());
    this->scene->getRootNode()->addChild(transformGizmoNode);
    transformGizmoNode->initArrows();
    transformGizmoNode->setRelativeTransformation(glm::scale(glm::vec3(100.0f, 100.0f, 100.0f)));
    for (const auto &arrow : transformGizmoNode->getChildren()) {
        arrow->layer = constants::TRANSFORM_GIZMO_LAYER;
    }
    this->scene->elementTreeChanged();
}

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

    std::vector<TriangleVertex> baseVertices{
            backCoverVertex,
            lineBackVertex,
            lineBeforeTipVertex,
            ringInnerVertex,
            ringOuterVertex,
            tipEdgeVertex,
            tipVertex,
    };
    unsigned long baseVertexCount = baseVertices.size();

    auto color = ldr_color_repo::getInstanceDummyColor();

    unsigned int firstIndex = mesh->getNextVertexIndex(color);

    for (uint16_t i = 0; i < numCorners; ++i) {
        auto rotationMatrix = glm::rotate(2 * M_PIf32 * i / numCorners, glm::vec3(1.0f, 0.0f, 0.0f));
        for (const auto &vertex : baseVertices) {
            mesh->addRawTriangleVertex(color, {vertex.position * rotationMatrix, glm::vec4(vertex.normal, 0.0f) * rotationMatrix});
        }
    }

    for (const auto &item : mesh->getVerticesList(color)) {
        std::cout << glm::to_string(item.position) << "\t" << glm::to_string(item.normal) << std::endl;
    }

    //special treatment for back circle
    for (uint16_t i1 = 1, i2 = 2; i2 < numCorners; ++i2, ++i1) {
        mesh->addRawTriangleIndex(color, firstIndex);
        mesh->addRawTriangleIndex(color, firstIndex + baseVertexCount * i1);
        mesh->addRawTriangleIndex(color, firstIndex + baseVertexCount * i2);
    }

    //       vtx0     vtx1
    // i1 -----+-------+
    //         |  \    |
    //         |    \  |
    // i2 -----+-------+
    for (uint16_t i0 = 0, i1 = 1; i0 < numCorners; ++i0, i1=(i0 + 1) % numCorners) {
        std::cout << i0 << "\t" << i1 << std::endl;
        for (int vtx0 = 0, vtx1 = 1; vtx1 < baseVertexCount; ++vtx0, ++vtx1) {
            mesh->addRawTriangleIndex(color, firstIndex + i0 * baseVertexCount + vtx0);
            mesh->addRawTriangleIndex(color, firstIndex + i0 * baseVertexCount + vtx1);
            mesh->addRawTriangleIndex(color, firstIndex + i1 * baseVertexCount + vtx1);

            mesh->addRawTriangleIndex(color, firstIndex + i1 * baseVertexCount + vtx1);
            mesh->addRawTriangleIndex(color, firstIndex + i1 * baseVertexCount + vtx0);
            mesh->addRawTriangleIndex(color, firstIndex + i0 * baseVertexCount + vtx0);
        }
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

TransformGizmoNode::TransformGizmoNode(const std::shared_ptr<Node> &parent) : Node(parent) {
    visibleInElementTree = true;
    visible = true;
    displayName = "Transform Gizmo";
}

void TransformGizmoNode::initArrows() {
    uint8_t i = 0;
    for (const auto &color : {"#FF0000", "#00FF00", "#0000FF"}) {
        translateArrows[i] = std::make_shared<ArrowMeshNode>(ldr_color_repo::getPureColor(color), shared_from_this());
        addChild(translateArrows[i]);
        ++i;
    }
    translateArrows[1]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));
    translateArrows[2]->setRelativeTransformation(glm::rotate(glm::radians(-90.0f), glm::vec3(0, 0, 1)));
}
