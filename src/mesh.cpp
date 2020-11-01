// mesh.cpp
// Created by bb1950328 on 20.09.20.
//

#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "mesh.h"
#include "config.h"
#include "ldr_colors.h"
#include <glm/gtx/normal.hpp>
#include "statistic.h"
#include "lib/Miniball.hpp"

void Mesh::addLdrFile(const LdrFile &file, glm::mat4 transformation=glm::mat4(1.0f), LdrColor *mainColor= nullptr, bool bfcInverted=false) {
    for (auto element : file.elements) {
        switch (element->getType()) {
            case 0:
                break;
            case 1:
                addLdrSubfileReference(mainColor, dynamic_cast<LdrSubfileReference *>(element), transformation, bfcInverted);
                break;
            case 2:
                addLdrLine(mainColor, dynamic_cast<LdrLine &&>(*element), transformation);
                break;
            case 3:
                addLdrTriangle(mainColor, dynamic_cast<LdrTriangle &&>(*element), transformation, bfcInverted);
                break;
            case 4:
                addLdrQuadrilateral(mainColor, dynamic_cast<LdrQuadrilateral &&>(*element), transformation, bfcInverted);
                break;
            case 5:
                addLdrOptionalLine(mainColor, dynamic_cast<LdrOptionalLine &&>(*element), transformation);
                break;
        }
    }
}

