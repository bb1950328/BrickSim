#include "orientation_cube.h"
#include "../constant_data/resources.h"
#include "../controller.h"
#include <magic_enum.hpp>
#include <magic_enum_utility.hpp>
#include <spdlog/spdlog.h>

namespace bricksim::graphics::orientation_cube {
    namespace {
        std::shared_ptr<Scene> scene;
        uint16_t size = 512;//todo config
        float lastPitch = 1e9, lastYaw = 1e9;

        void updateCamera() {
            const auto& activeEditor = controller::getActiveEditor();
            if (activeEditor != nullptr) {
                const auto camera = activeEditor->getCamera();
                auto pitch = glm::radians(camera->getPitch());
                auto yaw = glm::radians(camera->getYaw());
                if (pitch != lastPitch || yaw != lastYaw) {
                    std::dynamic_pointer_cast<OrientationCubeCamera>(scene->getCamera())->setPitchYaw(pitch, yaw);
                    lastPitch = pitch;
                    lastYaw = yaw;
                }
            }
        }
    }

    void initialize() {
        static bool initialized = false;
        if (initialized) {
            return;
        }
        scene = scenes::create(scenes::ORIENTATION_CUBE_SCENE_ID);

        const auto& rootNode = std::make_shared<etree::RootNode>();
        scene->setRootNode(rootNode);
        magic_enum::enum_for_each<CubeSide>([&rootNode](const auto side) {
            rootNode->addChild(std::make_shared<OrientationCubeSideMeshNode>(rootNode, side));
        });
        rootNode->incrementVersion();

        scene->setCamera(std::make_shared<OrientationCubeCamera>());
        scene->setImageSize({size, size});

        initialized = true;
    }

    unsigned int getImage() {
        updateCamera();
        scene->updateImage();
        return scene->getImage().getTexBO();
    }

    unsigned int getSelectionImage() {
        updateCamera();
        scene->getSelectionPixel(0, 0);
        return scene->getSelectionImage()->getTexBO();
    }

    void cleanup() {
        scene = nullptr;
    }

    std::optional<CubeSide> getSide(glm::usvec2 pos) {
        updateCamera();
        const element_id_t elementId = scene->getSelectionPixel(pos.x, size - pos.y);
        if (elementId != 0) {
            const auto& element = scene->getMeshCollection().getElementById(elementId);
            if (element != nullptr) {
                return std::dynamic_pointer_cast<OrientationCubeSideMeshNode>(element)->getSide();
            }
        }
        return {};
    }

    uint16_t getSize() {
        return size;
    }

    mesh_identifier_t OrientationCubeSideMeshNode::getMeshIdentifier() const {
        return constants::MESH_ID_ORIENTATION_CUBE_FIRST + static_cast<int>(side);
    }

    bool OrientationCubeSideMeshNode::isDisplayNameUserEditable() const {
        return false;
    }

    OrientationCubeSideMeshNode::OrientationCubeSideMeshNode(const std::shared_ptr<etree::Node>& parent, CubeSide side) :
        MeshNode(ldr::color_repo::getInstanceDummyColor(), parent, nullptr), side(side) {}

    void OrientationCubeSideMeshNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        auto texture = std::make_shared<Texture>(resources::orientation_cube_png.data(), resources::orientation_cube_png.size());
        auto& data = mesh->getTexturedTriangleData(texture);
        /*
         0      1/6      1/3    1/2    2/3   5/6      1
         ********************************************** 1
         * Right * Bottom * Back * Left * Top * Front *
         *  +X   *   +Y   *  +Z  *  -X  * -Y  *  -Z   *
         * ******************************************** 0
         * */
        /*
         * 2      1----3
         * | \     \ B |
         * | A \     \ |
         * 3----1      2
         */
        switch (side) {
            case CubeSide::RIGHT:
                data.addVertex({{+1, +1, +1}, {1.0f / 6, 1}});//rearRightBottom
                data.addVertex({{+1, -1, -1}, {0, 0}});       //frontRightTop
                data.addVertex({{+1, +1, -1}, {0, 1}});       //frontRightBottom

                data.addVertex({{+1, -1, -1}, {0, 0}});       //frontRightTop
                data.addVertex({{+1, +1, +1}, {1.0f / 6, 1}});//rearRightBottom
                data.addVertex({{+1, -1, +1}, {1.0f / 6, 0}});//rearRightTop
                break;
            case CubeSide::BOTTOM:
                data.addVertex({{+1, -1, -1}, {1.0f / 3, 1}});//frontRightTop
                data.addVertex({{-1, -1, +1}, {1.0f / 6, 0}});//rearLeftTop
                data.addVertex({{-1, -1, -1}, {1.0f / 6, 1}});//frontLeftTop

                data.addVertex({{-1, -1, +1}, {1.0f / 6, 0}});//rearLeftTop
                data.addVertex({{+1, -1, -1}, {1.0f / 3, 1}});//frontRightTop
                data.addVertex({{+1, -1, +1}, {1.0f / 3, 0}});//rearRightTop
                break;
            case CubeSide::BACK:
                data.addVertex({{-1, +1, +1}, {1.0f / 2, 1}});//rearLeftBottom
                data.addVertex({{+1, -1, +1}, {1.0f / 3, 0}});//rearRightTop
                data.addVertex({{+1, +1, +1}, {1.0f / 3, 1}});//rearRightBottom

                data.addVertex({{+1, -1, +1}, {1.0f / 3, 0}});//rearRightTop
                data.addVertex({{-1, +1, +1}, {1.0f / 2, 1}});//rearLeftBottom
                data.addVertex({{-1, -1, +1}, {1.0f / 2, 0}});//rearLeftTop
                break;
            case CubeSide::LEFT:
                data.addVertex({{-1, +1, -1}, {2.0f / 3, 1}});//frontLeftBottom
                data.addVertex({{-1, -1, +1}, {1.0f / 2, 0}});//rearLeftTop
                data.addVertex({{-1, +1, +1}, {1.0f / 2, 1}});//rearLeftBottom

                data.addVertex({{-1, -1, +1}, {1.0f / 2, 0}});//rearLeftTop
                data.addVertex({{-1, +1, -1}, {2.0f / 3, 1}});//frontLeftBottom
                data.addVertex({{-1, -1, -1}, {2.0f / 3, 0}});//frontLeftTop
                break;
            case CubeSide::TOP:
                data.addVertex({{+1, +1, +1}, {5.0f / 6, 1}});//rearRightBottom
                data.addVertex({{-1, +1, -1}, {2.0f / 3, 0}});//frontLeftBottom
                data.addVertex({{-1, +1, +1}, {2.0f / 3, 1}});//rearLeftBottom

                data.addVertex({{-1, +1, -1}, {2.0f / 3, 0}});//frontLeftBottom
                data.addVertex({{+1, +1, +1}, {5.0f / 6, 1}});//rearRightBottom
                data.addVertex({{+1, +1, -1}, {5.0f / 6, 0}});//frontRightBottom
                break;
            case CubeSide::FRONT:
                data.addVertex({{+1, +1, -1}, {1, 1}});       //frontRightBottom
                data.addVertex({{-1, -1, -1}, {5.0f / 6, 0}});//frontLeftTop
                data.addVertex({{-1, +1, -1}, {5.0f / 6, 1}});//frontLeftBottom

                data.addVertex({{-1, -1, -1}, {5.0f / 6, 0}});//frontLeftTop
                data.addVertex({{+1, +1, -1}, {1, 1}});       //frontRightBottom
                data.addVertex({{+1, -1, -1}, {1, 0}});       //frontRightTop
        }
        mesh->name = std::string("Orientation Cube Side ");
        mesh->name.append(magic_enum::enum_name(side));
    }

    CubeSide OrientationCubeSideMeshNode::getSide() const {
        return side;
    }

    void OrientationCubeCamera::setPitchYaw(float newPitch, float newYaw) {
        newPitch *= -1;
        if (pitch != newPitch || yaw != newYaw) {
            pitch = newPitch;
            yaw = newYaw;
            const auto distance = 0.040f;
            viewPos = glm::vec3(
                    distance * std::cos(pitch) * std::cos(yaw),
                    distance * std::sin(pitch),
                    distance * std::sin(yaw) * std::cos(pitch));
            viewMatrix = glm::lookAt(viewPos, target, {0.0f, 1.0f, 0.0f});
        }
    }

    const glm::mat4& OrientationCubeCamera::getViewMatrix() const {
        return viewMatrix;
    }

    const glm::vec3& OrientationCubeCamera::getCameraPos() const {
        return viewPos;
    }

    const glm::vec3& OrientationCubeCamera::getTargetPos() const {
        return target;
    }
}
