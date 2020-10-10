//
// Created by bb1950328 on 07.10.2020.
//

#include <imgui.h>
#include "renderer.h"
#include "gui.h"
#include "controller.h"

bool Renderer::setup() {
    if (setupCalled) {
        return true;
    }

    triangleShader = new Shader("src/shaders/triangle_shader.vsh", "src/shaders/triangle_shader.fsh");
    lineShader = new Shader("src/shaders/line_shader.vsh", "src/shaders/line_shader.fsh");

    LdrFileRepository::initializeNames();
    auto before = std::chrono::high_resolution_clock::now();

    elementTree->loadLdrFile("~/Downloads/arocs.mpd");
    //elementTree.print();
    auto between = std::chrono::high_resolution_clock::now();

    //meshCollection.elementTree = &elementTree;
    meshCollection.readElementTree();
    //meshCollection.addLdrFile(LdrColorRepository::getInstance()->get_color(4), mainFile, glm::mat4(1.0f));
    auto after = std::chrono::high_resolution_clock::now();
    long ms_load = std::chrono::duration_cast<std::chrono::milliseconds>(between - before).count();
    long ms_mesh = std::chrono::duration_cast<std::chrono::milliseconds>(after - between).count();
    unsigned long triangle_vertices_count = 0, triangle_indices_count = 0;
    for (const auto &pair: meshCollection.meshes) {
        auto mesh = pair.second;
        for (const auto &entry: mesh->triangleVertices) {
            triangle_vertices_count += entry.second->size();
        }
        for (const auto &entry: mesh->triangleIndices) {
            triangle_indices_count += entry.second->size();
        }
    }
    std::cout << "meshes count: " << meshCollection.meshes.size() << "\n";
    //std::cout << "main model estimated complexity: " << mainFile->estimatedComplexity << "\n";
    std::cout << "total triangle vertices count: " << triangle_vertices_count << "\n";
    std::cout << "total triangle indices count: " << triangle_indices_count << "\n";
    std::cout << "every triangle vertex is used " << (float)triangle_indices_count / (float)triangle_vertices_count << "times.\n";
    //std::cout << "total line vertices count: " << mesh.lineVertices.size() << "\n";
    //std::cout << "total line indices count: " << mesh.lineIndices.size() << "\n";
    //std::cout << "every line vertex is used " << (float)mesh.lineIndices.size() / (float)mesh.lineVertices.size() << "times.\n";
    std::cout << "ldr file loading time: " << ms_load << "ms.\n";
    std::cout << "meshing time: " << ms_mesh << "ms.\n";

    stats::print();

    /*for (const auto &meshPair: meshCollection.meshes) {
        std::cout << meshPair.first->getDescription() << "\n";
        for (const auto &instance: meshPair.second->instances) {
            std::cout << "\t" << instance.first->name  << "\n";
            auto mat_str = glm::to_string(instance.second);
            util::replaceAll(mat_str, "), (", "),\n\t\t       (");
            std::cout << "\t\t" << mat_str << "\n";
        }
    }*/

    updateProjectionMatrix();

    meshCollection.initializeGraphics();

    triangleShader->use();

    triangleShader->setVec3("light.position", lightPos);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.9f); // low influence
    triangleShader->setVec3("light.ambient", ambientColor);
    triangleShader->setVec3("light.diffuse", diffuseColor);
    triangleShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    setupCalled = true;
    return true;
}

void Renderer::updateProjectionMatrix() {
    projection = glm::perspective(glm::radians(50.0f), (float) windowWidth / (float) windowHeight, 0.1f, 1000.0f);
}

bool Renderer::loop() {
    if (!setupCalled) {
        throw std::invalid_argument("call setup first!");
    }
    processInput(window);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.getViewMatrix();
    const glm::mat4 &projectionView = projection * view;

    triangleShader->use();
    triangleShader->setVec3("viewPos", camera.getCameraPos());
    triangleShader->setMat4("projectionView", projectionView);
    meshCollection.drawTriangleGraphics(triangleShader);
    lineShader->use();
    lineShader->setMat4("projectionView", projectionView);
    meshCollection.drawLineGraphics(lineShader);

    return true;
}

bool Renderer::cleanup() {
    meshCollection.deallocateGraphics();
    glfwTerminate();
    return true;
}

Renderer::Renderer(ElementTree *elementTree) : meshCollection(elementTree) {
    this->elementTree = elementTree;
    triangleShader = nullptr;
    lineShader = nullptr;
}

void Renderer::setWindowSize(unsigned int width, unsigned int height) {
    windowWidth = width;
    windowHeight = height;
    updateProjectionMatrix();
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    //this gets called when the window is resized
    glViewport(0, 0, width, height);
    Controller::getInstance()->setWindowSize(width, height);
}


void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    auto *renderer = &Controller::getInstance()->renderer;
    float xoffset = xpos - renderer->lastX;
    float yoffset = renderer->lastY - ypos;

    renderer->lastX = xpos;
    renderer->lastY = ypos;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        renderer->camera.mouseRotate(xoffset, yoffset);
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        renderer->camera.mousePan(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    Controller::getInstance()->renderer.camera.moveForwardBackward(yoffset);
}
