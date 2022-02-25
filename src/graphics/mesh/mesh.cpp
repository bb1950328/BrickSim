#include "mesh.h"
#include "../../config.h"
#include "../../controller.h"
#include "../../helpers/geometry.h"
#include "../../helpers/util.h"
#include "../../lib/Miniball.hpp"
#include "../../metrics.h"
#include "../opengl_native_or_replacement.h"
#include "../texmap_projection.h"
#include "mesh_line_data.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/normal.hpp>
#include <palanteer.h>

namespace bricksim::mesh {

    void Mesh::addLdrFile(ldr::ColorReference mainColor, const std::shared_ptr<ldr::File>& file, const glm::mat4& transformation, bool bfcInverted, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        for (const auto& element: file->elements) {
            if (element->hidden) {
                continue;
            }
            switch (element->getType()) {
                case 0: break;
                case 1:
                    addLdrSubfileReference(mainColor, std::dynamic_pointer_cast<ldr::SubfileReference>(element), transformation, bfcInverted, texmap);
                    break;
                case 2:
                    addLdrLine(mainColor, std::dynamic_pointer_cast<ldr::Line>(element), transformation);
                    break;
                case 3:
                    addLdrTriangle(mainColor, std::dynamic_pointer_cast<ldr::Triangle>(element), transformation, bfcInverted, texmap);
                    break;
                case 4:
                    addLdrQuadrilateral(mainColor, std::dynamic_pointer_cast<ldr::Quadrilateral>(element), transformation, bfcInverted, texmap);
                    break;
                case 5:
                    addLdrOptionalLine(mainColor, std::dynamic_pointer_cast<ldr::OptionalLine>(element), transformation);
                    break;
            }
        }
    }

    void Mesh::addLdrSubfileReference(ldr::ColorReference mainColor, const std::shared_ptr<ldr::SubfileReference>& sfElement, const glm::mat4& transformation, bool bfcInverted, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        auto sub_transformation = sfElement->getTransformationMatrix();
        const auto color = sfElement->color.get()->code == ldr::Color::MAIN_COLOR_CODE ? mainColor : sfElement->color;
        addLdrFile(color, sfElement->getFile(), sub_transformation * transformation, sfElement->bfcInverted ^ bfcInverted, sfElement->directTexmap != nullptr ? sfElement->directTexmap : texmap);
    }

    void Mesh::addLdrTriangle(const ldr::ColorReference mainColor, const std::shared_ptr<ldr::Triangle>& triangleElement, const glm::mat4& transformation, bool bfcInverted, const std::shared_ptr<ldr::TexmapStartCommand>& texmapOfParent) {
        auto p1 = glm::vec3(triangleElement->x1, triangleElement->y1, triangleElement->z1);
        auto p2 = glm::vec3(triangleElement->x2, triangleElement->y2, triangleElement->z2);
        auto p3 = glm::vec3(triangleElement->x3, triangleElement->y3, triangleElement->z3);

        const auto color = triangleElement->color.get()->code == ldr::Color::MAIN_COLOR_CODE ? mainColor : triangleElement->color;
        const auto normal = glm::triangleNormal(p1, p2, p3);
        const auto transformedNormal = glm::normalize(glm::vec4(normal, 0.0f) * transformation);

        if (geometry::doesTransformationInverseWindingOrder(transformation) ^ bfcInverted) {
            std::swap(p2, p3);
        }

        std::vector<glm::vec3> transformedPoints = {
                glm::vec4(p1, 1.0f) * transformation,
                glm::vec4(p2, 1.0f) * transformation,
                glm::vec4(p3, 1.0f) * transformation,
        };

        const auto& appliedTexmap = triangleElement->directTexmap != nullptr ? triangleElement->directTexmap : texmapOfParent;
        if (appliedTexmap == nullptr) {
            auto& data = getTriangleData(color);
            auto idx1 = data.getVertexCount();
            for (const auto& tp: transformedPoints) {
                data.addVertexWithIndex({tp, transformedNormal});
            }
        } else {
            calculateAndAddTexmapVertices(color, appliedTexmap, transformedPoints);
        }

        if (config::get(config::SHOW_NORMALS)) {
            auto lp1 = glm::vec4(geometry::triangleCentroid(p1, p2, p3), 1.0f) * transformation;
            auto lp2 = lp1 + (transformedNormal * 5.0f);
            LineVertex lv1{lp1, transformedNormal};
            LineVertex lv2{lp2, transformedNormal};
            lineData.addVertex(lv1);
            lineData.addVertex(lv2);
        }
    }

