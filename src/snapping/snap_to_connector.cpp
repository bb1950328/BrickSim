#include "snap_to_connector.h"
#include "../connection/connector_data_provider.h"
#include "../controller.h"
#include "../graphics/overlay2d/regular_polygon_element.h"
#include "../helpers/almost_comparations.h"
#include "../helpers/debug_nodes.h"
#include "../helpers/geometry.h"
#include "Seb.h"
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

namespace bricksim::snap {
    void SnapToConnectorProcess::updateCursorPos(const glm::vec2& currentCursorPos) {
        if (glm::distance2(currentCursorPos, lastCursorPos) < 1) {
            return;
        }

        auto& connectionEngine = editor->getConnectionEngine();
        connectionEngine.update(editor->getEditingModel());
        bestResults.resize(0);

        const auto ray = editor->getScene()->screenCoordinatesToWorldRay(currentCursorPos + cursorOffset) * constants::OPENGL_TO_LDU;

        const auto rootNode = editor->getScene()->getRootNode();

        auto nodesNearRay = connectionEngine.getNodesNearRay(ray, 50000, subjectRadius);
        if (!nodesNearRay.empty()) {
            spdlog::stopwatch sw;

            util::compare_pair_first<float, std::shared_ptr<etree::MeshNode>> distanceCompare;
            std::sort(nodesNearRay.begin(), nodesNearRay.end(), distanceCompare);
            const float maxDist2 = std::pow(std::sqrt(nodesNearRay[0].first) + subjectRadius * 2, 2);
            nodesNearRay.erase(std::upper_bound(nodesNearRay.cbegin(), nodesNearRay.cend(), std::pair<float, std::shared_ptr<etree::MeshNode>>(maxDist2, nullptr), distanceCompare), nodesNearRay.end());
            nodesNearRay.erase(std::remove_if(nodesNearRay.begin(),
                                              nodesNearRay.end(),
                                              [this](const auto& pair) {
                                                  return std::find(subjectNodes.cbegin(), subjectNodes.cend(), pair.second) != subjectNodes.cend();
                                              }),
                               nodesNearRay.end());
            connection::connector_container_t allConnectors;
            const auto& connectionGraph = connectionEngine.getConnections();
            for (const auto& [dist2, node]: nodesNearRay) {
                const auto allNodeConnectors = connection::getConnectorsOfNode(node);
                std::set<std::shared_ptr<connection::Connector>> nodeConnectors = {allNodeConnectors->begin(), allNodeConnectors->end()};
                for (const auto& [otherNode, connections]: connectionGraph.getConnections(node)) {
                    if (std::find(subjectNodes.cbegin(), subjectNodes.cend(), otherNode) == subjectNodes.end()) {
                        for (const auto& item: connections) {
                            nodeConnectors.erase(item->connectorA);
                            nodeConnectors.erase(item->connectorB);
                        }
                    }
                }
                for (const auto& conn: nodeConnectors) {
                    auto transf = conn->transform(glm::transpose(node->getAbsoluteTransformation()));
                    allConnectors.push_back(transf);
                }
            }
            connection::removeConnected(allConnectors);

            constexpr std::size_t resultLimit = 10;
            auto resultCompare = util::compare_pair_first<float, glm::mat4, std::greater<float>>();
            for (const auto& fixedConn: allConnectors) {
                for (const auto& subjConn: *subjectConnectors) {
                    if (fixedConn->type == connection::Connector::Type::CYLINDRICAL && subjConn->type == connection::Connector::Type::CYLINDRICAL) {
                        const auto otherCyl = std::dynamic_pointer_cast<connection::CylindricalConnector>(fixedConn);
                        const auto subjCyl = std::dynamic_pointer_cast<connection::CylindricalConnector>(subjConn);
                        if (otherCyl->gender == subjCyl->gender) {
                            continue;
                        }
                        const auto angle = geometry::getAngleBetweenTwoVectors(fixedConn->direction, subjConn->direction);
                        glm::quat rotation;
                        bool sameDir;
                        if (angle < glm::radians(46.f)) {
                            //todo configurable
                            rotation = glm::rotation(subjConn->direction, fixedConn->direction);
                            sameDir = true;
                        } else if (angle > glm::radians(134.f)) {
                            rotation = glm::rotation(subjConn->direction, -fixedConn->direction);
                            sameDir = false;
                        } else {
                            continue;
                        }
                        const auto preTransform = glm::toMat4(rotation) * userTransformation;
                        const glm::vec3 subjStartPreTransformed = preTransform * glm::vec4(subjCyl->start, 1.f);
                        const auto baseTranslation = fixedConn->start - subjStartPreTransformed;
                        for (const auto& offset: getPossibleCylTranslations(otherCyl, subjCyl, sameDir)) {
                            const auto offsetTranslation = offset * fixedConn->direction;
                            const auto resultTranslation = baseTranslation + offsetTranslation;
                            const auto transf = glm::translate(preTransform, resultTranslation);
                            const auto candCenter = initialRelativeTransformations[0] * transf * glm::vec4(0, 0, 0, 1);
                            const auto cr = glm::cross(glm::normalize(ray.direction), glm::vec3(candCenter) - ray.origin);
                            const auto score = -glm::length2(cr);

                            bestResults.push_back({score, transf});
                        }
                    }
                }
            }
            std::sort(bestResults.begin(), bestResults.end(), resultCompare);
            spdlog::debug("SnapToConnectorProcess: {} results in {}s", bestResults.size(), sw);
            if (bestResults.size() > resultLimit) {
                bestResults.resize(resultLimit);
            }
        }

        lastCursorPos = currentCursorPos;
    }

