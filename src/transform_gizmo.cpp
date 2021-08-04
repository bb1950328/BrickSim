#include "transform_gizmo.h"
#include "config.h"
#include "controller.h"
#include "helpers/util.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <magic_enum.hpp>
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

bricksim::Ray3 bricksim::transform_gizmo::TransformOperation::calculateAxisRay(int axis) {
    const auto arrowTransformation = gizmo.node->translate1dArrows[axis]->getAbsoluteTransformation();
    auto ray = Ray3({0, 0, 0}, {1, 0, 0});
    ray *= arrowTransformation;
    ray.origin = glm::vec3(glm::transpose(gizmo.node->getAbsoluteTransformation())[3]);
    return ray;
}

namespace bricksim::transform_gizmo {

    TransformGizmo::TransformGizmo(std::shared_ptr<graphics::Scene> scene) :
        scene(std::move(scene)),
        node(std::make_shared<TGNode>(this->scene->getRootNode())) {
        this->scene->getRootNode()->addChild(node);
        node->initElements();
        for (const auto& arrow: node->getChildren()) {
            arrow->layer = constants::TRANSFORM_GIZMO_LAYER;
        }
    }

    void TransformGizmo::update() {
        uoset_t<std::shared_ptr<etree::Node>>& selectedNodes = controller::getSelectedNodes();
        currentlySelectedNode = selectedNodes.size() == 1 ? *selectedNodes.begin() : nullptr;

        std::optional<glm::mat4> nowTransformation;
        PovState nowPovState;
        if (currentlySelectedNode != nullptr) {
            const static float configScale = config::get(config::GUI_SCALE) * config::get(config::TRANSFORM_GIZMO_SIZE);

            static glm::mat4 lastNodeAbsTransf;
            static glm::vec3 lastCameraPosLdu;

            const glm::mat4 nodeAbsTransf = currentlySelectedNode->getAbsoluteTransformation();
            glm::vec3 cameraPosLdu = glm::vec4(scene->getCamera()->getCameraPos(), 1.0f) * constants::OPENGL_TO_LDU;

            if (lastNodeAbsTransf != nodeAbsTransf || lastCameraPosLdu != cameraPosLdu) {
                lastNodeAbsTransf = nodeAbsTransf;
                lastCameraPosLdu = cameraPosLdu;

                const auto selectedNodeTransf = util::decomposeTransformationToStruct(glm::transpose(nodeAbsTransf));

                const auto cameraTargetDistance = glm::length(scene->getCamera()->getTargetPos() - scene->getCamera()->getCameraPos());
                nodePosition = selectedNodeTransf.translation;

                nowTransformation = glm::scale(glm::translate(nodePosition), glm::vec3(cameraTargetDistance / constants::LDU_TO_OPENGL_SCALE / 10 * configScale));
                if (controller::getTransformGizmoRotationState() == RotationState::SELECTED_ELEMENT) {
                    nodeRotation = selectedNodeTransf.orientationAsMat4();
                    nowTransformation.value() *= nodeRotation;
                } else {
                    nodeRotation = glm::mat4(1.0f);
                }
                node->setRelativeTransformation(glm::transpose(nowTransformation.value()));

                const glm::mat4 absTransf = node->getAbsoluteTransformation();
                int povState = 0;
                for (int i = 0; i < 3; ++i) {
                    const glm::vec3 pt = absTransf * axisDirectionOffsetVectors[i];
                    const float angle = util::getAngleBetweenThreePointsUnsigned(pt, absTransf[3], cameraPosLdu);
                    povState = povState << 1 | (angle < glm::radians(90.f));
                }
                nowPovState = static_cast<PovState>(povState);
                node->setPovState(nowPovState);
            }

            node->visible = true;
        } else {
            node->visible = false;
            nowTransformation = {};
        }
        if (nowTransformation != lastTransformation || (node->visible && nowPovState != lastState)) {
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
        switch (type) {
            case TRANSLATE_1D:
                currentTransformationOperation = std::make_unique<Translate1dOperation>(*this, glm::vec2(initialCursorPos), axis);
                break;
            case TRANSLATE_2D:
                currentTransformationOperation = std::make_unique<Translate2dOperation>(*this, glm::vec2(initialCursorPos), axis);
                break;
            case ROTATE:
                currentTransformationOperation = std::make_unique<RotateOperation>(*this, glm::vec2(initialCursorPos), axis);
                break;
            default: break;
        }
    }

    void TransformGizmo::updateCurrentDragDelta(glm::svec2 totalDragDelta) {
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
        visibleInElementTree = false;
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
        //todo i think this can be more efficient if all 8 possibilities are precalculated
        bool newX = (uint8_t)(povState) & (1 << 2);
        bool newY = (uint8_t)(povState) & (1 << 1);
        bool newZ = (uint8_t)(povState) & (1 << 0);

        const auto translationArrowX = glm::translate(glm::vec3(newX ? 0.0f : -1.0f, 0.0f, 0.0f));
        translate1dArrows[0]->setRelativeTransformation(glm::transpose(translationArrowX));

        const auto rotationArrowY = glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1));
        const auto translationArrowY = glm::translate(glm::vec3(newY ? 0.0f : -1.0f, 0.0f, 0.0f));
        translate1dArrows[1]->setRelativeTransformation(glm::transpose(rotationArrowY * translationArrowY));

