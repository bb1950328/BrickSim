

#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

#include <glm/glm.hpp>
#include <vector>
#include <set>
#include <cstring>
#include "ldr_files/ldr_files.h"
#include "shaders/shader.h"
#include "helpers/camera.h"
#include "constant_data/constants.h"
#include "types.h"

struct TriangleVertex {
    glm::vec4 position;
    glm::vec3 normal;

    //glm::vec3 color;
    bool operator==(const TriangleVertex &other) const {
        //return position == other.position && normal == other.normal;
        return std::memcmp(this, &other, sizeof(TriangleVertex)) == 0;
    }
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

    std::vector<MeshInstance> instances;
    bool instancesHaveChanged = false;
    std::map<layer_t, std::pair<unsigned int, unsigned int>> instanceCountOfLayer;//value.first is instance count in this layer, value.second is instance count sum of all layers before

    std::string name = "?";

    Mesh()=default;

    void addLdrFile(const std::shared_ptr<LdrFile> &file, glm::mat4 transformation, LdrColorReference mainColor, bool bfcInverted);
    void addLdrSubfileReference(LdrColorReference mainColor, const std::shared_ptr<LdrSubfileReference>& sfElement, glm::mat4 transformation, bool bfcInverted);
    void addLdrLine(LdrColorReference mainColor, const LdrLine &lineElement, glm::mat4 transformation);
    void addLdrTriangle(LdrColorReference mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation, bool bfcInverted);
    void addLdrQuadrilateral(LdrColorReference mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation, bool bfcInverted);
    void addLdrOptionalLine(LdrColorReference mainColor, const LdrOptionalLine &optionalLineElement, glm::mat4 transformation);

    std::vector<unsigned int> & getIndicesList(LdrColorReference color);
    std::vector<TriangleVertex> & getVerticesList(LdrColorReference color);

    void writeGraphicsData();

    void drawTriangleGraphics(layer_t layer=constants::DEFAULT_LAYER);
    void drawLineGraphics(layer_t layer=constants::DEFAULT_LAYER);
    void drawOptionalLineGraphics(layer_t layer=constants::DEFAULT_LAYER);

    void deallocateGraphics();
    virtual ~Mesh();

    //this is the conversion from the ldraw coordinate system to the OpenGL coordinate system
    glm::mat4 globalModel = glm::scale(constants::LDU_TO_OPENGL_ROTATION,
                                       glm::vec3(constants::LDU_TO_OPENGL_SCALE, constants::LDU_TO_OPENGL_SCALE, constants::LDU_TO_OPENGL_SCALE)); // and make 100 times smaller

    std::pair<glm::vec3, float> getMinimalEnclosingBall();
private:
    std::optional<std::pair<glm::vec3, float>> minimalEnclosingBall;

    unsigned int lineVAO, lineVertexVBO, lineInstanceVBO, lineEBO;

    unsigned int optionalLineVAO, optionalLineVertexVBO, optionalLineInstanceVBO, optionalLineEBO;
    bool already_initialized = false;

    size_t lastInstanceBufferSize = 0;

    static void setInstanceColor(TriangleInstance *instance, LdrColorReference color) ;

    std::unique_ptr<TriangleInstance[], std::default_delete<TriangleInstance[]>> generateInstancesArray(LdrColorReference color);

    void initializeTriangleGraphics();
    void initializeLineGraphics();
    void initializeOptionalLineGraphics();

    void rewriteInstanceBuffer();

    void addLineVertex(const LineVertex &vertex);
    void addOptionalLineVertex(const LineVertex &vertex);

    void addMinEnclosingBallLines();
    void updateInstanceCountOfLayer();
    void sortInstancesByLayer();
};

#endif //BRICKSIM_MESH_H
