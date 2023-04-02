#pragma once

#include <glm/ext/quaternion_float.hpp>
#include <glm/glm.hpp>

namespace bricksim::aabb {
    struct RotatedBoundingBox;

    struct AABB {
        glm::vec3 pMin;
        glm::vec3 pMax;

        AABB();
        AABB(const glm::vec3& pMin, const glm::vec3& pMax);

        void includePoint(const glm::vec3& p);
        void includeAABB(const AABB& other);
        void includeBBox(const RotatedBoundingBox& bbox);

        [[nodiscard]] AABB transform(const glm::mat4& transformation) const;

        [[nodiscard]] bool isDefined() const;

        [[nodiscard]] glm::vec3 getCenter() const;
        [[nodiscard]] glm::vec3 getSize() const;
    };

    struct RotatedBoundingBox {
        glm::vec3 centerOffset;
        glm::vec3 origin;
        glm::vec3 size;
        glm::quat rotation;

        RotatedBoundingBox();
        explicit RotatedBoundingBox(const AABB& aabb);
        RotatedBoundingBox(const AABB& aabb, glm::vec3 origin, glm::quat rotation) :
            centerOffset(aabb.getCenter() - origin), origin(origin), size(aabb.getSize()), rotation(rotation) {
        }

        [[nodiscard]] glm::mat4 getUnitBoxTransformation() const;
        [[nodiscard]] RotatedBoundingBox transform(const glm::mat4& transformation) const;
        [[nodiscard]] glm::vec3 getCenter() const;
    };
}
