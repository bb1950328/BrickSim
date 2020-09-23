// mesh.h
// Created by bab21 on 20.09.20.
//

#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

#include <glm/glm.hpp>
#include <vector>
#include "ldr_objects.h"

struct Vertex {
    //Vertex(const glm::vec4 &position, const glm::vec3 &normal, const glm::vec3 &color);

    glm::vec4 position;
    glm::vec3 normal;
    //glm::vec3 color;
    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal;
    }
};

struct Line {
    glm::vec4 start;
    glm::vec4 end;
    glm::vec3 color;
};

class Mesh {
public:
    std::map<LdrColor*, std::vector<Vertex>*> vertices;
    std::map<LdrColor*, std::vector<unsigned int>*> indices;
    std::vector<Line> lines;

    Mesh() = default;

    void addLdrFile(const LdrFile &file);

    void addLdrFile(const LdrFile &file, glm::mat4 transformation, LdrColor *mainColor);

    void addLdrSubfileReference(LdrColor *mainColor,
                                LdrSubfileReference *sfElement,
                                glm::mat4 transformation);



    void printTriangles();

    void addVertex(glm::vec4 pos, glm::vec3 normal, LdrColor *color);

    void addLdrTriangle(LdrColor *mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation);

    void addLdrFile(const LdrFile &file, LdrColor *color);

    void addLdrLine(LdrColor *mainColor, const LdrLine &lineElement, glm::mat4 transformation);

    void addLdrQuadrilateral(LdrColor *mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation);

    std::vector<unsigned int> *getIndicesList(LdrColor *color);
    std::vector<Vertex> *getVerticesList(LdrColor *color);
};

#endif //BRICKSIM_MESH_H
