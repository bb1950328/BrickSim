#include "transform_gizmo.h"
#include "config.h"
#include "controller.h"
#include "helpers/util.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

std::array<glm::vec4, 3> axisDirectionOffsetVectors = {
        glm::vec4(1, 0, 0, 1),
        glm::vec4(0, 1, 0, 1),
        glm::vec4(0, 0, 1, 1),
};
std::array<glm::vec4, 3> axisDirectionVectors = {
        glm::vec4(1, 0, 0, 0),
        glm::vec4(0, 1, 0, 0),
        glm::vec4(0, 0, 1, 0),
};

namespace bricksim::transform_gizmo {

    TransformGizmo::TransformGizmo(std::shared_ptr<graphics::Scene> scene) :
        scene(std::move(scene)) {
        node = std::make_shared<TGNode>(this->scene->getRootNode());
        this->scene->getRootNode()->addChild(node);
        node->initElements();
        for (const auto& arrow: node->getChildren()) {
            arrow->layer = constants::TRANSFORM_GIZMO_LAYER;
        }
    }

    void TransformGizmo::update() {
        currentlySelectedNode = getFirstSelectedNode(scene->getRootNode());
        std::optional<glm::mat4> nowTransformation;
        PovState nowPovState;
        if (currentlySelectedNode != nullptr) {
            const static float configScale = config::get(config::GUI_SCALE) * config::get(config::TRANSFORM_GIZMO_SIZE);

            const auto selectedNodeTransf = util::decomposeTransformationToStruct(glm::transpose(currentlySelectedNode->getAbsoluteTransformation()));

            glm::vec3 cameraPosLdu = glm::vec4(scene->getCamera()->getCameraPos(), 1.0f) * constants::OPENGL_TO_LDU;
            glm::vec3 direction = glm::normalize(selectedNodeTransf.translation - cameraPosLdu);
            const auto cameraTargetDistance = glm::length(scene->getCamera()->getTargetPos() - scene->getCamera()->getCameraPos());
            nodePosition = selectedNodeTransf.translation;

            nowTransformation = glm::scale(glm::translate(nodePosition), glm::vec3(cameraTargetDistance / constants::LDU_TO_OPENGL_SCALE / 10 * configScale));
            if (controller::getTransformGizmoRotationState() == RotationState::SELECTED_ELEMENT) {
                nodeRotation = selectedNodeTransf.orientationAsMat4();
                nowTransformation.value() *= nodeRotation;
                direction = glm::vec4(direction, 0.0f) * nowTransformation.value();
            } else {
                nodeRotation = glm::mat4(1.0f);
            }
            node->setRelativeTransformation(glm::transpose(nowTransformation.value()));

            float yaw = std::atan2(direction.x, direction.z);
            float pitch = std::atan2(direction.y, direction.y);

            if (yaw <= 0) {
                if (yaw <= -0.5 * M_PI) {
                    nowPovState = pitch < 0
                                          ? PovState::XPOS_YPOS_ZPOS
                                          : PovState::XPOS_YNEG_ZPOS;
                } else {
                    nowPovState = pitch < 0
                                          ? PovState::XPOS_YPOS_ZNEG
                                          : PovState::XPOS_YNEG_ZNEG;
                }
            } else {
                if (yaw <= 0.5 * M_PI) {
                    nowPovState = pitch < 0
                                          ? PovState::XNEG_YPOS_ZNEG
                                          : PovState::XNEG_YNEG_ZNEG;
                } else {
                    nowPovState = pitch < 0
                                          ? PovState::XNEG_YPOS_ZPOS
                                          : PovState::XNEG_YNEG_ZPOS;
                }
            }

            node->setPovState(nowPovState);

            node->visible = true;
        } else {
            node->visible = false;
            nowTransformation = {};
        }
        if (nowTransformation != lastTransformation || nowPovState != lastState) {
            scene->elementTreeChanged();
            lastTransformation = nowTransformation;
            lastState = nowPovState;
        }
    }

    TransformGizmo::~TransformGizmo() {
        scene->getRootNode()->removeChild(node);
    }

    bool TransformGizmo::ownsNode(const std::shared_ptr<etree::Node>& node_) {
        return node_->isChildOf(this->node);
    }

