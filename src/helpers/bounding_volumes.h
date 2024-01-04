#pragma once

#include <array>
#include <glm/ext/quaternion_float.hpp>
#include <glm/glm.hpp>
#include <memory>

namespace bricksim::aabb {
    struct OBB;

    struct AABB {
        glm::vec3 pMin;
        glm::vec3 pMax;

        AABB();
        AABB(const glm::vec3& pMin, const glm::vec3& pMax);
        AABB(const AABB& a, const AABB& b);

        void includePoint(const glm::vec3& p);
        void includeAABB(const AABB& other);
        void includeOBB(const OBB& bbox);

        [[nodiscard]] AABB transform(const glm::mat4& transformation) const;
        [[nodiscard]] glm::mat4 getUnitBoxTransformation() const;

        [[nodiscard]] bool isDefined() const;

        [[nodiscard]] glm::vec3 getCenter() const;
        [[nodiscard]] glm::vec3 getSize() const;
        [[nodiscard]] float getSurfaceArea() const;
        [[nodiscard]] float getVolume() const;
        [[nodiscard]] bool intersects(const AABB& other) const;
    };

    template<typename UserData>
    struct AabbTreeNode {
        AabbTreeNode* parent;
        std::array<std::unique_ptr<AabbTreeNode<UserData>>, 2> children = {nullptr, nullptr};
        AABB aabb;
        UserData userData;

        AabbTreeNode() = default;

        AabbTreeNode(const UserData& userData, const AABB& aabb) :
            userData(userData),
            aabb(aabb) {}

        [[nodiscard]] bool isLeaf() const {
            return children[0] != nullptr && children[1] != nullptr;
        }

        [[nodiscard]] bool isRoot() const {
            return parent == nullptr;
        }

        void updateAABB() {
            if (isLeaf()) {
                aabb = AABB(children[0]->aabb, children[1]->aabb);
            }
            if (!isRoot()) {
                parent->updateAABB();
            }
        }

        void setChildren(std::unique_ptr<AabbTreeNode<UserData>> a, std::unique_ptr<AabbTreeNode<UserData>> b) {
            children[0] = std::move(a);
            children[1] = std::move(b);
            for (const auto& ch: children) {
                ch->parent = this;
            }
        }
    };

    template<typename UserData>
    struct AabbTree {
        using node_t = AabbTreeNode<UserData>;
        std::unique_ptr<node_t> root;

        void add(const UserData& userData, const AABB& aabb) {
            if (root != nullptr) {
                insertNode(std::make_unique<node_t>(userData, aabb), &root);
            } else {
                root = std::make_unique<node_t>(userData, aabb);
            }
        }

        void insertNode(std::unique_ptr<node_t> node, node_t* parent) {
            if (parent->isLeaf()) {
                auto newParent = std::make_unique<node_t>();
                newParent->parent = parent->parent;
                newParent->setChildren(node, parent);
            }
        }
    };

    struct OBB {
        glm::vec3 centerOffset;
        glm::vec3 origin;
        glm::vec3 size;
        glm::quat rotation;

        OBB();
        explicit OBB(const AABB& aabb);

        OBB(const AABB& aabb, glm::vec3 origin, glm::quat rotation) :
            centerOffset(aabb.getCenter() - origin), origin(origin), size(aabb.getSize()), rotation(rotation) {}

        [[nodiscard]] glm::mat4 getUnitBoxTransformation() const;
        [[nodiscard]] OBB transform(const glm::mat4& transformation) const;
        [[nodiscard]] glm::vec3 getCenter() const;
    };
}
