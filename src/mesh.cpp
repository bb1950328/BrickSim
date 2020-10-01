// mesh.cpp
// Created by bab21 on 20.09.20.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "mesh.h"
#include "camera.h"
#include "config.h"
#include "util.h"
#include <glm/gtx/normal.hpp>
#include <glm/gtx/string_cast.hpp>

void Mesh::addLdrFile(const LdrFile &file) {
    LdrColor *defaultColor = LdrColorRepository::getInstance()->get_color(1);
    addLdrFile(file, glm::mat4(1.0f), defaultColor);
}

void Mesh::addLdrFile(const LdrFile &file, LdrColor *mainColor) {
    addLdrFile(file, glm::mat4(1.0f), mainColor);
}

void Mesh::addLdrFile(const LdrFile &file, glm::mat4 transformation, LdrColor *mainColor) {
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

void Mesh::addLdrTriangle(LdrColor *mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation) {
    auto p1 = glm::vec3(triangleElement.x1, triangleElement.y1, triangleElement.z1);
    auto p2 = glm::vec3(triangleElement.x2, triangleElement.y2, triangleElement.z2);
    auto p3 = glm::vec3(triangleElement.x3, triangleElement.y3, triangleElement.z3);
    LdrColor *color = triangleElement.color->code == 16 ? mainColor : triangleElement.color;
    auto normal = glm::triangleNormal(p1, p2, p3);
    addTriangleVertex(glm::vec4(p1, 1.0f) * transformation, normal, color);
    addTriangleVertex(glm::vec4(p2, 1.0f) * transformation, normal, color);
    addTriangleVertex(glm::vec4(p3, 1.0f) * transformation, normal, color);
}

void Mesh::addTriangleVertex(glm::vec4 pos, glm::vec3 normal, LdrColor *color) {
    TriangleVertex vertex{pos, normal};
    /*for (int i = triangleVertices.size(); i > triangleVertices.size()-10; --i) {//just check the last 10 because the probability that there's a better vertex farther front is very small
        if (triangleVertices[i]==vertex) {
            triangleIndices.push_back(i);
            return;
        }
    }*///todo check if this still gives a performance boost after adding normals
    getIndicesList(color)->push_back(triangleVertices.size());
    getVerticesList(color)->push_back(vertex);
}

void Mesh::addLdrSubfileReference(LdrColor *mainColor, LdrSubfileReference *sfElement, glm::mat4 transformation) {
    long instanced_min = Configuration::getInstance()->get_long(config::KEY_INSTANCED_MIN_COMPLEXITY);
    auto sub_transformation = glm::mat4(
            sfElement->a, sfElement->b, sfElement->c, sfElement->x * 1,
            sfElement->d, sfElement->e, sfElement->f, sfElement->y * 1,
            sfElement->g, sfElement->h, sfElement->i, sfElement->z * 1,
            0.0f, 0.0f, 0.0f, 1.0f
    );
    LdrColor *color = sfElement->color->code == 16 ? mainColor : sfElement->color;
    if (sfElement->getFile()->estimatedComplexity > instanced_min && sfElement->getFile()->referenceCount > 1) {
        collection->addLdrFile(color, sfElement->getFile(), sub_transformation * transformation, this);
    } else {
        addLdrFile(*sfElement->getFile(), sub_transformation * transformation, color);
    }
}

void Mesh::addLdrQuadrilateral(LdrColor *mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation) {
    auto p1 = glm::vec4(quadrilateral.x1, quadrilateral.y1, quadrilateral.z1, 1.0f) * transformation;
    auto p2 = glm::vec4(quadrilateral.x2, quadrilateral.y2, quadrilateral.z2, 1.0f) * transformation;
    auto p3 = glm::vec4(quadrilateral.x3, quadrilateral.y3, quadrilateral.z3, 1.0f) * transformation;
    auto p4 = glm::vec4(quadrilateral.x4, quadrilateral.y4, quadrilateral.z4, 1.0f) * transformation;
    LdrColor *color = quadrilateral.color->code == 16 ? mainColor : quadrilateral.color;
    auto normal = glm::triangleNormal(glm::vec3(p1), glm::vec3(p2), glm::vec3(p3));

    TriangleVertex vertex1{p1, normal};
    TriangleVertex vertex2{p2, normal};
    TriangleVertex vertex3{p3, normal};
    TriangleVertex vertex4{p4, normal};
    auto vertices_list = getVerticesList(color);
    unsigned int idx = vertices_list->size();
    vertices_list->push_back(vertex1);
    vertices_list->push_back(vertex2);
    vertices_list->push_back(vertex3);
    vertices_list->push_back(vertex4);

    auto indices_list = getIndicesList(color);
    //triangle 1
    indices_list->push_back(idx);
    indices_list->push_back(idx + 1);
    indices_list->push_back(idx + 2);

    //triangle 2
    indices_list->push_back(idx + 2);
    indices_list->push_back(idx + 3);
    indices_list->push_back(idx);
}

std::vector<unsigned int> *Mesh::getIndicesList(LdrColor *color) {
    auto entry = triangleIndices.find(color);
    if (entry == triangleIndices.end()) {
        auto vec = new std::vector<unsigned int>;
        triangleIndices[color] = vec;
        //std::cout << "created index vector for " << color->name << "\n";
        return vec;
    }
    return entry->second;
}


std::vector<TriangleVertex> *Mesh::getVerticesList(LdrColor *color) {
    auto entry = triangleVertices.find(color);
    if (entry == triangleVertices.end()) {
        auto vec = new std::vector<TriangleVertex>;
        triangleVertices[color] = vec;
        //std::cout << "created vertex vector for " << color->name << "\n";
        return vec;
    }
    return entry->second;
}

void Mesh::printTriangles() {
    /*for (int i = 0; i < triangleIndices.size(); i += 3) {
        auto v1 = triangleVertices[i];
        auto v2 = triangleVertices[i + 1];
        auto v3 = triangleVertices[i + 2];
        std::cout << "Triangle " << i / 3;
        std::cout << " cords=(" << glm::to_string(v1.position);
        std::cout << ", " << glm::to_string(v2.position);
        std::cout << ", " << glm::to_string(v3.position) << "\n";
    }*/
    //todo remove or use
}

void Mesh::addLdrLine(LdrColor *mainColor, const LdrLine &lineElement, glm::mat4 transformation) {

    const glm::vec3 &color =
            lineElement.color->code == 16
            ? mainColor->edge.asGlmVector()
            : lineElement.color->edge.asGlmVector();
    LineVertex lv1{glm::vec4(lineElement.x1, lineElement.y1, lineElement.z1, 1.0f) * transformation, color};
    LineVertex lv2{glm::vec4(lineElement.x2, lineElement.y2, lineElement.z2, 1.0f) * transformation, color};
    addLineVertex(lv1);
    addLineVertex(lv2);
}

void Mesh::addLineVertex(const LineVertex &vertex) {
    for (int i = lineVertices.size(); i > lineVertices.size() - 12; --i) {
        if (vertex.position == lineVertices[i].position && vertex.color == lineVertices[i].color) {
            //std::cout << lineVertices.size()-i << "\n";
            lineIndices.push_back(i);
            return;
        }
    }
    lineIndices.push_back(lineVertices.size());
    lineVertices.push_back(vertex);
}

void Mesh::initializeGraphics() {
    const auto instance_count = getTotalInstanceCount();
    std::cout << "Mesh " << name << " Total Instance Count: " << instance_count << std::endl;
    for (const auto &entry: triangleIndices) {
        LdrColor *color = entry.first;
        std::vector<unsigned int> *indices = entry.second;
        std::vector<TriangleVertex> *vertices = triangleVertices.find(color)->second;

        unsigned int vao, vertexVbo, instanceVbo, ebo;

        //vao
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        //vertexVbo
        glGenBuffers(1, &vertexVbo);
        glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
        size_t vertex_size = sizeof(TriangleVertex);
        glBufferData(GL_ARRAY_BUFFER, vertices->size() * vertex_size, &(*vertices)[0], GL_STATIC_DRAW);

        // position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void *) nullptr);
        // normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) offsetof(TriangleVertex, normal));

        //instanceVbo
        auto instancesArray = generateInstancesArray(color);
        /*std::cout << color->name << std::endl;
        for (int i = 0; i < instance_count; ++i) {
            util::cout_mat4(instancesArray[i].transformation);
        }
        std::cout << "--------------------------------------------------------\n";*/

        glGenBuffers(1, &instanceVbo);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
        size_t instance_size = sizeof(Instance);
        glBufferData(GL_ARRAY_BUFFER, instance_count * instance_size, &instancesArray[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(Instance, diffuseColor));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(Instance, ambientFactor));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(Instance, specularBrightness));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(Instance, shininess));
        for (int j = 0; j < 4; ++j) {
            glEnableVertexAttribArray(6 + j);
            glVertexAttribPointer(6 + j, 4, GL_FLOAT, GL_FALSE, instance_size,
                                  (void *) (offsetof(Instance, transformation) + 4 * j * sizeof(float)));
        }

        for (int i = 2; i < 10; ++i) {
            glVertexAttribDivisor(i, 1);
        }
        delete instancesArray;

        //ebo
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices->size(), &(*indices)[0], GL_STATIC_DRAW);

        VAOs[color] = vao;
        vertexVBOs[color] = vertexVbo;
        instanceVBOs[color] = instanceVbo;
        EBOs[color] = ebo;
    }
}

