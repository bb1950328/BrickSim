#include <glad/glad.h>

#include "mesh.h"
#include "config.h"
#include <glm/gtx/normal.hpp>
#include <mutex>
#include "metrics.h"
#include "lib/Miniball.hpp"
#include "controller.h"

void Mesh::addLdrFile(const std::shared_ptr<LdrFile> &file, glm::mat4 transformation = glm::mat4(1.0f), const LdrColorReference mainColor = {}, bool bfcInverted = false) {
    for (const auto &element : file->elements) {
        switch (element->getType()) {
            case 0: break;
            case 1:addLdrSubfileReference(mainColor, std::dynamic_pointer_cast<LdrSubfileReference>(element), transformation, bfcInverted);
                break;
            case 2:addLdrLine(mainColor, dynamic_cast<LdrLine &&>(*element), transformation);
                break;
            case 3:addLdrTriangle(mainColor, dynamic_cast<LdrTriangle &&>(*element), transformation, bfcInverted);
                break;
            case 4:addLdrQuadrilateral(mainColor, dynamic_cast<LdrQuadrilateral &&>(*element), transformation, bfcInverted);
                break;
            case 5:addLdrOptionalLine(mainColor, dynamic_cast<LdrOptionalLine &&>(*element), transformation);
                break;
        }
    }
}

