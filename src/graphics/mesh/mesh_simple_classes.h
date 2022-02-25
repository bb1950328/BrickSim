#pragma once

#include "../../ldr/colors.h"
#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>

namespace bricksim::mesh {
    struct TriangleVertex {
        glm::vec3 position;
        glm::vec3 normal;

        bool operator==(const TriangleVertex& other) const;
    };

    struct TexturedTriangleVertex {
        glm::vec3 position;
        glm::vec2 textureCoord;
        bool operator==(const TexturedTriangleVertex& other) const;

        TexturedTriangleVertex(const glm::vec3& position, const glm::vec2& textureCoord) :
            position(position), textureCoord(textureCoord) {}
    };

    struct LineVertex {
        glm::vec3 position;
        glm::vec3 color;

        bool operator==(const LineVertex& other) const;
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
    };

    struct MeshInstance {
        ldr::ColorReference color;
        glm::mat4 transformation;
        unsigned int elementId;
        bool selected;
        layer_t layer;
        scene_id_t scene;
        bool operator==(const MeshInstance& other) const;
        bool operator!=(const MeshInstance& other) const;
    };

    struct InstanceRange {
        unsigned int start;
        unsigned int count;
    };

    struct AxisAlignedBoundingBox {
        glm::vec3 pMin;
        glm::vec3 pMax;

        AxisAlignedBoundingBox();
        AxisAlignedBoundingBox(const glm::vec3& pMin, const glm::vec3& pMax);

        void addPoint(const glm::vec3& p);
        void addAABB(const AxisAlignedBoundingBox& other);

        [[nodiscard]] AxisAlignedBoundingBox transform(const glm::mat4& transformation) const;

        [[nodiscard]] bool isDefined() const;

        [[nodiscard]] glm::vec3 getCenter() const;
        [[nodiscard]] glm::vec3 getSize() const;
    };

    struct RotatedBoundingBox {
        glm::vec3 center;
        glm::vec3 size;
        glm::quat rotation;

        RotatedBoundingBox(const AxisAlignedBoundingBox& aabb, glm::quat rotation) :
            center(aabb.getCenter()), size(aabb.getSize()), rotation(rotation) {
        }

        [[nodiscard]] glm::mat4 getUnitBoxTransformation() const;
    };

    struct OuterDimensions {
        AxisAlignedBoundingBox aabb;
        glm::vec3 minEnclosingBallCenter;
        float minEnclosingBallRadius;
    };
}
