// mesh.h
// Created by bb1950328 on 20.09.20.
//

#ifndef BRICKSIM_MESH_H
#define BRICKSIM_MESH_H

#include <glm/glm.hpp>
#include <vector>
#include <set>
#include "ldr_files.h"
#include "shaders/shader.h"
#include "helpers/camera.h"
#include "constants.h"

struct TriangleVertex {
    //TriangleVertex(const glm::vec4 &position, const glm::vec3 &normal, const glm::vec3 &color);

    glm::vec4 position;
    glm::vec3 normal;

    //glm::vec3 color;
    bool operator==(const TriangleVertex &other) const {
        return position == other.position && normal == other.normal;
    }
};

struct LineVertex {
    glm::vec4 position;
    glm::vec3 color;
    bool operator==(const LineVertex &other) const {
        return position == other.position && color == other.color;
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
    const LdrColor * color;
    glm::mat4 transformation;
    unsigned int elementId;
    bool selected;
    bool operator==(const MeshInstance& other) const;
    bool operator!=(const MeshInstance& other) const;
};

class Mesh {
public:
    std::map<const LdrColor *, std::vector<TriangleVertex> *> triangleVertices;
    std::map<const LdrColor *, std::vector<unsigned int> *> triangleIndices;

    std::vector<LineVertex> lineVertices;
    std::vector<unsigned int> lineIndices;

    std::vector<LineVertex> optionalLineVertices;
    std::vector<unsigned int> optionalLineIndices;

    std::map<const LdrColor *, unsigned int> VAOs, vertexVBOs, instanceVBOs, EBOs;

    std::vector<MeshInstance> instances;
    bool instancesHaveChanged = false;

    std::string name = "?";

    Mesh()=default;

    void addLdrFile(const LdrFile &file, glm::mat4 transformation, const LdrColor *mainColor, bool bfcInverted);
    void addLdrSubfileReference(const LdrColor *mainColor, LdrSubfileReference *sfElement, glm::mat4 transformation, bool bfcInverted);
    void addLdrLine(const LdrColor *mainColor, const LdrLine &lineElement, glm::mat4 transformation);
    void addLdrTriangle(const LdrColor *mainColor, const LdrTriangle &triangleElement, glm::mat4 transformation, bool bfcInverted);
    void addLdrQuadrilateral(const LdrColor *mainColor, LdrQuadrilateral &&quadrilateral, glm::mat4 transformation, bool bfcInverted);
    void addLdrOptionalLine(const LdrColor *mainColor, const LdrOptionalLine &optionalLineElement, glm::mat4 transformation);

    std::vector<unsigned int> *getIndicesList(const LdrColor *color);
    std::vector<TriangleVertex> *getVerticesList(const LdrColor *color);

    void writeGraphicsData();

    void drawTriangleGraphics();
    void drawLineGraphics();
    void drawOptionalLineGraphics();

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

    static void setInstanceColor(TriangleInstance *instance, const LdrColor *color) ;

    TriangleInstance * generateInstancesArray(const LdrColor *color);

    void initializeTriangleGraphics();
    void initializeLineGraphics();
    void initializeOptionalLineGraphics();

    void rewriteInstanceBuffer();

    void addLineVertex(const LineVertex &vertex);
    void addOptionalLineVertex(const LineVertex &vertex);

    void addMinEnclosingBallLines();
};

#endif //BRICKSIM_MESH_H
