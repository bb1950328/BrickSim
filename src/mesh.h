// mesh.h
// Created by bab21 on 20.09.20.
//

#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

#include <glm/glm.hpp>
#include <vector>
#include "ldr_objects.h"
#include "shaders/shader.h"
#include "camera.h"

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

class MeshCollection;

class Mesh {
public:
    std::map<LdrColor *, std::vector<TriangleVertex> *> triangleVertices;
    std::map<LdrColor *, std::vector<unsigned int> *> triangleIndices;

    std::vector<LineVertex> lineVertices;
    std::vector<unsigned int> lineIndices;

    std::map<LdrColor *, unsigned int> VAOs, VBOs, EBOs;

    std::map<LdrColor *, std::vector<glm::mat4>> instances;

    MeshCollection *collection;

    explicit Mesh(MeshCollection *collection);

    void addLdrFile(const LdrFile &file);

    void addLdrFile(const LdrFile &file, LdrColor *color);

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

    void initializeGraphics();

    void drawGraphics(const Shader *triangleShader, const CadCamera *camera, glm::vec3 lightPos);

    void deallocateGraphics();

private:
    //this is the conversion from the ldraw coordinate system to the OpenGL coordinate system
    glm::mat4 globalModel = glm::scale(glm::rotate(glm::mat4(1.0f),//base
                        glm::radians(180.0f),//rotate 180° around
                        glm::vec3(1.0f, 0.0f, 0.0f)),// x axis
            glm::vec3(0.01f, 0.01f, 0.01f)); // and make 100 times smaller
    void changeShaderColor(const Shader *triangleShader, const LdrColor *color) const;

    void bindBuffers(LdrColor *color);
};

class MeshCollection {
public:
    std::map<LdrFile *, Mesh> meshes;

    MeshCollection();

    void addLdrFile(LdrColor *mainColor, LdrFile *file, glm::mat4 transformation);
};

#endif //BRICKSIM_MESH_H