void Mesh::addLdrTriangle(const LdrColorReference mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation, bool bfcInverted) {
    const auto color = triangleElement.color.get()->code == LdrColor::MAIN_COLOR_CODE ? mainColor : triangleElement.color;
    auto &verticesList = getVerticesList(color);
    auto &indicesList = getIndicesList(color);
    auto p1 = glm::vec3(triangleElement.x1, triangleElement.y1, triangleElement.z1);
    auto p2 = glm::vec3(triangleElement.x2, triangleElement.y2, triangleElement.z2);
    auto p3 = glm::vec3(triangleElement.x3, triangleElement.y3, triangleElement.z3);
    auto normal = glm::triangleNormal(p1, p2, p3);
    auto transformedNormal = glm::normalize(glm::vec4(normal, 0.0f) * transformation);
    TriangleVertex vertex1{glm::vec4(p1, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex2{glm::vec4(p2, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex3{glm::vec4(p3, 1.0f) * transformation, transformedNormal};

    if (util::doesTransformationInverseWindingOrder(transformation) ^ bfcInverted) {
        std::swap(vertex2, vertex3);
    }

    auto idx1 = verticesList.size();
    verticesList.push_back(vertex1);
    verticesList.push_back(vertex2);
    verticesList.push_back(vertex3);
    indicesList.push_back(idx1);
    indicesList.push_back(idx1 + 1);
    indicesList.push_back(idx1 + 2);

    if (config::getBool(config::SHOW_NORMALS)) {
        auto lp1 = glm::vec4(util::triangleCentroid(p1, p2, p3), 1.0f) * transformation;
        auto lp2 = lp1 + (transformedNormal * 5.0f);
        LineVertex lv1{lp1, transformedNormal};
        LineVertex lv2{lp2, transformedNormal};
        addLineVertex(lv1);
        addLineVertex(lv2);
    }
}

void Mesh::addRawTriangle(const LdrColorReference color, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
    auto &verticesList = getVerticesList(color);
    auto &indicesList = getIndicesList(color);
    auto normal = glm::triangleNormal(p1, p2, p3);
    auto transformedNormal = glm::normalize(glm::vec4(normal, 0.0f));
    TriangleVertex vertex1{glm::vec4(p1, 1.0f), transformedNormal};
    TriangleVertex vertex2{glm::vec4(p2, 1.0f), transformedNormal};
    TriangleVertex vertex3{glm::vec4(p3, 1.0f), transformedNormal};

    auto idx1 = verticesList.size();
    verticesList.push_back(vertex1);
    verticesList.push_back(vertex2);
    verticesList.push_back(vertex3);
    indicesList.push_back(idx1);
    indicesList.push_back(idx1 + 1);
    indicesList.push_back(idx1 + 2);

    if (config::getBool(config::SHOW_NORMALS)) {
        auto lp1 = glm::vec4(util::triangleCentroid(p1, p2, p3), 1.0f);
        auto lp2 = lp1 + (transformedNormal * 5.0f);
        LineVertex lv1{lp1, transformedNormal};
        LineVertex lv2{lp2, transformedNormal};
        addLineVertex(lv1);
        addLineVertex(lv2);
    }
}

void Mesh::addLdrSubfileReference(LdrColorReference mainColor, const std::shared_ptr<LdrSubfileReference> &sfElement, glm::mat4 transformation, bool bfcInverted) {
    auto sub_transformation = sfElement->getTransformationMatrix();
    const auto color = sfElement->color.get()->code == LdrColor::MAIN_COLOR_CODE ? mainColor : sfElement->color;
    addLdrFile(sfElement->getFile(), sub_transformation * transformation, color, sfElement->bfcInverted ^ bfcInverted);
}

void Mesh::addLdrQuadrilateral(LdrColorReference mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation, bool bfcInverted) {
    auto p1 = glm::vec3(quadrilateral.x1, quadrilateral.y1, quadrilateral.z1);
    auto p2 = glm::vec3(quadrilateral.x2, quadrilateral.y2, quadrilateral.z2);
    auto p3 = glm::vec3(quadrilateral.x3, quadrilateral.y3, quadrilateral.z3);
    auto p4 = glm::vec3(quadrilateral.x4, quadrilateral.y4, quadrilateral.z4);
    const auto color = quadrilateral.color.get()->code == LdrColor::MAIN_COLOR_CODE ? mainColor : quadrilateral.color;
    auto normal = glm::triangleNormal(p1, p2, p3);
    auto transformedNormal = glm::normalize(glm::vec4(normal, 0.0f) * transformation);

    TriangleVertex vertex1{glm::vec4(p1, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex2{glm::vec4(p2, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex3{glm::vec4(p3, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex4{glm::vec4(p4, 1.0f) * transformation, transformedNormal};

    if (util::doesTransformationInverseWindingOrder(transformation) ^ bfcInverted) {
        std::swap(vertex2, vertex4);
    }

    auto &vertices_list = getVerticesList(color);
    unsigned int idx = vertices_list.size();
    vertices_list.push_back(vertex1);
    vertices_list.push_back(vertex2);
    vertices_list.push_back(vertex3);
    vertices_list.push_back(vertex4);

    auto &indices_list = getIndicesList(color);
    //triangle 1
    indices_list.push_back(idx);
    indices_list.push_back(idx + 1);
    indices_list.push_back(idx + 2);

    //triangle 2
    indices_list.push_back(idx + 2);
    indices_list.push_back(idx + 3);
    indices_list.push_back(idx);

    if (config::getBool(config::SHOW_NORMALS)) {
        auto lp1 = glm::vec4(util::quadrilateralCentroid(p1, p2, p3, p4), 1.0f) * transformation;
        auto lp2 = lp1 + (transformedNormal * 5.0f);
        LineVertex lv1{lp1, transformedNormal};
        LineVertex lv2{lp2, transformedNormal};
        addLineVertex(lv1);
        addLineVertex(lv2);
    }
}

std::vector<unsigned int> &Mesh::getIndicesList(const LdrColorReference color) {
    auto entry = triangleIndices.find(color);
    if (entry == triangleIndices.end()) {
        return triangleIndices[color] = std::vector<unsigned int>();
    }
    return entry->second;
}


std::vector<TriangleVertex> &Mesh::getVerticesList(const LdrColorReference color) {
    auto entry = triangleVertices.find(color);
    if (entry == triangleVertices.end()) {
        return triangleVertices[color] = std::vector<TriangleVertex>();
    }
    return entry->second;
}

void Mesh::addLdrLine(const LdrColorReference mainColor, const LdrLine &lineElement, glm::mat4 transformation) {
    glm::vec3 color;
    const auto lineElementColor = lineElement.color.get();
    const auto mainColorLocked = mainColor.get();
    if (lineElementColor->code == LdrColor::MAIN_COLOR_CODE) {
        color = mainColorLocked->edge.asGlmVector();
    } else if (lineElementColor->code == LdrColor::LINE_COLOR_CODE) {
        color = glm::vec3(1 - util::vectorSum(mainColorLocked->value.asGlmVector()) / 3);//todo look up specification
    } else {
        color = lineElementColor->edge.asGlmVector();
    }
    LineVertex lv1{glm::vec4(lineElement.x1, lineElement.y1, lineElement.z1, 1.0f) * transformation, color};
    LineVertex lv2{glm::vec4(lineElement.x2, lineElement.y2, lineElement.z2, 1.0f) * transformation, color};
    addLineVertex(lv1);
    addLineVertex(lv2);
}

void Mesh::addLdrOptionalLine(const LdrColorReference mainColor, const LdrOptionalLine &optionalLineElement, glm::mat4 transformation) {
    glm::vec3 color;
    const auto elementColor = optionalLineElement.color.get();
    const auto mainColorLocked = mainColor.get();
    if (elementColor->code == LdrColor::MAIN_COLOR_CODE) {
        color = mainColorLocked->edge.asGlmVector();
    } else if (elementColor->code == LdrColor::LINE_COLOR_CODE) {
        color = glm::vec3(1 - util::vectorSum(mainColorLocked->value.asGlmVector()) / 3);//todo look up specification
    } else {
        color = elementColor->edge.asGlmVector();
    }
    LineVertex cv1{glm::vec4(optionalLineElement.controlX1, optionalLineElement.controlY1, optionalLineElement.controlZ1, 1.0f) * transformation, color};
    LineVertex lv1{glm::vec4(optionalLineElement.x1, optionalLineElement.y1, optionalLineElement.z1, 1.0f) * transformation, color};
    LineVertex lv2{glm::vec4(optionalLineElement.x2, optionalLineElement.y2, optionalLineElement.z2, 1.0f) * transformation, color};
    LineVertex cv2{glm::vec4(optionalLineElement.controlX2, optionalLineElement.controlY2, optionalLineElement.controlZ2, 1.0f) * transformation, color};
    addOptionalLineVertex(cv1);
    addOptionalLineVertex(lv1);
    addOptionalLineVertex(lv2);
    addOptionalLineVertex(cv2);
}

void Mesh::addTexturedTriangle(const std::shared_ptr<Texture> &texture, glm::vec3 pt1, glm::vec2 tc1, glm::vec3 pt2, glm::vec2 tc2, glm::vec3 pt3, glm::vec2 tc3) {
    auto &vertices = textureVertices[texture->getID()];
    vertices.emplace_back(pt1, tc1);
    vertices.emplace_back(pt2, tc2);
    vertices.emplace_back(pt3, tc3);
    textures[texture->getID()] = texture;
}

void Mesh::addOptionalLineVertex(const LineVertex &vertex) {
    if (!optionalLineVertices.empty()) {
        const auto stop = optionalLineVertices.size() >= 8 ? 8 : optionalLineVertices.size();
        for (size_t i = 1; i < stop; ++i) {
            const auto index = optionalLineVertices.size() - i;
            if (optionalLineVertices[index] == vertex) {
                optionalLineIndices.push_back(index);
                return;
            }
        }
    }
    optionalLineIndices.push_back(optionalLineVertices.size());
    optionalLineVertices.push_back(vertex);
}

void Mesh::addLineVertex(const LineVertex &vertex) {
    for (int i = (int) lineVertices.size() - 1; i >= std::max((int) lineVertices.size() - 12, 0); --i) {
        if (vertex.position == lineVertices[i].position && vertex.color == lineVertices[i].color) {
            lineIndices.push_back(i);
            return;
        }
    }
    lineIndices.push_back(lineVertices.size());
    lineVertices.push_back(vertex);
}

void Mesh::writeGraphicsData() {
    if (!already_initialized) {
        if (config::getBool(config::DRAW_MINIMAL_ENCLOSING_BALL_LINES)) {
            addMinEnclosingBallLines();
        }

        initializeTriangleGraphics();
        initializeTexturedTriangleGraphics();
        initializeLineGraphics();
        initializeOptionalLineGraphics();

        already_initialized = true;
    } else {
        rewriteInstanceBuffer();
    }
}

void Mesh::addMinEnclosingBallLines() {
    const auto ball = getMinimalEnclosingBall();
    auto center = ball.first;
    auto radius = ball.second;
    addLineVertex({glm::vec4(center.x + radius, center.y, center.z, 1), glm::vec3(1, 0, 0)});
    addLineVertex({glm::vec4(center.x - radius, center.y, center.z, 1), glm::vec3(1, 0, 0)});
    addLineVertex({glm::vec4(center.x, center.y + radius, center.z, 1), glm::vec3(0, 1, 0)});
    addLineVertex({glm::vec4(center.x, center.y - radius, center.z, 1), glm::vec3(0, 1, 0)});
    addLineVertex({glm::vec4(center.x, center.y, center.z + radius, 1), glm::vec3(0, 0, 1)});
    addLineVertex({glm::vec4(center.x, center.y, center.z - radius, 1), glm::vec3(0, 0, 1)});
}

void Mesh::initializeTriangleGraphics() {
    controller::executeOpenGL([this]() {
        for (const auto &entry: triangleIndices) {
            const auto color = entry.first;
            const std::vector<unsigned int> &indices = entry.second;
            const std::vector<TriangleVertex> &vertices = triangleVertices.find(color)->second;

            unsigned int vao, vertexVbo, instanceVbo, ebo;

            //vao
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            //vertexVbo
            glGenBuffers(1, &vertexVbo);
            glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
            size_t vertex_size = sizeof(TriangleVertex);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * vertex_size, &(vertices[0]), GL_STATIC_DRAW);
            metrics::vramUsageBytes += vertices.size() * vertex_size;

            // position attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void *) nullptr);
            // normal attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) offsetof(TriangleVertex, normal));

            //instanceVbo
            auto instancesArray = generateTriangleInstancesArray(color);

            glGenBuffers(1, &instanceVbo);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
            size_t instance_size = sizeof(TriangleInstance);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(TriangleInstance, diffuseColor));
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(TriangleInstance, ambientFactor));
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(TriangleInstance, specularBrightness));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(TriangleInstance, shininess));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(TriangleInstance, idColor));
            for (int j = 0; j < 4; ++j) {
                glEnableVertexAttribArray(7 + j);
                glVertexAttribPointer(7 + j, 4, GL_FLOAT, GL_FALSE, instance_size,
                                      (void *) (offsetof(TriangleInstance, transformation) + 4 * j * sizeof(float)));
            }

            for (int i = 2; i < 11; ++i) {
                glVertexAttribDivisor(i, 1);
            }

            //ebo
            glGenBuffers(1, &ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &(indices[0]), GL_STATIC_DRAW);
            metrics::vramUsageBytes += sizeof(unsigned int) * indices.size();

            VAOs[color] = vao;
            vertexVBOs[color] = vertexVbo;
            instanceVBOs[color] = instanceVbo;
            EBOs[color] = ebo;
        }
    });
}

void Mesh::initializeTexturedTriangleGraphics() {
    if (textureVertices.empty()) {
        return;
    }
    auto instancesArray = generateTexturedTriangleInstancesArray();

    controller::executeOpenGL([this, &instancesArray]() {
        for (const auto &item : textureVertices) {
            const auto id = item.first;
            const auto &vertices = item.second;

            unsigned int vao, vertexVbo, instanceVbo;

            //vao
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            //vertexVbo
            glGenBuffers(1, &vertexVbo);
            glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
            constexpr auto vertexSize = sizeof(TexturedTriangleVertex);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * vertexSize, &vertices[0], GL_STATIC_DRAW);
            metrics::vramUsageBytes += vertices.size() * vertexSize;

            //position attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void *) (offsetof(TexturedTriangleVertex, position)));

            //texCoord attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void *) (offsetof(TexturedTriangleVertex, textureCoord)));

            //instanceVbo
            glGenBuffers(1, &instanceVbo);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
            size_t instance_size = sizeof(TexturedTriangleInstance);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, instance_size, (void *) offsetof(TexturedTriangleInstance, idColor));
            glVertexAttribDivisor(2, 1);

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (offsetof(TexturedTriangleInstance, transformation) + 0 * sizeof(float)));
            glVertexAttribDivisor(3, 1);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (offsetof(TexturedTriangleInstance, transformation) + 4 * sizeof(float)));
            glVertexAttribDivisor(4, 1);
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (offsetof(TexturedTriangleInstance, transformation) + 8 * sizeof(float)));
            glVertexAttribDivisor(5, 1);
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (offsetof(TexturedTriangleInstance, transformation) + 12 * sizeof(float)));
            glVertexAttribDivisor(6, 1);

            /*for (int j = 3; j < 7; ++j) {
                glEnableVertexAttribArray(j);
                glVertexAttribPointer(j, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (4 * (j - 3) * sizeof(float)));
                glVertexAttribDivisor(j, 1);
            }*/

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            textureTriangleVaoVertexVboInstanceVbo[id] = {vao, vertexVbo, instanceVbo};
        }
    });
}

