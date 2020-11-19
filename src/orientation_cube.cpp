//
// Created by Bader on 18.11.2020.
//

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include "orientation_cube.h"
#include "controller.h"
#include "lib/stb_image.h"

namespace orientation_cube {
    namespace {
/*
 0      1/6      1/3    1/2    2/3   5/6      1
 ********************************************** 1
 * Right * Bottom * Back * Left * Top * Front *
 *  +X   *   +Y   *  +Z  *  -X  * -Y  *  -Z   *
 * ******************************************** 0
 * */

        //const glm::vec3 frontLeftBottom{-1, +1, -1};
        //const glm::vec3 frontLeftTop{-1, -1, -1};
        //const glm::vec3 rearLeftTop{-1, -1, +1};
        //const glm::vec3 rearLeftBottom{-1, +1, +1};
        //const glm::vec3 rearRightBottom{+1, +1, +1};
        //const glm::vec3 rearRightTop{+1, -1, +1};
        //const glm::vec3 frontRightTop{+1, -1, -1};
        //const glm::vec3 frontRightBottom{+1, +1, -1};

/*
 * 2      1----3
 * | \     \ B |
 * | A \     \ |
 * 3----1      2
 */
        const float cubeVertices[]{
                /*rearRightBottom*/ +1, +1, +1, 1.0f / 6, 0,
                /*frontRightTop*/   +1, -1, -1, 0, 1, // Right A
                /*frontRightBottom*/+1, +1, -1, 0, 0,

                /*frontRightTop*/   +1, -1, -1, 0, 1,
                /*rearRightBottom*/ +1, +1, +1, 1.0f / 6, 0, // Right B
                /*rearRightTop*/    +1, -1, +1, 1.0f / 6, 1,

                /*rearRightBottom*/ +1, +1, +1, 5.0f / 6, 0,
                /*frontLeftBottom*/ -1, +1, -1, 2.0f / 3, 1,//Bottom A
                /*rearLeftBottom*/  -1, +1, +1, 2.0f / 3, 0,

                /*frontLeftBottom*/ -1, +1, -1, 2.0f / 3, 1,
                /*rearRightBottom*/ +1, +1, +1, 5.0f / 6, 0,//Bottom B
                /*frontRightBottom*/+1, +1, -1, 5.0f / 6, 1,

                /*rearLeftBottom*/  -1, +1, +1, 1.0f / 2, 0,
                /*rearRightTop*/    +1, -1, +1, 1.0f / 3, 1,//Back A
                /*rearRightBottom*/ +1, +1, +1, 1.0f / 3, 0,

                /*rearRightTop*/    +1, -1, +1, 1.0f / 3, 1,
                /*rearLeftBottom*/  -1, +1, +1, 1.0f / 2, 0,//Back B
                /*rearLeftTop*/     -1, -1, +1, 1.0f / 2, 1,

                /*frontLeftBottom*/ -1, +1, -1, 2.0f / 3, 0,
                /*rearLeftTop*/     -1, -1, +1, 1.0f / 2, 1,//Left A
                /*rearLeftBottom*/  -1, +1, +1, 1.0f / 2, 0,

                /*rearLeftTop*/     -1, -1, +1, 1.0f / 2, 1,
                /*frontLeftBottom*/ -1, +1, -1, 2.0f / 3, 0,//Left B
                /*frontLeftTop*/    -1, -1, -1, 2.0f / 3, 1,

                /*frontRightTop*/   +1, -1, -1, 1.0f / 3, 0,
                /*rearLeftTop*/     -1, -1, +1, 1.0f / 6, 1,//Top A
                /*frontLeftTop*/    -1, -1, -1, 1.0f / 6, 0,

                /*rearLeftTop*/     -1, -1, +1, 1.0f / 6, 1,
                /*frontRightTop*/   +1, -1, -1, 1.0f / 3, 0,// Top B
                /*rearRightTop*/    +1, -1, +1, 1.0f / 3, 1,

                /*frontRightBottom*/+1, +1, -1, 1, 0,
                /*frontLeftTop*/    -1, -1, -1, 5.0f / 6, 1,//Front A
                /*frontLeftBottom*/ -1, +1, -1, 5.0f / 6, 0,

                /*frontLeftTop*/    -1, -1, -1, 5.0f / 6, 1,
                /*frontRightBottom*/+1, +1, -1, 1, 0,//Front B
                /*frontRightTop*/   +1, -1, -1, 1, 1,
        };
        constexpr auto cubeVertexCount = 36;

        unsigned int fbo, tbo, rbo;
        short framebufferSize = 0;
        unsigned int VBO, VAO;
        short size = 512;//todo config
        unsigned int texture;
        float lastPitch = 1e9, lastYaw = 1e9;
        const glm::mat4 projection = glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, 1000.0f);

        glm::mat4 getMatrix(float pitch, float yaw) {
            pitch *= -1;
            glm::vec3 viewPos = glm::vec3(
                    4.0f * std::cos(pitch) * std::cos(yaw),
                    4.0f * std::sin(pitch),
                    4.0f * std::sin(yaw) * std::cos(pitch)
            );
            const glm::mat4 view = glm::lookAt(viewPos, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
            return projection * view * constants::LDU_TO_OPENGL_ROTATION;
        }

        void initialize() {
            static bool initialized = false;
            if (initialized) {
                return;
            }
            int imgWidth, imgHeight, nrChannels;
            const auto filename = std::string("resources") + util::PATH_SEPARATOR + "orientation_cube.jpg";
            unsigned char *data = stbi_load(filename.c_str(), &imgWidth, &imgHeight, &nrChannels, 3);

            if (data) {
                //std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());

                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);

                glBindVertexArray(VAO);

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
                std::cout << "orientation_cube VAO, VBO: " << VAO << ", " << VBO << std::endl;

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), nullptr);
                glEnableVertexAttribArray(0);

                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)(3*sizeof(float)));
                glEnableVertexAttribArray(1);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);

                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);

                GLenum format;
                if (nrChannels == 1)
                    format = GL_RED;
                else if (nrChannels == 3)
                    format = GL_RGB;
                else if (nrChannels == 4)
                    format = GL_RGBA;

                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, format, imgWidth, imgHeight, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            } else {
                throw std::invalid_argument("texture for orientation cube not read successfully: " + filename);
            }
            stbi_image_free(data);
            initialized = true;
        }
    }


    unsigned int getImage() {
        const auto camera = controller::getRenderer()->camera;
        auto pitch = glm::radians(camera.getPitch());
        auto yaw = glm::radians(camera.getYaw());
        if (pitch != lastPitch || yaw != lastYaw) {
            Renderer *renderer = controller::getRenderer();
            if (framebufferSize != size) {
                renderer->createFramebuffer(&fbo, &tbo, &rbo, size, size);
                framebufferSize = size;
            }
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            initialize();

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, size, size);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glBindTexture(GL_TEXTURE_2D, texture);

            auto textureShader = renderer->textureShader;
            textureShader->use();
            textureShader->setMat4("modelView", getMatrix(pitch, yaw));

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glDisable(GL_DEPTH_TEST);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
            glEnable(GL_DEPTH_TEST);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            lastPitch = pitch;
            lastYaw = yaw;
            glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);
        }
        return tbo;
    }
}