    void TransformGizmo::startDrag(std::shared_ptr<etree::Node>& draggedNode, const glm::svec2 initialCursorPos) {
        auto [type, axis] = node->getTransformTypeAndAxis(draggedNode);
        spdlog::debug("start transform gizmo drag (type={}, axis={}, initialCursorPos={})", type, axis, glm::to_string(initialCursorPos));
        if (type == TRANSLATE_1D) {
            currentTransformationOperation = std::make_unique<Translate1dOperation>(*this, glm::vec2(initialCursorPos), axis);
        }
    }

    void TransformGizmo::updateCurrentDragDelta(glm::svec2 totalDragDelta) {
        spdlog::debug("update drag delta: {}", glm::to_string(totalDragDelta));
        currentTransformationOperation->update(totalDragDelta);
    }

    void TransformGizmo::endDrag() {
        currentTransformationOperation = nullptr;
    }

    bool TGNode::isTransformationUserEditable() const {
        return false;
    }

    bool TGNode::isDisplayNameUserEditable() const {
        return false;
    }

    TGNode::TGNode(const std::shared_ptr<etree::Node>& parent) :
        etree::Node(parent) {
        visibleInElementTree = true;//todo change this back to false when debugging is finished
        visible = true;
        displayName = "Transform Gizmo";
        povState = PovState::XNEG_YNEG_ZNEG;
    }