void Mesh::rewriteInstanceBuffer() {
    if (instancesHaveChanged) {
        //todo just clear buffer data when no instances
        controller::executeOpenGL([this]() {
            size_t newBufferSize = (sizeof(TriangleInstance) * triangleIndices.size() + 2 * sizeof(glm::mat4)) * instances.size();
            metrics::vramUsageBytes -= lastInstanceBufferSize;
            metrics::vramUsageBytes += newBufferSize;
            lastInstanceBufferSize = newBufferSize;
            for (const auto &entry: triangleIndices) {
                const auto color = entry.first;
                auto instanceVbo = instanceVBOs[color];
                auto instancesArray = generateTriangleInstancesArray(color);
                size_t instance_size = sizeof(TriangleInstance);
                glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
                glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);
            }
            glm::mat4 instancesArray[instances.size()];
            for (int i = 0; i < instances.size(); ++i) {
                instancesArray[i] = glm::transpose(instances[i].transformation * globalModel);
                if (instances[i].selected) {
                    instancesArray[i][2][3] = 1;
                }
            }
            size_t instance_size = sizeof(glm::mat4);
            glBindBuffer(GL_ARRAY_BUFFER, lineInstanceVBO);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &(instancesArray[0]), GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, optionalLineInstanceVBO);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &(instancesArray[0]), GL_STATIC_DRAW);

            if (!textureTriangleVaoVertexVboInstanceVbo.empty()) {
                const auto &texturedTriangleInstancesArray = generateTexturedTriangleInstancesArray();
                for (const auto &item : textureTriangleVaoVertexVboInstanceVbo) {
                    glBindBuffer(GL_ARRAY_BUFFER, std::get<2>(item.second));
                    glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(TexturedTriangleInstance), &texturedTriangleInstancesArray[0], GL_STATIC_DRAW);
                }
            }
        });

        instancesHaveChanged = false;
    }
}

