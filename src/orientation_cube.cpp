#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include "orientation_cube.h"
#include "controller.h"
#include "constant_data/resources.h"

namespace orientation_cube {
    namespace {
        std::shared_ptr<Scene> scene;
        short size = 512;//todo config
        float lastPitch = 1e9, lastYaw = 1e9;

        class OrientationCubeMeshNode : public etree::MeshNode {
        public:
            void *getMeshIdentifier() const override {
                return reinterpret_cast<void *>(12234);
            }

            bool isDisplayNameUserEditable() const override {
                return false;
            }

            OrientationCubeMeshNode() : MeshNode(ldr_color_repo::getInstanceDummyColor(), nullptr) {}

            void addToMesh(std::shared_ptr<Mesh> mesh, bool windingInversed) override {
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
        };

        class OrientationCubeCamera : public Camera {
        private:
            float pitch, yaw;
            glm::vec3 viewPos;
            glm::mat4 viewMatrix;
        public:
            void setPitchYaw(float newPitch, float newYaw) {
                newPitch *= -1;
                if (pitch != newPitch || yaw != newYaw) {
                    pitch = newPitch;
                    yaw = newYaw;
                    viewPos = glm::vec3(
                            4.0f * std::cos(pitch) * std::cos(yaw),
                            4.0f * std::sin(pitch),
                            4.0f * std::sin(yaw) * std::cos(pitch)
                    );
                    viewMatrix = glm::lookAt(viewPos, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
                }
            }

            [[nodiscard]] const glm::mat4 &getViewMatrix() const override {
                return viewMatrix;
            }

            [[nodiscard]] const glm::vec3 &getCameraPos() const override {
                return viewPos;
            }
        };
    }

    void initialize() {
        static bool initialized = false;
        if (initialized) {
            return;
        }
        scene = scenes::create(scenes::ORIENTATION_CUBE_SCENE_ID);
        scene->setRootNode(std::make_shared<OrientationCubeMeshNode>());
        scene->setCamera(std::make_shared<OrientationCubeCamera>());

        initialized = true;
    }

    unsigned int getImage() {
        const auto camera = controller::getMainSceneCamera();
        auto pitch = glm::radians(camera->getPitch());
        auto yaw = glm::radians(camera->getYaw());
        if (pitch != lastPitch || yaw != lastYaw) {
            std::dynamic_pointer_cast<OrientationCubeCamera>(scene->getCamera())->setPitchYaw(pitch, yaw);
        }
        scene->updateImage();
        return scene->getImage().getTexBO();
    }

    void cleanup() {
        scene = nullptr;
    }
}