    void Mesh::calculateAndAddTexmapVertices(const ldr::ColorReference& color, const std::shared_ptr<ldr::TexmapStartCommand>& appliedTexmap, std::vector<glm::vec3>& transformedPoints) {
        const auto pointCount = transformedPoints.size();
        std::vector<glm::vec2> UVs;
        UVs.reserve(pointCount);
        bool allUVsInsideImage = true;
        for (const auto& point: transformedPoints) {
            auto uv = graphics::texmap_projection::getPlanarUVCoord(appliedTexmap, point);
            allUVsInsideImage &= util::isUvInsideImage(uv);
            UVs.push_back(uv);
        }
        auto texture = graphics::texmap_projection::getTexture(appliedTexmap);
        auto& texturedData = getTexturedTriangleData(texture);
        if (allUVsInsideImage) {
            if (pointCount == 3) {
                for (int i = 0; i < pointCount; ++i) {
                    texturedData.addVertex({transformedPoints[i], UVs[i]});
                }
            } else if (pointCount ==4) {
                //triangle 1
                texturedData.addVertex({transformedPoints[0], UVs[0]});
                texturedData.addVertex({transformedPoints[1], UVs[1]});
                texturedData.addVertex({transformedPoints[2], UVs[2]});
                
                //triangle 2
                texturedData.addVertex({transformedPoints[2], UVs[2]});
                texturedData.addVertex({transformedPoints[3], UVs[3]});
                texturedData.addVertex({transformedPoints[0], UVs[0]});
            } else {
                //todo add earcut
            }
        } else {
            auto [plainIndices, plainVertices, texturedVertices] = graphics::texmap_projection::splitPolygonBiggerThanTexturePlanar(appliedTexmap, transformedPoints);
            auto& plainData = getTriangleData(color);
            auto baseIndex = plainData.getVertexCount();
            for (const auto& vtx: plainVertices) {
                plainData.addRawVertex(vtx);
            }
            for (const auto& idx: plainIndices) {
                plainData.addRawIndex(baseIndex + idx);
            }
            for (const auto& vtx: texturedVertices) {
                texturedData.addVertex(vtx);
            }
        }
    }

    void Mesh::addLdrQuadrilateral(ldr::ColorReference mainColor, const std::shared_ptr<ldr::Quadrilateral>& quadrilateral, const glm::mat4& transformation, bool bfcInverted, const std::shared_ptr<ldr::TexmapStartCommand>& texmapOfParent) {
        auto p1 = glm::vec3(quadrilateral->x1, quadrilateral->y1, quadrilateral->z1);
        auto p2 = glm::vec3(quadrilateral->x2, quadrilateral->y2, quadrilateral->z2);
        auto p3 = glm::vec3(quadrilateral->x3, quadrilateral->y3, quadrilateral->z3);
        auto p4 = glm::vec3(quadrilateral->x4, quadrilateral->y4, quadrilateral->z4);

        const auto color = quadrilateral->color.get()->code == ldr::Color::MAIN_COLOR_CODE ? mainColor : quadrilateral->color;
        const auto normal = glm::triangleNormal(p1, p2, p3);
        const auto transformedNormal = glm::normalize(glm::vec4(normal, 0.0f) * transformation);

        if (geometry::doesTransformationInverseWindingOrder(transformation) ^ bfcInverted) {
            std::swap(p2, p4);
        }

        std::vector<glm::vec3> transformedPoints = {
                glm::vec4(p1, 1.0f) * transformation,
                glm::vec4(p2, 1.0f) * transformation,
                glm::vec4(p3, 1.0f) * transformation,
                glm::vec4(p4, 1.0f) * transformation,
        };

        const auto& appliedTexmap = quadrilateral->directTexmap != nullptr ? quadrilateral->directTexmap : texmapOfParent;
        if (appliedTexmap == nullptr) {
            auto& data = getTriangleData(color);
            auto idx1 = data.getVertexCount();
            unsigned int idx = data.getVertexCount();
            for (const auto& tp: transformedPoints) {
                data.addRawVertex({tp, transformedNormal});
            }
            //triangle 1
            data.addRawIndex(idx);
            data.addRawIndex(idx + 1);
            data.addRawIndex(idx + 2);

            //triangle 2
            data.addRawIndex(idx + 2);
            data.addRawIndex(idx + 3);
            data.addRawIndex(idx);
        } else {
            calculateAndAddTexmapVertices(color, appliedTexmap, transformedPoints);
        }

        if (config::get(config::SHOW_NORMALS)) {
            auto lp1 = glm::vec4(geometry::quadrilateralCentroid(p1, p2, p3, p4), 1.0f) * transformation;
            auto lp2 = lp1 + (transformedNormal * 5.0f);
            LineVertex lv1{lp1, transformedNormal};
            LineVertex lv2{lp2, transformedNormal};
            lineData.addVertex(lv1);
            lineData.addVertex(lv2);
        }
    }