        const auto rotationArrowZ = glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0));
        const auto translationArrowZ = glm::translate(glm::vec3(newZ ? 0.0f : -1.0f, 0.0f, 0.0f));
        translate1dArrows[2]->setRelativeTransformation(glm::transpose(rotationArrowZ * translationArrowZ));

        const auto torusScale = glm::scale(glm::vec3(1.3f));
        const auto move2dArrowScale = glm::scale(glm::vec3(0.6f));

        float angleTorusX = newZ ? (newY ? 0.0f : -90.0f) : (newY ? 90.0f : 180.0f);
        const auto rotateTorusX = glm::rotate(glm::radians(angleTorusX), glm::vec3(1, 0, 0));
        rotateQuarterTori[0]->setRelativeTransformation(torusScale * rotateTorusX);
        translate2dArrows[0]->setRelativeTransformation(move2dArrowScale * rotateTorusX);

        float angleTorusY = newX ? (newZ ? 0.0f : 90.0f) : (newZ ? -90.0f : 180.0f);
        const auto rotateTorusY1 = glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1));
        const auto rotateTorusY2 = glm::rotate(glm::radians(angleTorusY), glm::vec3(1, 0, 0));
        rotateQuarterTori[1]->setRelativeTransformation(torusScale * rotateTorusY2 * rotateTorusY1);
        translate2dArrows[1]->setRelativeTransformation(move2dArrowScale * rotateTorusY2 * rotateTorusY1);

        float angleTorusZ = newX ? (newY ? 90.0f : 180.0f) : (newY ? 0.0f : -90.0f);
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
        auto& triangleData = mesh->getTriangleData(color);
        auto baseIndex = triangleData.getVertexCount();
        /*0*/ triangleData.addRawVertex(mesh::TriangleVertex{{0.0f, 0.0f, 0.0f}, normal});
        /*1*/ triangleData.addRawVertex(mesh::TriangleVertex{{0.0f, outerSideLength, 0.0f}, normal});
        /*2*/ triangleData.addRawVertex(mesh::TriangleVertex{{0.0f, outerSideLength - tipLength, tipWidth}, normal});
        /*3*/ triangleData.addRawVertex(mesh::TriangleVertex{{0.0f, outerSideLength - tipLength, lineWidth}, normal});
        /*4*/ triangleData.addRawVertex(mesh::TriangleVertex{{0.0f, lineWidth, lineWidth}, normal});
        /*5*/ triangleData.addRawVertex(mesh::TriangleVertex{{0.0f, lineWidth, outerSideLength - tipLength}, normal});
        /*6*/ triangleData.addRawVertex(mesh::TriangleVertex{{0.0f, tipWidth, outerSideLength - tipLength}, normal});
        /*7*/ triangleData.addRawVertex(mesh::TriangleVertex{{0.0f, 0.0f, outerSideLength}, normal});

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
            triangleData.addRawIndex(baseIndex + indices[3 * i + 0]);
            triangleData.addRawIndex(baseIndex + indices[3 * i + 1]);
            triangleData.addRawIndex(baseIndex + indices[3 * i + 2]);

            //adding the same but with the other winding order so it's visible from both sides
            triangleData.addRawIndex(baseIndex + indices[3 * i + 0]);
            triangleData.addRawIndex(baseIndex + indices[3 * i + 2]);
            triangleData.addRawIndex(baseIndex + indices[3 * i + 1]);
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
        TransformOperation(gizmo, startMousePos),
        transformRay(calculateAxisRay(axis)),
        startPointOnTransformRay(getClosestPointOnTransformRay(startMousePos)) {
    }

    glm::vec3 Translate1dOperation::getClosestPointOnTransformRay(const glm::svec2& mouseCoords) {
        Ray3 currentMouseRay = gizmo.scene->screenCoordinatesToWorldRay(mouseCoords);
        currentMouseRay *= constants::OPENGL_TO_LDU;
        return util::closestLineBetweenTwoRays(currentMouseRay, transformRay).pointOnB;
    }

    void TransformOperation::cancel() {
        gizmo.currentlySelectedNode->setRelativeTransformation(glm::transpose(startNodeRelTransformation));
        gizmo.node->setRelativeTransformation(glm::transpose(startGizmoRelTransformation));
    }

    TransformOperation::TransformOperation(TransformGizmo& gizmo, const glm::vec2& startMousePos) :
        gizmo(gizmo),
        startNodeRelTransformation(glm::transpose(gizmo.currentlySelectedNode->getRelativeTransformation())),
        startGizmoRelTransformation(glm::transpose(gizmo.node->getRelativeTransformation())),
        startMousePos(startMousePos) {}

    TransformOperation::~TransformOperation() = default;

    Translate2dOperation::Translate2dOperation(TransformGizmo& gizmo, const glm::vec2& startMousePos, int axis) :
        TransformOperation(gizmo, startMousePos),
        planeNormal(calculateAxisRay(axis)) {
        Ray3 startMouseRay = gizmo.scene->screenCoordinatesToWorldRay(startMousePos);
        startMouseRay *= constants::OPENGL_TO_LDU;
        startPointOnPlane = util::rayPlaneIntersection(startMouseRay, planeNormal).value();
        this->planeNormal = Ray3(startPointOnPlane, planeNormal.direction);
    }

    void Translate2dOperation::update(const glm::vec2& mouseDelta) {
        Ray3 currentMouseRay = gizmo.scene->screenCoordinatesToWorldRay(startMousePos + mouseDelta);
        currentMouseRay *= constants::OPENGL_TO_LDU;
        const auto currentPointOnPlane = util::rayPlaneIntersection(currentMouseRay, planeNormal);
        if (currentPointOnPlane) {
            const glm::vec3 delta = currentPointOnPlane.value() - startPointOnPlane;
            const auto translation = glm::translate(delta);
            gizmo.currentlySelectedNode->setRelativeTransformation(glm::transpose(translation * startNodeRelTransformation));
            gizmo.node->setRelativeTransformation(glm::transpose(translation * startGizmoRelTransformation));
        }
    }

    constexpr TransformType Translate2dOperation::getType() {
        return TRANSLATE_2D;
    }

    RotateOperation::RotateOperation(TransformGizmo& gizmo, const glm::vec2& startMousePos, int axis) :
        TransformOperation(gizmo, startMousePos), axis(calculateAxisRay(axis)) {
        Ray3 startMouseRay = gizmo.scene->screenCoordinatesToWorldRay(startMousePos);
        startMouseRay *= constants::OPENGL_TO_LDU;
        startPointOnPlane = util::rayPlaneIntersection(startMouseRay, this->axis).value();

        startNodeTransfDecomposed = util::decomposeTransformationToStruct(startNodeRelTransformation);
        startGizmoTransfDecomposed = util::decomposeTransformationToStruct(startGizmoRelTransformation);
    }

    void RotateOperation::update(const glm::vec2& mouseDelta) {
        Ray3 currentMouseRay = gizmo.scene->screenCoordinatesToWorldRay(startMousePos + mouseDelta);
        currentMouseRay *= constants::OPENGL_TO_LDU;
        const auto currentPointOnPlane = util::rayPlaneIntersection(currentMouseRay, this->axis).value();
        const float totalRotationAngle = util::getAngleBetweenThreePointsSigned(startPointOnPlane, axis.origin, currentPointOnPlane, axis.direction);
        const auto rotation = glm::rotate(totalRotationAngle, glm::normalize(axis.direction));

        const glm::mat4 newNodeTransf = startNodeTransfDecomposed.translationAsMat4() * rotation * startNodeTransfDecomposed.orientationAsMat4() * startNodeTransfDecomposed.scaleAsMat4();
        const glm::mat4 newGizmoTransf = startGizmoTransfDecomposed.translationAsMat4() * rotation * startGizmoTransfDecomposed.orientationAsMat4() * startGizmoTransfDecomposed.scaleAsMat4();
        gizmo.currentlySelectedNode->setRelativeTransformation(glm::transpose(newNodeTransf));
        gizmo.node->setRelativeTransformation(glm::transpose(newGizmoTransf));
        gizmo.scene->elementTreeChanged();
    }

    constexpr TransformType RotateOperation::getType() {
        return ROTATE;
    }
}