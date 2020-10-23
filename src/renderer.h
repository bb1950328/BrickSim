//
// Created by bb1950328 on 07.10.2020.
//

#ifndef BRICKSIM_RENDERER_H
#define BRICKSIM_RENDERER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

/*#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"*/

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shaders/shader.h"
#include "helpers/camera.h"
#include "config.h"
#include "element_tree.h"
#include "mesh_collection.h"
#include "statistic.h"
#include "ldr_file_repository.h"

#include <iostream>
#include <chrono>

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);


class Renderer {
public:
    Shader *triangleShader;
    Shader *lineShader;
    MeshCollection meshCollection;

    unsigned int windowWidth = 0;
    unsigned int windowHeight = 0;

    CadCamera camera;

    bool unrenderedChanges = true;

    glm::vec3 lightPos = glm::vec3(4.46, 7.32, 6.2);//todo customizable
    glm::mat4 projection{};
    GLFWwindow *window;

    unsigned int imageFramebuffer;
    unsigned int imageTextureColorbuffer;
    unsigned int imageRenderBufferObject;

    unsigned int selectionFramebuffer;
    unsigned int selectionTextureColorbuffer;
    unsigned int selectionRenderBufferObject;
    unsigned int currentSelectionBuffersWidth = 0;
    unsigned int currentSelectionBuffersHeight = 0;

    bool setup();

    void updateProjectionMatrix();

    bool loop();

    unsigned int getSelectionPixel(unsigned int x, unsigned int y);

    bool cleanup();

    explicit Renderer(etree::ElementTree *elementTree);

    void setWindowSize(unsigned int width, unsigned int height);

    void elementTreeChanged();

    bool saveImage(const std::string& path);

    void createFramebuffer(unsigned int* framebufferIdLocation,
                           unsigned int* textureColorbufferIdLocation,
                           unsigned int* renderBufferObjectIdLocation);

    void createFramebuffer(unsigned int *framebufferIdLocation,
                           unsigned int *textureColorbufferIdLocation,
                           unsigned int *renderBufferObjectIdLocation,
                           unsigned int width,
                           unsigned int height);

    static void deleteFramebuffer(unsigned int* framebufferIdLocation,
                                  unsigned int* textureColorbufferIdLocation,
                                  unsigned int* renderBufferObjectIdLocation);

private:

    bool setupCalled = false;
};

#endif //BRICKSIM_RENDERER_H