void Mesh::initializeLineGraphics() {
    controller::executeOpenGL([this]() {
        //vao
        glGenVertexArrays(1, &lineVAO);
        glBindVertexArray(lineVAO);

        //vertexVbo
        glGenBuffers(1, &lineVertexVBO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVertexVBO);
        size_t vertex_size = sizeof(LineVertex);
        glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * vertex_size, &(lineVertices[0]), GL_STATIC_DRAW);
        metrics::vramUsageBytes += lineVertices.size() * vertex_size;

        // position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void *) nullptr);
        // color attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) offsetof(LineVertex, color));

        //instanceVbo
        auto *instancesArray = new glm::mat4[instances.size()];//todo smart pointer/array
        for (int i = 0; i < instances.size(); ++i) {
            instancesArray[i] = glm::transpose(instances[i].transformation * globalModel);
        }

        glGenBuffers(1, &lineInstanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, lineInstanceVBO);
        size_t instance_size = sizeof(glm::mat4);
        glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);

        for (int j = 2; j < 6; ++j) {
            glEnableVertexAttribArray(j);
            glVertexAttribPointer(j, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (4 * (j - 2) * sizeof(float)));
            glVertexAttribDivisor(j, 1);
        }

        //ebo
        glGenBuffers(1, &lineEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * lineIndices.size(), &(lineIndices)[0], GL_STATIC_DRAW);
        metrics::vramUsageBytes += sizeof(unsigned int) * lineIndices.size();

        delete[] instancesArray;
    });
}

