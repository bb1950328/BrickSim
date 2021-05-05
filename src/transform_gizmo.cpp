#include "transform_gizmo.h"
#include "controller.h"

#include <utility>
#include <algorithm>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>

namespace transform_gizmo {

    TransformGizmo::TransformGizmo(std::shared_ptr<Scene> scene) : scene(std::move(scene)) {
        node = std::make_shared<TGNode>(this->scene->getRootNode());
        this->scene->getRootNode()->addChild(node);
        node->initElements();
        for (const auto &arrow : node->getChildren()) {
            arrow->layer = constants::TRANSFORM_GIZMO_LAYER;
        }
    }

    void TransformGizmo::update() {
        auto selectedNode = getFirstSelectedNode(scene->getRootNode());
        std::optional<glm::mat4> nowTransformation;
        PovState nowPovState;
        if (selectedNode != nullptr) {
            auto nodeTransformation = glm::transpose(selectedNode->getAbsoluteTransformation());
            const static float configScale = config::getDouble(config::GUI_SCALE) * config::getDouble(config::TRANSFORM_GIZMO_SIZE);

            glm::quat rotation;
            glm::vec3 skewIgnore;
            glm::vec4 perspectiveIgnore;
            glm::vec3 translation;
            glm::vec3 scaleIgnore;
            glm::decompose(nodeTransformation, scaleIgnore, rotation, translation, skewIgnore, perspectiveIgnore);

            glm::vec3 cameraPosLdu = glm::vec4(scene->getCamera()->getCameraPos(), 1.0f) * constants::OPENGL_TO_LDU;
            glm::vec3 direction = glm::normalize(translation - cameraPosLdu);
            glm::vec3 gizmoPos = cameraPosLdu + direction * 5.0f * configScale;

            nowTransformation = glm::translate(gizmoPos);
            if (controller::getTransformGizmoRotationState() == RotationState::SELECTED_ELEMENT) {
                nowTransformation.value() *= glm::toMat4(rotation);
                direction = glm::vec4(direction, 0.0f) * nowTransformation.value();
            }
            node->setRelativeTransformation(glm::transpose(nowTransformation.value()));

            float yaw = std::atan2(direction.x, direction.z);
            float pitch = std::atan2(direction.y, direction.y);


            if (yaw <= 0) {
                if (yaw <= -0.5 * M_PI) {
                    nowPovState = pitch < 0
                                  ? PovState::XPOS_YPOS_ZPOS
                                  : PovState::XPOS_YPOS_ZNEG;
                } else {
                    nowPovState = pitch < 0
                                  ? PovState::XPOS_YNEG_ZPOS
                                  : PovState::XPOS_YNEG_ZNEG;
                }
            } else {
                if (yaw <= 0.5 * M_PI) {
                    nowPovState = pitch < 0
                                  ? PovState::XNEG_YNEG_ZPOS
                                  : PovState::XNEG_YNEG_ZNEG;
                } else {
                    nowPovState = pitch < 0
                                  ? PovState::XNEG_YPOS_ZPOS
                                  : PovState::XNEG_YPOS_ZNEG;
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

    bool TGNode::isTransformationUserEditable() const {
        return false;
    }

    bool TGNode::isDisplayNameUserEditable() const {
        return false;
    }

    TGNode::TGNode(const std::shared_ptr<etree::Node> &parent) : etree::Node(parent) {
        visibleInElementTree = false;
        visible = true;
        displayName = "Transform Gizmo";
        povState = PovState::XNEG_YNEG_ZNEG;
    }

    void TGNode::initElements() {
        uint8_t i = 0;
        for (const auto &colorCode : {"#FF0000", "#00FF00", "#0000FF"}) {
            const auto &colorRef = ldr_color_repo::getPureColor(colorCode);
            translateArrows[i] = std::make_shared<generated_mesh::ArrowNode>(colorRef, shared_from_this());
            addChild(translateArrows[i]);

            rotateTori[i] = std::make_shared<generated_mesh::QuarterTorusNode>(colorRef, shared_from_this());
            addChild(rotateTori[i]);

            ++i;
        }

        i=0;
        for (const auto &colorCode: {"#ffff00", "#ff00ff", "#00ffff"}) {
            const auto &colorRef = ldr_color_repo::getPureColor(colorCode);

            move2dArrows[i] = std::make_shared<TG2DArrowNode>(colorRef, shared_from_this());
            addChild(move2dArrows[i]);

            ++i;
        }

        translateArrows[1]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));
        translateArrows[2]->setRelativeTransformation(glm::rotate(glm::radians(-90.0f), glm::vec3(0, 0, 1)));

        rotateTori[0]->setRelativeTransformation(glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)));
        rotateTori[1]->setRelativeTransformation(glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0)));
        rotateTori[2]->setRelativeTransformation(glm::rotate(glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)), glm::radians(90.0f), glm::vec3(0, 0, 1)));

        move2dArrows[0]->setRelativeTransformation(glm::translate(glm::vec3(1.0f, 0, 0)));
        move2dArrows[1]->setRelativeTransformation(glm::translate(glm::vec3(2.0f, 0, 0)));
        move2dArrows[2]->setRelativeTransformation(glm::translate(glm::vec3(3.0f, 0, 0)));

        centerBall = std::make_shared<generated_mesh::UVSphereNode>(ldr_color_repo::getPureColor("#ffffff"), shared_from_this());
        centerBall->setRelativeTransformation(glm::scale(glm::vec3(generated_mesh::ArrowNode::LINE_RADIUS * 4)));
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
        bool newX = (uint8_t) (povState) & (1 << 2);
        bool newY = (uint8_t) (povState) & (1 << 1);
        bool newZ = (uint8_t) (povState) & (1 << 0);

        const auto translationArrowX = glm::translate(glm::vec3(newX ? 0.0f : -1.0f, 0.0f, 0.0f));
        translateArrows[0]->setRelativeTransformation(glm::transpose(translationArrowX));

        const auto rotationArrowY = glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0));
        const auto translationArrowY = glm::translate(glm::vec3(newY ? 0.0f : -1.0f, 0.0f, 0.0f));
        translateArrows[1]->setRelativeTransformation(glm::transpose(rotationArrowY * translationArrowY));

        const auto rotationArrowZ = glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1));
        const auto translationArrowZ = glm::translate(glm::vec3(newZ ? 0.0f : -1.0f, 0.0f, 0.0f));
        translateArrows[2]->setRelativeTransformation(glm::transpose(rotationArrowZ * translationArrowZ));

        const auto torusScale = glm::scale(glm::vec3(1.3f));

        float angleTorusX = newY ? (newZ ? -90.0f : 180.0f) : (newZ ? 0.0f : 90.0f);
        const auto rotateTorusX = glm::rotate(glm::radians(angleTorusX), glm::vec3(1, 0, 0));
        rotateTori[0]->setRelativeTransformation(torusScale * rotateTorusX);

        float angleTorusY = newX ? (newZ ? 0.0f : 90.0f) : (newZ ? -90.0f : 180.0f);
        const auto rotateTorusY1 = glm::rotate(glm::radians(90.0f), glm::vec3(0, 1, 0));
        const auto rotateTorusY2 = glm::rotate(glm::radians(angleTorusY), glm::vec3(1, 0, 0));
        rotateTori[1]->setRelativeTransformation(torusScale * rotateTorusY2 * rotateTorusY1);

        float angleTorusZ = newX ? (newY ? -90.0f : 0.0f) : (newY ? 180.0f : 90.0f);
        const auto rotateTorusZ1 = glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1));
        const auto rotateTorusZ2 = glm::rotate(glm::radians(angleTorusZ), glm::vec3(1, 0, 0));
        rotateTori[2]->setRelativeTransformation(torusScale * rotateTorusZ2 * rotateTorusZ1);
    }

    mesh_identifier_t TG2DArrowNode::getMeshIdentifier() const {
        return constants::MESH_ID_TRANSFORM_GIZMO_2D_ARROW;
    }

    void TG2DArrowNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {
        // 1
        // | \
        // |  \
        // |   \
        // | 3--2     6              ^ y
        // | |        | \            |
        // | 4--------5   \          |     x
        // 0---------------7         z----->
        constexpr float outerSideLength = 1.0f;
        constexpr float lineWidth = 0.1f;
        constexpr float tipWidth = 0.2f;
        constexpr float tipLength = 0.2f;
        constexpr glm::vec3 normal(0, 0, 1);

        auto color = ldr_color_repo::getInstanceDummyColor();
        auto baseIndex = mesh->getNextVertexIndex(color);
        /*0*/mesh->addRawTriangleVertex(color, TriangleVertex{{0.0f, 0.0f, 0.0f, 1.0f}, normal});
        /*1*/mesh->addRawTriangleVertex(color, TriangleVertex{{0.0f, outerSideLength, 0.0f, 1.0f}, normal});
        /*2*/mesh->addRawTriangleVertex(color, TriangleVertex{{tipWidth, outerSideLength - tipLength, 0.0f, 1.0f}, normal});
        /*3*/mesh->addRawTriangleVertex(color, TriangleVertex{{lineWidth, outerSideLength - tipLength, 0.0f, 1.0f}, normal});
        /*4*/mesh->addRawTriangleVertex(color, TriangleVertex{{lineWidth, lineWidth, 0.0f, 1.0f}, normal});
        /*5*/mesh->addRawTriangleVertex(color, TriangleVertex{{outerSideLength - tipLength, lineWidth, 0.0f, 1.0f}, normal});
        /*6*/mesh->addRawTriangleVertex(color, TriangleVertex{{outerSideLength - tipLength, tipWidth, 0.0f, 1.0f}, normal});
        /*7*/mesh->addRawTriangleVertex(color, TriangleVertex{{outerSideLength, 0.0f, 0.0f, 1.0f}, normal});

        constexpr std::array<unsigned int, 3 * 6> indices = {
                0, 4, 1,
                4, 3, 1,
                3, 2, 1,
                0, 7, 4,
                4, 7, 5,
                5, 7, 6,
        };

        for (const auto &idx : indices) {
            mesh->addRawTriangleIndex(color, baseIndex + idx);
        }
    }

    TG2DArrowNode::TG2DArrowNode(const LdrColorReference &color, const std::shared_ptr<Node> &parent) : MeshNode(color, parent) {
        displayName = "transform gizmo 2D move arrow";
    }

    bool TG2DArrowNode::isDisplayNameUserEditable() const {
        return false;
    }
}