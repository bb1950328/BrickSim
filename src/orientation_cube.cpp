//
// Created by Bader on 18.11.2020.
//

#include <glm/gtx/euler_angles.hpp>
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

        inline glm::vec3 frontLeftBottom{-1, +1, -1};
        inline glm::vec3 frontLeftTop{-1, -1, -1};
        inline glm::vec3 rearLeftTop{-1, -1, +1};
        inline glm::vec3 rearLeftBottom{-1, +1, +1};
        inline glm::vec3 rearRightBottom{+1, +1, +1};
        inline glm::vec3 rearRightTop{+1, -1, +1};
        inline glm::vec3 frontRightTop{+1, -1, -1};
        inline glm::vec3 frontRightBottom{+1, +1, -1};

/*
 * 2      1----3
 * | \     \ B |
 * | A \     \ |
 * 3----1      2
 */
        const TexturedVertex cubeVertices[]{
                {rearRightBottom,  {1 / 6, 0}},
                {frontRightTop,    {0,     1}}, // Right A
                {frontRightBottom, {0,     0}},

                {frontRightTop,    {0,     1}},
                {rearRightBottom,  {1 / 6, 0}}, // Right B
                {rearRightTop,     {1 / 6, 1}},

                {rearRightBottom,  {1 / 3, 0}},
                {frontLeftBottom,  {1 / 6, 1}},//Bottom A
                {rearLeftBottom,   {1 / 6, 0}},

                {frontLeftBottom,  {1 / 6, 1}},
                {rearRightBottom,  {1 / 3, 0}},//Bottom B
                {frontRightBottom, {1 / 3, 1}},

                {rearLeftBottom,   {1 / 2, 0}},
                {rearRightTop,     {1 / 3, 1}},//Back A
                {rearRightBottom,  {1 / 3, 0}},

                {rearRightTop,     {1 / 3, 1}},
                {rearLeftBottom,   {1 / 2, 0}},//Back B
                {rearLeftTop,      {1 / 2, 1}},

                {frontLeftBottom,  {2 / 3, 0}},
                {rearLeftTop,      {1 / 2, 1}},//Left A
                {rearLeftBottom,   {1 / 2, 0}},

                {rearLeftTop,      {1 / 2, 1}},
                {frontLeftBottom,  {2 / 3, 0}},//Left B
                {frontLeftTop,     {2 / 3, 1}},

                {frontRightTop,    {5 / 6, 0}},
                {rearLeftTop,      {2 / 3, 1}},//Top A
                {frontLeftTop,     {2 / 3, 0}},

                {rearLeftTop,      {2 / 3, 1}},
                {frontRightTop,    {5 / 6, 0}},// Top B
                {rearRightTop,     {5 / 6, 1}},

                {frontRightBottom, {1,     0}},
                {frontLeftTop,     {5 / 6, 1}},//Front A
                {frontLeftBottom,  {5 / 6, 0}},

                {frontLeftTop,     {5 / 6, 1}},
                {frontRightBottom, {1,     0}},//Front B
                {frontRightTop,    {1,     1}},
        };
        constexpr auto cubeVertexCount = std::extent<decltype(cubeVertices)>::value;

        unsigned int fbo, tbo, rbo;
        short framebufferSize = 0;
        unsigned int VBO, VAO;
        short size = 512;//todo config
        unsigned int texture;
        float lastPitch=1e9, lastYaw=1e9;
        const glm::mat4 projection = glm::perspective(glm::radians(350.0f), 1.0f, 0.1f, 1000.0f);

        glm::mat4 getMatrix(float pitch, float yaw) {
            glm::mat4 model = glm::eulerAngleYXZ(yaw, pitch, 0.0f);
            return projection * model;
        }
    }

    void initialize() {
        int imgWidth, imgHeight, nrChannels;
        const auto filename = std::string("resources") + util::PATH_SEPARATOR + "orientation_cube.png";
        unsigned char *data = stbi_load(filename.c_str(), &imgWidth, &imgHeight, &nrChannels, 3);
        if (data) {
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, cubeVertexCount, cubeVertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *) offsetof(TexturedVertex, position));
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *) offsetof(TexturedVertex, texCoord));
            glEnableVertexAttribArray(1);


            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            throw std::invalid_argument("texture for orientation cube not read successfully: " + filename);
        }
        stbi_image_free(data);
    }

    unsigned int getImage(float pitch, float yaw) {
        if (pitch != lastPitch || yaw != lastYaw) {
            Renderer *renderer = controller::getRenderer();
            if (framebufferSize != size) {
                renderer->createFramebuffer(&fbo, &tbo, &rbo, size, size);
            }
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, size, size);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glBindTexture(GL_TEXTURE_2D, texture);

            auto textureShader = renderer->textureShader;
            textureShader->use();
            textureShader->setMat4("modelView", getMatrix(pitch, yaw));

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            lastPitch = pitch;
            lastYaw = yaw;
            glViewport(0, 0, renderer->windowWidth, renderer->windowHeight);
        }
        return tbo;
    }
}