    SnapToConnectorProcess::SnapToConnectorProcess(const std::vector<std::shared_ptr<etree::Node>>& subjectNodes,
                                                   const std::shared_ptr<Editor>& editor,
                                                   const glm::vec2& initialCursorPos) :
        subjectNodes(subjectNodes),
        editor(editor),
        userTransformation(1.f),
        initialCursorPos(initialCursorPos) {
        initialRelativeTransformations.reserve(subjectNodes.size());
        glm::vec4 centerSum(0, 0, 0, 0);
        subjectConnectors = std::make_shared<connection::connector_container_t>();
        std::vector<glm::vec3> nodeAabbPoints;
        for (const auto& subj: subjectNodes) {
            const auto relTransf = glm::transpose(subj->getRelativeTransformation());
            const auto absTransf = glm::transpose(subj->getAbsoluteTransformation());
            initialRelativeTransformations.push_back(relTransf);
            centerSum += absTransf * glm::vec4(0, 0, 0, 1);
            const auto meshSubj = std::dynamic_pointer_cast<etree::MeshNode>(subj);
            if (meshSubj != nullptr) {
                for (const auto& conn: *connection::getConnectorsOfNode(meshSubj)) {
                    subjectConnectors->push_back(conn->transform(absTransf));
                }
            }
            const auto meshKey = mesh::SceneMeshCollection::getMeshKey(meshSubj, false, nullptr);
            const auto mesh = editor->getScene()->getMeshCollection().getMesh(meshKey, meshSubj, nullptr);
            const auto transformedAABB = mesh->getOuterDimensions().aabb.transform(absTransf);
            for (int ix = 0; ix < 2; ++ix) {
                for (int iy = 0; iy < 2; ++iy) {
                    for (int iz = 0; iz < 2; ++iz) {
                        nodeAabbPoints.emplace_back(ix == 0 ? transformedAABB.pMin.x : transformedAABB.pMax.x,
                                                    iy == 0 ? transformedAABB.pMin.y : transformedAABB.pMax.y,
                                                    iy == 0 ? transformedAABB.pMin.y : transformedAABB.pMax.y);
                    }
                }
            }
        }
        this->initialAbsoluteCenter = centerSum / static_cast<float>(subjectNodes.size());
        Seb::Smallest_enclosing_ball<float, glm::vec3> seb(3, nodeAabbPoints);
        this->subjectRadius = seb.radius();

        const auto nodeCenterOnScreen = editor->getScene()->worldToScreenCoordinates(initialAbsoluteCenter);
        this->cursorOffset = {0, 0};//todo glm::vec2(nodeCenterOnScreen.x, nodeCenterOnScreen.y) - initialCursorPos;
    }

    void SnapToConnectorProcess::applyInitialTransformations() {
        for (int i = 0; i < subjectNodes.size(); ++i) {
            setRelativeTransformationIfDifferent(i, initialRelativeTransformations[i]);
        }
    }

