// mesh.cpp
// Created by bab21 on 20.09.20.
//

#include <iostream>
#include "mesh.h"

void Mesh::addLdrFile(const LdrFile& file) {
    addLdrFile(file, glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
}

void Mesh::addLdrFile(const LdrFile &file, glm::mat4 transformation, glm::vec3 mainColor) {
    for (auto element : file.elements) {
        switch (element->getType()) {
            case 0:
                break;
            case 1:
                addLdrSubfileReference(dynamic_cast<LdrSubfileReference &&>(*element), transformation);
                break;
            case 2:
                break;//todo implement
            case 3:
                addLdrTriangle(mainColor, reinterpret_cast<LdrTriangle &&>(element), transformation);
                break;
            case 4:
                addLdrQuadrilateral(mainColor, reinterpret_cast<LdrQuadrilateral &&>(element), transformation);
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

void Mesh::addTriangle(const glm::vec3 &color, const glm::mat4 transformation, const glm::vec3 &p1, const glm::vec3 &p2,
                       const glm::vec3 &p3) {
    auto normal = glm::cross(p1 - p2, p2 - p3);//todo this is not always correct
    addVertex(glm::vec4(p1, 1.0f) * transformation, normal, color);
    addVertex(glm::vec4(p2, 1.0f) * transformation, normal, color);
    addVertex(glm::vec4(p3, 1.0f) * transformation, normal, color);
}

void Mesh::addVertex(glm::vec4 pos, glm::vec3 normal, glm::vec3 color) {
    indicies.push_back(vertices.size());//todo reuse vertices if they are equal
    vertices.emplace_back(pos, normal, color);
}

void Mesh::addLdrSubfileReference(const LdrSubfileReference &castedElement, glm::mat4 transformation) {
    auto sub_transformation = glm::mat4();
    sub_transformation[0] = glm::vec4(castedElement.a, castedElement.b, castedElement.c, castedElement.x);
    sub_transformation[1] = glm::vec4(castedElement.d, castedElement.e, castedElement.f, castedElement.y);
    sub_transformation[2] = glm::vec4(castedElement.g, castedElement.h, castedElement.i, castedElement.z);
    sub_transformation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    addLdrFile(*castedElement.file, transformation*sub_transformation, castedElement.color->asGlmVector());//todo unsure if i can just multiply these two matrices
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
    for (int i = 0; i < indicies.size(); i+=3) {
        auto v1 = vertices[i];
        auto v2 = vertices[i+1];
        auto v3 = vertices[i+2];
        std::cout << "Triangle " << i/3;
        std::cout << " color=" << v1.color.x*255 << "," << v1.color.y*255 << "," << v1.color.z*255 << " coords=(";
        std::cout << v1.position.x << ", " << v1.position.y << ", " << v1.position.z<<")-(";
        std::cout << v2.position.x << ", " << v2.position.y << ", " << v2.position.z<<")-(";
        std::cout << v3.position.x << ", " << v3.position.y << ", " << v3.position.z<<")\n";
    }
}

Vertex::Vertex(const glm::vec4 &position, const glm::vec3 &normal, const glm::vec3 &color) {
    this->position = 0.00005f*position;
    this->color = color;
    this->normal = normal;
}
