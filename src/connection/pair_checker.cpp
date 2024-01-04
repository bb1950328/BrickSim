#include "pair_checker.h"
#include "../helpers/almost_comparations.h"
#include "../helpers/geometry.h"

namespace bricksim::connection {
    PairChecker::PairChecker(const PairCheckData& a, const PairCheckData& b, PairCheckResultConsumer& resultConsumer) :
        a(a),
        b(b),
        parallel(geometry::isAlmostParallel(a.absDirection, b.absDirection)),
        sameDir(parallel && glm::dot(a.absDirection, b.absDirection) > 0),
        oppositeDir(!sameDir && parallel),
        resultConsumer(resultConsumer) {}

    void PairChecker::findConnections() {
        using Type = Connector::Type;
        const auto bType = b.connector->type;
        const auto aType = a.connector->type;
        if (aType == Type::CYLINDRICAL) {
            if (bType == Type::CYLINDRICAL) {
                findCylCyl();
            } else if (bType == Type::CLIP) {
                findClipCyl(b, a);
            }
        } else if (aType == Type::CLIP && bType == Type::CYLINDRICAL) {
            findClipCyl(a, b);
        } else if (aType == Type::FINGER && bType == Type::FINGER) {
            findFingerFinger();
        } else if (aType == Type::GENERIC && bType == Type::GENERIC) {
            findGenericGeneric();
        }
    }

    void PairChecker::findGenericGeneric() {
        if (a.generic->group == b.generic->group
            && a.generic->gender != b.generic->gender
            //&& a.generic->bounding == b.generic->bounding
            && glm::length2(a.absStart - b.absStart) < std::pow(POSITION_TOLERANCE_LDU, 2)) {
            //todo maybe check bounding shapes, but documentation is unclear
            // is it a connection when the two bounding shapes touch
            // or is it only a connection when the shapes have the same dimensions and the orientation matches too?
            addConnection({}, {true, true});
        }
    }

