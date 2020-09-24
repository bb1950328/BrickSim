#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION

#include "lib/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/shader.h"
#include "mesh.h"
#include "camera.h"
#include "config.h"

#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <chrono>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = Configuration::getInstance()->get_long("screenWidth");
const unsigned int SCR_HEIGHT = Configuration::getInstance()->get_long("screenHeight");

auto camera = CadCamera();
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;


glm::vec3 lightPos(4.46, 7.32, 6.2);//todo customizable

GLFWwindow* initialize() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //glfwWindowHint(GLFW_DECORATED, false);//removes the title bar
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    //glfwSetWindowPos(window, 20, 40);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);
    return window;
}

int main() {
    auto window = initialize();
    if (window== nullptr) {
        return -1;
    }

    Shader triangleShader("src/shaders/shader.vsh", "src/shaders/shader.fsh");

    auto before = std::chrono::high_resolution_clock::now();
    LdrFile *mainFile = LdrFileRepository::get_file("~/Downloads/42043_arocs.mpd"/*"3001.dat"*/);
    mainFile->preLoadSubfilesAndEstimateComplexity();
    //mainFile->printStructure();
    auto between = std::chrono::high_resolution_clock::now();
    auto mesh = TriangleMesh();
    mesh.addLdrFile(*mainFile);
    auto after = std::chrono::high_resolution_clock::now();
    long ms_load = std::chrono::duration_cast<std::chrono::milliseconds>(between - before).count();
    long ms_mesh = std::chrono::duration_cast<std::chrono::milliseconds>(after - between).count();
    unsigned long triangle_vertices_count = 0, triangle_indices_count = 0;
    for (const auto &entry: mesh.triangleVertices) {
        triangle_vertices_count += entry.second->size();
    }
    for (const auto &entry: mesh.triangleIndices) {
        triangle_indices_count += entry.second->size();
    }
    std::cout << "materials count: " << mesh.triangleVertices.size() << "\n";
    std::cout << "main model estimated complexity: " << mainFile->estimatedComplexity << "\n";
    std::cout << "total triangle vertices count: " << triangle_vertices_count << "\n";
    std::cout << "total triangle indices count: " << triangle_indices_count << "\n";
    std::cout << "every triangle vertex is used " << (float)triangle_indices_count / (float)triangle_vertices_count << "times.\n";
    std::cout << "total line vertices count: " << mesh.lineVertices.size() << "\n";
    std::cout << "total line indices count: " << mesh.lineIndices.size() << "\n";
    std::cout << "every line vertex is used " << (float)mesh.lineIndices.size() / (float)mesh.lineVertices.size() << "times.\n";
    std::cout << "ldr file loading time: " << ms_load << "ms.\n";
    std::cout << "meshing time: " << ms_mesh << "ms.\n";

    for (const auto& x: LdrFileRepository::files) {
        if (x.second->estimatedComplexity>1000) {
            std::cout << x.first << "\t" << x.second->estimatedComplexity << "\n";
        }
    }

    std::map<LdrColor *, unsigned int> VAOs, VBOs, EBOs;
    for (const auto &entry: mesh.triangleIndices) {
        LdrColor *color = entry.first;
        std::vector<unsigned int> *indices = entry.second;
        std::vector<TriangleVertex> *vertices = mesh.triangleVertices.find(color)->second;

        unsigned int vao, vbo, ebo;

        //vao
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        //vbo
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        size_t vertex_size = sizeof(TriangleVertex);
        glBufferData(GL_ARRAY_BUFFER, vertices->size() * vertex_size, &(*vertices)[0], GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void *) nullptr);
        glEnableVertexAttribArray(0);
        // normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) offsetof(TriangleVertex, normal));
        glEnableVertexAttribArray(1);

        //ebo
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices->size(), &(*indices)[0], GL_STATIC_DRAW);

        VAOs[color] = vao;
        VBOs[color] = vbo;
        EBOs[color] = ebo;
    }

    triangleShader.use();

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        triangleShader.use();

        float aspect = (float) SCR_WIDTH / (float) SCR_HEIGHT;
        glm::mat4 projection = glm::perspective(glm::radians(50.0f), aspect, 0.1f,1000.0f);
        triangleShader.setMat4("projection", projection);

        glm::mat4 view = camera.getViewMatrix();
        triangleShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
        triangleShader.setMat4("model", model);

        for (const auto &entry: mesh.triangleIndices) {
            LdrColor *color = entry.first;
            std::vector<unsigned int> *indices = entry.second;
            unsigned int vao = VAOs[color];
            unsigned int vbo = VBOs[color];
            unsigned int ebo = EBOs[color];
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

            //std::cout << glm::to_string(camera.getCameraPos()) << "\n";
            triangleShader.setVec3("light.position", lightPos);
            triangleShader.setVec3("viewPos", camera.getCameraPos());

            glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
            glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
            glm::vec3 ambientColor = diffuseColor * glm::vec3(0.9f); // low influence
            triangleShader.setVec3("light.ambient", ambientColor);
            triangleShader.setVec3("light.diffuse", diffuseColor);
            triangleShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

            glm::vec3 diffuse, specular;
            const glm::vec3 &ambient = color->value.asGlmVector();
            float shininess = 32.0f;

            switch (color->finish) {
                case LdrColor::METAL:
                case LdrColor::CHROME:
                case LdrColor::PEARLESCENT:
                    //todo find out what's the difference
                    shininess *= 2;
                    diffuse = glm::vec3(1.0, 1.0, 1.0);
                    specular = glm::vec3(1.0, 1.0, 1.0);
                    break;
                case LdrColor::MATTE_METALLIC:
                    diffuse = glm::vec3(1.0, 1.0, 1.0);
                    specular = glm::vec3(0.2, 0.2, 0.2);
                    break;
                case LdrColor::RUBBER:
                    diffuse = glm::vec3(0.0, 0.0, 0.0);
                    specular = glm::vec3(0.0, 0.0, 0.0);
                    break;
                default:
                    diffuse = ambient;
                    specular = glm::vec3(0.5, 0.5, 0.5);
                    break;
            }

            triangleShader.setVec3("material.ambient", ambient);
            triangleShader.setVec3("material.diffuse", diffuse);
            triangleShader.setVec3("material.specular", specular);
            triangleShader.setFloat("material.shininess", shininess);

            glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, 0);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (const auto &entry: mesh.triangleIndices) {
        LdrColor *color = entry.first;
        unsigned int vao = VAOs[color];
        unsigned int vbo = VBOs[color];
        unsigned int ebo = EBOs[color];
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    //this gets called when the window is resized
    glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        camera.mouseRotate(xoffset, yoffset);
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        camera.mousePan(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.moveForwardBackward(yoffset);
}
