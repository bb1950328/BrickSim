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
#include "helpers/camera.h"
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
    glm::vec3 idColor;
    glm::mat4 transformation;
};

struct MeshInstance {
    LdrColor * color;
    glm::mat4 transformation;
    unsigned int elementId;
    bool operator==(const MeshInstance& other) const;
    bool operator!=(const MeshInstance& other) const;
};

class Mesh {
public:
    std::map<LdrColor *, std::vector<TriangleVertex> *> triangleVertices;
    std::map<LdrColor *, std::vector<unsigned int> *> triangleIndices;

    std::vector<LineVertex> lineVertices;
    std::vector<unsigned int> lineIndices;

    std::vector<LineVertex> optionalLineVertices;

    std::map<LdrColor *, unsigned int> VAOs, vertexVBOs, instanceVBOs, EBOs;

    std::vector<MeshInstance> instances;
    bool instancesHaveChanged = false;

    std::string name = "?";

    Mesh()=default;

    void addLdrFile(const LdrFile &file, glm::mat4 transformation, LdrColor *mainColor, bool bfcInverted);

    void addLdrSubfileReference(LdrColor *mainColor, LdrSubfileReference *sfElement, glm::mat4 transformation, bool bfcInverted);

    void printTriangles();

    void addLineVertex(const LineVertex &vertex);

    void addLdrLine(LdrColor *mainColor, const LdrLine &lineElement, glm::mat4 transformation);

    void addLdrTriangle(LdrColor *mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation, bool bfcInverted);

    void addLdrQuadrilateral(LdrColor *mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation, bool bfcInverted);

    void addLdrOptionalLine(LdrColor *mainColor, const LdrOptionalLine &optionalLineElement, glm::mat4 transformation);

    std::vector<unsigned int> *getIndicesList(LdrColor *color);
    std::vector<TriangleVertex> *getVerticesList(LdrColor *color);

    void writeGraphicsData();

    void drawTriangleGraphics();
    void drawLineGraphics();
    void drawOptionalLineGraphics();

    void deallocateGraphics();
    virtual ~Mesh();

    //this is the conversion from the ldraw coordinate system to the OpenGL coordinate system
    glm::mat4 globalModel = glm::scale(glm::rotate(glm::mat4(1.0f),//base
                                                   glm::radians(180.0f),//rotate 180° around
                                                   glm::vec3(1.0f, 0.0f, 0.0f)),// x axis
                                       glm::vec3(constants::LDU_TO_OPENGL, constants::LDU_TO_OPENGL, constants::LDU_TO_OPENGL)); // and make 100 times smaller

private:
    unsigned int lineVAO, lineVertexVBO, lineInstanceVBO, lineEBO;

    unsigned int optionalLineVAO, optionalLineVertexVBO, optionalLineInstanceVBO;

    bool already_initialized = false;

    static void setInstanceColor(TriangleInstance *instance, const LdrColor *color) ;

    TriangleInstance * generateInstancesArray(const LdrColor *color);

    void initializeTriangleGraphics();

    void initializeLineGraphics();

    void initializeOptionalLineGraphics();

    void rewriteInstanceBuffer();
};

#endif //BRICKSIM_MESH_H
