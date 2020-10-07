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
    static Renderer *instance;
    static Renderer* getInstance();

    Shader *triangleShader;
    Shader *lineShader;
    ElementTree elementTree;
    MeshCollection meshCollection;
    unsigned int windowWidth = Configuration::getInstance()->get_long(config::KEY_SCREEN_WIDTH);
    unsigned int windowHeight = Configuration::getInstance()->get_long(config::KEY_SCREEN_HEIGHT);

    CadCamera camera;
    float lastX = windowWidth / 2.0f;
    float lastY = windowHeight / 2.0f;

    int i_frame = 64;
    double time_sum = 0.0;

    glm::vec3 lightPos = glm::vec3(4.46, 7.32, 6.2);//todo customizable
    glm::mat4 projection{};
    GLFWwindow *window;

    bool setup();

    void updateProjectionMatrix();

    bool loop();

    bool cleanup();

private:

    Renderer();

    bool setupCalled = false;
    bool initialize();
};

#endif //BRICKSIM_RENDERER_H
