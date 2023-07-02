#include "engine.h"

#include "../editor.h"
#include "../helpers/glm_eigen_conversion.h"
#include "connector_data_provider.h"
#include "pair_checker.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include <fcl/broadphase/broadphase_dynamic_AABB_tree.h>
#include <utility>

namespace bricksim::connection {
    namespace {
        std::vector<std::shared_ptr<etree::LdrNode>> getAllLdrNodesFlat(const std::shared_ptr<etree::Node>& node) {
            std::vector<std::shared_ptr<etree::LdrNode>> flat;
            std::function<void(const std::shared_ptr<etree::Node>&)> traverse = [&flat, &traverse](const std::shared_ptr<etree::Node>& node) {
                const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(node);
                if (ldrNode != nullptr) {
                    flat.push_back(ldrNode);
                }
                for (const auto& item: node->getChildren()) {
                    traverse(item);
                }
            };
            traverse(node);
            return flat;
        }

    }

    Engine::Engine(Editor& editor) :
        editor(editor) {
    }
    void Engine::updateCollisionData() {
        const auto& rootNode = editor.getEditingModel();
        const auto it = nodeData.find(rootNode);
        if (it == nodeData.end()) {
            resetData();
            updateNodeData(rootNode);
        } else {
            updateNodeData(rootNode, it);
        }

        std::vector<fcl::CollisionObjectf*> objs;
        manager.getObjects(objs);
        for (const auto& item: objs) {
            std::string& title = static_cast<etree::LdrNode*>(item->getUserData())->ldrFile->metaInfo.title;
            const auto& aabb = item->getAABB();
            std::cout << fmt::format("{}\t[{}, {}, {}]\t[{}, {}, {}]", title, aabb.min_.x(), aabb.min_.y(), aabb.min_.z(), aabb.max_.x(), aabb.max_.y(), aabb.max_.z()) << std::endl;
        }
    }
    void Engine::updateNodeData(const std::shared_ptr<etree::Node>& node) {
        auto it = nodeData.find(node);
        if (it != nodeData.end()) {
            updateNodeData(node, it);
        } else {
            std::unique_ptr<fcl::CollisionObjectf> collisionObject;
            const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(node);
            if (ldrNode != nullptr) {
                const auto box = std::make_shared<fcl::Boxf>(fcl::Vector3f(2.f, 2.f, 2.f));//todo try to use one instance for all objects
                collisionObject = std::make_unique<fcl::CollisionObjectf>(box, getCollisionBoxTransform(ldrNode));
                collisionObject->setUserData(node.get());
                manager.registerObject(collisionObject.get());
                outdatedInGraph.insert(ldrNode);
            } else {
                collisionObject = nullptr;
            }
            nodeData.emplace(node,
                             NodeData{
                                     node->getVersion(),
                                     node->getSelfVersion(),
                                     std::move(collisionObject),
                                     {node->getChildren().cbegin(), node->getChildren().cend()},
                             });
            for (const auto& child: node->getChildren()) {
                updateNodeData(child);
            }
        }
    }
    void Engine::updateNodeData(const std::shared_ptr<etree::Node>& node, decltype(nodeData)::iterator it) {
        auto& data = it->second;
        if (data.lastUpdatedSelfVersion != node->getSelfVersion()) {
            const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(node);
            if (ldrNode != nullptr) {
                const auto aabb = editor.getScene()->getMeshCollection().getAbsoluteAABB(ldrNode);
                data.collisionObj->setTransform(getCollisionBoxTransform(ldrNode));
                manager.update(it->second.collisionObj.get());
                outdatedInGraph.insert(ldrNode);
            }
            data.lastUpdatedSelfVersion = node->getSelfVersion();
        }
        if (data.lastUpdatedVersion != node->getVersion()) {
            for (const auto& child: node->getChildren()) {
                updateNodeData(child);
            }
            std::vector<std::shared_ptr<etree::Node>> deletedChildren;
            std::set_difference(data.children.cbegin(), data.children.cend(),
                                node->getChildren().cbegin(), node->getChildren().cend(),
                                std::back_inserter(deletedChildren));
            for (const auto& item: deletedChildren) {
                removeNodeData(item);
            }
            data.children = {node->getChildren().cbegin(), node->getChildren().cend()};
            data.lastUpdatedVersion = node->getVersion();
        }
    }
    void Engine::removeNodeData(const std::shared_ptr<etree::Node>& node) {
        const auto it = nodeData.find(node);
        if (it != nodeData.end()) {
            if (it->second.collisionObj != nullptr) {
                manager.unregisterObject(it->second.collisionObj.get());
                outdatedInGraph.insert(std::dynamic_pointer_cast<etree::LdrNode>(it->first));
            }
            nodeData.erase(it);
        }
    }
    void Engine::resetData() {
        manager.~DynamicAABBTreeCollisionManager();
        new (&manager) fcl::DynamicAABBTreeCollisionManagerf();
        nodeData.clear();
    }

    bool updateCallback(fcl::CollisionObjectf* o0, fcl::CollisionObjectf* o1, void* cdata) {
        return static_cast<Engine*>(cdata)->fclCallback(o0, o1);
    }

    void Engine::updateGraph() {
        graph.removeAllConnections(outdatedInGraph);
        for (const auto& item: outdatedInGraph) {
            const auto& data = nodeData.find(item)->second;
            manager.collide(data.collisionObj.get(), static_cast<void*>(this), updateCallback);
        }
        outdatedInGraph.clear();
    }
    const ConnectionGraph& Engine::getGraph() const {
        return graph;
    }
    void Engine::update() {
        updateCollisionData();
        updateGraph();
    }
    bool Engine::fclCallback(fcl::CollisionObjectf* o0, fcl::CollisionObjectf* o1) {
        if (o0 == o1) {
            return false;
        }
        const auto ldrNode0 = std::dynamic_pointer_cast<etree::LdrNode>(convertRawNodePtr(o0->getUserData()));
        const auto ldrNode1 = std::dynamic_pointer_cast<etree::LdrNode>(convertRawNodePtr(o1->getUserData()));
        const auto& connectorsA = getConnectorsOfNode(ldrNode0);
        const auto& connectorsB = getConnectorsOfNode(ldrNode1);

        spdlog::debug("{} <--> {}", ldrNode0->ldrFile->metaInfo.title, ldrNode1->ldrFile->metaInfo.title);

        for (const auto& ca: *connectorsA) {
            const PairCheckData aData(ldrNode0, ca);
            for (const auto& cb: *connectorsB) {
                const PairCheckData bData(ldrNode1, cb);
                PairChecker checker(aData, bData, graph);
                checker.findConnections();
            }
        }
        return false;
    }
    const std::shared_ptr<etree::Node>& Engine::convertRawNodePtr(void* rawPtr) const {
        std::shared_ptr<etree::Node> key0(static_cast<etree::Node*>(rawPtr), [](auto*) {});
        return nodeData.find(key0)->first;
    }
    fcl::Transform3f Engine::getCollisionBoxTransform(const std::shared_ptr<etree::LdrNode>& node) const {
        const auto aabb = editor.getScene()->getMeshCollection().getAbsoluteAABB(node);
        const auto transformation = aabb.getUnitBoxTransformation();
        return fcl::Transform3f(glm2eigen(transformation));
    }
}
