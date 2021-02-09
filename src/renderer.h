

#ifndef BRICKSIM_RENDERER_H
#define BRICKSIM_RENDERER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shaders/shader.h"
#include "helpers/camera.h"
#include "config.h"
#include "element_tree.h"
#include "mesh_collection.h"
#include "statistic.h"


#include <iostream>
#include <chrono>

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

class Renderer {
public:
    Shader *triangleShader;
    Shader *lineShader;
    Shader *optionalLineShader;
    Shader *textureShader;
    std::shared_ptr<MeshCollection> meshCollection;

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

    bool initialize();

    void updateProjectionMatrix();

    bool loop();

    unsigned int getSelectionPixel(unsigned int x, unsigned int y);

    bool cleanup();

    explicit Renderer(std::shared_ptr<MeshCollection> meshCollection);

    void setWindowSize(unsigned int width, unsigned int height);

    void elementTreeChanged();

    bool saveImage(const std::string& path) const;

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
