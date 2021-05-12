#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/string_cast.hpp"
#include "orientation_cube.h"
#include "../controller.h"
#include "../constant_data/resources.h"
#include "../ldr_files/ldr_file_repo.h"

namespace orientation_cube {
    namespace {
        std::shared_ptr<Scene> scene;
        short size = 512;//todo config
        float lastPitch = 1e9, lastYaw = 1e9;

        void updateCamera() {
            const auto camera = controller::getMainSceneCamera();
            auto pitch = glm::radians(camera->getPitch());
            auto yaw = glm::radians(camera->getYaw());
            if (pitch != lastPitch || yaw != lastYaw) {
                std::dynamic_pointer_cast<OrientationCubeCamera>(scene->getCamera())->setPitchYaw(pitch, yaw);
                lastPitch = pitch;
                lastYaw = yaw;
            }
        }
    }

    void initialize() {
        static bool initialized = false;
        if (initialized) {
            return;
        }
        scene = scenes::create(scenes::ORIENTATION_CUBE_SCENE_ID);

        const auto &rootNode = std::make_shared<etree::RootNode>();
        scene->setRootNode(rootNode);
        rootNode->addChild(std::make_shared<OrientationCubeSideMeshNode>(rootNode, CubeSide::RIGHT));
        rootNode->addChild(std::make_shared<OrientationCubeSideMeshNode>(rootNode, CubeSide::BOTTOM));
        rootNode->addChild(std::make_shared<OrientationCubeSideMeshNode>(rootNode, CubeSide::BACK));
        rootNode->addChild(std::make_shared<OrientationCubeSideMeshNode>(rootNode, CubeSide::LEFT));
        rootNode->addChild(std::make_shared<OrientationCubeSideMeshNode>(rootNode, CubeSide::TOP));
        rootNode->addChild(std::make_shared<OrientationCubeSideMeshNode>(rootNode, CubeSide::FRONT));

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
        const element_id_t elementId = scene->getSelectionPixel(pos.x, size-pos.y);
        if (elementId != 0)  {
            const auto &element = scene->getMeshCollection().getElementById(elementId);
            if (element != nullptr) {
                return std::dynamic_pointer_cast<OrientationCubeSideMeshNode>(element)->getSide();
            }
        }
        return {};
    }

    short getSize() {
        return size;
    }

    mesh_identifier_t OrientationCubeSideMeshNode::getMeshIdentifier() const {
        return constants::MESH_ID_ORIENTATION_CUBE_FIRST+static_cast<int>(side);
    }

    bool OrientationCubeSideMeshNode::isDisplayNameUserEditable() const {
        return false;
    }

    OrientationCubeSideMeshNode::OrientationCubeSideMeshNode(const std::shared_ptr<etree::Node> &parent, CubeSide side) : MeshNode(ldr_color_repo::getInstanceDummyColor(), parent), side(side) {}

