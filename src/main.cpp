#include "renderer.h"
#include "gui.h"

int main() {
    Renderer *renderer = Renderer::getInstance();
    renderer->setup();
    initGui(renderer->window);
    while (!glfwWindowShouldClose(renderer->window)) {
        renderer->loop();
    }
    cleanupGui();
    renderer->cleanup();
    return 0;
}
