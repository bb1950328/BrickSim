#include "engine.h"

#include "../editor.h"
#include "../helpers/custom_hash.h"
#include "../helpers/glm_eigen_conversion.h"
#include "connector_data_provider.h"
#include "pair_checker.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include "spdlog/stopwatch.h"
#include <fcl/broadphase/broadphase_dynamic_AABB_tree.h>
#include <glm/gtx/string_cast.hpp>
#include <utility>

namespace bricksim::connection {
    Engine::Engine(Editor& editor) :
        editor(editor) {
    }
    void Engine::updateCollisionData(float* progress, float progressMultiplicator) {
        const auto& rootNode = editor.getEditingModel();
        const auto it = nodeData.find(rootNode);
        *progress = 0.f;
        if (it == nodeData.end()) {
            resetData();
            updateNodeData(rootNode);
        } else {
            updateNodeData(rootNode, it);
        }
        *progress = progressMultiplicator;
    }

    void Engine::updateNodeData(const std::shared_ptr<etree::Node>& node) {
        auto it = nodeData.find(node);
        if (it != nodeData.end()) {
            updateNodeData(node, it);
        } else {
            std::unique_ptr<fcl::CollisionObjectf> collisionObject;
            std::shared_ptr<etree::LdrNode> ldrNode;
            if constexpr (partNodeCollsionOnly) {
                ldrNode = std::dynamic_pointer_cast<etree::PartNode>(node);
            } else {
                ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(node);
            }
            if (ldrNode != nullptr) {
                const auto aabb = editor.getScene()->getMeshCollection().getAbsoluteAABB(ldrNode);
                const auto box = std::make_shared<fcl::Boxf>(glm2eigen(aabb.getSize()));
                collisionObject = std::make_unique<fcl::CollisionObjectf>(box, fcl::Matrix3f::Identity(), glm2eigen(aabb.getCenter()));
                collisionObject->setUserData(node.get());
                manager.registerObject(collisionObject.get());
                outdatedInGraph.insert(ldrNode);
            } else {
                const auto miNode = std::dynamic_pointer_cast<etree::ModelInstanceNode>(node);
                if (miNode != nullptr) {
                } else {
                    collisionObject = nullptr;
                }
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
        if (o0 != o1) {
            auto& set = *static_cast<uoset_t<std::array<fcl::CollisionObjectf*, 2>>*>(cdata);
            set.insert(std::to_array({
                    std::min(o0, o1),//because [a,b] is the same as [b,a] duplicates must be filtered
                    std::max(o0, o1),
            }));
        }
        return false;
    }

    void Engine::updateGraph(float* progress, float progressStart) {
        graph.removeAllConnections(outdatedInGraph);
        uoset_t<broadphase_collision_pair_t> intersections;
        for (const auto& item: outdatedInGraph) {
            const auto& data = nodeData.find(item)->second;
            manager.collide(data.collisionObj.get(), static_cast<void*>(&intersections), updateCallback);
        }

        *progress = progressStart;
        const auto threadCount = std::thread::hardware_concurrency();
        if (intersections.size() > threadCount * 100) {
            std::mutex lock;
            std::size_t nextStart = 0;
            std::size_t intersectionsCount = intersections.size();

            spdlog::debug("Using {} threads to handle {} detected part intersections", threadCount, intersections.size());

            const auto getNewWorkUnit = [&lock, &nextStart, &progress, progressStart, intersectionsCount]() -> std::pair<std::size_t, std::size_t> {
                std::lock_guard<std::mutex> lg(lock);
                *progress = progressStart + (1.f - progressStart) * nextStart / intersectionsCount;
                const auto start = nextStart;
                nextStart = std::min(intersectionsCount, start + 100);
                return {start, nextStart};
            };

            std::vector<std::thread> threads;
            for (std::size_t i = 0; i < threadCount; ++i) {
                threads.emplace_back([this,
                                      &i,
                                      &getNewWorkUnit,
                                      &intersections]() {
                    std::string threadName = fmt::format("Narrowphase collision checker #{}", i);
                    util::setThreadName(threadName.c_str());
                    while (true) {
                        const auto [iStart, iEnd] = getNewWorkUnit();
                        if (iStart == iEnd) {
                            break;
                        }
                        auto it = intersections.begin() + iStart;
                        const auto end = intersections.begin() + iEnd;
                        while (it < end) {
                            handleBroadphaseCollision(*it);
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
            for (auto it = intersections.cbegin(); it != intersections.end(); ++i, ++it) {
                handleBroadphaseCollision(*it);
                *progress = progressStart + (1.f - progressStart) * i / intersections.size();
            }
        }
        *progress = 1.f;
        outdatedInGraph.clear();
    }
    void Engine::handleBroadphaseCollision(const broadphase_collision_pair_t& item) {
        const auto ldrNode0 = std::dynamic_pointer_cast<etree::LdrNode>(convertRawNodePtr(item[0]->getUserData()));
        const auto ldrNode1 = std::dynamic_pointer_cast<etree::LdrNode>(convertRawNodePtr(item[1]->getUserData()));
        const auto& connectorsA = getConnectorsOfNode(ldrNode0);
        const auto& connectorsB = getConnectorsOfNode(ldrNode1);

        //spdlog::debug("broadphase collision {} <--> {}", ldrNode0->ldrFile->metaInfo.title, ldrNode1->ldrFile->metaInfo.title);

        for (const auto& ca: *connectorsA) {
            const PairCheckData aData(ldrNode0, ca);
            for (const auto& cb: *connectorsB) {
                const PairCheckData bData(ldrNode1, cb);
                ConnectionGraphPairChecker checker(aData, bData, graph);
                checker.findConnections();
            }
        }
    }
    const ConnectionGraph& Engine::getGraph() const {
        return graph;
    }
    void Engine::update(float* progress) {
        spdlog::stopwatch sw;

        updateCollisionData(progress, .2f);

        const auto between = sw.elapsed();

        updateGraph(progress, .2f);

        const auto after = sw.elapsed();
        spdlog::info("Updated collision data in {}+{}={}ms",
                     std::chrono::duration_cast<std::chrono::microseconds>(between).count() / 1000.f,
                     std::chrono::duration_cast<std::chrono::microseconds>(after - between).count() / 1000.f,
                     std::chrono::duration_cast<std::chrono::microseconds>(after).count() / 1000.f);
    }
    const std::shared_ptr<etree::Node>& Engine::convertRawNodePtr(void* rawPtr) const {
        std::shared_ptr<etree::Node> key0(static_cast<etree::Node*>(rawPtr), [](auto*) {});
        return nodeData.find(key0)->first;
    }
    void Engine::update() {
        float ignoredProgress;
        update(&ignoredProgress);
    }
}