void Mesh::initializeOptionalLineGraphics() {
    controller::executeOpenGL([this]() {
        //vao
        glGenVertexArrays(1, &optionalLineVAO);
        glBindVertexArray(optionalLineVAO);

        //vertexVbo
        glGenBuffers(1, &optionalLineVertexVBO);
        glBindBuffer(GL_ARRAY_BUFFER, optionalLineVertexVBO);
        size_t vertex_size = sizeof(LineVertex);
        glBufferData(GL_ARRAY_BUFFER, optionalLineVertices.size() * vertex_size, &(optionalLineVertices[0]), GL_STATIC_DRAW);
        metrics::vramUsageBytes += optionalLineVertices.size() * vertex_size;

        // position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void *) nullptr);
        // color attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) offsetof(LineVertex, color));

        //instanceVbo
        auto *instancesArray = new glm::mat4[instances.size()];
        for (int i = 0; i < instances.size(); ++i) {
            instancesArray[i] = glm::transpose(instances[i].transformation * globalModel);
        }

        glGenBuffers(1, &optionalLineInstanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, optionalLineInstanceVBO);
        size_t instance_size = sizeof(glm::mat4);
        glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);

        for (int j = 2; j < 6; ++j) {
            glEnableVertexAttribArray(j);
            glVertexAttribPointer(j, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (4 * (j - 2) * sizeof(float)));
            glVertexAttribDivisor(j, 1);
        }

        //ebo
        glGenBuffers(1, &optionalLineEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, optionalLineEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * optionalLineIndices.size(), &(optionalLineIndices)[0], GL_STATIC_DRAW);
        metrics::vramUsageBytes += sizeof(unsigned int) * optionalLineIndices.size();

        delete[] instancesArray;
    });
}

