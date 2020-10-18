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
#include "camera.h"
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
    Shader *selectionShader;
    ElementTree *elementTree;
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
    unsigned int currendSelectionBuffersWidth = 0;
    unsigned int currendSelectionBuffersHeight = 0;

    bool setup();

    void updateProjectionMatrix();

    bool loop();

    unsigned int getSelectionPixel(unsigned int x, unsigned int y);

    bool cleanup();

    explicit Renderer(ElementTree *elementTree);

    void setWindowSize(unsigned int width, unsigned int height);

    void elementTreeChanged();

private:

    bool setupCalled = false;

    void createFramebuffer(unsigned int* framebufferIdLocation,
                           unsigned int* textureColorbufferIdLocation,
                           unsigned int* renderBufferObjectIdLocation);
};

#endif //BRICKSIM_RENDERER_H
