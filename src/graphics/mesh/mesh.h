#pragma once

#include "../../ldr/colors.h"
#include "../../ldr/files.h"
#include "../../types.h"
#include "../texture.h"
#include "mesh_line_data.h"
#include "mesh_simple_classes.h"
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
        [[nodiscard]] TriangleData& getTriangleData(const ldr::ColorReference color);

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

        void addLdrFile(const std::shared_ptr<ldr::File>& file, glm::mat4 transformation, ldr::ColorReference mainColor, bool bfcInverted);
        void addLdrSubfileReference(ldr::ColorReference mainColor, const std::shared_ptr<ldr::SubfileReference>& sfElement, glm::mat4 transformation, bool bfcInverted);
        void addLdrLine(ldr::ColorReference mainColor, const ldr::Line& lineElement, glm::mat4 transformation);
        void addLdrTriangle(ldr::ColorReference mainColor, const ldr::Triangle& triangleElement, glm::mat4 transformation, bool bfcInverted);
        void addLdrQuadrilateral(ldr::ColorReference mainColor, ldr::Quadrilateral&& quadrilateral, glm::mat4 transformation, bool bfcInverted);
        void addLdrOptionalLine(ldr::ColorReference mainColor, const ldr::OptionalLine& optionalLineElement, glm::mat4 transformation);

        void addTexturedTriangle(const std::shared_ptr<graphics::Texture>& texture, glm::vec3 pt1, glm::vec2 tc1, glm::vec3 pt2, glm::vec2 tc2, glm::vec3 pt3, glm::vec2 tc3);

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

        std::optional<OuterDimensions> outerDimensions = {};

        std::map<texture_id_t, std::shared_ptr<graphics::Texture>> textures;
        std::map<texture_id_t, std::vector<TexturedTriangleVertex>> textureVertices;
        std::map<texture_id_t, std::tuple<unsigned int, unsigned int, unsigned int>> textureTriangleVaoVertexVboInstanceVbo;

        bool already_initialized = false;

        std::unique_ptr<TexturedTriangleInstance[], std::default_delete<TexturedTriangleInstance[]>> generateTexturedTriangleInstancesArray();

        void initializeTexturedTriangleGraphics();

        void rewriteInstanceBuffer();

        void addMinEnclosingBallLines();
        void calculateOuterDimensions();
        void appendNewSceneInstancesAtEnd(scene_id_t sceneId, const std::vector<MeshInstance>& newSceneInstances);
    };
}
