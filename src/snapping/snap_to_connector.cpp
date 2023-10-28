#include "snap_to_connector.h"
#include "../connection/connector_data_provider.h"
#include "../controller.h"
#include "../helpers/geometry.h"

namespace bricksim::snap {

    void SnapToConnectorProcess::updateCursorPos(const glm::vec2& currentCursorPos) {
        if (glm::distance2(currentCursorPos, lastCursorPos) < 1) {
            return;
        }

        bestResults.resize(0);
        const auto ray = editor->getScene()->screenCoordinatesToWorldRay(currentCursorPos + cursorOffset);
        auto nodesNearRay = editor->getConnectionEngine().getNodesNearRay(ray, 50000, subjectRadius);
        if (!nodesNearRay.empty()) {
            editor->getConnectionEngine().update(editor->getEditingModel());

            util::compare_pair_first<float, std::shared_ptr<etree::MeshNode>> distanceCompare;
            std::sort(nodesNearRay.begin(), nodesNearRay.end(), distanceCompare);
            const float maxDist2 = std::pow(std::sqrt(nodesNearRay[0].first) + subjectRadius * 2, 2);
            nodesNearRay.erase(std::upper_bound(nodesNearRay.cbegin(), nodesNearRay.cend(), std::pair<float, std::shared_ptr<etree::MeshNode>>(maxDist2, nullptr), distanceCompare), nodesNearRay.end());
            nodesNearRay.erase(std::remove_if(nodesNearRay.begin(),
                                              nodesNearRay.end(),
                                              [this](const auto& pair) {
                                                  return pair.second == subjectNode;
                                              }),
                               nodesNearRay.end());
            connection::connector_container_t allConnectors;
            const auto& connectionGraph = editor->getConnectionEngine().getConnections();
            for (const auto& [dist2, node]: nodesNearRay) {
                const auto allNodeConnectors = connection::getConnectorsOfNode(node);
                std::set<std::shared_ptr<connection::Connector>> nodeConnectors = {allNodeConnectors->begin(), allNodeConnectors->end()};
                for (const auto& [otherNode, connections]: connectionGraph.getConnections(node)) {
                    for (const auto& item: connections) {
                        nodeConnectors.erase(item->connectorA);
                        nodeConnectors.erase(item->connectorB);
                    }
                }
                for (const auto& conn: nodeConnectors) {
                    auto transf = conn->transform(glm::transpose(node->getAbsoluteTransformation()));
                    allConnectors.push_back(transf);
                }
            }
            connection::removeConnected(allConnectors);

            std::vector<glm::mat4> candidateTransformations;
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
                        if (angle < 46.f) {//todo configurable
                            rotation = glm::rotation(subjConn->direction, fixedConn->direction);
                            sameDir = true;
                        } else if (angle > 134.f) {
                            rotation = glm::rotation(subjConn->direction, -fixedConn->direction);
                            sameDir = false;
                        } else {
                            continue;
                        }
                        const auto rotationMat = glm::toMat4(rotation);
                        const glm::vec3 subjStartRotated = glm::vec4(subjCyl->start, 1.f);
                        const auto baseTransformation = glm::translate(rotationMat, fixedConn->start - subjStartRotated);
                        for (const auto& offset: getPossibleCylTranslations(otherCyl, subjCyl, sameDir)) {
                            const auto transf = glm::translate(baseTransformation, offset * fixedConn->direction);
                            candidateTransformations.push_back(transf);
                        }
                    }
                }
            }
            constexpr std::size_t resultLimit = 10;
            auto resultCompare = util::compare_pair_first<float, glm::mat4, std::greater<float>>();
            for (const auto& candTransf: candidateTransformations) {
                const auto candCenter = candTransf * glm::vec4(0, 0, 0, 1);
                glm::vec2 candScreenCoords = editor->getScene()->worldToScreenCoordinates(candCenter);
                const auto score = -glm::distance2(candScreenCoords, currentCursorPos + cursorOffset);
                const std::pair<float, glm::mat4> candResult(score, candTransf);
                const auto it = std::lower_bound(bestResults.begin(), bestResults.end(), candResult, resultCompare);
                if (it != bestResults.end()) {
                    bestResults.insert(it, candResult);
                    if (bestResults.size() > resultLimit) {
                        bestResults.resize(resultLimit);
                    }
                } else if (bestResults.size() < resultLimit) {
                    bestResults.push_back(candResult);
                }
            }
        }
    }
    SnapToConnectorProcess::SnapToConnectorProcess(const std::shared_ptr<etree::LdrNode>& subjectNode,
                                                   const std::shared_ptr<Editor>& editor,
                                                   const glm::vec2& initialCursorPos) :
        subjectNode(subjectNode),
        editor(editor),
        initialRelativeTransformation(glm::transpose(subjectNode->getRelativeTransformation())) {
        const auto absTransf = glm::transpose(subjectNode->getAbsoluteTransformation());
        this->initialAbsoluteCenter = absTransf * glm::vec4(0, 0, 0, 1);
        const auto nodeCenterOnScreen = editor->getScene()->worldToScreenCoordinates(initialAbsoluteCenter);
        this->cursorOffset = glm::vec2(nodeCenterOnScreen.x, nodeCenterOnScreen.y) - initialCursorPos;

        subjectConnectors = std::make_shared<connection::connector_container_t>();
        for (const auto& conn: *connection::getConnectorsOfNode(subjectNode)) {
            subjectConnectors->push_back(conn->transform(glm::transpose(subjectNode->parent.lock()->getAbsoluteTransformation())));
        }

        const auto meshKey = mesh::SceneMeshCollection::getMeshKey(subjectNode, false, nullptr);
        const auto mesh = editor->getScene()->getMeshCollection().getMesh(meshKey, subjectNode, nullptr);
        this->subjectRadius = mesh->getOuterDimensions().minEnclosingBallRadius;
    }
    void SnapToConnectorProcess::cancel() {
        subjectNode->setRelativeTransformation(glm::transpose(initialRelativeTransformation));
    }

    std::vector<float> SnapToConnectorProcess::getPossibleCylTranslations(const std::shared_ptr<connection::CylindricalConnector> fixed, const std::shared_ptr<connection::CylindricalConnector> moving, bool sameDir) {
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
        const auto linearSnap = controller::getSnapHandler().getLinear().getCurrentPreset().stepXZ;
        const auto minOverlap = linearSnap;//todo maybe make configurable separately

        std::vector<float> result;
        float currentOffset = fixed->openStart ? minOverlap - moving->totalLength : 0;
        auto maxOffset = fixed->totalLength - std::max(static_cast<float>(minOverlap), fixed->openEnd ? 0 : moving->totalLength);
        while (currentOffset < maxOffset) {
            float overlapMin = std::max(0.f, currentOffset) + .5;
            float overlapMax = std::min(fixed->totalLength, currentOffset + moving->totalLength) - .5;
            float i = overlapMin;
            int fixedPartIdx = 0;
            int movingPartIdx = 0;
            float fixedNextPartBoundary = fixed->parts[fixedPartIdx].length;
            float movingNextPartBoundary = movingParts[movingPartIdx].length;
            bool possible = true;
            bool contact = false;
            while (i < overlapMax) {
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
                contact |= femaleRadius - maleRadius < connection::CONNECTION_RADIUS_TOLERANCE;
                i = std::min(fixedNextPartBoundary, movingNextPartBoundary) + .5f;
            }
            if (possible && contact) {
                result.push_back(currentOffset + startOffset);
            }

            currentOffset += linearSnap;
        }

        return result;
    }
}
