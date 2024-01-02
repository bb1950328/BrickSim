#include "engine.h"

#include "../editor/editor.h"
#include "../helpers/custom_hash.h"
#include "../helpers/glm_eigen_conversion.h"
#include "connection_check.h"
#include "pair_checker.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include "spdlog/stopwatch.h"
#include <fcl/broadphase/broadphase_dynamic_AABB_tree.h>
#include <glm/gtx/string_cast.hpp>
#include <utility>

#include "../helpers/geometry.h"

namespace bricksim::connection {
    Engine::Engine() = default;

    void Engine::updateCollisionData(const std::shared_ptr<etree::Node>& rootNode, float* progress, float progressMultiplicator) {
        *progress = 0.f;
        updateNodeData(rootNode);//todo update progress

        *progress = progressMultiplicator;
    }

    void Engine::updateNodeData(const std::shared_ptr<etree::Node>& node) {
        auto it = nodeData.find(node);
        if (it != nodeData.end()) {
            updateNodeData(node, it);
        } else {
            std::unique_ptr<fcl::CollisionObjectf> collisionObject;
            std::shared_ptr<etree::MeshNode> meshNode;
            if constexpr (partNodeCollsionOnly) {
                meshNode = std::dynamic_pointer_cast<etree::PartNode>(node);
            } else {
                meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node);
            }
            if (meshNode != nullptr) {
                const auto aabb = scene.lock()->getMeshCollection().getAbsoluteAABB(meshNode);
                const auto box = std::make_shared<fcl::Boxf>(glm2eigen(aabb.getSize()));
                collisionObject = std::make_unique<fcl::CollisionObjectf>(box, fcl::Matrix3f::Identity(), glm2eigen(aabb.getCenter()));
                collisionObject->setUserData(node.get());
                manager.registerObject(collisionObject.get());
                outdatedInGraphs.insert(meshNode);
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
        //todo i think there has to be a separate fcl collision manager per rootNode (=ModelNode)
        auto& data = it->second;
        if (data.lastUpdatedSelfVersion != node->getSelfVersion()) {
            const auto meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node);
            if (meshNode != nullptr) {
                const auto aabb = scene.lock()->getMeshCollection().getAbsoluteAABB(meshNode);
                const auto sizeDifference = std::dynamic_pointer_cast<const fcl::Boxf>(data.collisionObj->collisionGeometry())->side - glm2eigen(aabb.getSize());
                if (sizeDifference.squaredNorm() > .01f) {
                    manager.unregisterObject(data.collisionObj.get());
                    const auto box = std::make_shared<fcl::Boxf>(glm2eigen(aabb.getSize()));
                    data.collisionObj = std::make_unique<fcl::CollisionObjectf>(box, fcl::Matrix3f::Identity(), glm2eigen(aabb.getCenter()));//todo share this with initial obj creation
                    data.collisionObj->setUserData(node.get());
                    manager.registerObject(data.collisionObj.get());
                } else {
                    data.collisionObj->setTranslation(glm2eigen(aabb.getCenter()));
                    data.collisionObj->computeAABB();
                    manager.update(data.collisionObj.get());
                }
                outdatedInGraphs.insert(meshNode);
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
                outdatedInGraphs.insert(std::dynamic_pointer_cast<etree::MeshNode>(it->first));
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
        if (o0 != o1) {
            auto& set = *static_cast<uoset_t<broadphase_collision_pair_t>*>(cdata);
            set.insert(std::to_array({
                    std::min(o0, o1),//because [a,b] is the same as [b,a] duplicates must be filtered
                    std::max(o0, o1),
            }));
        }
        return false;
    }

    void Engine::updateGraph(float* progress, float progressStart) {
        intersections.removeAllNodes(outdatedInGraphs);
        connections.removeAllConnections(outdatedInGraphs);
        uoset_t<broadphase_collision_pair_t> newIntersections;
        for (const auto& item: outdatedInGraphs) {
            const auto it = nodeData.find(item);
            if (it != nodeData.end()) {
                const auto& data = it->second;
                if (data.collisionObj != nullptr) {
                    manager.collide(data.collisionObj.get(), static_cast<void*>(&newIntersections), updateCallback);
                }
            }
        }

        *progress = progressStart;
        const auto threadCount = std::thread::hardware_concurrency();
        const auto newIntersectionsCount = newIntersections.size();
        if (newIntersectionsCount > threadCount * 100) {
            std::mutex lock;
            std::size_t nextStart = 0;

            spdlog::debug("Using {} threads to handle {} detected part intersections", threadCount, newIntersectionsCount);

            const auto getNewWorkUnit = [&lock, &nextStart, &progress, progressStart, newIntersectionsCount]() -> std::pair<std::size_t, std::size_t> {
                std::lock_guard<std::mutex> lg(lock);
                *progress = progressStart + (1.f - progressStart) * nextStart / newIntersectionsCount;
                const auto start = nextStart;
                nextStart = std::min(newIntersectionsCount, start + 100);
                return {start, nextStart};
            };

            std::vector<std::thread> threads;
            for (std::size_t i = 0; i < threadCount; ++i) {
                threads.emplace_back([this,
                                      &i,
                                      &getNewWorkUnit,
                                      &newIntersections]() {
                    std::string threadName = fmt::format("Narrowphase collision checker #{}", i);
                    util::setThreadName(threadName.c_str());
                    while (true) {
                        const auto [iStart, iEnd] = getNewWorkUnit();
                        if (iStart == iEnd) {
                            break;
                        }
                        auto it = newIntersections.begin() + iStart;
                        const auto end = newIntersections.begin() + iEnd;
                        while (it < end) {
                            handleNewIntersection(*it);
                            ++it;
                        }
                    }
                });
            }

            for (auto& t: threads) {
                t.join();
            }
        } else {
            std::size_t i = 0;
            for (auto it = newIntersections.cbegin(); it != newIntersections.end(); ++i, ++it) {
                handleNewIntersection(*it);
                *progress = progressStart + (1.f - progressStart) * i / newIntersectionsCount;
            }
        }
        *progress = 1.f;
        outdatedInGraphs.clear();
    }

    void Engine::handleNewIntersection(const broadphase_collision_pair_t& item) {
        const auto nodeA = std::dynamic_pointer_cast<etree::MeshNode>(convertRawNodePtr(item[0]->getUserData()));
        const auto nodeB = std::dynamic_pointer_cast<etree::MeshNode>(convertRawNodePtr(item[1]->getUserData()));

        //spdlog::debug("broadphase collision {} <--> {}", nodeA->displayName, nodeB->displayName);

        ConnectionGraphPairCheckResultConsumer result(nodeA, nodeB, connections);
        ConnectionCheck connCheck(result);
        connCheck.checkForConnected(nodeA, nodeB);
        intersections.addEdge(nodeA, nodeB);
    }
    const ConnectionGraph& Engine::getConnections() const {
        return connections;
    }
    void Engine::update(const std::shared_ptr<etree::Node>& rootNode, float* progress) {
        spdlog::stopwatch sw;

        updateCollisionData(rootNode, progress, .2f);

        const auto between = sw.elapsed();

        updateGraph(progress, .2f);

        const auto after = sw.elapsed();
        spdlog::info("Updated collision data in {}+{}={}ms",
                     std::chrono::duration_cast<std::chrono::microseconds>(between).count() / 1000.f,
                     std::chrono::duration_cast<std::chrono::microseconds>(after - between).count() / 1000.f,
                     std::chrono::duration_cast<std::chrono::microseconds>(after).count() / 1000.f);
    }
    const std::shared_ptr<etree::Node>& Engine::convertRawNodePtr(void* rawPtr) const {
        auto* typedPtr = static_cast<etree::Node*>(rawPtr);
        std::shared_ptr<etree::Node> key0(typedPtr, [](auto*) {});
        return nodeData.find(key0)->first;
    }
    void Engine::update(const std::shared_ptr<etree::Node>& rootNode) {
        float ignoredProgress;
        update(rootNode, &ignoredProgress);
    }
    const IntersectionGraph& Engine::getIntersections() const {
        return intersections;
    }
    void Engine::setScene(const std::weak_ptr<graphics::Scene>& scene) {
        Engine::scene = scene;
    }

    bool rayIntersectionCallback(fcl::CollisionObjectf* o0, fcl::CollisionObjectf* o1, void* cdata) {
        auto nodes = static_cast<std::vector<void*>*>(cdata);
        if (o0->getUserData() != nullptr) {
            nodes->push_back(o0->getUserData());
        } else if (o1->getUserData() != nullptr) {
            nodes->push_back(o1->getUserData());
        }
        return false;
    }
    std::vector<std::pair<float, std::shared_ptr<etree::MeshNode>>> Engine::getNodesNearRay(Ray3 ray, float distanceLimit, float radiusLimit) {
        const auto cylinder = std::make_shared<fcl::Cylinderf>(radiusLimit, distanceLimit);
        ray.normalizeDirection();
        const auto cylRotation = glm::toMat3(glm::rotation({0, 0, 1}, ray.direction));
        const auto cylCenter = ray.origin + ray.direction * (distanceLimit / 2);
        const auto collisionObj = std::make_unique<fcl::CollisionObjectf>(cylinder, glm2eigen(cylRotation), glm2eigen(cylCenter));
        collisionObj->setUserData(nullptr);

        std::vector<void*> nodes;
        manager.registerObject(collisionObj.get());
        manager.collide(collisionObj.get(), &nodes, rayIntersectionCallback);
        manager.unregisterObject(collisionObj.get());

        std::vector<std::pair<float, std::shared_ptr<etree::MeshNode>>> result;
        for (const auto& item: nodes) {
            const auto node = std::dynamic_pointer_cast<etree::MeshNode>(convertRawNodePtr(item));
            const glm::vec3 nodeCenter = glm::transpose(node->getAbsoluteTransformation()) * glm::vec4(0, 0, 0, 1);
            result.emplace_back(glm::distance2(ray.origin, nodeCenter), node);
        }
        return result;
    }
}
