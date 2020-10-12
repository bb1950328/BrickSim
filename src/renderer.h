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

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);


class Renderer {
public:
    Shader *triangleShader;
    Shader *lineShader;
    ElementTree *elementTree;
    MeshCollection meshCollection;

    unsigned int windowWidth = 0;
    unsigned int windowHeight = 0;

    unsigned int windowPosX1;
    unsigned int windowPosY1;
    unsigned int windowPosX2;
    unsigned int windowPosY2;

    CadCamera camera;
    float lastX = windowWidth / 2.0f;
    float lastY = windowHeight / 2.0f;

    glm::vec3 lightPos = glm::vec3(4.46, 7.32, 6.2);//todo customizable
    glm::mat4 projection{};
    GLFWwindow *window;

    unsigned int framebuffer;
    unsigned int textureColorbuffer;
    unsigned int renderBufferObject;

    bool setup();

    void updateProjectionMatrix();

    bool loop();

    bool cleanup();

    explicit Renderer(ElementTree *elementTree);

    void setWindowSize(unsigned int width, unsigned int height);

    void setWindowPos(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

    bool isInWindow(double x, double y);

private:

    bool setupCalled = false;

    void createFramebuffer();

};

#endif //BRICKSIM_RENDERER_H