void Mesh::drawGraphics(const Shader *triangleShader) {
    for (const auto &entry: triangleIndices) {
        LdrColor *color = entry.first;
        std::vector<unsigned int> *indices = entry.second;
        bindBuffers(color);
        glDrawElementsInstanced(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, nullptr, getTotalInstanceCount());
    }
}

void Mesh::bindBuffers(LdrColor *color) {
    unsigned int vao = VAOs[color];
    glBindVertexArray(vao);
}

void Mesh::deallocateGraphics() {
    for (const auto &entry: triangleIndices) {
        LdrColor *color = entry.first;
        unsigned int vao = VAOs[color];
        unsigned int vertexVbo = vertexVBOs[color];
        unsigned int instanceVbo = instanceVBOs[color];
        unsigned int ebo = EBOs[color];
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vertexVbo);
        glDeleteBuffers(1, &instanceVbo);
        glDeleteBuffers(1, &ebo);
    }
}

Mesh::Mesh(MeshCollection *collection) : collection(collection) {
}

Mesh::~Mesh() {
    for (const auto &entry: triangleIndices) {
        delete entry.second;
    }
    for (const auto &entry: triangleVertices) {
        delete entry.second;
    }
}

void Mesh::setInstanceColor(Instance *instance, const LdrColor *color) {
    instance->diffuseColor = color->value.asGlmVector();
    instance->shininess = 32.0f;
    //useful tool: http://www.cs.toronto.edu/~jacobson/phong-demo/
    switch (color->finish) {
        case LdrColor::METAL:
        case LdrColor::CHROME:
        case LdrColor::PEARLESCENT:
            //todo find out what's the difference
            instance->shininess *= 2;
            instance->ambientFactor = 1;
            instance->specularBrightness = 1;
            break;
        case LdrColor::MATTE_METALLIC:
            instance->ambientFactor = 0.6;
            instance->specularBrightness = 0.2;
            break;
        case LdrColor::RUBBER:
            instance->ambientFactor = 0.75;
            instance->specularBrightness = 0;
            break;
        default:
            instance->ambientFactor = 0.5;
            instance->specularBrightness = 0.5;
            break;
    }
}