void Mesh::drawTriangleGraphics(scene_id_t sceneId, layer_t layer) {
    auto range = getSceneLayerInstanceRange(sceneId, layer);
    if (range.has_value() && range->count > 0) {
        for (const auto &entry: triangleIndices) {
            const std::vector<unsigned int> &indices = entry.second;
            if (!indices.empty()) {
                const auto color = entry.first;
                glBindVertexArray(VAOs[color]);
                glDrawElementsInstancedBaseInstance(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr, range->count, range->start);
            }
        }
    }
}

void Mesh::drawTexturedTriangleGraphics(scene_id_t sceneId, layer_t layer) {
    auto range = getSceneLayerInstanceRange(sceneId, layer);
    if (range.has_value() && range->count > 0) {
        for (const auto &item : textureVertices) {
            if (!item.second.empty()) {
                textures[item.first]->bind();
                glBindVertexArray(std::get<0>(textureTriangleVaoVertexVboInstanceVbo[item.first]));
                glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, item.second.size(), range->count, range->start);
            }
        }
    }
}

void Mesh::drawLineGraphics(scene_id_t sceneId, layer_t layer) {
    auto range = getSceneLayerInstanceRange(sceneId, layer);
    if (range.has_value() && range->count > 0 && !lineIndices.empty()) {
        glBindVertexArray(lineVAO);
        glDrawElementsInstancedBaseInstance(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, nullptr, range->count, range->start);
    }
}

void Mesh::drawOptionalLineGraphics(scene_id_t sceneId, layer_t layer) {
    auto range = getSceneLayerInstanceRange(sceneId, layer);
    if (range.has_value() && range->count > 0 && !optionalLineIndices.empty()) {
        glBindVertexArray(optionalLineVAO);
        glDrawElementsInstancedBaseInstance(GL_LINES_ADJACENCY, optionalLineIndices.size(), GL_UNSIGNED_INT, nullptr, range->count, range->start);
    }
}

void Mesh::deallocateGraphics() {
    controller::executeOpenGL([this]() {
        for (const auto &entry: triangleIndices) {
            const auto color = entry.first;
            unsigned int vao = VAOs[color];
            unsigned int vertexVbo = vertexVBOs[color];
            unsigned int instanceVbo = instanceVBOs[color];
            unsigned int ebo = EBOs[color];
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vertexVbo);
            glDeleteBuffers(1, &instanceVbo);
            glDeleteBuffers(1, &ebo);
        }
        glDeleteVertexArrays(1, &lineVAO);
        glDeleteBuffers(1, &lineVertexVBO);
        glDeleteBuffers(1, &lineInstanceVBO);
        glDeleteBuffers(1, &lineEBO);

        glDeleteVertexArrays(1, &optionalLineVAO);
        glDeleteBuffers(1, &optionalLineVertexVBO);
        glDeleteBuffers(1, &optionalLineInstanceVBO);
        glDeleteBuffers(1, &optionalLineEBO);

        for (const auto &item : textureTriangleVaoVertexVboInstanceVbo) {
            const auto &[vao, vertexVbo, instanceVbo] = item.second;
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vertexVbo);
            glDeleteBuffers(1, &instanceVbo);
        }
    });
}

Mesh::~Mesh() = default;

void Mesh::setInstanceColor(TriangleInstance *instance, const LdrColorReference color) {
    const auto colorLocked = color.get();
    instance->diffuseColor = colorLocked->value.asGlmVector();
    instance->shininess = 32.0f;
    //useful tool: http://www.cs.toronto.edu/~jacobson/phong-demo/
    switch (colorLocked->finish) {
        case LdrColor::METAL:
        case LdrColor::CHROME:
        case LdrColor::PEARLESCENT:
            //todo find out what's the difference
            instance->shininess *= 2;
            instance->ambientFactor = 1;
            instance->specularBrightness = 1;
            break;
        case LdrColor::MATTE_METALLIC:instance->ambientFactor = 0.6;
            instance->specularBrightness = 0.2;
            break;
        case LdrColor::RUBBER:instance->ambientFactor = 0.75;
            instance->specularBrightness = 0;
            break;
        default:instance->ambientFactor = 0.5;
            instance->specularBrightness = 0.5;
            break;
    }
}

