// mesh.h
// Created by bab21 on 20.09.20.
//

#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

static const float SCALE = 0.1f;

#include <glm/glm.hpp>
#include <vector>
#include "ldr_objects.h"

struct Vertex {
    //Vertex(const glm::vec4 &position, const glm::vec3 &normal, const glm::vec3 &color);

    glm::vec4 position;
    //glm::vec3 normal;//todo reenable
    glm::vec3 color;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned long> indices;
    Mesh()=default;
    void addLdrFile(const LdrFile& file);
    void addLdrFile(const LdrFile& file, glm::mat4 transformation, glm::vec3 mainColor);

    void addLdrSubfileReference(const LdrSubfileReference &castedElement, glm::mat4 transformation);

    void addVertex(glm::vec4 pos, glm::vec3 normal, glm::vec3 color);

    void addTriangle(const glm::vec3 &color, const glm::mat4 transformation, const glm::vec3 &p1, const glm::vec3 &p2,
                     const glm::vec3 &p3);

    void addLdrTriangle(const glm::vec3 &mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation);

    void addLdrQuadrilateral(glm::vec3 mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation);

    void printTriangles();
};

#endif //BRICKSIM_MESH_H