size_t Mesh::getTotalInstanceCount() const {
    if (parentInstances.empty()) {
        return instances.size();
    } else {
        return parentInstances.size() * instances.size();
    }
}

Instance *Mesh::generateInstancesArray(const LdrColor *color) {
    auto instance_count = getTotalInstanceCount();
    auto *instancesArray = new Instance[instance_count];
    unsigned int arr_cursor = 0;
    //todo optimize this method
    if (parentInstances.empty()) {
        if (color == &LdrColorRepository::instDummyColor) {
            for (auto &instPair : instances) {
                //todo optimize by creating instance here and copy it
                Instance inst{};
                inst.transformation = glm::transpose(instPair.second * globalModel);
                setInstanceColor(&inst, instPair.first);
                instancesArray[arr_cursor] = inst;
                arr_cursor++;
            }
        } else {
            Instance inst{};
            setInstanceColor(&inst, color);
            std::fill_n(instancesArray, instance_count, inst);
            for (auto &instance : instances) {
                instancesArray[arr_cursor].transformation = glm::transpose(instance.second * globalModel);
                arr_cursor++;
            }
        }
    } else {
        if (color == &LdrColorRepository::instDummyColor) {
            for (auto &instPair : instances) {
                //todo optimize by creating instance here and copy it
                for (const auto &parentTransf: parentInstances) {
                    Instance inst{};
                    inst.transformation = glm::transpose(parentTransf * instPair.second * globalModel);
                    setInstanceColor(&inst, instPair.first);
                    instancesArray[arr_cursor] = inst;
                    arr_cursor++;
                }
            }
        } else {
            Instance inst{};
            setInstanceColor(&inst, color);
            std::fill_n(instancesArray, instance_count, inst);
            for (auto &instance : instances) {
                for (const auto &parentTransf: parentInstances) {
                    instancesArray[arr_cursor].transformation = glm::transpose(
                            parentTransf * instance.second * globalModel);
                    arr_cursor++;
                }
            }
        }
    }
    return instancesArray;
}

