#include "pair_checker.h"
#include "../helpers/geometry.h"

namespace bricksim::connection {
    PairChecker::PairChecker(const PairCheckData& a, const PairCheckData& b) :
        a(a),
        b(b),
        sameDir(glm::length2(glm::cross(a.absDirection, b.absDirection)) < PARALLELITY_ANGLE_TOLERANCE_SQUARED),
        oppositeDir(!sameDir && glm::length2(glm::cross(-a.absDirection, b.absDirection)) < PARALLELITY_ANGLE_TOLERANCE_SQUARED) {
    }
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
    bool PairChecker::findGenericGeneric() {
        if (a.generic->group == b.generic->group
            && a.generic->gender != b.generic->gender
            //&& a.generic->bounding == b.generic->bounding
            && glm::length2(a.absStart - b.absStart) < std::pow(POSITION_TOLERANCE_LDU, 2)) {
            //todo maybe check bounding shapes, but documentation is unclear
            // is it a connection when the two bounding shapes touch
            // or is it only a connection when the shapes have the same dimensions and the orientation matches too?
            addConnection(a.connector, b.connector, {});
            return true;
        }
        return false;
    }
    bool PairChecker::findCylCyl() {
        if ((!sameDir && !oppositeDir)
            || a.cyl->gender == b.cyl->gender) {
            return false;
        }
        const auto startOffset = projectConnectorsWithLength(a.cyl->totalLength, b.cyl->totalLength);
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
        if (aBoundaries.front() <= bBoundaries.front()) {
            aCursor = 0;
        }
        if (aBoundaries.front() >= bBoundaries.front()) {
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
                if ((static_cast<unsigned int>(aCursor) >= aBoundaries.size() - 2 && aBoundaries[aCursor + 1] < bBoundaries[bCursor])
                    || (static_cast<unsigned int>(bCursor) >= bBoundaries.size() - 2 && bBoundaries[bCursor + 1] < aBoundaries[aCursor])) {
                    //no more common parts
                    break;
                }
            }
            if (aBoundaries[aCursor + 1] <= bBoundaries[bCursor + 1]) {
                ++aCursor;
            }
            if (aBoundaries[aCursor + 1] >= bBoundaries[bCursor + 1]) {
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
        addConnection(a.connector, b.connector, dof);
        return true;
    }
    std::optional<float> PairChecker::projectConnectorsWithLength(float aLength, float bLength) {
        const auto startDiff = b.absStart - a.absStart;
        const auto projectionLength = glm::dot(startDiff, a.absDirection);
        const auto distancePointToLine = glm::length(glm::cross(startDiff, a.absDirection));

        if ((sameDir && projectionLength > aLength)                 // Aaaaaaa   Bbbbbbbbb
            || (sameDir && projectionLength < -bLength)             // Bbbbbb  Aaaaaa
            || (oppositeDir && projectionLength > aLength + bLength)// Aaaaaaa  bbbbbbbbB
            || (oppositeDir && projectionLength < 0)                // bbbbbbB   Aaaaaaaa
            || distancePointToLine >= COLINEARITY_TOLERANCE_LDU) {  //a and b aren't colinear
            return std::nullopt;
        } else {
            return {projectionLength};
        }
    }
    bool PairChecker::findFingerFinger() {
        if ((!sameDir && !oppositeDir)
            || a.finger->group != b.finger->group
            || std::abs(a.finger->radius - b.finger->radius) > CONNECTION_RADIUS_TOLERANCE) {
            return false;
        }
        const auto startOffsetOpt = projectConnectorsWithLength(a.finger->totalWidth, b.finger->totalWidth);
        if (!startOffsetOpt.has_value()) {
            return false;
        }
        int aCursorIdx = 0;
        int bCursorIdx = sameDir ? 0 : static_cast<int>(b.finger->fingerWidths.size()) - 1;
        const int bCursorStep = sameDir ? 1 : -1;
        const float aOffset = sameDir ? startOffsetOpt.value() : startOffsetOpt.value() - b.finger->totalWidth;

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
        addConnection(a.connector, b.connector, dof);
        return true;
    }
    bool PairChecker::findClipCyl(const PairCheckData& clipData, const PairCheckData& cylData) {
        const auto clip = clipData.clip;
        const auto cyl = cylData.cyl;
        if ((!sameDir && !oppositeDir)
            || cyl->gender != Gender::M) {
            return false;
        }
        const auto projOnA = geometry::normalProjectionOnLine<3>(cylData.absStart, cylData.absEnd, clipData.absStart, false);
        if (projOnA.distancePointToLine > POSITION_TOLERANCE_LDU) {
            return false;
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
        bool touching = false;
        while (offset > -clip->width) {
            float radiusDiff = cyl->parts[i].radius - clip->radius;
            if (radiusDiff > CONNECTION_RADIUS_TOLERANCE) {
                return false;
            }
            touching |= radiusDiff > -CONNECTION_RADIUS_TOLERANCE;
            offset -= cyl->parts[i].length;
            ++i;
        }
        if (!touching) {
            return false;
        }
        DegreesOfFreedom dof;
        dof.rotationPossibilities.emplace_back(cylData.absStart, cylData.absDirection);
        if (cyl->slide && clip->slide) {
            dof.slideDirections.push_back(cylData.absDirection);
        }
        addConnection(a.connector, b.connector, dof);
        return true;
    }
    PairCheckData::PairCheckData(const std::shared_ptr<etree::LdrNode>& node,
                                 const glm::mat4& absTransformation,
                                 const std::shared_ptr<Connector>& connector) :
        node(node),
        absTransformation(absTransformation),
        absStart(absTransformation * glm::vec4(connector->start, 1.f)),
        absEnd(absStart),
        absDirection(absTransformation * glm::vec4(connector->direction, 0.f)),
        connector(connector) {
        switch (connector->type) {
            case Connector::Type::CYLINDRICAL:
                cyl = std::dynamic_pointer_cast<CylindricalConnector>(connector);
                absEnd = absStart + absDirection * cyl->totalLength;
                break;
            case Connector::Type::CLIP:
                clip = std::dynamic_pointer_cast<ClipConnector>(connector);
                absEnd = absStart + absDirection * clip->width;
                break;
            case Connector::Type::FINGER:
                finger = std::dynamic_pointer_cast<FingerConnector>(connector);
                absEnd = absStart + absDirection * finger->totalWidth;
                break;
            case Connector::Type::GENERIC:
                generic = std::dynamic_pointer_cast<GenericConnector>(connector);
                break;
        }
    }

    PairCheckData::PairCheckData(const std::shared_ptr<etree::LdrNode>& node,
                                 const std::shared_ptr<Connector>& connector) :
        PairCheckData(node, glm::transpose(node->getAbsoluteTransformation()), connector) {
    }

    PairCheckData::PairCheckData(const glm::mat4& absTransformation,
                                 const std::shared_ptr<Connector>& connector) :
        PairCheckData(nullptr, absTransformation, connector) {
    }
    ConnectionGraphPairChecker::ConnectionGraphPairChecker(const PairCheckData& a, const PairCheckData& b, ConnectionGraph& result) :
        PairChecker(a, b), result(result) {
    }
    void ConnectionGraphPairChecker::addConnection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom dof) {
        result.addConnection(a.node, b.node, std::make_shared<Connection>(connectorA, connectorB, dof));
    }
    void VectorPairChecker::addConnection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom dof) {
        result.push_back({connectorA, connectorB});
    }
    VectorPairChecker::VectorPairChecker(const PairCheckData& a, const PairCheckData& b, std::vector<std::array<std::shared_ptr<Connector>, 2>>& result) :
        PairChecker(a, b), result(result) {}
}
