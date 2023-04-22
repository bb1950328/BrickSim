#include "engine.h"

#include "../helpers/geometry.h"
#include "connector_data_provider.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include <utility>

namespace bricksim::connection::engine {
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

        ConnectionChecker::ConnectionChecker(ConnectorCheckData a, ConnectorCheckData b, std::vector<std::shared_ptr<Connection>>& result) :
            a(std::move(a)),
            b(std::move(b)),
            result(result),
            absoluteDirectionAngleDifference(geometry::getAngleBetweenTwoVectors(a.absDirection, b.absDirection)),
            sameDir(absoluteDirectionAngleDifference < PARALLELITY_ANGLE_TOLERANCE),
            oppositeDir(absoluteDirectionAngleDifference > M_PI - PARALLELITY_ANGLE_TOLERANCE) {
        }
        void ConnectionChecker::findConnections() {
            if (a.generic != nullptr || b.generic != nullptr) {
                if (a.generic != nullptr && b.generic != nullptr) {
                    findGenericGeneric();
                }
                return;
            }
            if (a.cyl != nullptr && b.cyl != nullptr) {
                findCylCyl();
                return;
            }
            if (a.finger != nullptr && b.finger != nullptr) {
                findFingerFinger();
                return;
            }
        }
        bool ConnectionChecker::findGenericGeneric() {
            if (a.generic->group == b.generic->group
                && a.generic->gender != b.generic->gender
                //&& a.generic->bounding == b.generic->bounding
                && glm::length2(a.absStart - b.absStart) < std::pow(POSITION_TOLERANCE_LDU, 2)) {
                //todo maybe check bounding shapes, but documentation is unclear
                // is it a connection when the two bounding shapes touch
                // or is it only a connection when the shapes have the same dimensions and the orientation matches too?
                result.push_back(std::make_shared<Connection>(a.connector, b.connector));
                return true;
            }
            return false;
        }
        bool ConnectionChecker::findCylCyl() {
            if ((!sameDir && !oppositeDir)
                || a.cyl->gender == b.cyl->gender) {
                return false;
            }
            const auto startOffset = projectConnectorsWithLength();
            if (!startOffset.has_value()) {
                return false;
            }

            const int maleIdx = a.cyl->gender == Gender::M ? 0 : 1;
            const int femaleIdx = 1 - maleIdx;

            std::vector<float> aBoundaries;
            aBoundaries.reserve(a.cyl->parts.size() + 1);
            aBoundaries.push_back(0);
            float offset = 0;
            for (const auto& item: a.cyl->parts) {
                offset += item.length;
                aBoundaries.push_back(offset);
            }

            std::vector<float> bBoundaries;
            bBoundaries.reserve(b.cyl->parts.size() + 1);
            std::vector<CylindricalShapePart> bParts;
            bParts.reserve(b.cyl->parts.size());
            if (sameDir) {
                offset = startOffset.value();
                bBoundaries.push_back(offset);
                for (const auto& item: b.cyl->parts) {
                    offset += item.length;
                    bBoundaries.push_back(offset);
                    bParts.push_back(item);
                }
            } else {
                offset = startOffset.value() - b.cyl->getTotalLength();
                bBoundaries.push_back(offset);
                for (auto it = b.cyl->parts.rbegin(); it < b.cyl->parts.rend(); ++it) {
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
            bool slidePossible = a.cyl->slide || b.cyl->slide;
            while (aCursor < static_cast<int>(a.cyl->parts.size()) && bCursor < static_cast<int>(bParts.size())) {
                if (aCursor >= 0 && bCursor >= 0) {
                    const auto& pa = a.cyl->parts[aCursor];
                    const auto& pb = bParts[bCursor];
                    const float radii[2] = {pa.radius, pb.radius};
                    const float maleRadius = radii[maleIdx];
                    const float femaleRadius = radii[femaleIdx];
                    if (maleRadius - femaleRadius > CONNECTION_RADIUS_TOLERANCE) {
                        radialCollision = true;
                        break;
                    }
                    contact |= femaleRadius - maleRadius < CONNECTION_RADIUS_TOLERANCE;
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
            if (radialCollision || !contact) {
                return false;
            }
            DegreesOfFreedom dof;
            if (slidePossible) {
                dof.slideDirections.push_back(a.absDirection);
            }
            if (rotationPossible) {
                dof.rotationPossibilities.emplace_back(a.absStart, a.absDirection);
            }
            result.push_back(std::make_shared<Connection>(a.connector, b.connector, dof));
            return true;
        }
        bool bricksim::connection::engine::ConnectionChecker::findFingerFinger() {
            if ((!sameDir && !oppositeDir)
                || a.finger->group != b.finger->group
                || std::abs(a.finger->radius - b.finger->radius) > CONNECTION_RADIUS_TOLERANCE) {
                return false;
            }
            const auto startOffsetOpt = projectConnectorsWithLength();
            if (!startOffsetOpt.has_value()) {
                return false;
            }
            int aCursorIdx = 0;
            int bCursorIdx = sameDir ? 0 : static_cast<int>(b.finger->fingerWidths.size()) - 1;
            const int bCursorStep = sameDir ? 1 : -1;
            const float aOffset = sameDir ? startOffsetOpt.value() : startOffsetOpt.value() - b.finger->getTotalLength();

            if (aOffset < -POSITION_TOLERANCE_LDU) {
                float offset = -aOffset;
                while (offset > POSITION_TOLERANCE_LDU) {
                    offset -= b.finger->fingerWidths[bCursorIdx];
                    bCursorIdx += bCursorStep;
                }
                if (offset < -POSITION_TOLERANCE_LDU) {
                    return false;
                }
            } else if (aOffset > POSITION_TOLERANCE_LDU) {
                float offset = aOffset;
                while (offset > POSITION_TOLERANCE_LDU) {
                    offset -= a.finger->fingerWidths[aCursorIdx];
                    ++aCursorIdx;
                }
                if (offset < -POSITION_TOLERANCE_LDU) {
                    return false;
                }
            }
            if ((std::abs(aCursorIdx - bCursorIdx) % 2 == 0
                 && a.finger->firstFingerGender == b.finger->firstFingerGender)
                || (std::abs(aCursorIdx - bCursorIdx) % 2 == 1
                    && a.finger->firstFingerGender != b.finger->firstFingerGender)) {
                return false;
            }
            while (aCursorIdx < static_cast<int>(a.finger->fingerWidths.size())
                   && bCursorIdx < static_cast<int>(b.finger->fingerWidths.size())) {
                if (std::abs(a.finger->fingerWidths[aCursorIdx] - b.finger->fingerWidths[bCursorIdx]) > POSITION_TOLERANCE_LDU) {
                    return false;
                }
                ++aCursorIdx;
                bCursorIdx += bCursorStep;
            }
            DegreesOfFreedom dof;
            dof.rotationPossibilities.emplace_back(a.absStart, a.absDirection);
            result.push_back(std::make_shared<Connection>(a.connector, b.connector, dof));
            return true;
        }
        std::optional<float> bricksim::connection::engine::ConnectionChecker::projectConnectorsWithLength() {
            const auto projOnA = geometry::normalProjectionOnLine<3>(a.absStart, a.absEnd, b.absStart, false);
            const float aLength = a.connectorWithLength->getTotalLength();
            const float bLength = b.connectorWithLength->getTotalLength();

            if ((sameDir && projOnA.projectionLength > aLength)                 // Aaaaaaa   Bbbbbbbbb
                || (sameDir && projOnA.projectionLength < -bLength)             // Bbbbbb  Aaaaaa
                || (oppositeDir && projOnA.projectionLength > aLength + bLength)// Aaaaaaa  bbbbbbbbB
                || (oppositeDir && projOnA.projectionLength < 0)                // bbbbbbB   Aaaaaaaa
                || projOnA.distancePointToLine >= COLINEARITY_TOLERANCE_LDU) {  //a and b aren't colinear
                return std::nullopt;
            } else {
                return {projOnA.projectionLength};
            }
        }
    }

    std::vector<std::shared_ptr<Connection>> findConnections(const std::shared_ptr<etree::LdrNode>& a, const std::shared_ptr<etree::LdrNode>& b) {
        std::vector<std::shared_ptr<Connection>> result;
        const auto& connectorsA = getConnectorsOfPart(a->ldrFile->metaInfo.name);
        const auto& connectorsB = getConnectorsOfPart(b->ldrFile->metaInfo.name);

        for (const auto& ca: connectorsA) {
            const ConnectorCheckData aData(a, ca);
            for (const auto& cb: connectorsB) {
                const ConnectorCheckData bData(b, cb);
                ConnectionChecker checker(aData, bData, result);
                checker.findConnections();
            }
        }
        return result;
    }

    ConnectionGraph findConnections(const std::shared_ptr<etree::Node>& node, const mesh::SceneMeshCollection& meshCollection) {
        std::vector<std::shared_ptr<etree::LdrNode>> flat = getAllLdrNodesFlat(node);
        ConnectionGraph result = ConnectionGraph();

        // TODO replace this O(n^2) garbage with an AABB tree
        //  also add an AABB tree on `Editor` which is constantly updated
        for (int i = 0; i < flat.size(); ++i) {
            const auto a = flat[i];
            const auto aAABB = meshCollection.getAbsoluteAABB(a);
            for (int j = 0; j < i; ++j) {
                const auto b = flat[j];
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
        ConnectionGraph result;
        findConnections(activeNode, passiveNode, meshCollection, result);
        return result;
    }

    void findConnections(const std::shared_ptr<etree::LdrNode>& activeNode, const std::shared_ptr<etree::Node>& passiveNode, const mesh::SceneMeshCollection& meshCollection, ConnectionGraph& result) {
        std::vector<std::shared_ptr<etree::LdrNode>> flat = getAllLdrNodesFlat(passiveNode);
        const auto activeAABB = meshCollection.getAbsoluteAABB(activeNode);
        for (const auto& a: flat) {
            const auto aAABB = meshCollection.getAbsoluteAABB(a);
            if (aAABB.intersects(activeAABB)) {
                for (const auto& conn: findConnections(a, activeNode)) {
                    result.addConnection(a, activeNode, conn);
                }
            }
        }
    }
    ConnectorCheckData::ConnectorCheckData(const std::shared_ptr<etree::LdrNode>& node, const std::shared_ptr<Connector>& connector) :
        absTransformation(glm::transpose(node->getAbsoluteTransformation())),
        absStart(absTransformation * glm::vec4(connector->start, 1.f)),
        absEnd(absStart),
        absDirection(absTransformation * glm::vec4(connector->direction, 0.f)),
        connector(connector) {
        clip = std::dynamic_pointer_cast<ClipConnector>(connector);
        if (clip == nullptr) {
            cyl = std::dynamic_pointer_cast<CylindricalConnector>(connector);
            if (cyl == nullptr) {
                finger = std::dynamic_pointer_cast<FingerConnector>(connector);
                if (finger == nullptr) {
                    generic = std::dynamic_pointer_cast<GenericConnector>(connector);
                }
            }
        }
        if (generic == nullptr) {
            connectorWithLength = std::dynamic_pointer_cast<ConnectorWithLength>(connector);
            absEnd = absTransformation * glm::vec4(connectorWithLength->getEnd(), 1.f);
        }
    }
}
