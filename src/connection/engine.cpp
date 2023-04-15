#include "engine.h"
#include "../helpers/geometry.h"
#include "connector_data_provider.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

namespace bricksim::connection::engine {
    namespace {
        std::vector<std::shared_ptr<etree::LdrNode>> getAllLdrNodesFlat(const std::shared_ptr<etree::Node>& node) {
            std::vector<std::shared_ptr<etree::LdrNode>> flat;
            std::function<void(const std::shared_ptr<etree::Node>&)> traverse = [&flat, &traverse](const std::shared_ptr<etree::Node>& node) {
                const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(node);
                if (ldrNode != nullptr) {
                    flat.push_back(ldrNode);
                }
                for (const auto& item: ldrNode->getChildren()) {
                    traverse(item);
                }
            };
            traverse(node);
            return flat;
        }
    }

    std::vector<std::shared_ptr<Connection>> findConnections(const std::shared_ptr<etree::LdrNode>& a, const std::shared_ptr<etree::LdrNode>& b) {
        std::vector<std::shared_ptr<Connection>> result;

        const auto& connectorsA = getConnectorsOfPart(a->ldrFile->metaInfo.name);
        const auto& connectorsB = getConnectorsOfPart(b->ldrFile->metaInfo.name);
        for (const auto& ca: connectorsA) {
            const glm::mat4 aAbsTransf = glm::transpose(a->getAbsoluteTransformation());
            const glm::vec3 caAbsoluteStart = aAbsTransf * glm::vec4(ca->start, 1.f);
            const glm::vec3 caAbsoluteDirection = aAbsTransf * glm::vec4(ca->direction, 0.f);
            const auto caClip = std::dynamic_pointer_cast<ClipConnector>(ca);
            const auto caCyl = std::dynamic_pointer_cast<CylindricalConnector>(ca);//todo maybe only call dynamic_pointer_cast if previous results are nullptr
            const auto caFinger = std::dynamic_pointer_cast<FingerConnector>(ca);
            const auto caGeneric = std::dynamic_pointer_cast<GenericConnector>(ca);
            for (const auto& cb: connectorsB) {
                const glm::mat4 bAbsTransf = glm::transpose(b->getAbsoluteTransformation());
                const glm::vec3 cbAbsoluteStart = bAbsTransf * glm::vec4(cb->start, 1.f);
                const glm::vec3 cbAbsoluteDirection = bAbsTransf * glm::vec4(cb->direction, 0.f);
                const auto cbClip = std::dynamic_pointer_cast<ClipConnector>(cb);
                const auto cbCyl = std::dynamic_pointer_cast<CylindricalConnector>(cb);
                const auto cbFinger = std::dynamic_pointer_cast<FingerConnector>(cb);
                const auto cbGeneric = std::dynamic_pointer_cast<GenericConnector>(cb);

                const float absoluteDirectionAngleDifference = geometry::getAngleBetweenTwoVectors(caAbsoluteDirection, cbAbsoluteDirection);

                if (caGeneric != nullptr || cbGeneric != nullptr) {
                    if (caGeneric != nullptr
                        && cbGeneric != nullptr
                        && caGeneric->group == cbGeneric->group
                        && caGeneric->gender != cbGeneric->gender
                        //&& caGeneric->bounding == cbGeneric->bounding
                        && glm::length2(caAbsoluteStart - cbAbsoluteStart) < std::pow(POSITION_TOLERANCE_LDU, 2)) {
                        //todo maybe check bounding shapes, but documentation is unclear
                        // is it a connection when the two bounding shapes touch
                        // or is it only a connection when the shapes have the same dimensions and the orientation matches too?
                        result.push_back(std::make_shared<Connection>(ca, cb));
                        continue;
                    }
                } else {
                    if (caCyl != nullptr) {
                        const glm::vec3 caAbsoluteEnd = aAbsTransf * glm::vec4(caCyl->getEnd(), 1.f);
                        if (cbCyl != nullptr) {
                            const glm::vec3 cbAbsoluteEnd = bAbsTransf * glm::vec4(cbCyl->getEnd(), 1.f);
                            if (caCyl->gender != cbCyl->gender) {
                                const bool sameDir = absoluteDirectionAngleDifference < PARALLELITY_ANGLE_TOLERANCE;
                                const bool oppositeDir = absoluteDirectionAngleDifference > M_PI - PARALLELITY_ANGLE_TOLERANCE;
                                if (sameDir || oppositeDir) {
                                    const geometry::NormalProjectionResult<3> projOnA = geometry::normalProjectionOnLine<3>(caAbsoluteStart, caAbsoluteEnd, cbAbsoluteStart, false);
                                    if ((sameDir && projOnA.projectionLength > caCyl->getTotalLength())                                 // Aaaaaaa   Bbbbbbbbb
                                        || (sameDir && projOnA.projectionLength < -cbCyl->getTotalLength())                             // Bbbbbb  Aaaaaa
                                        || (oppositeDir && projOnA.projectionLength > caCyl->getTotalLength() + cbCyl->getTotalLength())// Aaaaaaa  bbbbbbbbB
                                        || (oppositeDir && projOnA.projectionLength < 0)) {                                             // bbbbbbB   Aaaaaaaa
                                        //the directions are colinear, but the parts are too far away from each other
                                        continue;
                                    }
                                    if (projOnA.distancePointToLine < COLINEARITY_TOLERANCE_LDU) {
                                        const int maleIdx = caCyl->gender == Gender::M ? 0 : 1;
                                        const int femaleIdx = 1 - maleIdx;

                                        std::vector<float> aBoundaries;
                                        aBoundaries.reserve(caCyl->parts.size() + 1);
                                        aBoundaries.push_back(0);
                                        float offset = 0;
                                        for (const auto& item: caCyl->parts) {
                                            offset += item.length;
                                            aBoundaries.push_back(offset);
                                        }

                                        std::vector<float> bBoundaries;
                                        bBoundaries.reserve(cbCyl->parts.size() + 1);
                                        std::vector<CylindricalShapePart> bParts;
                                        bParts.reserve(cbCyl->parts.size());
                                        if (sameDir) {
                                            offset = projOnA.projectionLength;
                                            bBoundaries.push_back(offset);
                                            for (const auto& item: cbCyl->parts) {
                                                offset += item.length;
                                                bBoundaries.push_back(offset);
                                                bParts.push_back(item);
                                            }
                                        } else {
                                            offset = projOnA.projectionLength - cbCyl->getTotalLength();
                                            bBoundaries.push_back(offset);
                                            for (auto it = cbCyl->parts.rbegin(); it < cbCyl->parts.rend(); ++it) {
                                                offset += it->length;
                                                bBoundaries.push_back(offset);
                                                bParts.push_back(*it);
                                            }
                                        }

                                        //float lengthCursor = std::min(aBoundaries[0], bBoundaries[0]);
                                        //const float maxLengthCursorValue = std::max(aBoundaries.back(), bBoundaries.back());
                                        int aCursor = -1;
                                        int bCursor = -1;
                                        if (aBoundaries.front() < bBoundaries.front()) {
                                            aCursor = 0;
                                        } else {
                                            bCursor = 0;
                                        }
                                        bool radialCollision = false;
                                        bool rotationPossible = true;
                                        bool contact = false;
                                        bool slidePossible = caCyl->slide || cbCyl->slide;
                                        while (aCursor < static_cast<int>(caCyl->parts.size()) && bCursor < static_cast<int>(bParts.size())) {
                                            if (aCursor >= 0 && bCursor >= 0) {
                                                const auto& pa = caCyl->parts[aCursor];
                                                const auto& pb = bParts[bCursor];
                                                const float radii[2] = {pa.radius, pb.radius};
                                                const float maleRadius = radii[maleIdx];
                                                const float femaleRadius = radii[femaleIdx];
                                                if (maleRadius - femaleRadius > CYLINDRICAL_CONNECTION_RADIUS_TOLERANCE) {
                                                    radialCollision = true;
                                                    break;
                                                }
                                                contact |= femaleRadius - maleRadius < CYLINDRICAL_CONNECTION_RADIUS_TOLERANCE;
                                                if (pa.type != CylindricalShapeType::ROUND && pa.type == pb.type) {
                                                    rotationPossible = false;
                                                }
                                                if ((aCursor >= aBoundaries.size() - 2 && aBoundaries[aCursor + 1] < bBoundaries[bCursor])
                                                    || (bCursor >= bBoundaries.size() - 2 && bBoundaries[bCursor + 1] < aBoundaries[aCursor])) {
                                                    //no more common parts
                                                    break;
                                                }
                                            }
                                            if (aBoundaries[aCursor + 1] < bBoundaries[bCursor + 1]) {
                                                ++aCursor;
                                            } else {
                                                ++bCursor;
                                            }
                                        }
                                        if (!radialCollision && contact) {
                                            std::vector<glm::vec3> slideDirections;
                                            std::vector<glm::vec3> rotationAxes;
                                            if (slidePossible) {
                                                slideDirections.push_back(ca->direction);
                                            }
                                            if (rotationPossible) {
                                                rotationAxes.push_back(ca->direction);
                                            }
                                            result.push_back(std::make_shared<Connection>(ca, cb, DegreesOfFreedom(slideDirections, rotationAxes)));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return result;
    }

    ConnectionGraph findConnections(const std::shared_ptr<etree::Node>& node, const mesh::SceneMeshCollection& meshCollection) {
        std::vector<std::shared_ptr<etree::LdrNode>> flat = getAllLdrNodesFlat(node);
        ConnectionGraph result = ConnectionGraph();

        // TODO replace this O(n^2) garbage with an AABB tree
        //  also add an AABB tree on `Editor` which is constantly updated
        for (const auto& a: flat) {
            const auto aAABB = meshCollection.getAbsoluteAABB(a);
            for (const auto& b: flat) {
                const auto bAABB = meshCollection.getAbsoluteAABB(b);
                if (aAABB.intersects(bAABB)) {
                    for (const auto& conn: findConnections(a, b)) {
                        result.addConnection(a, b, conn);
                    }
                }
            }
        }

        return result;
    }

    ConnectionGraph findConnections(const std::shared_ptr<etree::LdrNode>& activeNode, const std::shared_ptr<etree::Node>& passiveNode, const mesh::SceneMeshCollection& meshCollection) {
        std::vector<std::shared_ptr<etree::LdrNode>> flat = getAllLdrNodesFlat(passiveNode);
        ConnectionGraph result = ConnectionGraph();

        const auto activeAABB = meshCollection.getAbsoluteAABB(activeNode);
        for (const auto& a: flat) {
            const auto aAABB = meshCollection.getAbsoluteAABB(a);
            if (aAABB.intersects(activeAABB)) {
                for (const auto& conn: findConnections(a, activeNode)) {
                    result.addConnection(a, activeNode, conn);
                }
            }
        }

        spdlog::debug("tested against {} other parts, found {} connections", flat.size(), result.countTotalConnections());

        return result;
    }
}
