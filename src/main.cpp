#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shaders/shader.h"
#include "camera.h"
#include "config.h"
#include "element_tree.h"
#include "mesh_collection.h"
#include "ldr_file_repository.h"
#include "renderer.h"


int main() {
    Renderer *renderer = Renderer::getInstance();
    renderer->setup();
    while (!glfwWindowShouldClose(renderer->window)) {
        renderer->loop();
    }
    renderer->cleanup();
    return 0;
}
