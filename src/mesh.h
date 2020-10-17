// mesh.h
// Created by bb1950328 on 20.09.20.
//

#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

#include <glm/glm.hpp>
#include <vector>
#include <set>
#include "ldr_files.h"
#include "shaders/shader.h"
#include "camera.h"
#include "constants.h"

struct TriangleVertex {
    //TriangleVertex(const glm::vec4 &position, const glm::vec3 &normal, const glm::vec3 &color);

    glm::vec4 position;
    glm::vec3 normal;

    //glm::vec3 color;
    bool operator==(const TriangleVertex &other) const {
        return position == other.position && normal == other.normal;
    }
};

struct LineVertex {
    glm::vec4 position;
    glm::vec3 color;
};

struct TriangleInstance {
    glm::vec3 diffuseColor;
    float ambientFactor;//ambient=diffuseColor*ambientFactor
    float specularBrightness;//specular=vec4(1.0)*specularBrightness
    float shininess;
    glm::mat4 transformation;
};

class Mesh {
public:
    std::map<LdrColor *, std::vector<TriangleVertex> *> triangleVertices;
    std::map<LdrColor *, std::vector<unsigned int> *> triangleIndices;

    std::vector<LineVertex> lineVertices;
    std::vector<unsigned int> lineIndices;

    std::map<LdrColor *, unsigned int> VAOs, vertexVBOs, instanceVBOs, EBOs;

    std::vector<std::pair<LdrColor *, glm::mat4>> instances;
    bool instancesHaveChanged = false;

    std::string name = "?";

    Mesh()=default;

    void addLdrFile(const LdrFile &file);

    void addLdrFile(const LdrFile &file, LdrColor *mainColor);

    void addLdrFile(const LdrFile &file, glm::mat4 transformation, LdrColor *mainColor);

    void addLdrSubfileReference(LdrColor *mainColor,
                                LdrSubfileReference *sfElement,
                                glm::mat4 transformation);

    void printTriangles();

    void addTriangleVertex(glm::vec4 pos, glm::vec3 normal, LdrColor *color);

    void addLineVertex(const LineVertex &vertex);

    void addLdrLine(LdrColor *mainColor, const LdrLine &lineElement, glm::mat4 transformation);

    void addLdrTriangle(LdrColor *mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation);

    void addLdrQuadrilateral(LdrColor *mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation);

    std::vector<unsigned int> *getIndicesList(LdrColor *color);

    std::vector<TriangleVertex> *getVerticesList(LdrColor *color);

    void writeGraphicsData();
    void drawTriangleGraphics(const Shader *triangleShader);

    void drawLineGraphics(const Shader *lineShader);

    void deallocateGraphics();

    virtual ~Mesh();
private:

    //this is the conversion from the ldraw coordinate system to the OpenGL coordinate system
    glm::mat4 globalModel = glm::scale(glm::rotate(glm::mat4(1.0f),//base
                        glm::radians(180.0f),//rotate 180° around
                        glm::vec3(1.0f, 0.0f, 0.0f)),// x axis
            glm::vec3(constants::LDU_TO_OPENGL, constants::LDU_TO_OPENGL, constants::LDU_TO_OPENGL)); // and make 100 times smaller
    unsigned int lineVAO, lineVertexVBO, lineInstanceVBO, lineEBO;

    bool already_initialized = false;

    static void setInstanceColor(TriangleInstance *instance, const LdrColor *color) ;

    void bindBuffers(LdrColor *color);

    TriangleInstance * generateInstancesArray(const LdrColor *color);

    void initializeLineGraphics();

    void initializeTriangleGraphics();

    void rewriteInstanceBuffer();
};

#endif //BRICKSIM_MESH_H