    void Mesh::addLdrLine(const ldr::ColorReference mainColor, const std::shared_ptr<ldr::Line>& lineElement, const glm::mat4& transformation) {
        glm::vec3 color;
        const auto lineElementColor = lineElement->color.get();
        const auto mainColorLocked = mainColor.get();
        if (lineElementColor->code == ldr::Color::MAIN_COLOR_CODE) {
            color = mainColorLocked->edge.asGlmVector();
        } else if (lineElementColor->code == ldr::Color::LINE_COLOR_CODE) {
            color = glm::vec3(1 - util::vectorSum(mainColorLocked->value.asGlmVector()) / 3);//todo look up specification
        } else {
            color = lineElementColor->edge.asGlmVector();
        }
        LineVertex lv1{glm::vec4(lineElement->x1, lineElement->y1, lineElement->z1, 1.0f) * transformation, color};
        LineVertex lv2{glm::vec4(lineElement->x2, lineElement->y2, lineElement->z2, 1.0f) * transformation, color};
        lineData.addVertex(lv1);
        lineData.addVertex(lv2);
    }

    void Mesh::addLdrOptionalLine(const ldr::ColorReference mainColor, const std::shared_ptr<ldr::OptionalLine>& optionalLineElement, const glm::mat4& transformation) {
        glm::vec3 color;
        const auto elementColor = optionalLineElement->color.get();
        const auto mainColorLocked = mainColor.get();
        if (elementColor->code == ldr::Color::MAIN_COLOR_CODE) {
            color = mainColorLocked->edge.asGlmVector();
        } else if (elementColor->code == ldr::Color::LINE_COLOR_CODE) {
            color = glm::vec3(1 - util::vectorSum(mainColorLocked->value.asGlmVector()) / 3);//todo look up specification
        } else {
            color = elementColor->edge.asGlmVector();
        }
        LineVertex cv1{glm::vec4(optionalLineElement->controlX1, optionalLineElement->controlY1, optionalLineElement->controlZ1, 1.0f) * transformation, color};
        LineVertex lv1{glm::vec4(optionalLineElement->x1, optionalLineElement->y1, optionalLineElement->z1, 1.0f) * transformation, color};
        LineVertex lv2{glm::vec4(optionalLineElement->x2, optionalLineElement->y2, optionalLineElement->z2, 1.0f) * transformation, color};
        LineVertex cv2{glm::vec4(optionalLineElement->controlX2, optionalLineElement->controlY2, optionalLineElement->controlZ2, 1.0f) * transformation, color};
        optionalLineData.addVertex(cv1);
        optionalLineData.addVertex(lv1);
        optionalLineData.addVertex(lv2);
        optionalLineData.addVertex(cv2);
    }