void Mesh::addLdrTriangle(LdrColor *mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation, bool bfcInverted) {
    LdrColor *color = triangleElement.color->code == LdrColor::MAIN_COLOR_CODE ? mainColor : triangleElement.color;
    auto *verticesList = getVerticesList(color);
    auto *indicesList = getIndicesList(color);
    auto p1 = glm::vec3(triangleElement.x1, triangleElement.y1, triangleElement.z1);
    auto p2 = glm::vec3(triangleElement.x2, triangleElement.y2, triangleElement.z2);
    auto p3 = glm::vec3(triangleElement.x3, triangleElement.y3, triangleElement.z3);
    auto normal = glm::triangleNormal(p1, p2, p3);
    auto transformedNormal = glm::normalize(glm::vec4(normal, 0.0f) * transformation);
    TriangleVertex vertex1{glm::vec4(p1, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex2{glm::vec4(p2, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex3{glm::vec4(p3, 1.0f) * transformation, transformedNormal};

    if (util::doesTransformationInverseWindingOrder(transformation)^bfcInverted) {
        std::swap(vertex2, vertex3);
    }

    auto idx1 = verticesList->size();
    verticesList->push_back(vertex1);
    verticesList->push_back(vertex2);
    verticesList->push_back(vertex3);
    indicesList->push_back(idx1);
    indicesList->push_back(idx1+1);
    indicesList->push_back(idx1+2);

    if (config::get_string(config::SHOW_NORMALS)=="true") {
        auto lp1 = glm::vec4(util::triangleCentroid(p1, p2, p3), 1.0f)*transformation;
        auto lp2 = lp1 + (transformedNormal * 5.0f);
        LineVertex lv1{lp1, transformedNormal};
        LineVertex lv2{lp2, transformedNormal};
        addLineVertex(lv1);
        addLineVertex(lv2);
    }
}

void
Mesh::addLdrSubfileReference(LdrColor *mainColor, LdrSubfileReference *sfElement, glm::mat4 transformation, bool bfcInverted) {
    auto sub_transformation = sfElement->getTransformationMatrix();
    LdrColor *color = sfElement->color->code == LdrColor::MAIN_COLOR_CODE ? mainColor : sfElement->color;
    addLdrFile(*sfElement->getFile(), sub_transformation * transformation, color, sfElement->bfcInverted^bfcInverted);
}

void Mesh::addLdrQuadrilateral(LdrColor *mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation, bool bfcInverted) {
    auto p1 = glm::vec3(quadrilateral.x1, quadrilateral.y1, quadrilateral.z1);
    auto p2 = glm::vec3(quadrilateral.x2, quadrilateral.y2, quadrilateral.z2);
    auto p3 = glm::vec3(quadrilateral.x3, quadrilateral.y3, quadrilateral.z3);
    auto p4 = glm::vec3(quadrilateral.x4, quadrilateral.y4, quadrilateral.z4);
    LdrColor *color = quadrilateral.color->code == LdrColor::MAIN_COLOR_CODE ? mainColor : quadrilateral.color;
    auto normal = glm::triangleNormal(p1, p2, p3);
    auto transformedNormal = glm::normalize(glm::vec4(normal, 0.0f) * transformation);

    TriangleVertex vertex1{glm::vec4(p1, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex2{glm::vec4(p2, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex3{glm::vec4(p3, 1.0f) * transformation, transformedNormal};
    TriangleVertex vertex4{glm::vec4(p4, 1.0f) * transformation, transformedNormal};

    if (util::doesTransformationInverseWindingOrder(transformation)^bfcInverted) {
        std::swap(vertex2, vertex4);
    }

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

    if (config::get_string(config::SHOW_NORMALS) == "true") {
        auto lp1 = glm::vec4(util::quadrilateralCentroid(p1, p2, p3, p4), 1.0f)*transformation;
        auto lp2 = lp1 + (transformedNormal * 5.0f);
        LineVertex lv1{lp1, transformedNormal};
        LineVertex lv2{lp2, transformedNormal};
        addLineVertex(lv1);
        addLineVertex(lv2);
    }
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

void Mesh::addLdrLine(LdrColor *mainColor, const LdrLine &lineElement, glm::mat4 transformation) {
    glm::vec3 color;
    if (lineElement.color->code == LdrColor::MAIN_COLOR_CODE) {
        color = mainColor->edge.asGlmVector();
    } else if (lineElement.color->code == LdrColor::LINE_COLOR_CODE) {
        color = glm::vec3(1-util::vector_sum(mainColor->value.asGlmVector())/3);//todo look up specification
    } else {
        color = lineElement.color->edge.asGlmVector();
    }
    LineVertex lv1{glm::vec4(lineElement.x1, lineElement.y1, lineElement.z1, 1.0f) * transformation, color};
    LineVertex lv2{glm::vec4(lineElement.x2, lineElement.y2, lineElement.z2, 1.0f) * transformation, color};
    addLineVertex(lv1);
    addLineVertex(lv2);
}

void Mesh::addLdrOptionalLine(LdrColor *mainColor, const LdrOptionalLine &optionalLineElement, glm::mat4 transformation) {
    glm::vec3 color;
    if (optionalLineElement.color->code == LdrColor::MAIN_COLOR_CODE) {
        color = mainColor->edge.asGlmVector();
    } else if (optionalLineElement.color->code == LdrColor::LINE_COLOR_CODE) {
        color = glm::vec3(1-util::vector_sum(mainColor->value.asGlmVector())/3);//todo look up specification
    } else {
        color = optionalLineElement.color->edge.asGlmVector();
    }
    LineVertex cv1{glm::vec4(optionalLineElement.control_x1, optionalLineElement.control_y1, optionalLineElement.control_z1, 1.0f) * transformation, color};
    LineVertex lv1{glm::vec4(optionalLineElement.x1, optionalLineElement.y1, optionalLineElement.z1, 1.0f) * transformation, color};
    LineVertex lv2{glm::vec4(optionalLineElement.x2, optionalLineElement.y2, optionalLineElement.z2, 1.0f) * transformation, color};
    LineVertex cv2{glm::vec4(optionalLineElement.control_x2, optionalLineElement.control_y2, optionalLineElement.control_z2, 1.0f) * transformation, color};
    addOptionalLineVertex(cv1);
    addOptionalLineVertex(lv1);
    addOptionalLineVertex(lv2);
    addOptionalLineVertex(cv2);
}

void Mesh::addOptionalLineVertex(const LineVertex &vertex) {
    if (!optionalLineVertices.empty()) {
        const auto stop = optionalLineVertices.size() >= 8 ? 8 : optionalLineVertices.size();
        for (size_t i = 0; i < stop; ++i) {
            size_t index = optionalLineVertices.size()-i;
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
    for (int i = lineVertices.size(); i > lineVertices.size() - 12; --i) {
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
        if (config::get_bool(config::DRAW_MINIMAL_ENCLOSING_BALL_LINES)) {
            addMinEnclosingBallLines();
        }
        initializeTriangleGraphics();
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
        statistic::Counters::vramUsageBytes += vertices->size() * vertex_size;

        // position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void *) nullptr);
        // normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) offsetof(TriangleVertex, normal));

        //instanceVbo
        TriangleInstance *instancesArray = generateInstancesArray(color);

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
        delete[] instancesArray;

        //ebo
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices->size(), &(*indices)[0], GL_STATIC_DRAW);
        statistic::Counters::vramUsageBytes += sizeof(unsigned int) * indices->size();

        VAOs[color] = vao;
        vertexVBOs[color] = vertexVbo;
        instanceVBOs[color] = instanceVbo;
        EBOs[color] = ebo;
    }
}

void Mesh::rewriteInstanceBuffer() {
    if (instancesHaveChanged) {
        size_t newBufferSize = (sizeof(TriangleInstance)*triangleIndices.size()+2*sizeof(glm::mat4))*instances.size();
        statistic::Counters::vramUsageBytes -= this->lastInstanceBufferSize;
        statistic::Counters::vramUsageBytes += newBufferSize;
        lastInstanceBufferSize = newBufferSize;
        for (const auto &entry: triangleIndices) {
            LdrColor *color = entry.first;
            auto instanceVbo = instanceVBOs[color];
            auto instancesArray = generateInstancesArray(color);
            size_t instance_size = sizeof(TriangleInstance);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);
        }
        glm::mat4 instancesArray[instances.size()];
        for (int i = 0; i < instances.size(); ++i) {
            instancesArray[i] = glm::transpose(instances[i].transformation*globalModel);
        }
        size_t instance_size = sizeof(glm::mat4);
        glBindBuffer(GL_ARRAY_BUFFER, lineInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &(instancesArray[0]), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, optionalLineInstanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &(instancesArray[0]), GL_STATIC_DRAW);
        instancesHaveChanged = false;
    }
}

void Mesh::initializeLineGraphics() {

    //vao
    glGenVertexArrays(1, &lineVAO);
    glBindVertexArray(lineVAO);

    //vertexVbo
    glGenBuffers(1, &lineVertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVertexVBO);
    size_t vertex_size = sizeof(LineVertex);
    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * vertex_size, &(lineVertices[0]), GL_STATIC_DRAW);
    statistic::Counters::vramUsageBytes += lineVertices.size() * vertex_size;

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void *) nullptr);
    // color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) offsetof(LineVertex, color));

    //instanceVbo
    auto* instancesArray = new glm::mat4[instances.size()];
    for (int i = 0; i < instances.size(); ++i) {
        instancesArray[i] = glm::transpose(instances[i].transformation*globalModel);
    }

    glGenBuffers(1, &lineInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lineInstanceVBO);
    size_t instance_size = sizeof(glm::mat4);
    glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);

    for (int j = 2; j < 6; ++j) {
        glEnableVertexAttribArray(j);
        glVertexAttribPointer(j, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (4 * (j-2) * sizeof(float)));
        glVertexAttribDivisor(j, 1);
    }

    //ebo
    glGenBuffers(1, &lineEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * lineIndices.size(), &(lineIndices)[0], GL_STATIC_DRAW);
    statistic::Counters::vramUsageBytes += sizeof(unsigned int) * lineIndices.size();

    delete [] instancesArray;
}
void Mesh::initializeOptionalLineGraphics() {

    //vao
    glGenVertexArrays(1, &optionalLineVAO);
    glBindVertexArray(optionalLineVAO);

    //vertexVbo
    glGenBuffers(1, &optionalLineVertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, optionalLineVertexVBO);
    size_t vertex_size = sizeof(LineVertex);
    glBufferData(GL_ARRAY_BUFFER, optionalLineVertices.size() * vertex_size, &(optionalLineVertices[0]), GL_STATIC_DRAW);
    statistic::Counters::vramUsageBytes += optionalLineVertices.size() * vertex_size;

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex_size, (void *) nullptr);
    // color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (void *) offsetof(LineVertex, color));

    //instanceVbo
    auto* instancesArray = new glm::mat4[instances.size()];
    for (int i = 0; i < instances.size(); ++i) {
        instancesArray[i] = glm::transpose(instances[i].transformation*globalModel);
    }

    glGenBuffers(1, &optionalLineInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, optionalLineInstanceVBO);
    size_t instance_size = sizeof(glm::mat4);
    glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);

    for (int j = 2; j < 6; ++j) {
        glEnableVertexAttribArray(j);
        glVertexAttribPointer(j, 4, GL_FLOAT, GL_FALSE, instance_size, (void *) (4 * (j-2) * sizeof(float)));
        glVertexAttribDivisor(j, 1);
    }

    //ebo
    glGenBuffers(1, &optionalLineEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, optionalLineEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * optionalLineIndices.size(), &(optionalLineIndices)[0], GL_STATIC_DRAW);
    statistic::Counters::vramUsageBytes += sizeof(unsigned int) * optionalLineIndices.size();

    delete [] instancesArray;
}

void Mesh::drawTriangleGraphics() {
    for (const auto &entry: triangleIndices) {
        LdrColor *color = entry.first;
        std::vector<unsigned int> *indices = entry.second;
        glBindVertexArray(VAOs[color]);
        glDrawElementsInstanced(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, nullptr, instances.size());
    }
}

void Mesh::drawLineGraphics() {
    glBindVertexArray(lineVAO);
    glDrawElementsInstanced(GL_LINES, lineIndices.size(), GL_UNSIGNED_INT, nullptr, instances.size());
}

void Mesh::drawOptionalLineGraphics() {
    glBindVertexArray(optionalLineVAO);
    glDrawElementsInstanced(GL_LINES_ADJACENCY, optionalLineIndices.size(), GL_UNSIGNED_INT, nullptr, instances.size());
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
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVertexVBO);
    glDeleteBuffers(1, &lineInstanceVBO);
    glDeleteBuffers(1, &lineEBO);
}

Mesh::~Mesh() {
    for (const auto &entry: triangleIndices) {
        delete entry.second;
    }
    for (const auto &entry: triangleVertices) {
        delete entry.second;
    }
}

void Mesh::setInstanceColor(TriangleInstance *instance, const LdrColor *color) {
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

TriangleInstance *Mesh::generateInstancesArray(const LdrColor *color) {
    auto *instancesArray = new TriangleInstance[instances.size()];
    unsigned int arr_cursor = 0;
    if (color == &LdrColorRepository::instDummyColor) {
        for (auto &instance : instances) {
            instancesArray[arr_cursor].transformation = glm::transpose(instance.transformation * globalModel);
            setInstanceColor(&instancesArray[arr_cursor], instance.color);
            instancesArray[arr_cursor].idColor = util::convertIntToColorVec3(instance.elementId);
            arr_cursor++;
        }
    } else {
        TriangleInstance inst{};
        setInstanceColor(&inst, color);
        std::fill_n(instancesArray, instances.size(), inst);
        for (auto &instance : instances) {
            instancesArray[arr_cursor].transformation = glm::transpose(instance.transformation * globalModel);
            instancesArray[arr_cursor].idColor = util::convertIntToColorVec3(instance.elementId);
            arr_cursor++;
        }
    }
    return instancesArray;
}

std::pair<glm::vec3, float> Mesh::getMinimalEnclosingBall() {
    if (!minimalEnclosingBall.has_value()) {
        if (triangleVertices.empty()) {
            minimalEnclosingBall = std::make_pair(glm::vec3(0.0f), 0.0f);
        } else {
            std::list<std::vector<float> > lp;
            for (const auto &entry : triangleVertices) {
                for (const auto &vertex : *entry.second) {
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

bool MeshInstance::operator==(const MeshInstance& other) const {
    return transformation==other.transformation&&color==other.color&&elementId==other.elementId;
}

bool MeshInstance::operator!=(const MeshInstance &other) const {
    return transformation!=other.transformation||color!=other.color||elementId!=other.elementId;
}