    void TGNode::initElements() {
        uint8_t i = 0;
        for (const auto& colorCode: {"#FF0000", "#00FF00", "#0000FF"}) {
            const auto& colorRef = ldr::color_repo::getPureColor(colorCode);
            translate1dArrows[i] = std::make_shared<mesh::generated::ArrowNode>(colorRef, shared_from_this());
            addChild(translate1dArrows[i]);

            rotateQuarterTori[i] = std::make_shared<mesh::generated::QuarterTorusNode>(colorRef, shared_from_this());
            addChild(rotateQuarterTori[i]);

            ++i;
        }

        i = 0;
        for (const auto& colorCode: {"#00ffff", "#ff00ff", "#ffff00"}) {
            const auto& colorRef = ldr::color_repo::getPureColor(colorCode);

            translate2dArrows[i] = std::make_shared<TG2DArrowNode>(colorRef, shared_from_this());
            addChild(translate2dArrows[i]);

            ++i;
        }

        translate1dArrows[1]->setRelativeTransformation(glm::rotate(glm::radians(-90.0f), glm::vec3(0, 0, 1)));
        translate1dArrows[2]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));

        rotateQuarterTori[0]->setRelativeTransformation(glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)));
        rotateQuarterTori[1]->setRelativeTransformation(glm::rotate(glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)), glm::radians(90.0f), glm::vec3(0, 0, 1)));
        rotateQuarterTori[2]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));

        centerBall = std::make_shared<mesh::generated::UVSphereNode>(ldr::color_repo::getPureColor("#ffffff"), shared_from_this());
        centerBall->setRelativeTransformation(glm::scale(glm::vec3(mesh::generated::ArrowNode::LINE_RADIUS * 4)));
        addChild(centerBall);
    }

    PovState TGNode::getPovState() const {
        return povState;
    }

    void TGNode::setPovState(PovState newState) {
        if (povState != newState) {
            povState = newState;
            updateTransformations();
        }
    }

    void TGNode::updateTransformations() {
        bool newX = (uint8_t)(povState) & (1 << 2);
        bool newY = (uint8_t)(povState) & (1 << 1);
        bool newZ = (uint8_t)(povState) & (1 << 0);

        const auto translationArrowX = glm::translate(glm::vec3(newX ? 0.0f : -1.0f, 0.0f, 0.0f));
        translate1dArrows[0]->setRelativeTransformation(glm::transpose(translationArrowX));

        const auto rotationArrowY = glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1));
        const auto translationArrowY = glm::translate(glm::vec3(newZ ? 0.0f : -1.0f, 0.0f, 0.0f));
        translate1dArrows[1]->setRelativeTransformation(glm::transpose(rotationArrowY * translationArrowY));

        const auto rotationArrowZ = glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0));
        const auto translationArrowZ = glm::translate(glm::vec3(newY ? 0.0f : -1.0f, 0.0f, 0.0f));
        translate1dArrows[2]->setRelativeTransformation(glm::transpose(rotationArrowZ * translationArrowZ));

        const auto torusScale = glm::scale(glm::vec3(1.3f));
        const auto move2dArrowScale = glm::scale(glm::vec3(0.6f));

        float angleTorusX = newY ? (newZ ? 0.0f : -90.0f) : (newZ ? 90.0f : 180.0f);
        const auto rotateTorusX = glm::rotate(glm::radians(angleTorusX), glm::vec3(1, 0, 0));
        rotateQuarterTori[0]->setRelativeTransformation(torusScale * rotateTorusX);
        translate2dArrows[0]->setRelativeTransformation(move2dArrowScale * rotateTorusX);

        float angleTorusY = newX ? (newY ? 0.0f : 90.0f) : (newY ? -90.0f : 180.0f);
        const auto rotateTorusY1 = glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1));
        const auto rotateTorusY2 = glm::rotate(glm::radians(angleTorusY), glm::vec3(1, 0, 0));
        rotateQuarterTori[1]->setRelativeTransformation(torusScale * rotateTorusY2 * rotateTorusY1);
        translate2dArrows[1]->setRelativeTransformation(move2dArrowScale * rotateTorusY2 * rotateTorusY1);

        float angleTorusZ = newX ? (newZ ? 90.0f : 180.0f) : (newZ ? 0.0f : -90.0f);
        const auto rotateTorusZ1 = glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0));
        const auto rotateTorusZ2 = glm::rotate(glm::radians(angleTorusZ), glm::vec3(1, 0, 0));
        rotateQuarterTori[2]->setRelativeTransformation(torusScale * rotateTorusZ2 * rotateTorusZ1);
        translate2dArrows[2]->setRelativeTransformation(move2dArrowScale * rotateTorusZ2 * rotateTorusZ1);
    }

    std::string TGNode::getDescription() {
        return "Transform Gizmo";
    }

    std::pair<TransformType, int> TGNode::getTransformTypeAndAxis(std::shared_ptr<etree::Node>& node) {
        if (node == centerBall) {
            return {TRANSLATE_3D, 0};
        }
        for (int i = 0; i < 3; ++i) {
            if (node == translate1dArrows[i]) {
                return {TRANSLATE_1D, i};
            } else if (node == translate2dArrows[i]) {
                return {TRANSLATE_2D, i};
            } else if (node == rotateQuarterTori[i]) {
                return {ROTATE, i};
            }
        }
        return {NONE, 0};
    }

    mesh_identifier_t TG2DArrowNode::getMeshIdentifier() const {
        return constants::MESH_ID_TRANSFORM_GIZMO_2D_ARROW;
    }

    void TG2DArrowNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed) {
        // 1
        // | \
        // |  \
        // |   \
        // | 3--2     6              ^ y
        // | |        | \            |
        // | 4--------5   \          |     z
        // 0---------------7         x----->
        constexpr float outerSideLength = 1.0f;
        constexpr float lineWidth = 0.25f;
        constexpr float tipWidth = 0.4f;
        constexpr float tipLength = 0.4f;
        constexpr glm::vec3 normal(1, 0, 0);

        auto color = ldr::color_repo::getInstanceDummyColor();
        auto baseIndex = mesh->getNextVertexIndex(color);
        /*0*/ mesh->addRawTriangleVertex(color, mesh::TriangleVertex{{0.0f, 0.0f, 0.0f, 1.0f}, normal});
        /*1*/ mesh->addRawTriangleVertex(color, mesh::TriangleVertex{{0.0f, outerSideLength, 0.0f, 1.0f}, normal});
        /*2*/ mesh->addRawTriangleVertex(color, mesh::TriangleVertex{{0.0f, outerSideLength - tipLength, tipWidth, 1.0f}, normal});
        /*3*/ mesh->addRawTriangleVertex(color, mesh::TriangleVertex{{0.0f, outerSideLength - tipLength, lineWidth, 1.0f}, normal});
        /*4*/ mesh->addRawTriangleVertex(color, mesh::TriangleVertex{{0.0f, lineWidth, lineWidth, 1.0f}, normal});
        /*5*/ mesh->addRawTriangleVertex(color, mesh::TriangleVertex{{0.0f, lineWidth, outerSideLength - tipLength, 1.0f}, normal});
        /*6*/ mesh->addRawTriangleVertex(color, mesh::TriangleVertex{{0.0f, tipWidth, outerSideLength - tipLength, 1.0f}, normal});
        /*7*/ mesh->addRawTriangleVertex(color, mesh::TriangleVertex{{0.0f, 0.0f, outerSideLength, 1.0f}, normal});

        constexpr auto triangleCount = 6;

        constexpr std::array<unsigned int, 3 * triangleCount> indices = {
                0,
                4,
                1,
                4,
                3,
                1,
                3,
                2,
                1,
                0,
                7,
                4,
                4,
                7,
                5,
                5,
                7,
                6,
        };

        for (int i = 0; i < triangleCount; ++i) {
            mesh->addRawTriangleIndex(color, baseIndex + indices[3 * i + 0]);
            mesh->addRawTriangleIndex(color, baseIndex + indices[3 * i + 1]);
            mesh->addRawTriangleIndex(color, baseIndex + indices[3 * i + 2]);

            //adding the same but with the other winding order so it's visible from both sides
            mesh->addRawTriangleIndex(color, baseIndex + indices[3 * i + 0]);
            mesh->addRawTriangleIndex(color, baseIndex + indices[3 * i + 2]);
            mesh->addRawTriangleIndex(color, baseIndex + indices[3 * i + 1]);
        }
    }

    TG2DArrowNode::TG2DArrowNode(const ldr::ColorReference& color, const std::shared_ptr<Node>& parent) :
        MeshNode(color, parent) {
        displayName = "transform gizmo 2D move arrow";
    }

    bool TG2DArrowNode::isDisplayNameUserEditable() const {
        return false;
    }

    std::string TG2DArrowNode::getDescription() {
        return "Transform Gizmo 2D Arrow Node";
    }

    void Translate1dOperation::update(const glm::vec2& mouseDelta) {
        const auto currentPointOnTransformRay = getClosestPointOnTransformRay(startMousePos + mouseDelta);
        auto translation = glm::translate(currentPointOnTransformRay - startPointOnTransformRay);
        const auto newSelectedNodeTransformation = translation * startNodeRelTransformation;
        const auto newGizmoTransformation = translation * startGizmoRelTransformation;
        gizmo.currentlySelectedNode->setRelativeTransformation(glm::transpose(newSelectedNodeTransformation));
        gizmo.node->setRelativeTransformation(glm::transpose(newGizmoTransformation));
    }

    constexpr TransformType Translate1dOperation::getType() {
        return TRANSLATE_1D;
    }

    Translate1dOperation::Translate1dOperation(TransformGizmo& gizmo, const glm::vec2& startMousePos, int axis) :
        TransformOperation(gizmo),
        transformRay(calculateTransformRay(axis)),
        startPointOnTransformRay(getClosestPointOnTransformRay(startMousePos)),
        startMousePos(startMousePos),
        startNodeRelTransformation(glm::transpose(gizmo.currentlySelectedNode->getRelativeTransformation())),
        startGizmoRelTransformation(glm::transpose(gizmo.node->getRelativeTransformation())) {
    }

    glm::vec3 Translate1dOperation::getClosestPointOnTransformRay(const glm::svec2& mouseCoords) {
        Ray3 currentMouseRay = gizmo.scene->screenCoordinatesToWorldRay(mouseCoords);
        currentMouseRay *= constants::OPENGL_TO_LDU;
        return util::closestLineBetweenTwoRays(currentMouseRay, transformRay).pointOnB;
    }

    Ray3 Translate1dOperation::calculateTransformRay(int axis) {
        const auto arrowTransformation = gizmo.node->translate1dArrows[axis]->getAbsoluteTransformation();
        auto ray = Ray3({0, 0, 0}, {1, 0, 0});
        ray *= arrowTransformation;
        return ray;
    }

    void Translate1dOperation::cancel() {
        gizmo.currentlySelectedNode->setRelativeTransformation(startNodeRelTransformation);
        gizmo.node->setRelativeTransformation(startGizmoRelTransformation);
    }

    TransformOperation::TransformOperation(TransformGizmo& gizmo) :
        gizmo(gizmo) {}

    TransformOperation::~TransformOperation() = default;
}