std::unique_ptr<TriangleInstance[], std::default_delete<TriangleInstance[]>> Mesh::generateTriangleInstancesArray(const LdrColorReference color) {
    auto instancesArray = std::make_unique<TriangleInstance[]>(instances.size());
    unsigned int arr_cursor = 0;
    if (color.get()->code == ldr_color_repo::INSTANCE_DUMMY_COLOR_CODE) {
        for (auto &instance : instances) {
            instancesArray[arr_cursor].transformation = glm::transpose(instance.transformation * globalModel);
            //std::cout << "instance: " << std::endl;
            //util::coutMat4(instancesArray[arr_cursor].transformation);
            setInstanceColor(&instancesArray[arr_cursor], instance.color);
            instancesArray[arr_cursor].idColor = color::convertIntToColorVec3(instance.elementId);
            arr_cursor++;
        }
    } else {
        TriangleInstance inst{};
        setInstanceColor(&inst, color);
        std::fill_n(instancesArray.get(), instances.size(), inst);
        for (auto &instance : instances) {
            instancesArray[arr_cursor].transformation = glm::transpose(instance.transformation * globalModel);
            instancesArray[arr_cursor].idColor = color::convertIntToColorVec3(instance.elementId);
            arr_cursor++;
        }
    }
    return instancesArray;
}

std::unique_ptr<TexturedTriangleInstance[], std::default_delete<TexturedTriangleInstance[]>> Mesh::generateTexturedTriangleInstancesArray() {
    auto array = std::make_unique<TexturedTriangleInstance[]>(instances.size());
    for (int i = 0; i < instances.size(); ++i) {
        auto &instance = instances[i];
        array[i].idColor = color::convertIntToColorVec3(instance.elementId);
        array[i].transformation = glm::transpose(instance.transformation * globalModel);
    }
    return array;
}

std::pair<glm::vec3, float> Mesh::getMinimalEnclosingBall() {
    if (!minimalEnclosingBall.has_value()) {
        if (triangleVertices.empty()) {
            minimalEnclosingBall = std::make_pair(glm::vec3(0.0f), 0.0f);
        } else {
            std::list<std::vector<float>> lp;
            for (const auto &entry : triangleVertices) {
                for (const auto &vertex : entry.second) {
                    lp.push_back((std::vector<float>) {vertex.position.x, vertex.position.y, vertex.position.z});
                }
            }
            for (const auto &item : textureVertices) {
                for (const auto &vertex : item.second) {
                    lp.push_back((std::vector<float>) {vertex.position.x, vertex.position.y, vertex.position.z});
                }
            }

            typedef std::list<std::vector<float> >::const_iterator PointIterator;
            typedef std::vector<float>::const_iterator CoordIterator;

            Miniball::Miniball<Miniball::CoordAccessor<PointIterator, CoordIterator>> mb(3, lp.begin(), lp.end());
            glm::vec3 center(mb.center()[0], mb.center()[1], mb.center()[2]);
            minimalEnclosingBall = std::make_pair(center, std::sqrt(mb.squared_radius()));
        }
    }
    return minimalEnclosingBall.value();
}

std::optional<Mesh::InstanceRange> Mesh::getSceneInstanceRange(scene_id_t sceneId) {
    auto it = instanceSceneLayerRanges.find(sceneId);
    if (it == instanceSceneLayerRanges.end()) {
        return {};
    }
    const auto &layerMap = it->second;
    auto start = layerMap.cbegin()->second.start;
    unsigned int count;
    if (layerMap.size() > 1) {
        count = 0;
        for (const auto &item : layerMap) {
            count += item.second.count;
        }
    } else {
        count = layerMap.cbegin()->second.count;
    }

    return {{start, count}};
}

std::optional<Mesh::InstanceRange> Mesh::getSceneLayerInstanceRange(scene_id_t sceneId, layer_t layer) {
    const auto &it = instanceSceneLayerRanges.find(sceneId);
    if (it != instanceSceneLayerRanges.end()) {
        const auto &it2 = it->second.find(layer);
        if (it2 != it->second.end()) {
            return it2->second;
        }
    }
    return {};
}

