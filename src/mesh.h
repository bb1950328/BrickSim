// mesh.h
// Created by bb1950328 on 20.09.20.
//

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
    std::shared_ptr<const LdrColor> color;
    glm::mat4 transformation;
    unsigned int elementId;
    bool selected;
    layer_t layer;
    bool operator==(const MeshInstance& other) const;
    bool operator!=(const MeshInstance& other) const;
};

class Mesh {
public:
    std::map<std::shared_ptr<const LdrColor>, std::vector<TriangleVertex>> triangleVertices;
    std::map<std::shared_ptr<const LdrColor>, std::vector<unsigned int>> triangleIndices;

    std::vector<LineVertex> lineVertices;
    std::vector<unsigned int> lineIndices;

    std::vector<LineVertex> optionalLineVertices;
    std::vector<unsigned int> optionalLineIndices;

    std::map<std::shared_ptr<const LdrColor>, unsigned int> VAOs, vertexVBOs, instanceVBOs, EBOs;

    std::vector<MeshInstance> instances;
    bool instancesHaveChanged = false;
    std::map<layer_t, unsigned int> instanceCountOfLayerAndGreater;//for example element with key 4 is the number of elements in layer 4 and above

    std::string name = "?";

    Mesh()=default;

    void addLdrFile(const std::shared_ptr<LdrFile> &file, glm::mat4 transformation, const std::shared_ptr<const LdrColor>& mainColor, bool bfcInverted);
    void addLdrSubfileReference(std::shared_ptr<const LdrColor> mainColor, std::shared_ptr<LdrSubfileReference> sfElement, glm::mat4 transformation, bool bfcInverted);
    void addLdrLine(const std::shared_ptr<const LdrColor>& mainColor, const LdrLine &lineElement, glm::mat4 transformation);
    void addLdrTriangle(const std::shared_ptr<const LdrColor>& mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation, bool bfcInverted);
    void addLdrQuadrilateral(std::shared_ptr<const LdrColor> mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation, bool bfcInverted);
    void addLdrOptionalLine(const std::shared_ptr<const LdrColor>& mainColor, const LdrOptionalLine &optionalLineElement, glm::mat4 transformation);

    std::vector<unsigned int> & getIndicesList(const std::shared_ptr<const LdrColor>& color);
    std::vector<TriangleVertex> & getVerticesList(const std::shared_ptr<const LdrColor>& color);

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

    static void setInstanceColor(TriangleInstance *instance, const std::shared_ptr<const LdrColor>& color) ;

    TriangleInstance * generateInstancesArray(const std::shared_ptr<const LdrColor>& color);

    void initializeTriangleGraphics();
    void initializeLineGraphics();
    void initializeOptionalLineGraphics();

    void rewriteInstanceBuffer();

    void addLineVertex(const LineVertex &vertex);
    void addOptionalLineVertex(const LineVertex &vertex);

    void addMinEnclosingBallLines();
    void updateInstanceCountOfLayerAndGreater();
    void sortInstancesByLayer();
};

#endif //BRICKSIM_MESH_H
