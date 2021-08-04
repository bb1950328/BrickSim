#pragma once

#include "../../ldr/colors.h"
#include "../../ldr/files.h"
#include "../../types.h"
#include "../texture.h"
#include "mesh_line_data.h"
#include "mesh_simple_classes.h"
#include "mesh_textured_triangle_data.h"
#include "mesh_triangle_data.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bricksim::mesh {

    class Mesh {
    public:
        /**
           |      scene=0      | scene=1 |
           | layer=0 | layer=1 | layer=0 |
           | in | in | in | in | in | in |
         idx 0    1    2    3    4    5
         */
        std::vector<MeshInstance> instances;
        bool instancesHaveChanged = false;

        [[nodiscard]] LineData& getLineData();
        [[nodiscard]] LineData& getOptionalLineData();
        [[nodiscard]] std::map<ldr::ColorReference, TriangleData>& getAllTriangleData();
        [[nodiscard]] TriangleData& getTriangleData(ldr::ColorReference color);
        [[nodiscard]] std::map<texture_id_t, TexturedTriangleData>& getAllTexturedTriangleData();
        [[nodiscard]] TexturedTriangleData& getTexturedTriangleData(std::shared_ptr<graphics::Texture>& texture);

        std::map<scene_id_t, std::map<layer_t, InstanceRange>> instanceSceneLayerRanges;
        std::optional<InstanceRange> getSceneInstanceRange(scene_id_t sceneId);
        std::optional<InstanceRange> getSceneLayerInstanceRange(scene_id_t sceneId, layer_t layer);
        /**
         * @param sceneId
         * @param newSceneInstances must be ordered by layer
         */
        void updateInstancesOfScene(scene_id_t sceneId, const std::vector<MeshInstance>& newSceneInstances);
        void deleteInstancesOfScene(scene_id_t sceneId);

        std::string name = "?";

        Mesh() = default;
        Mesh& operator=(Mesh&) = delete;
        Mesh(const Mesh&) = delete;

        void addLdrFile(ldr::ColorReference mainColor, const std::shared_ptr<ldr::File>& file, const glm::mat4& transformation, bool bfcInverted);
        void addLdrSubfileReference(ldr::ColorReference mainColor, const std::shared_ptr<ldr::SubfileReference>& sfElement, const glm::mat4& transformation, bool bfcInverted);
        void addLdrLine(ldr::ColorReference mainColor, const std::shared_ptr<ldr::Line>& lineElement, const glm::mat4& transformation);
        void addLdrTriangle(ldr::ColorReference mainColor, const std::shared_ptr<ldr::Triangle>& triangleElement, const glm::mat4& transformation, bool bfcInverted);
        void addLdrQuadrilateral(ldr::ColorReference mainColor, const std::shared_ptr<ldr::Quadrilateral>& quadrilateral, const glm::mat4& transformation, bool bfcInverted);
        void addLdrOptionalLine(ldr::ColorReference mainColor, const std::shared_ptr<ldr::OptionalLine>& optionalLineElement, const glm::mat4& transformation);

        void writeGraphicsData();

        void drawTriangleGraphics(scene_id_t sceneId, layer_t layer);
        void drawTexturedTriangleGraphics(scene_id_t sceneId, layer_t layer);

        void deallocateGraphics();
        virtual ~Mesh();

        size_t getTriangleCount();
        const std::optional<OuterDimensions>& getOuterDimensions();

    private:
        LineData lineData{GL_LINES};
        LineData optionalLineData{GL_LINES_ADJACENCY};
        std::map<ldr::ColorReference, TriangleData> triangleData;
        std::map<texture_id_t, TexturedTriangleData> texturedTriangleData;


        bool alreadyInitialized = false;

        void addMinEnclosingBallLines();
        void calculateOuterDimensions();
        std::optional<OuterDimensions> outerDimensions = {};

        void appendNewSceneInstancesAtEnd(scene_id_t sceneId, const std::vector<MeshInstance>& newSceneInstances);
        std::vector<glm::mat4> getInstancesForLineData();
        std::vector<TexturedTriangleInstance> getInstancesForTexturedTriangleData();
        void rewriteInstanceBuffer();
    };
}
