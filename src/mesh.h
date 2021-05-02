

#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include <vector>
#include <set>
#include <cstring>
#include "ldr_files/ldr_files.h"
#include "shaders/shaders.h"
#include "constant_data/constants.h"
#include "types.h"
#include "texture.h"

struct TriangleVertex {
    glm::vec4 position;
    glm::vec3 normal;

    //glm::vec3 color;
    bool operator==(const TriangleVertex &other) const {
        //return position == other.position && normal == other.normal;
        return std::memcmp(this, &other, sizeof(TriangleVertex)) == 0;
    }
};

struct TexturedTriangleVertex {
    glm::vec3 position;
    glm::vec2 textureCoord;
    bool operator==(const TexturedTriangleVertex &other) const {
        return std::memcmp(this, &other, sizeof(*this)) == 0;
    }

    TexturedTriangleVertex(const glm::vec3 &position, const glm::vec2 &textureCoord) : position(position), textureCoord(textureCoord) {}
};

struct LineVertex {
    glm::vec4 position;
    glm::vec3 color;

    bool operator==(const LineVertex &other) const {
        //return position == other.position && color == other.color;
        return std::memcmp(this, &other, sizeof(LineVertex)) == 0;
    }
};

struct TriangleInstance {
    glm::vec3 diffuseColor;
    float ambientFactor;//ambient=diffuseColor*ambientFactor
    float specularBrightness;//specular=vec4(1.0)*specularBrightness
    float shininess;
    glm::vec3 idColor;
    glm::mat4 transformation;
};

struct TexturedTriangleInstance {
    glm::vec3 idColor;
    glm::mat4 transformation;
};

struct MeshInstance {
    LdrColorReference color;
    glm::mat4 transformation;
    unsigned int elementId;
    bool selected;
    layer_t layer;
    scene_id_t scene;
    bool operator==(const MeshInstance& other) const;
    bool operator!=(const MeshInstance& other) const;
};

class Mesh {
public:
    std::map<LdrColorReference, std::vector<TriangleVertex>> triangleVertices;
    std::map<LdrColorReference, std::vector<unsigned int>> triangleIndices;

    std::vector<LineVertex> lineVertices;
    std::vector<unsigned int> lineIndices;

    std::vector<LineVertex> optionalLineVertices;
    std::vector<unsigned int> optionalLineIndices;

    std::map<LdrColorReference, unsigned int> VAOs, vertexVBOs, instanceVBOs, EBOs;

    /**
       |      scene=0      | scene=1 |
       | layer=0 | layer=1 | layer=0 |
       | in | in | in | in | in | in |
     idx 0    1    2    3    4    5
     */
    std::vector<MeshInstance> instances;
    bool instancesHaveChanged = false;

    struct InstanceRange {
        unsigned int start;
        unsigned int count;
    };

    std::map<scene_id_t, std::map<layer_t, InstanceRange>> instanceSceneLayerRanges;
    std::optional<Mesh::InstanceRange> getSceneInstanceRange(scene_id_t sceneId);
    std::optional<Mesh::InstanceRange> getSceneLayerInstanceRange(scene_id_t sceneId, layer_t layer);
    /**
     * @param sceneId
     * @param newSceneInstances must be ordered by layer
     */
    void updateInstancesOfScene(scene_id_t sceneId, const std::vector<MeshInstance>& newSceneInstances);
    void deleteInstancesOfScene(scene_id_t sceneId);

    std::string name = "?";

    Mesh()=default;
    Mesh & operator=(Mesh&) = delete;
    Mesh(const Mesh&) = delete;

    void addLdrFile(const std::shared_ptr<LdrFile> &file, glm::mat4 transformation, LdrColorReference mainColor, bool bfcInverted);
    void addLdrSubfileReference(LdrColorReference mainColor, const std::shared_ptr<LdrSubfileReference>& sfElement, glm::mat4 transformation, bool bfcInverted);
    void addLdrLine(LdrColorReference mainColor, const LdrLine &lineElement, glm::mat4 transformation);
    void addLdrTriangle(LdrColorReference mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation, bool bfcInverted);
    void addLdrQuadrilateral(LdrColorReference mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation, bool bfcInverted);
    void addLdrOptionalLine(LdrColorReference mainColor, const LdrOptionalLine &optionalLineElement, glm::mat4 transformation);

    void addTexturedTriangle(const std::shared_ptr<Texture> &texture, glm::vec3 pt1, glm::vec2 tc1, glm::vec3 pt2, glm::vec2 tc2, glm::vec3 pt3, glm::vec2 tc3);
    void addRawTriangle(LdrColorReference color, const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3);

    /**
     * @param color
     * @param vertex
     * @return the index of the vertex. You have to call addRawTriangleIndex otherwise the vertex won't show up
     */
    unsigned int addRawTriangleVertex(LdrColorReference color, const TriangleVertex& vertex);
    unsigned int getNextVertexIndex(LdrColorReference color);
    void addRawTriangleIndex(LdrColorReference color, unsigned int triangleIndex);

    void addLineVertex(const LineVertex &vertex);
    void addOptionalLineVertex(const LineVertex &vertex);

    std::vector<unsigned int> & getIndicesList(LdrColorReference color);
    std::vector<TriangleVertex> & getVerticesList(LdrColorReference color);

    void writeGraphicsData();

    void drawTriangleGraphics(scene_id_t sceneId, layer_t layer);
    void drawTexturedTriangleGraphics(scene_id_t sceneId, layer_t layer);
    void drawLineGraphics(scene_id_t sceneId, layer_t layer);
    void drawOptionalLineGraphics(scene_id_t sceneId, layer_t layer);

    void deallocateGraphics();
    virtual ~Mesh();

    std::pair<glm::vec3, float> getMinimalEnclosingBall();
private:
    std::optional<std::pair<glm::vec3, float>> minimalEnclosingBall;

    unsigned int lineVAO, lineVertexVBO, lineInstanceVBO, lineEBO;
    unsigned int optionalLineVAO, optionalLineVertexVBO, optionalLineInstanceVBO, optionalLineEBO;

    std::map<texture_id_t, std::shared_ptr<Texture>> textures;
    std::map<texture_id_t, std::vector<TexturedTriangleVertex>> textureVertices;
    std::map<texture_id_t, std::tuple<unsigned int, unsigned int, unsigned int>> textureTriangleVaoVertexVboInstanceVbo;

    bool already_initialized = false;

    size_t lastInstanceBufferSize = 0;

    static void setInstanceColor(TriangleInstance *instance, LdrColorReference color);

    std::unique_ptr<TriangleInstance[], std::default_delete<TriangleInstance[]>> generateTriangleInstancesArray(LdrColorReference color);
    std::unique_ptr<TexturedTriangleInstance[], std::default_delete<TexturedTriangleInstance[]>> generateTexturedTriangleInstancesArray();

    void initializeTriangleGraphics();
    void initializeTexturedTriangleGraphics();
    void initializeLineGraphics();
    void initializeOptionalLineGraphics();

    void rewriteInstanceBuffer();

    void addMinEnclosingBallLines();
    void appendNewSceneInstancesAtEnd(scene_id_t sceneId, const std::vector<MeshInstance> &newSceneInstances);
};

#endif //BRICKSIM_MESH_H