    void Mesh::writeGraphicsData() {
        plFunction();
        if (!alreadyInitialized) {
            if (!outerDimensions.has_value()) {
                calculateOuterDimensions();
            }
            if (config::get(config::DRAW_MINIMAL_ENCLOSING_BALL_LINES)) {
                addMinEnclosingBallLines();
            }

            for (auto& item: triangleData) {
                item.second.initBuffers(instances);
            }

            const std::vector<TexturedTriangleInstance>& instancesForTexturedTriangleData = getInstancesForTexturedTriangleData();
            for (auto& item: texturedTriangleData) {
                item.second.initBuffers(instancesForTexturedTriangleData);
            }

            const std::vector<glm::mat4> instancesForLineData = getInstancesForLineData();
            lineData.initBuffers(instancesForLineData);
            optionalLineData.initBuffers(instancesForLineData);

            alreadyInitialized = true;
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

    void Mesh::rewriteInstanceBuffer() {
        if (instancesHaveChanged) {
            //todo just clear buffer data when no instances
            controller::executeOpenGL([this]() {
                std::vector<glm::mat4> lineInstances = getInstancesForLineData();
                lineData.rewriteInstanceBuffer(lineInstances);
                optionalLineData.rewriteInstanceBuffer(lineInstances);

                for (auto& item: triangleData) {
                    item.second.rewriteInstanceBuffer(instances);
                }

                if (!texturedTriangleData.empty()) {
                    const auto texturedTriangleInstances = getInstancesForTexturedTriangleData();
                    for (auto& item: texturedTriangleData) {
                        item.second.rewriteInstanceBuffer(texturedTriangleInstances);
                    }
                }
            });

            instancesHaveChanged = false;
        }
    }
    std::vector<glm::mat4> Mesh::getInstancesForLineData() {
        std::vector<glm::mat4> instancesArray;
        instancesArray.resize(instances.size());
        for (int i = 0; i < instances.size(); ++i) {
            instancesArray[i] = glm::transpose(instances[i].transformation * constants::LDU_TO_OPENGL);
            instancesArray[i][2][3] = instances[i].selected;
        }
        return instancesArray;
    }

    void Mesh::drawTexturedTriangleGraphics(scene_id_t sceneId, layer_t layer) {
        auto range = getSceneLayerInstanceRange(sceneId, layer);
        if (range.has_value() && range->count > 0) {
            for (auto& item: texturedTriangleData) {
                item.second.draw(range.value());
            }
        }
    }

    void Mesh::deallocateGraphics() {
        for (auto& item: triangleData) {
            item.second.freeBuffers();
        }
        for (auto& item: texturedTriangleData) {
            item.second.freeBuffers();
        }
        lineData.freeBuffers();
        optionalLineData.freeBuffers();
    }

    Mesh::~Mesh() = default;

    std::vector<TexturedTriangleInstance> Mesh::getInstancesForTexturedTriangleData() {
        std::vector<TexturedTriangleInstance> array;
        array.reserve(instances.size());
        for (const auto& inst: instances) {
            array.push_back({
                    .idColor = color::convertIntToColorVec3(inst.elementId),
                    .transformation = glm::transpose(inst.transformation * constants::LDU_TO_OPENGL),
            });
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
        for (const auto& item: texturedTriangleData) {
            count += item.second.getVertexCount();
        }
        return count / 3;
    }

    LineData& Mesh::getLineData() {
        return lineData;
    }

    LineData& Mesh::getOptionalLineData() {
        return optionalLineData;
    }
    uomap_t<ldr::ColorReference, TriangleData>& Mesh::getAllTriangleData() {
        return triangleData;
    }

    TriangleData& Mesh::getTriangleData(const ldr::ColorReference color) {
        auto it = triangleData.find(color);
        if (it == triangleData.end()) {
            return triangleData.emplace(color, color).first->second;
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
        for (const auto& item: texturedTriangleData) {
            vertexCount += item.second.getVertexCount();
        }
        for (const auto& item: triangleData) {
            vertexCount += item.second.getVertexCount();
        }
        if (vertexCount > 0) {
            auto coords = std::vector<const float*>();
            coords.resize(vertexCount);
            size_t coordsCursor = 0;

            for (const auto& item: triangleData) {
                item.second.fillVerticesForOuterDimensions(coords, coordsCursor);
            }
            for (const auto& item: texturedTriangleData) {
                item.second.fillVerticesForOuterDimensions(coords, coordsCursor);
            }

            Miniball::Miniball<Miniball::CoordAccessor<const float* const*, const float*>> mb(3, &coords[0], &coords[0] + vertexCount);

            AxisAlignedBoundingBox aabb;
            
            for (const float* c : coords) {
                aabb.addPoint({c[0], c[1], c[2]});
            }

            outerDimensions = OuterDimensions{
                    .aabb = aabb,
                    .minEnclosingBallCenter = {mb.center()[0], mb.center()[1], mb.center()[2]},
                    .minEnclosingBallRadius = std::sqrt(mb.squared_radius()),
            };
        } else {
            outerDimensions = OuterDimensions{
                    .aabb = {{0, 0, 0}, {0, 0, 0}},
                    .minEnclosingBallCenter = {0, 0, 0},
                    .minEnclosingBallRadius = 0.f,
            };
        }
    }
    uomap_t<texture_id_t, TexturedTriangleData>& Mesh::getAllTexturedTriangleData() {
        return texturedTriangleData;
    }
    TexturedTriangleData& Mesh::getTexturedTriangleData(std::shared_ptr<graphics::Texture>& texture) {
        auto it = texturedTriangleData.find(texture->getID());
        if (it == texturedTriangleData.end()) {
            it = texturedTriangleData.emplace(texture->getID(), texture).first;
        }
        return it->second;
    }
}