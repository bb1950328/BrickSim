#include "engine.h"
#include "../helpers/geometry.h"
#include "connector_data_provider.h"

namespace bricksim::connection::engine {

    std::vector<Connection> findConnections(const std::shared_ptr<etree::LdrNode>& a, const std::shared_ptr<etree::LdrNode>& b) {
        std::vector<Connection> result;

        const auto& connectorsA = getConnectorsOfPart(a->ldrFile->metaInfo.name);
        const auto& connectorsB = getConnectorsOfPart(b->ldrFile->metaInfo.name);
        for (const auto& ca: connectorsA) {
            const glm::vec3 caAbsoluteStart = a->getAbsoluteTransformation() * glm::vec4(ca->start, 1.f);
            const glm::vec3 caAbsoluteDirection = a->getAbsoluteTransformation() * glm::vec4(ca->direction, 0.f);
            const auto caClip = std::dynamic_pointer_cast<ClipConnector>(ca);
            const auto caCyl = std::dynamic_pointer_cast<CylindricalConnector>(ca);//todo maybe only call dynamic_pointer_cast if previous results are nullptr
            const auto caFinger = std::dynamic_pointer_cast<FingerConnector>(ca);
            const auto caGeneric = std::dynamic_pointer_cast<GenericConnector>(ca);
            for (const auto& cb: connectorsB) {
                const glm::vec3 cbAbsoluteStart = b->getAbsoluteTransformation() * glm::vec4(cb->start, 1.f);
                const glm::vec3 cbAbsoluteDirection = b->getAbsoluteTransformation() * glm::vec4(cb->direction, 0.f);
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
                        result.emplace_back(ca, cb);
                        continue;
                    }
                } else {
                    if (caCyl != nullptr) {
                        if (cbCyl != nullptr) {
                            if (caCyl->gender != cbCyl->gender) {
                                const bool sameDir = absoluteDirectionAngleDifference < PARALLELITY_ANGLE_TOLERANCE;
                                const bool oppositeDir = absoluteDirectionAngleDifference > M_PI - PARALLELITY_ANGLE_TOLERANCE;
                                if (sameDir || oppositeDir) {
                                    const geometry::NormalProjectionResult<3> projOnA = geometry::normalProjectionOnLineClamped<3>(caCyl->start, caCyl->getEnd(), cbCyl->start);
                                    if ((sameDir && projOnA.projectionLength > caCyl->getTotalLength())                                          // Aaaaaaa   Bbbbbbbbb
                                        || (sameDir && projOnA.projectionLengthUnclamped < cbCyl->getTotalLength())                              // Bbbbbb  Aaaaaa
                                        || (oppositeDir && projOnA.projectionLengthUnclamped > caCyl->getTotalLength() + cbCyl->getTotalLength())// Aaaaaaa  bbbbbbbbB
                                        || (oppositeDir && projOnA.projectionLengthUnclamped < 0)) {                                             // bbbbbbB   Aaaaaaaa
                                        //the directions are colinear, but the parts are too far away from each other
                                        continue;
                                    }
                                    if (projOnA.distancePointToLine < COLINEARITY_TOLERANCE_LDU) {
                                        const int maleIdx = caCyl->gender == Gender::M ? 0 : 1;
                                        const int femaleIdx = 1 - maleIdx;

                                        std::vector<float> aBoundaries(caCyl->parts.size() + 1);
                                        aBoundaries.push_back(0);
                                        float offset = 0;
                                        for (const auto& item: caCyl->parts) {
                                            offset += item.length;
                                            aBoundaries.push_back(offset);
                                        }

                                        std::vector<float> bBoundaries(cbCyl->parts.size() + 1);
                                        std::vector<CylindricalShapePart> bParts(cbCyl->parts.size());
                                        if (sameDir) {
                                            offset = projOnA.projectionLengthUnclamped;
                                            bBoundaries.push_back(offset);
                                            for (const auto& item: cbCyl->parts) {
                                                offset += item.length;
                                                bBoundaries.push_back(offset);
                                                bParts.push_back(item);
                                            }
                                        } else {
                                            offset = projOnA.projectionLengthUnclamped - cbCyl->getTotalLength();
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
                                        bool slidePossible = caCyl->slide || cbCyl->slide;
                                        while (true) {
                                            if (aCursor <= 0 && bCursor <= 0) {
                                                const auto& pa = caCyl->parts[aCursor];
                                                const auto& pb = bParts[bCursor];
                                                const float ra = pa.radius;
                                                const float rb = pb.radius;
                                                if ((caCyl->gender == Gender::M && ra - rb > CYLINDRICAL_CONNECTION_RADIUS_TOLERANCE)
                                                    || (caCyl->gender != Gender::M && rb - ra > CYLINDRICAL_CONNECTION_RADIUS_TOLERANCE)) {
                                                    radialCollision = true;
                                                    break;
                                                }
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
                                        if (!radialCollision) {
                                            std::vector<glm::vec3> slideDirections;
                                            std::vector<glm::vec3> rotationAxes;
                                            if (slidePossible) {
                                                slideDirections.push_back(ca->direction);
                                            }
                                            if (rotationPossible) {
                                                rotationAxes.push_back(ca->direction);
                                            }
                                            result.emplace_back(ca, cb, DegreesOfFreedom(slideDirections, rotationAxes));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    ConnectionGraph findConnections(const std::shared_ptr<etree::Node>& node, mesh::SceneMeshCollection& meshCollection) {
        //todo implement aabb tree
        // each editor should have an aabb tree of all nodes that is always up to date and passed to this method
        // traverse the aabb tree and call findConnections(node, node) for all collisions
        // insert the resulting connections into the connectiongraph
        // maybe use https://github.com/lohedges/aabbcc https://github.com/albin-johansson/abby https://github.com/iauns/cpm-glm-aabb
        ConnectionGraph result = ConnectionGraph();
        addConnections(node, result);
        return result;
    }
    void addConnections(const std::shared_ptr<etree::Node>& node, ConnectionGraph& graph) {
    }
}
