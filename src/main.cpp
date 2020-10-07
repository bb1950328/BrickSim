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