void Mesh::updateInstancesOfScene(scene_id_t sceneId, const std::vector<MeshInstance> &newSceneInstances) {
    if (newSceneInstances.empty()) {
        deleteInstancesOfScene(sceneId);
        return;
    }
    auto sceneRange = getSceneInstanceRange(sceneId);
    if (sceneRange.has_value()) {
        if (sceneRange->count != newSceneInstances.size()) {
            //can't update in-place, going to add at the end
            auto firstIndex = sceneRange->start;
            auto afterLastIndex = firstIndex + sceneRange->count;
            instances.erase(instances.begin() + firstIndex, instances.begin() + afterLastIndex);
            for (auto &item : instanceSceneLayerRanges) {
                auto &layerMap = item.second;
                if (layerMap.cbegin()->second.start >= afterLastIndex) {
                    //the instances of this scene are located after the sceneId, we have to update the ranges
                    for (auto &layerRange : layerMap) {
                        layerRange.second.start -= sceneRange->count;
                    }
                }
            }
            appendNewSceneInstancesAtEnd(sceneId, newSceneInstances);
        } else {
            //maybe the instances haven't changed at all
            //going to replace the existing ones

            //todo use glBufferSubData
            auto destinationIt = instances.begin() + sceneRange->start;
            auto sourceIt = newSceneInstances.begin();
            layer_t currentLayer = sourceIt->layer;
            unsigned int layerStart = sceneRange->start;
            unsigned int currentLayerInstanceCount = 0;
            auto &thisSceneRanges = instanceSceneLayerRanges[sceneId];
            thisSceneRanges.clear();
            while (sourceIt != newSceneInstances.end()) {
                if (destinationIt->operator!=(*sourceIt)) {
                    instancesHaveChanged = true;
                    *destinationIt = *sourceIt;
                }
                if (currentLayer == sourceIt->layer) {
                    currentLayerInstanceCount++;
                } else {
                    thisSceneRanges.emplace(currentLayer, InstanceRange{layerStart, currentLayerInstanceCount});
                    currentLayer = sourceIt->layer;
                    layerStart += currentLayerInstanceCount;
                    currentLayerInstanceCount = 1;
                }
                ++destinationIt;
                ++sourceIt;
            }
            thisSceneRanges.emplace(currentLayer, InstanceRange{layerStart, currentLayerInstanceCount});
        }
    } else {
        appendNewSceneInstancesAtEnd(sceneId, newSceneInstances);
    }
}

void Mesh::deleteInstancesOfScene(scene_id_t sceneId) {
    auto sceneRange = getSceneInstanceRange(sceneId);
    if (sceneRange.has_value()) {
        if (instances.size() == sceneRange->start + sceneRange->count) {
            instances.resize(sceneRange->start);
        } else {
            auto startIt = instances.begin() + sceneRange->start;
            instances.erase(startIt, startIt + sceneRange->count - 1);
        }
        instanceSceneLayerRanges.erase(sceneId);
        instancesHaveChanged = true;
    }
}

void Mesh::appendNewSceneInstancesAtEnd(scene_id_t sceneId, const std::vector<MeshInstance> &newSceneInstances) {
    auto &thisSceneRanges = instanceSceneLayerRanges[sceneId];
    thisSceneRanges.clear();

    auto sourceIt = newSceneInstances.cbegin();
    layer_t currentLayer = sourceIt->layer;
    unsigned int layerStart = instances.size();
    unsigned int currentLayerInstanceCount = 0;
    instances.reserve(instances.size() + newSceneInstances.size());

    while (sourceIt != newSceneInstances.cend()) {
        instances.push_back(*sourceIt);
        if (currentLayer == sourceIt->layer) {
            currentLayerInstanceCount++;
        } else {
            thisSceneRanges.emplace(currentLayer, InstanceRange{layerStart, currentLayerInstanceCount});
            currentLayer = sourceIt->layer;
            layerStart += currentLayerInstanceCount;
            currentLayerInstanceCount = 1;
        }
        ++sourceIt;
    }
    thisSceneRanges.emplace(currentLayer, InstanceRange{layerStart, currentLayerInstanceCount});

    //instances.insert(instances.end(), newSceneInstances.begin(), newSceneInstances.end());
    instancesHaveChanged = true;
}

bool MeshInstance::operator==(const MeshInstance &other) const {
    return memcmp(this, &other, sizeof(*this)) == 0;
    //return transformation == other.transformation && color.get() == other.color.get() && elementId == other.elementId && selected == other.selected && layer == other.layer && scene == other.scene;
}

bool MeshInstance::operator!=(const MeshInstance &other) const {
    return memcmp(this, &other, sizeof(*this)) != 0;
    //return transformation != other.transformation || color.get() != other.color.get() || elementId != other.elementId || selected != other.selected || layer != other.layer || scene != other.scene;
}