    void SnapToConnectorProcess::setRelativeTransformationIfDifferent(int subjectNodeIndex, const glm::mat4& newRelTransf) {
        const auto newRelTransfTransposed = glm::transpose(newRelTransf);
        if (!almostEqual(newRelTransfTransposed, subjectNodes[subjectNodeIndex]->getRelativeTransformation())) {
            subjectNodes[subjectNodeIndex]->setRelativeTransformation(newRelTransfTransposed);
            subjectNodes[subjectNodeIndex]->incrementVersion();
        }
    }

    void SnapToConnectorProcess::applyResultTransformation(const std::size_t index) {
        const auto result = bestResults[index].second;
        for (int i = 0; i < subjectNodes.size(); ++i) {
            const auto newRelTransf = initialRelativeTransformations[i] * userTransformation * result;//todo check if multiplication order is correct
            //const auto newRelTransf = result * userTransformation * initialRelativeTransformations[i];
            setRelativeTransformationIfDifferent(i, newRelTransf);
        }
    }

    std::vector<float> SnapToConnectorProcess::getPossibleCylTranslations(const std::shared_ptr<connection::CylindricalConnector>& fixed,
                                                                          const std::shared_ptr<connection::CylindricalConnector>& moving,
                                                                          const bool sameDir) {
        float startOffset;
        bool movingOpenStart = moving->openStart;
        bool movingOpenEnd = moving->openEnd;
        std::vector<connection::CylindricalShapePart> movingParts = moving->parts;
        if (!sameDir) {
            startOffset = fixed->totalLength;
            std::reverse(movingParts.begin(), movingParts.end());
            std::swap(movingOpenStart, movingOpenEnd);
        } else {
            startOffset = 0;
        }
        std::vector<float> result;
        if ((!movingOpenStart && !fixed->openStart) || (!movingOpenEnd && !fixed->openEnd)) {
            return result;
        }

        result.push_back(0);

        const auto linearSnap = controller::getSnapHandler().getLinear().getCurrentPreset().stepXZ;
        constexpr auto minOverlap = 1;//todo maybe make configurable separately
        float currentOffset = fixed->openStart ? minOverlap - moving->totalLength : 0;
        const float maxOffset = fixed->totalLength - std::max(static_cast<float>(minOverlap), fixed->openEnd ? 0 : moving->totalLength);
        while (currentOffset < maxOffset) {
            const float overlapMin = std::max(0.f, currentOffset) + .5;
            const float overlapMax = std::min(fixed->totalLength, currentOffset + moving->totalLength) - .5;
            float i = overlapMin;
            int fixedPartIdx = 0;
            int movingPartIdx = 0;
            float fixedNextPartBoundary = fixed->parts[fixedPartIdx].length;
            float movingNextPartBoundary = movingParts[movingPartIdx].length;
            bool possible = true;
            bool contact = false;
            while (i <= overlapMax) {
                while (fixedNextPartBoundary < i) {
                    ++fixedPartIdx;
                    fixedNextPartBoundary += fixed->parts[fixedPartIdx].length;
                }
                while (movingNextPartBoundary < i) {
                    ++movingPartIdx;
                    movingNextPartBoundary += movingParts[movingPartIdx].length;
                }
                const auto& fixedPart = fixed->parts[fixedPartIdx];
                const auto& movingPart = movingParts[movingPartIdx];
                const float maleRadius = fixed->gender == connection::Gender::M ? fixedPart.radius : movingPart.radius;
                const float femaleRadius = fixed->gender == connection::Gender::M ? movingPart.radius : fixedPart.radius;
                if (maleRadius - femaleRadius > connection::CONNECTION_RADIUS_TOLERANCE) {
                    possible = false;
                    break;
                }
                contact |= almostEqual(femaleRadius, maleRadius, connection::CONNECTION_RADIUS_TOLERANCE);
                i = std::min(fixedNextPartBoundary, movingNextPartBoundary) + .5f;
            }
            if (possible && contact) {
                result.push_back(currentOffset + startOffset);
            }

            if (currentOffset < 0 && currentOffset + linearSnap > 0) {
                currentOffset = 0;
            } else {
                currentOffset += linearSnap;
            }
        }

        return result;
    }

    std::size_t SnapToConnectorProcess::getResultCount() const {
        return bestResults.size();
    }

    void SnapToConnectorProcess::setUserTransformation(const glm::mat4& value) {
        userTransformation = value;
    }
}
