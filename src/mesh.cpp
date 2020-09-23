// mesh.cpp
// Created by bab21 on 20.09.20.
//

#include <iostream>
#include "mesh.h"
#include <glm/gtx/normal.hpp>
#include <glm/gtx/string_cast.hpp>

void Mesh::addLdrFile(const LdrFile &file) {
    addLdrFile(file, glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
}

void Mesh::addLdrFile(const LdrFile &file, const LdrColor &color) {
    addLdrFile(file, glm::mat4(1.0f), color.asGlmVector());
}

void Mesh::addLdrFile(const LdrFile &file, glm::mat4 transformation, glm::vec3 mainColor) {
    for (auto element : file.elements) {
        switch (element->getType()) {
            case 0:
                break;
            case 1:
                addLdrSubfileReference(mainColor, dynamic_cast<LdrSubfileReference *>(element), transformation);
                break;
            case 2:
                addLdrLine(mainColor, dynamic_cast<LdrLine &&>(*element), transformation);
                break;
            case 3:
                addLdrTriangle(mainColor, dynamic_cast<LdrTriangle &&>(*element), transformation);
                break;
            case 4:
                addLdrQuadrilateral(mainColor, dynamic_cast<LdrQuadrilateral &&>(*element), transformation);
                break;
            case 5:
                break;//todo implement
        }
    }
}

void Mesh::addLdrTriangle(const glm::vec3 &mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation) {
    auto p1 = glm::vec3(triangleElement.x1, triangleElement.y1, triangleElement.z1);
    auto p2 = glm::vec3(triangleElement.x2, triangleElement.y2, triangleElement.z2);
    auto p3 = glm::vec3(triangleElement.x3, triangleElement.y3, triangleElement.z3);
    addTriangle(triangleElement.color->code == 16 ? mainColor : triangleElement.color->asGlmVector(), transformation,
                p1, p2, p3);
}

void Mesh::addTriangle(const glm::vec3 &color,
                       const glm::mat4 &transformation,
                       const glm::vec3 &p1,
                       const glm::vec3 &p2,
                       const glm::vec3 &p3) {
    auto normal = glm::triangleNormal(p1, p2, p3);
    addVertex(glm::vec4(p1, 1.0f) * transformation, normal, color);
    addVertex(glm::vec4(p2, 1.0f) * transformation, normal, color);
    addVertex(glm::vec4(p3, 1.0f) * transformation, normal, color);
}

void Mesh::addVertex(glm::vec4 pos, glm::vec3 normal, glm::vec3 color) {
    indices.push_back(vertices.size());//todo reuse vertices if they are equal
    Vertex vertex{pos, /*normal,*/ color};
    vertices.push_back(vertex);
}

void Mesh::addLdrSubfileReference(const glm::vec3 &mainColor, LdrSubfileReference *sfElement,
                                  glm::mat4 transformation) {
    auto sub_transformation = glm::mat4();
    sub_transformation[0] = glm::vec4(sfElement->a, sfElement->b, sfElement->c, sfElement->x * 1);
    sub_transformation[1] = glm::vec4(sfElement->d, sfElement->e, sfElement->f, sfElement->y * 1);
    sub_transformation[2] = glm::vec4(sfElement->g, sfElement->h, sfElement->i, sfElement->z * 1);
    sub_transformation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const glm::vec3 &color = sfElement->color->code == 16 ? mainColor : sfElement->color->asGlmVector();
    addLdrFile(*sfElement->getFile(), sub_transformation * transformation, color);
}

void Mesh::addLdrQuadrilateral(glm::vec3 mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation) {
    auto p1 = glm::vec3(quadrilateral.x1, quadrilateral.y1, quadrilateral.z1);
    auto p2 = glm::vec3(quadrilateral.x2, quadrilateral.y2, quadrilateral.z2);
    auto p3 = glm::vec3(quadrilateral.x3, quadrilateral.y3, quadrilateral.z3);
    auto p4 = glm::vec3(quadrilateral.x4, quadrilateral.y4, quadrilateral.z4);
    const glm::vec3 &color = quadrilateral.color->code == 16 ? mainColor : quadrilateral.color->asGlmVector();
    addTriangle(color, transformation, p1, p2, p3);
    addTriangle(color, transformation, p3, p4, p1);
}

void Mesh::printTriangles() {
    for (int i = 0; i < indices.size(); i += 3) {
        auto v1 = vertices[i];
        auto v2 = vertices[i + 1];
        auto v3 = vertices[i + 2];
        std::cout << "Triangle " << i / 3;
        std::cout << " color=" << glm::to_string(v1.color);
        std::cout << " cords=(" << glm::to_string(v1.position);
        std::cout << ", " << glm::to_string(v2.position);
        std::cout << ", " << glm::to_string(v3.position) << "\n";
    }
}

void Mesh::addLdrLine(const glm::vec3 &mainColor, const LdrLine &lineElement, glm::mat4 transformation) {
    auto p1 = glm::vec3(lineElement.x1, lineElement.y1, lineElement.z1);
    auto p2 = glm::vec3(lineElement.x2, lineElement.y2, lineElement.z2);

    Line line{
            glm::vec4(p1, 1.0f) * transformation,
            glm::vec4(p2, 1.0f) * transformation,
            lineElement.color->code == 16 ? mainColor : lineElement.color->asGlmVector()
    };
    lines.push_back(line);
}

/*Vertex::Vertex(const glm::vec4 &position, const glm::vec3 &normal, const glm::vec3 &color) {
    this->position = 0.1f*position;
    this->color = color;
    this->normal = normal;
}*/