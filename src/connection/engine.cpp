#include "engine.h"
#include "../helpers/geometry.h"
#include "connector_data_provider.h"

namespace bricksim::connection::engine {

    std::vector<Connection> findConnections(const std::shared_ptr<etree::LdrNode>& a, const std::shared_ptr<etree::LdrNode>& b) {
        const auto& connectorsA = getConnectorsOfPart(a->ldrFile->metaInfo.name);
        const auto& connectorsB = getConnectorsOfPart(b->ldrFile->metaInfo.name);
        for (const auto& ca: connectorsA) {
            const auto caAbsoluteStart = a->getAbsoluteTransformation() * glm::vec4(ca->start, 1.f);
            const auto caClip = std::dynamic_pointer_cast<ClipConnector>(ca);
            const auto caCyl = std::dynamic_pointer_cast<CylindricalConnector>(ca);//todo maybe only call dynamic_pointer_cast if previous results are nullptr
            const auto caFinger = std::dynamic_pointer_cast<FingerConnector>(ca);
            const auto caGeneric = std::dynamic_pointer_cast<GenericConnector>(ca);
            for (const auto& cb: connectorsB) {
                const auto cbAbsoluteStart = b->getAbsoluteTransformation() * glm::vec4(cb->start, 1.f);
                const auto cbClip = std::dynamic_pointer_cast<ClipConnector>(cb);
                const auto cbCyl = std::dynamic_pointer_cast<CylindricalConnector>(cb);
                const auto cbFinger = std::dynamic_pointer_cast<FingerConnector>(cb);
                const auto cbGeneric = std::dynamic_pointer_cast<GenericConnector>(cb);

                if (caGeneric != nullptr || cbGeneric != nullptr) {
                    if (caGeneric != nullptr
                        && cbGeneric != nullptr
                        && caGeneric->group == cbGeneric->group
                        && caGeneric->gender != cbGeneric->gender
                        && caGeneric->bounding == cbGeneric->bounding
                        && glm::length2(caAbsoluteStart - cbAbsoluteStart) < std::pow(POSITION_TOLERANCE_LDU, 2)) {
                        //todo const auto caAbsoluteOrientation = a->getAbsoluteTransformation()*caGeneric
                    }
                } else {
                    if (caCyl != nullptr) {
                        if (cbCyl != nullptr) {
                            if (caCyl->gender != cbCyl->gender) {
                                if (geometry::getAngleBetweenTwoVectors(caCyl->direction, cbCyl->direction) < PARALLELITY_ANGLE_TOLERANCE) {
                                    const geometry::NormalProjectionResult<3> projOnA = geometry::normalProjectionOnLineClamped<3>(caCyl->start, caCyl->getEnd(), cbCyl->start);
                                    if (projOnA.distancePointToLine < COLINEARITY_TOLERANCE_LDU && projOnA.projectionLength < caCyl->getTotalLength()) {
                                        //todo check if part lengths and diameters match
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