    void OrientationCubeSideMeshNode::addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) {
        auto texture = std::make_shared<Texture>(resources::orientation_cube_jpg, resources::orientation_cube_jpg_len);

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
                mesh->addTexturedTriangle(texture,
                                          {+1, +1, +1}, {1.0f / 6, 1}, /*rearRightBottom*/
                                          {+1, -1, -1}, {0, 0}, /*frontRightTop*/
                                          {+1, +1, -1}, {0, 1}/*frontRightBottom*/
                );// Right A
                mesh->addTexturedTriangle(texture,
                                          {+1, -1, -1}, {0, 0}, /*frontRightTop*/
                                          {+1, +1, +1}, {1.0f / 6, 1}, /*rearRightBottom*/
                                          {+1, -1, +1}, {1.0f / 6, 0}/*rearRightTop*/
                );// Right B
                break;
            case CubeSide::BOTTOM:
                mesh->addTexturedTriangle(texture,
                                          {+1, -1, -1}, {1.0f / 3, 1},//frontRightTop
                                          {-1, -1, +1}, {1.0f / 6, 0},//rearLeftTop
                                          {-1, -1, -1}, {1.0f / 6, 1}//frontLeftTop
                );//Top A
                mesh->addTexturedTriangle(texture,
                                          {-1, -1, +1}, {1.0f / 6, 0},//rearLeftTop
                                          {+1, -1, -1}, {1.0f / 3, 1},//frontRightTop
                                          {+1, -1, +1}, {1.0f / 3, 0}//rearRightTop
                );//Top B
                break;
            case CubeSide::BACK:
                mesh->addTexturedTriangle(texture,
                                          {-1, +1, +1}, {1.0f / 2, 1},//rearLeftBottom
                                          {+1, -1, +1}, {1.0f / 3, 0},//rearRightTop
                                          {+1, +1, +1}, {1.0f / 3, 1}//rearRightBottom
                );//Back A
                mesh->addTexturedTriangle(texture,
                                          {+1, -1, +1}, {1.0f / 3, 0},//rearRightTop
                                          {-1, +1, +1}, {1.0f / 2, 1},//rearLeftBottom
                                          {-1, -1, +1}, {1.0f / 2, 0}//rearLeftTop
                );//Back B

                break;
            case CubeSide::LEFT:
                mesh->addTexturedTriangle(texture,
                                          {-1, +1, -1}, {2.0f / 3, 1},//frontLeftBottom
                                          {-1, -1, +1}, {1.0f / 2, 0},//rearLeftTop
                                          {-1, +1, +1}, {1.0f / 2, 1}//rearLeftBottom
                );//Left A
                mesh->addTexturedTriangle(texture,
                                          {-1, -1, +1}, {1.0f / 2, 0},//rearLeftTop
                                          {-1, +1, -1}, {2.0f / 3, 1},//frontLeftBottom
                                          {-1, -1, -1}, {2.0f / 3, 0}//frontLeftTop
                );//Left B

                break;
            case CubeSide::TOP:
                mesh->addTexturedTriangle(texture,
                                          {+1, +1, +1}, {5.0f / 6, 1},//rearRightBottom
                                          {-1, +1, -1}, {2.0f / 3, 0},//frontLeftBottom
                                          {-1, +1, +1}, {2.0f / 3, 1}//rearLeftBottom
                );//Bottom A
                mesh->addTexturedTriangle(texture,
                                          {-1, +1, -1}, {2.0f / 3, 0},//frontLeftBottom
                                          {+1, +1, +1}, {5.0f / 6, 1},//rearRightBottom
                                          {+1, +1, -1}, {5.0f / 6, 0}//frontRightBottom
                );//Bottom B
                break;
            case CubeSide::FRONT:
                mesh->addTexturedTriangle(texture,
                                          {+1, +1, -1}, {1, 1},//frontRightBottom
                                          {-1, -1, -1}, {5.0f / 6, 0},//frontLeftTop
                                          {-1, +1, -1}, {5.0f / 6, 1}//frontLeftBottom
                );//Front A
                mesh->addTexturedTriangle(texture,
                                          {-1, -1, -1}, {5.0f / 6, 0},//frontLeftTop
                                          {+1, +1, -1}, {1, 1},//frontRightBottom
                                          {+1, -1, -1}, {1, 0}//frontRightTop
                );//Front B
        }
        mesh->name = "Orientation Cube";
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
                    distance * std::sin(yaw) * std::cos(pitch)
            );
            viewMatrix = glm::lookAt(viewPos, target, {0.0f, 1.0f, 0.0f});
        }
    }

    const glm::mat4 &OrientationCubeCamera::getViewMatrix() const {
        return viewMatrix;
    }

    const glm::vec3 &OrientationCubeCamera::getCameraPos() const {
        return viewPos;
    }

    const glm::vec3 &OrientationCubeCamera::getTargetPos() const {
        return target;
    }
}