    void PairChecker::findCylCyl() {
        if (!parallel || a.cyl->gender == b.cyl->gender) {
            return;
        }
        const auto startOffset = projectConnectorsWithLength(a.cyl->totalLength, b.cyl->totalLength);
        if (!startOffset.has_value()) {
            return;
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
            offset = startOffset.value() - b.cyl->totalLength;
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
        if (almostLess(aBoundaries.front(), bBoundaries.front(), POSITION_TOLERANCE_LDU)) {
            aCursor = 0;
        }
        if (almostGreater(aBoundaries.front(), bBoundaries.front(), POSITION_TOLERANCE_LDU)) {
            bCursor = 0;
        }
        bool completelyUsedA = almostGreater(aBoundaries.front(), bBoundaries.front(), POSITION_TOLERANCE_LDU);
        bool completelyUsedB = almostGreater(bBoundaries.front(), aBoundaries.front(), POSITION_TOLERANCE_LDU);
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
                if ((static_cast<unsigned int>(aCursor) >= aBoundaries.size() - 2 && almostLess(aBoundaries[aCursor + 1], bBoundaries[bCursor]))
                    || (static_cast<unsigned int>(bCursor) >= bBoundaries.size() - 2 && almostLess(bBoundaries[bCursor + 1], aBoundaries[aCursor]))) {
                    //no more common parts
                    break;
                }
            }
            if (almostLess(aBoundaries[aCursor + 1], bBoundaries[bCursor + 1])) {
                ++aCursor;
            }
            if (almostGreater(aBoundaries[aCursor + 1], bBoundaries[bCursor + 1])) {
                ++bCursor;
            }
        }
        if (radialCollision || !contact) {
            return;
        }
        completelyUsedA &= static_cast<unsigned int>(aCursor) >= aBoundaries.size() - 2;
        completelyUsedB &= static_cast<unsigned int>(bCursor) >= bBoundaries.size() - 2;
        DegreesOfFreedom dof;
        if (slidePossible) {
            dof.slideDirections.push_back(a.absDirection);
        }
        if (rotationPossible) {
            dof.rotationPossibilities.emplace_back(a.absStart, a.absDirection);
        }
        addConnection(dof, {completelyUsedA, completelyUsedB});
    }

    std::optional<float> PairChecker::projectConnectorsWithLength(float aLength, float bLength) {
        const auto startDiff = b.absStart - a.absStart;
        const auto projectionLength = glm::dot(startDiff, a.absDirection);
        const auto distancePointToLine = glm::length(glm::cross(startDiff, a.absDirection));

        if ((sameDir && projectionLength > aLength)                 // Aaaaaaa   Bbbbbbbbb
            || (sameDir && projectionLength < -bLength)             // Bbbbbb  Aaaaaa
            || (oppositeDir && projectionLength > aLength + bLength)// Aaaaaaa  bbbbbbbbB
            || (oppositeDir && projectionLength < 0)                // bbbbbbB   Aaaaaaaa
            || distancePointToLine >= COLINEARITY_TOLERANCE_LDU) {
            //a and b aren't colinear
            return std::nullopt;
        } else {
            return {projectionLength};
        }
    }

    void PairChecker::findFingerFinger() {
        if (!parallel
            || a.finger->group != b.finger->group
            || std::abs(a.finger->radius - b.finger->radius) > CONNECTION_RADIUS_TOLERANCE) {
            return;
        }
        const auto startOffsetOpt = projectConnectorsWithLength(a.finger->totalWidth, b.finger->totalWidth);
        if (!startOffsetOpt.has_value()) {
            return;
        }
        bool completelyUsedA = true;
        bool completelyUsedB = true;
        const auto aFingerCount = static_cast<int>(a.finger->fingerWidths.size());
        const auto bFingerCount = static_cast<int>(b.finger->fingerWidths.size());
        int aCursorIdx = 0;
        int bCursorIdx = sameDir ? 0 : bFingerCount - 1;
        const int bCursorStep = sameDir ? 1 : -1;
        const float aOffset = sameDir ? startOffsetOpt.value() : startOffsetOpt.value() - b.finger->totalWidth;

        if (aOffset < -POSITION_TOLERANCE_LDU) {
            float offset = -aOffset;
            while (offset > POSITION_TOLERANCE_LDU) {
                offset -= b.finger->fingerWidths[bCursorIdx];
                bCursorIdx += bCursorStep;
            }
            if (offset < -POSITION_TOLERANCE_LDU) {
                return;
            }
            completelyUsedB = false;
        } else if (aOffset > POSITION_TOLERANCE_LDU) {
            float offset = aOffset;
            while (offset > POSITION_TOLERANCE_LDU) {
                offset -= a.finger->fingerWidths[aCursorIdx];
                ++aCursorIdx;
            }
            if (offset < -POSITION_TOLERANCE_LDU) {
                return;
            }
            completelyUsedA = false;
        }
        if ((std::abs(aCursorIdx - bCursorIdx) % 2 == 0
             && a.finger->firstFingerGender == b.finger->firstFingerGender)
            || (std::abs(aCursorIdx - bCursorIdx) % 2 == 1
                && a.finger->firstFingerGender != b.finger->firstFingerGender)) {
            return;
        }
        while (aCursorIdx < aFingerCount
               && bCursorIdx < bFingerCount
               && bCursorIdx >= 0) {
            if (std::abs(a.finger->fingerWidths[aCursorIdx] - b.finger->fingerWidths[bCursorIdx]) > POSITION_TOLERANCE_LDU) {
                return;
            }
            ++aCursorIdx;
            bCursorIdx += bCursorStep;
        }
        if (aCursorIdx < aFingerCount) {
            completelyUsedA = false;
        }
        if ((sameDir && bCursorIdx < bFingerCount)
            || (!sameDir && bCursorIdx >= 0)) {
            completelyUsedB = false;
        }
        DegreesOfFreedom dof;
        dof.rotationPossibilities.emplace_back(a.absStart, a.absDirection);
        addConnection(dof, {completelyUsedA, completelyUsedB});
    }

    void PairChecker::findClipCyl(const PairCheckData& clipData, const PairCheckData& cylData) {
        const auto clip = clipData.clip;
        const auto cyl = cylData.cyl;
        if (!parallel || cyl->gender != Gender::M) {
            return;
        }
        const auto projOnA = geometry::normalProjectionOnLine<3>(cylData.absStart, cylData.absEnd, clipData.absStart, false);
        if (projOnA.distancePointToLine > POSITION_TOLERANCE_LDU) {
            return;
        }
        float offset = projOnA.projectionLength;
        if (oppositeDir) {
            offset -= clip->width;
        }
        int i = 0;
        while (offset > cyl->parts[i].length) {
            offset -= cyl->parts[i].length;
            ++i;
        }
        bool cylCompletelyUsed = i == 0;

        bool touching = false;
        const auto cylPartsCount = static_cast<int>(cyl->parts.size());
        while (offset > -clip->width && i < cylPartsCount) {
            float radiusDiff = cyl->parts[i].radius - clip->radius;
            if (radiusDiff > CONNECTION_RADIUS_TOLERANCE) {
                return;
            }
            touching |= radiusDiff > -CONNECTION_RADIUS_TOLERANCE;
            offset -= cyl->parts[i].length;
            ++i;
        }
        cylCompletelyUsed &= i >= cylPartsCount - 1;
        bool clipCompletelyUsed = offset < -clip->width;
        if (!touching) {
            return;
        }
        DegreesOfFreedom dof;
        dof.rotationPossibilities.emplace_back(cylData.absStart, cylData.absDirection);
        if (cyl->slide && clip->slide) {
            dof.slideDirections.push_back(cylData.absDirection);
        }
        const auto aCompletelyUsed = &cylData == &a ? cylCompletelyUsed : clipCompletelyUsed;
        const auto bCompletelyUsed = &cylData == &a ? clipCompletelyUsed : cylCompletelyUsed;
        addConnection(dof, {aCompletelyUsed, bCompletelyUsed});
    }

    void PairChecker::addConnection(DegreesOfFreedom dof, const std::array<bool, 2>& completelyUsedConnector) {
        resultConsumer.addConnection(a.connector, b.connector, dof, completelyUsedConnector);
    }

    PairCheckData::PairCheckData(const glm::mat4& absTransformation,
                                 const std::shared_ptr<Connector>& connector,
                                 const glm::vec3 absStart,
                                 const glm::vec3 absDirection) :
        absTransformation(absTransformation),
        absStart(absStart),
        absEnd(absStart),
        absDirection(absDirection),
        connector(connector) {
        switch (connector->type) {
            case Connector::Type::CYLINDRICAL:
                cyl = dynamic_cast<CylindricalConnector*>(connector.get());
                absEnd = absStart + absDirection * cyl->totalLength;
                break;
            case Connector::Type::CLIP:
                clip = dynamic_cast<ClipConnector*>(connector.get());
                absEnd = absStart + absDirection * clip->width;
                break;
            case Connector::Type::FINGER:
                finger = dynamic_cast<FingerConnector*>(connector.get());
                absEnd = absStart + absDirection * finger->totalWidth;
                break;
            case Connector::Type::GENERIC:
                generic = dynamic_cast<GenericConnector*>(connector.get());
                break;
        }
    }

    PairCheckData::PairCheckData(const std::shared_ptr<etree::MeshNode>& node,
                                 const std::shared_ptr<Connector>& connector) :
        PairCheckData(glm::transpose(node->getAbsoluteTransformation()), connector) {}

    PairCheckData::PairCheckData(const glm::mat4& absTransformation, const std::shared_ptr<Connector>& connector) :
        PairCheckData(absTransformation,
                      connector,
                      absTransformation * glm::vec4(connector->start, 1.f),
                      absTransformation * glm::vec4(connector->direction, 0.f)) {}

    void ConnectionGraphPairCheckResultConsumer::addConnection(const std::shared_ptr<Connector>& connectorA,
                                                               const std::shared_ptr<Connector>& connectorB,
                                                               DegreesOfFreedom dof,
                                                               const std::array<bool, 2>& completelyUsedConnector) {
        result.addConnection(nodeA, nodeB, std::make_shared<Connection>(connectorA, connectorB, dof, completelyUsedConnector));
    }

    ConnectionGraphPairCheckResultConsumer::ConnectionGraphPairCheckResultConsumer(const ConnectionGraph::node_t& nodeA,
                                                                                   const ConnectionGraph::node_t& nodeB,
                                                                                   ConnectionGraph& result) :
        nodeA(nodeA),
        nodeB(nodeB), result(result) {}

    void VectorPairCheckResultConsumer::addConnection(const std::shared_ptr<Connector>& connectorA,
                                                      const std::shared_ptr<Connector>& connectorB,
                                                      DegreesOfFreedom dof,
                                                      const std::array<bool, 2>& completelyUsedConnector) {
        result.push_back({connectorA, connectorB, dof, completelyUsedConnector});
    }

    const std::vector<Connection>& VectorPairCheckResultConsumer::getResult() const {
        return result;
    }
}