void Mesh::addInstance(LdrColor *color, glm::mat4 transformation) {
    instances.emplace_back(color, transformation);
    for (auto const &subMesh: subMeshes) {
        subMesh->addParentInstance(transformation);
    }
}

void Mesh::addParentInstance(glm::mat4 transformation) {
    parentInstances.push_back(transformation);
    for (const auto &inst: instances) {
        for (const auto &sub: subMeshes) {
            sub->addParentInstance(transformation * inst.second);
        }
    }
}

void Mesh::addSubMesh(Mesh *newSubMesh) {
    if (subMeshes.find(newSubMesh) == subMeshes.end()) {
        subMeshes.insert(newSubMesh);
        for (const auto &inst: instances) {
            newSubMesh->addParentInstance(inst.second);
        }
    }
}


/*TriangleVertex::TriangleVertex(const glm::vec4 &position, const glm::vec3 &normal, const glm::vec3 &color) {
    this->position = 0.1f*position;
    this->color = color;
    this->normal = normal;
}*/
MeshCollection::MeshCollection() {
    //todo meshes["default"] = std::make_pair()
}

void MeshCollection::addLdrFile(LdrColor *mainColor, LdrFile *file, glm::mat4 transformation) {
    addLdrFile(mainColor, file, transformation, nullptr);
}

void MeshCollection::addLdrFile(LdrColor *mainColor, LdrFile *file, glm::mat4 transformation, Mesh *parentMesh) {
    //auto pair = std::make_pair(mainColor, transformation);
    auto it = meshes.find(file);
    Mesh *mesh;
    if (it != meshes.end()) {
        mesh = it->second;
    } else {
        mesh = new Mesh(this);
        meshes[file] = mesh;
        mesh->name = file->getDescription();
        mesh->addLdrFile(*file, &LdrColorRepository::instDummyColor);
    }
    mesh->addInstance(mainColor, transformation);
    if (nullptr != parentMesh) {
        parentMesh->addSubMesh(mesh);
    }
}

void MeshCollection::initializeGraphics() {
    for (const auto &pair: meshes) {
        pair.second->initializeGraphics();
    }
}

void MeshCollection::drawGraphics(Shader *triangleShader) {
    for (const auto &pair: meshes) {
        pair.second->drawGraphics(triangleShader);
    }
}

void MeshCollection::deallocateGraphics() {
    for (const auto &pair: meshes) {
        pair.second->deallocateGraphics();
    }
}
