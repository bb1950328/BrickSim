// mesh.h
// Created by bab21 on 20.09.20.
//

#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

#include <glm/glm.hpp>
#include <vector>
#include "ldr_objects.h"

struct TriangleVertex {
    //TriangleVertex(const glm::vec4 &position, const glm::vec3 &normal, const glm::vec3 &color);

    glm::vec4 position;
    glm::vec3 normal;
    //glm::vec3 color;
    bool operator==(const TriangleVertex& other) const {
        return position == other.position && normal == other.normal;
    }
};

struct LineVertex {
    glm::vec4 position;
    glm::vec3 color;
};

class TriangleMesh {
public:
    std::map<LdrColor*, std::vector<TriangleVertex>*> triangleVertices;
    std::map<LdrColor*, std::vector<unsigned int>*> triangleIndices;

    std::vector<LineVertex> lineVertices;
    std::vector<unsigned int> lineIndices;

    TriangleMesh() = default;

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
};

#endif //BRICKSIM_MESH_H
