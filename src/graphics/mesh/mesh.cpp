#include "mesh.h"
#include "../../config.h"
#include "../../controller.h"
#include "../../helpers/util.h"
#include "../../lib/Miniball.hpp"
#include "../../metrics.h"
#include "../opengl_native_or_replacement.h"
#include "mesh_line_data.h"
#include <glad/glad.h>
#include <glm/gtx/normal.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace bricksim::mesh {

    void Mesh::addLdrFile(const std::shared_ptr<ldr::File>& file, glm::mat4 transformation = glm::mat4(1.0f), const ldr::ColorReference mainColor = {}, bool bfcInverted = false) {
        for (const auto& element: file->elements) {
            switch (element->getType()) {
                case 0: break;
                case 1:
                    addLdrSubfileReference(mainColor, std::dynamic_pointer_cast<ldr::SubfileReference>(element), transformation, bfcInverted);
                    break;
                case 2:
                    addLdrLine(mainColor, dynamic_cast<ldr::Line&&>(*element), transformation);
                    break;
                case 3:
                    addLdrTriangle(mainColor, dynamic_cast<ldr::Triangle&&>(*element), transformation, bfcInverted);
                    break;
                case 4:
                    addLdrQuadrilateral(mainColor, dynamic_cast<ldr::Quadrilateral&&>(*element), transformation, bfcInverted);
                    break;
                case 5:
                    addLdrOptionalLine(mainColor, dynamic_cast<ldr::OptionalLine&&>(*element), transformation);
                    break;
            }
        }
    }

    void Mesh::addLdrTriangle(const ldr::ColorReference mainColor, const ldr::Triangle& triangleElement, glm::mat4 transformation, bool bfcInverted) {
        const auto color = triangleElement.color.get()->code == ldr::Color::MAIN_COLOR_CODE ? mainColor : triangleElement.color;
        auto& data = getTriangleData(color);
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

        auto idx1 = data.getVertexCount();
        data.addRawVertex(vertex1);
        data.addRawVertex(vertex2);
        data.addRawVertex(vertex3);
        data.addRawIndex(idx1);
        data.addRawIndex(idx1 + 1);
        data.addRawIndex(idx1 + 2);

        if (config::get(config::SHOW_NORMALS)) {
            auto lp1 = glm::vec4(util::triangleCentroid(p1, p2, p3), 1.0f) * transformation;
            auto lp2 = lp1 + (transformedNormal * 5.0f);
            LineVertex lv1{lp1, transformedNormal};
            LineVertex lv2{lp2, transformedNormal};
            lineData.addVertex(lv1);
            lineData.addVertex(lv2);
        }
    }

    void Mesh::addLdrSubfileReference(ldr::ColorReference mainColor, const std::shared_ptr<ldr::SubfileReference>& sfElement, glm::mat4 transformation, bool bfcInverted) {
        auto sub_transformation = sfElement->getTransformationMatrix();
        const auto color = sfElement->color.get()->code == ldr::Color::MAIN_COLOR_CODE ? mainColor : sfElement->color;
        addLdrFile(sfElement->getFile(), sub_transformation * transformation, color, sfElement->bfcInverted ^ bfcInverted);
    }

    void Mesh::addLdrQuadrilateral(ldr::ColorReference mainColor, ldr::Quadrilateral&& quadrilateral, glm::mat4 transformation, bool bfcInverted) {
        auto p1 = glm::vec3(quadrilateral.x1, quadrilateral.y1, quadrilateral.z1);
        auto p2 = glm::vec3(quadrilateral.x2, quadrilateral.y2, quadrilateral.z2);
        auto p3 = glm::vec3(quadrilateral.x3, quadrilateral.y3, quadrilateral.z3);
        auto p4 = glm::vec3(quadrilateral.x4, quadrilateral.y4, quadrilateral.z4);
        const auto color = quadrilateral.color.get()->code == ldr::Color::MAIN_COLOR_CODE ? mainColor : quadrilateral.color;
        auto normal = glm::triangleNormal(p1, p2, p3);
        auto transformedNormal = glm::normalize(glm::vec4(normal, 0.0f) * transformation);

        TriangleVertex vertex1{glm::vec4(p1, 1.0f) * transformation, transformedNormal};
        TriangleVertex vertex2{glm::vec4(p2, 1.0f) * transformation, transformedNormal};
        TriangleVertex vertex3{glm::vec4(p3, 1.0f) * transformation, transformedNormal};
        TriangleVertex vertex4{glm::vec4(p4, 1.0f) * transformation, transformedNormal};

        if (util::doesTransformationInverseWindingOrder(transformation) ^ bfcInverted) {
            std::swap(vertex2, vertex4);
        }

        auto& data = getTriangleData(color);

        unsigned int idx = data.getVertexCount();
        data.addRawVertex(vertex1);
        data.addRawVertex(vertex2);
        data.addRawVertex(vertex3);
        data.addRawVertex(vertex4);

        //triangle 1
        data.addRawIndex(idx);
        data.addRawIndex(idx + 1);
        data.addRawIndex(idx + 2);

        //triangle 2
        data.addRawIndex(idx + 2);
        data.addRawIndex(idx + 3);
        data.addRawIndex(idx);

        if (config::get(config::SHOW_NORMALS)) {
            auto lp1 = glm::vec4(util::quadrilateralCentroid(p1, p2, p3, p4), 1.0f) * transformation;
            auto lp2 = lp1 + (transformedNormal * 5.0f);
            LineVertex lv1{lp1, transformedNormal};
            LineVertex lv2{lp2, transformedNormal};
            lineData.addVertex(lv1);
            lineData.addVertex(lv2);
        }
    }

    void Mesh::addLdrLine(const ldr::ColorReference mainColor, const ldr::Line& lineElement, glm::mat4 transformation) {
        glm::vec3 color;
        const auto lineElementColor = lineElement.color.get();
        const auto mainColorLocked = mainColor.get();
        if (lineElementColor->code == ldr::Color::MAIN_COLOR_CODE) {
            color = mainColorLocked->edge.asGlmVector();
        } else if (lineElementColor->code == ldr::Color::LINE_COLOR_CODE) {
            color = glm::vec3(1 - util::vectorSum(mainColorLocked->value.asGlmVector()) / 3);//todo look up specification
        } else {
            color = lineElementColor->edge.asGlmVector();
        }
        LineVertex lv1{glm::vec4(lineElement.x1, lineElement.y1, lineElement.z1, 1.0f) * transformation, color};
        LineVertex lv2{glm::vec4(lineElement.x2, lineElement.y2, lineElement.z2, 1.0f) * transformation, color};
        lineData.addVertex(lv1);
        lineData.addVertex(lv2);
    }

    void Mesh::addLdrOptionalLine(const ldr::ColorReference mainColor, const ldr::OptionalLine& optionalLineElement, glm::mat4 transformation) {
        glm::vec3 color;
        const auto elementColor = optionalLineElement.color.get();
        const auto mainColorLocked = mainColor.get();
        if (elementColor->code == ldr::Color::MAIN_COLOR_CODE) {
            color = mainColorLocked->edge.asGlmVector();
        } else if (elementColor->code == ldr::Color::LINE_COLOR_CODE) {
            color = glm::vec3(1 - util::vectorSum(mainColorLocked->value.asGlmVector()) / 3);//todo look up specification
        } else {
            color = elementColor->edge.asGlmVector();
        }
        LineVertex cv1{glm::vec4(optionalLineElement.controlX1, optionalLineElement.controlY1, optionalLineElement.controlZ1, 1.0f) * transformation, color};
        LineVertex lv1{glm::vec4(optionalLineElement.x1, optionalLineElement.y1, optionalLineElement.z1, 1.0f) * transformation, color};
        LineVertex lv2{glm::vec4(optionalLineElement.x2, optionalLineElement.y2, optionalLineElement.z2, 1.0f) * transformation, color};
        LineVertex cv2{glm::vec4(optionalLineElement.controlX2, optionalLineElement.controlY2, optionalLineElement.controlZ2, 1.0f) * transformation, color};
        optionalLineData.addVertex(cv1);
        optionalLineData.addVertex(lv1);
        optionalLineData.addVertex(lv2);
        optionalLineData.addVertex(cv2);
    }

    void Mesh::addTexturedTriangle(const std::shared_ptr<graphics::Texture>& texture, glm::vec3 pt1, glm::vec2 tc1, glm::vec3 pt2, glm::vec2 tc2, glm::vec3 pt3, glm::vec2 tc3) {
        auto& vertices = textureVertices[texture->getID()];
        vertices.emplace_back(pt1, tc1);
        vertices.emplace_back(pt2, tc2);
        vertices.emplace_back(pt3, tc3);
        textures[texture->getID()] = texture;
    }

    void Mesh::writeGraphicsData() {
        if (!already_initialized) {
            if (!outerDimensions.has_value()) {
                calculateOuterDimensions();
            }
            if (config::get(config::DRAW_MINIMAL_ENCLOSING_BALL_LINES)) {
                addMinEnclosingBallLines();
            }

            for (auto& item: triangleData) {
                item.second.initBuffers(instances);
            }

            initializeTexturedTriangleGraphics();
            lineData.initBuffers(instances);
            optionalLineData.initBuffers(instances);

            already_initialized = true;
        } else {
            rewriteInstanceBuffer();
        }
    }

    void Mesh::addMinEnclosingBallLines() {
        auto center = outerDimensions.value().minEnclosingBallCenter;
        auto radius = outerDimensions.value().minEnclosingBallRadius;
        lineData.addVertex({{center.x + radius, center.y, center.z}, {1, 0, 0}});
        lineData.addVertex({{center.x - radius, center.y, center.z}, {1, 0, 0}});
        lineData.addVertex({{center.x, center.y + radius, center.z}, {0, 1, 0}});
        lineData.addVertex({{center.x, center.y - radius, center.z}, {0, 1, 0}});
        lineData.addVertex({{center.x, center.y, center.z + radius}, {0, 0, 1}});
        lineData.addVertex({{center.x, center.y, center.z - radius}, {0, 0, 1}});
    }

    void Mesh::initializeTexturedTriangleGraphics() {
        if (textureVertices.empty()) {
            return;
        }
        auto instancesArray = generateTexturedTriangleInstancesArray();

        controller::executeOpenGL([this, &instancesArray]() {
            for (const auto& item: textureVertices) {
                const auto id = item.first;
                const auto& vertices = item.second;

                unsigned int vao, vertexVbo, instanceVbo;

                //VAO
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
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(offsetof(TexturedTriangleVertex, position)));

                //texCoord attribute
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(offsetof(TexturedTriangleVertex, textureCoord)));

                //instanceVbo
                glGenBuffers(1, &instanceVbo);
                glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
                size_t instance_size = sizeof(TexturedTriangleInstance);
                glBufferData(GL_ARRAY_BUFFER, instances.size() * instance_size, &instancesArray[0], GL_STATIC_DRAW);

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, instance_size, (void*)offsetof(TexturedTriangleInstance, idColor));
                glVertexAttribDivisor(2, 1);

                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, instance_size, (void*)(offsetof(TexturedTriangleInstance, transformation) + 0 * sizeof(float)));
                glVertexAttribDivisor(3, 1);
                glEnableVertexAttribArray(4);
                glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, instance_size, (void*)(offsetof(TexturedTriangleInstance, transformation) + 4 * sizeof(float)));
                glVertexAttribDivisor(4, 1);
                glEnableVertexAttribArray(5);
                glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, instance_size, (void*)(offsetof(TexturedTriangleInstance, transformation) + 8 * sizeof(float)));
                glVertexAttribDivisor(5, 1);
                glEnableVertexAttribArray(6);
                glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, instance_size, (void*)(offsetof(TexturedTriangleInstance, transformation) + 12 * sizeof(float)));
                glVertexAttribDivisor(6, 1);

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
                std::vector<glm::mat4> instancesArray;
                instancesArray.resize(instances.size());
                for (int i = 0; i < instances.size(); ++i) {
                    instancesArray[i] = glm::transpose(instances[i].transformation * constants::LDU_TO_OPENGL);
                    if (instances[i].selected) {
                        instancesArray[i][2][3] = 1;
                    }
                }
                lineData.rewriteInstanceBuffer(instancesArray);
                optionalLineData.rewriteInstanceBuffer(instancesArray);

                for (auto& item: triangleData) {
                    item.second.rewriteInstanceBuffer(instances);
                }

                if (!textureTriangleVaoVertexVboInstanceVbo.empty()) {
                    const auto& texturedTriangleInstancesArray = generateTexturedTriangleInstancesArray();
                    for (const auto& item: textureTriangleVaoVertexVboInstanceVbo) {
                        glBindBuffer(GL_ARRAY_BUFFER, std::get<2>(item.second));
                        glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(TexturedTriangleInstance), &texturedTriangleInstancesArray[0], GL_STATIC_DRAW);
                    }
                }
            });

            instancesHaveChanged = false;
        }
    }

    void Mesh::drawTexturedTriangleGraphics(scene_id_t sceneId, layer_t layer) {
        auto range = getSceneLayerInstanceRange(sceneId, layer);
        if (range.has_value() && range->count > 0) {
            for (const auto& item: textureVertices) {
                if (!item.second.empty()) {
                    textures[item.first]->bind();
                    unsigned int& vao = std::get<0>(textureTriangleVaoVertexVboInstanceVbo[item.first]);
                    unsigned int& instanceVbo = std::get<2>(textureTriangleVaoVertexVboInstanceVbo[item.first]);
                    glBindVertexArray(vao);
                    graphics::opengl_native_or_replacement::drawArraysInstancedBaseInstance(
                            GL_TRIANGLES, 0, item.second.size(), range->count, range->start,
                            instanceVbo, instances.size() * sizeof(TexturedTriangleInstance), sizeof(TexturedTriangleInstance));
                }
            }
        }
    }

    void Mesh::deallocateGraphics() {
        controller::executeOpenGL([this]() {
            for (const auto& item: textureTriangleVaoVertexVboInstanceVbo) {
                const auto& [vao, vertexVbo, instanceVbo] = item.second;
                glDeleteVertexArrays(1, &vao);
                glDeleteBuffers(1, &vertexVbo);
                glDeleteBuffers(1, &instanceVbo);
            }
        });
        for (auto& item: triangleData) {
            item.second.freeBuffers();
        }
        lineData.freeBuffers();
        optionalLineData.freeBuffers();
    }

    Mesh::~Mesh() = default;

    std::unique_ptr<TexturedTriangleInstance[], std::default_delete<TexturedTriangleInstance[]>> Mesh::generateTexturedTriangleInstancesArray() {
        auto array = std::make_unique<TexturedTriangleInstance[]>(instances.size());
        for (int i = 0; i < instances.size(); ++i) {
            auto& instance = instances[i];
            array[i].idColor = color::convertIntToColorVec3(instance.elementId);
            array[i].transformation = glm::transpose(instance.transformation * constants::LDU_TO_OPENGL);
        }
        return array;
    }

    std::optional<InstanceRange> Mesh::getSceneInstanceRange(scene_id_t sceneId) {
        auto it = instanceSceneLayerRanges.find(sceneId);
        if (it == instanceSceneLayerRanges.end()) {
            return {};
        }
        const auto& layerMap = it->second;
        unsigned int start;
        unsigned int count;
        if (layerMap.size() > 1) {
            count = 0;
            for (const auto& item: layerMap) {
                count += item.second.count;
                start = std::min(start, item.second.start);
            }
        } else {
            start = layerMap.cbegin()->second.start;
            count = layerMap.cbegin()->second.count;
        }

        return {{start, count}};
    }

    std::optional<InstanceRange> Mesh::getSceneLayerInstanceRange(scene_id_t sceneId, layer_t layer) {
        const auto& it = instanceSceneLayerRanges.find(sceneId);
        if (it != instanceSceneLayerRanges.end()) {
            const auto& it2 = it->second.find(layer);
            if (it2 != it->second.end()) {
                return it2->second;
            }
        }
        return {};
    }

    void Mesh::updateInstancesOfScene(scene_id_t sceneId, const std::vector<MeshInstance>& newSceneInstances) {
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
                for (auto& item: instanceSceneLayerRanges) {
                    auto& layerMap = item.second;
                    if (layerMap.cbegin()->second.start >= afterLastIndex) {
                        //the instances of this scene are located after the sceneId, we have to update the ranges
                        for (auto& layerRange: layerMap) {
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
                auto& thisSceneRanges = instanceSceneLayerRanges[sceneId];
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

    void Mesh::appendNewSceneInstancesAtEnd(scene_id_t sceneId, const std::vector<MeshInstance>& newSceneInstances) {
        auto& thisSceneRanges = instanceSceneLayerRanges[sceneId];
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

        instancesHaveChanged = true;
    }

    size_t Mesh::getTriangleCount() {
        size_t count = 0;
        for (const auto& item: triangleData) {
            count += item.second.getIndexCount();
        }
        for (const auto& item: textureVertices) {
            count += item.second.size();
        }
        return count / 3;
    }

    LineData& Mesh::getLineData() {
        return lineData;
    }

    LineData& Mesh::getOptionalLineData() {
        return optionalLineData;
    }
    std::map<ldr::ColorReference, TriangleData>& Mesh::getAllTriangleData() {
        return triangleData;
    }
    TriangleData& Mesh::getTriangleData(const ldr::ColorReference color) {
        auto it = triangleData.find(color);
        if (it == triangleData.end()) {
            return triangleData.insert({color, TriangleData(color)}).first->second;
        }
        return it->second;
    }
    void Mesh::drawTriangleGraphics(scene_id_t sceneId, layer_t layer) {
        const std::optional<InstanceRange>& range = getSceneLayerInstanceRange(sceneId, layer);
        for (auto& item: triangleData) {
            item.second.draw(range);
        }
    }
    const std::optional<OuterDimensions>& Mesh::getOuterDimensions() {
        if (!outerDimensions.has_value()) {
            calculateOuterDimensions();
        }
        return outerDimensions;
    }

    void Mesh::calculateOuterDimensions() {
        size_t vertexCount = 0;
        for (const auto& item: textureVertices) {
            vertexCount += item.second.size();
        }
        for (const auto& item: triangleData) {
            vertexCount += item.second.getVertexCount();
        }
        if (vertexCount > 0) {
            auto coords = std::make_unique<float*[]>(vertexCount);
            size_t coordsCursor = 0;
            for (auto& item: textureVertices) {
                for (auto& vertex: item.second) {
                    coords[coordsCursor] = &vertex.position[0];
                    ++coordsCursor;
                }
            }

            for (auto& item: triangleData) {
                item.second.fillVerticesForOuterDimensions(coords, coordsCursor);
            }

            Miniball::Miniball<Miniball::CoordAccessor<float* const*, const float*>> mb(3, coords.get(), coords.get() + vertexCount);

            float minX, maxX = coords[0][0];
            float minY, maxY = coords[0][1];
            float minZ, maxZ = coords[0][2];
            for (coordsCursor = 1; coordsCursor < vertexCount; ++coordsCursor) {
                float x = coords[coordsCursor][0];
                float y = coords[coordsCursor][1];
                float z = coords[coordsCursor][2];
                if (minX > x) {
                    minX = x;
                } else if (x > maxX) {
                    maxX = x;
                }
                if (minY > y) {
                    minY = y;
                } else if (y > maxY) {
                    maxY = y;
                }
                if (minZ > z) {
                    minZ = z;
                } else if (z > maxZ) {
                    maxZ = z;
                }
            }

            outerDimensions = OuterDimensions{
                    .smallestBoxCorner1 = {minX, minY, minZ},
                    .smallestBoxCorner2 = {maxX, maxY, maxZ},
                    .minEnclosingBallCenter = {mb.center()[0], mb.center()[1], mb.center()[2]},
                    .minEnclosingBallRadius = std::sqrt(mb.squared_radius()),
            };
        } else {
            outerDimensions = OuterDimensions{
                .smallestBoxCorner1 = {0, 0, 0},
                .smallestBoxCorner2 = {0, 0, 0},
                .minEnclosingBallCenter = {0, 0, 0},
                .minEnclosingBallRadius = 0.f,
            };
        }
    }
}