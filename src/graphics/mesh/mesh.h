#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

#include <glm/glm.hpp>
#include "../../types.h"
#include "../../ldr_files/ldr_colors.h"
#include "../../ldr_files/ldr_files.h"
#include "../texture.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <glad/glad.h>
#include "mesh_simple_classes.h"
#include "mesh_line_data.h"


namespace bricksim::mesh {

    class Mesh {
    public:
        std::map<ldr::ColorReference, std::vector<TriangleVertex>> triangleVertices;
        std::map<ldr::ColorReference, std::vector<unsigned int>> triangleIndices;

        std::map<ldr::ColorReference, unsigned int> VAOs, vertexVBOs, instanceVBOs, EBOs;

        /**
           |      scene=0      | scene=1 |
           | layer=0 | layer=1 | layer=0 |
           | in | in | in | in | in | in |
         idx 0    1    2    3    4    5
         */
        std::vector<MeshInstance> instances;
        bool instancesHaveChanged = false;

        LineData& getLineData();
        LineData& getOptionalLineData();


        std::map<scene_id_t, std::map<layer_t, InstanceRange>> instanceSceneLayerRanges;
        std::optional<InstanceRange> getSceneInstanceRange(scene_id_t sceneId);
        std::optional<InstanceRange> getSceneLayerInstanceRange(scene_id_t sceneId, layer_t layer);
        /**
         * @param sceneId
         * @param newSceneInstances must be ordered by layer
         */
        void updateInstancesOfScene(scene_id_t sceneId, const std::vector<MeshInstance> &newSceneInstances);
        void deleteInstancesOfScene(scene_id_t sceneId);

        std::string name = "?";

        Mesh() = default;
        Mesh &operator=(Mesh &) = delete;
        Mesh(const Mesh &) = delete;

        void addLdrFile(const std::shared_ptr<ldr::File> &file, glm::mat4 transformation, ldr::ColorReference mainColor, bool bfcInverted);
        void addLdrSubfileReference(ldr::ColorReference mainColor, const std::shared_ptr<ldr::SubfileReference> &sfElement, glm::mat4 transformation, bool bfcInverted);
        void addLdrLine(ldr::ColorReference mainColor, const ldr::Line &lineElement, glm::mat4 transformation);
        void addLdrTriangle(ldr::ColorReference mainColor, const ldr::Triangle &triangleElement, glm::mat4 transformation, bool bfcInverted);
        void addLdrQuadrilateral(ldr::ColorReference mainColor, ldr::Quadrilateral &&quadrilateral, glm::mat4 transformation, bool bfcInverted);
        void addLdrOptionalLine(ldr::ColorReference mainColor, const ldr::OptionalLine &optionalLineElement, glm::mat4 transformation);

        void addTexturedTriangle(const std::shared_ptr<graphics::Texture> &texture, glm::vec3 pt1, glm::vec2 tc1, glm::vec3 pt2, glm::vec2 tc2, glm::vec3 pt3, glm::vec2 tc3);
        void addRawTriangle(ldr::ColorReference color, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3);

        /**
         * @param color
         * @param vertex
         * @return the index of the vertex. You have to call addRawTriangleIndex otherwise the vertex won't show up
         */
        unsigned int addRawTriangleVertex(ldr::ColorReference color, const TriangleVertex &vertex);
        unsigned int getNextVertexIndex(ldr::ColorReference color);
        void addRawTriangleIndex(ldr::ColorReference color, unsigned int triangleIndex);


        std::vector<unsigned int> &getIndicesList(ldr::ColorReference color);
        std::vector<TriangleVertex> &getVerticesList(ldr::ColorReference color);

        void writeGraphicsData();

        void drawTriangleGraphics(scene_id_t sceneId, layer_t layer);
        void drawTexturedTriangleGraphics(scene_id_t sceneId, layer_t layer);

        void deallocateGraphics();
        virtual ~Mesh();

        std::pair<glm::vec3, float> getMinimalEnclosingBall();
        size_t getTriangleCount();
    private:
        std::optional<std::pair<glm::vec3, float>> minimalEnclosingBall;
        LineData lineData{GL_LINES};
        LineData optionalLineData{GL_LINES_ADJACENCY};

        std::map<texture_id_t, std::shared_ptr<graphics::Texture>> textures;
        std::map<texture_id_t, std::vector<TexturedTriangleVertex>> textureVertices;
        std::map<texture_id_t, std::tuple<unsigned int, unsigned int, unsigned int>> textureTriangleVaoVertexVboInstanceVbo;

        bool already_initialized = false;

        size_t lastInstanceBufferSize = 0;

        static void setInstanceColor(TriangleInstance *instance, ldr::ColorReference color);

        std::unique_ptr<TriangleInstance[], std::default_delete<TriangleInstance[]>> generateTriangleInstancesArray(ldr::ColorReference color);
        std::unique_ptr<TexturedTriangleInstance[], std::default_delete<TexturedTriangleInstance[]>> generateTexturedTriangleInstancesArray();

        void initializeTriangleGraphics();
        void initializeTexturedTriangleGraphics();

        void rewriteInstanceBuffer();

        void addMinEnclosingBallLines();
        void appendNewSceneInstancesAtEnd(scene_id_t sceneId, const std::vector<MeshInstance> &newSceneInstances);
    };
}

#endif //BRICKSIM_MESH_H
