#pragma once

#include "../../helpers/aabb.h"
#include "../../ldr/colors.h"
#include <glm/glm.hpp>

namespace bricksim::mesh {
    struct TriangleVertex {
        glm::vec3 position;
        glm::vec3 normal;
        TriangleVertex(const glm::vec3& position, const glm::vec3& normal);
        bool operator==(const TriangleVertex& other) const = default;
    };

    struct TexturedTriangleVertex {
        glm::vec3 position;
        glm::vec2 textureCoord;
        bool operator==(const TexturedTriangleVertex& other) const = default;

        TexturedTriangleVertex(const glm::vec3& position, const glm::vec2& textureCoord) :
            position(position), textureCoord(textureCoord) {}
    };

    struct LineVertex {
        glm::vec3 position;
        glm::vec3 color;

        bool operator==(const LineVertex& other) const = default;
    };

    struct TriangleInstance {
        glm::vec3 diffuseColor;
        float ambientFactor;     //ambient=diffuseColor*ambientFactor
        float specularBrightness;//specular=vec4(1.0)*specularBrightness
        float shininess;
        glm::vec3 idColor;
        glm::mat4 transformation;
        void setColor(ldr::ColorReference color);
    };

    struct TexturedTriangleInstance {
        glm::vec3 idColor;
        glm::mat4 transformation;
        TexturedTriangleInstance(const glm::vec3& idColor, const glm::mat4& transformation);
    };

    struct MeshInstance {
        ldr::ColorReference color;
        glm::mat4 transformation;
        unsigned int elementId;
        bool selected;
        layer_t layer;
        scene_id_t scene;
        bool operator==(const MeshInstance& other) const = default;
    };

    struct InstanceRange {
        unsigned int start;
        unsigned int count;
    };



    struct OuterDimensions {
        aabb::AABB aabb;
        glm::vec3 minEnclosingBallCenter;
        float minEnclosingBallRadius;
    };
}
