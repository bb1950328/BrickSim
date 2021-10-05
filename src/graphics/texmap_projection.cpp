#include "texmap_projection.h"
#include "../helpers/earcut_hpp_with_glm.h"
#include "../helpers/polygon_clipping.h"
#include "../helpers/util.h"
#include <glm/gtx/normal.hpp>

namespace bricksim::graphics::texmap_projection {

    glm::vec2 getPlanarUVCoord(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, glm::vec3 point) {
        const glm::vec3 p1(startCommand->x1, startCommand->y1, startCommand->z1);
        const glm::vec3 p2(startCommand->x2, startCommand->y2, startCommand->z2);
        const glm::vec3 p3(startCommand->x3, startCommand->y3, startCommand->z3);
        const auto p1to2 = p2 - p1;
        const auto p1to3 = p3 - p1;
        const auto distToP1 = util::getDistanceBetweenPointAndPlane(Ray3(p1, p1to2), point);
        const auto distToP2 = util::getDistanceBetweenPointAndPlane(Ray3(p1, p1to3), point);
        return {distToP1 / glm::length(p1to2), distToP2 / glm::length(p1to3)};
    }

    PolygonSplittingResult splitPolygonBiggerThanTexturePlanar(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, const std::vector<glm::vec3>& points) {
        const glm::vec3 texP1(startCommand->x1, startCommand->y1, startCommand->z1);
        const glm::vec3 texP2(startCommand->x2, startCommand->y2, startCommand->z2);
        const glm::vec3 texP3(startCommand->x3, startCommand->y3, startCommand->z3);
        const glm::vec3 texP4 = texP2 + texP3 - texP1;

        const auto& p1 = points[0];
        const auto& p2 = points[1];
        const auto& p3 = points[2];

        const Ray3 triangleRay(p1, glm::triangleNormal(p1, p2, p3));
        const auto texturePlaneNormal = glm::triangleNormal(texP1, texP2, texP3);

        util::Plane3dTo2dConverter planeConverter(p1, p2, p3);

        std::vector<glm::vec3> texEndPointsOnTrianglePlane;
        std::vector<glm::vec2> texEndPointsIn2D;
        for (size_t i = 0; const auto& texP: {texP1, texP3, texP4, texP2}) {
            texEndPointsOnTrianglePlane.push_back(util::linePlaneIntersection(texP, texturePlaneNormal + texP, triangleRay).value());
            texEndPointsIn2D.push_back(planeConverter.convert3dTo2d(texEndPointsOnTrianglePlane[i]));
            ++i;
        }

        std::vector<glm::vec2> trianglePointsIn2D;
        for (size_t i = 0; const auto& trianglePoint: points) {
            trianglePointsIn2D.push_back(planeConverter.convert3dTo2d(trianglePoint));
            ++i;
        }

        if (!util::is2dPolygonClockwise(texEndPointsIn2D)) {
            std::reverse(texEndPointsOnTrianglePlane.begin(), texEndPointsOnTrianglePlane.end());
            std::reverse(texEndPointsIn2D.begin(), texEndPointsIn2D.end());
        }

        if (!util::is2dPolygonClockwise(trianglePointsIn2D)) {
            std::reverse(trianglePointsIn2D.begin(), trianglePointsIn2D.end());
        }

        PolygonSplittingResult res;
        {
            std::vector<std::vector<glm::vec2>> texturedPolygons;
            polyclip::Polygon triangleIn2dPolygon(trianglePointsIn2D);
            polyclip::Polygon texEndPointsPolygon(texEndPointsIn2D);

            polyclip::PolygonOperation::detectIntersection(triangleIn2dPolygon, texEndPointsPolygon);
            auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(triangleIn2dPolygon, texEndPointsPolygon, polyclip::MARK_INTERSECTION);
            if (normalIntersection) {
                texturedPolygons = polyclip::PolygonOperation::extractIntersectionResults(triangleIn2dPolygon);

                const auto p1to2 = texP2 - texP1;
                const auto p1to3 = texP3 - texP1;
                Ray3 plane1Ray(texP1, p1to2);
                Ray3 plane2Ray(texP1, p1to3);
                const auto texWidth = glm::length(p1to2);
                const auto texHeight = glm::length(p1to3);

                for (const auto& item: texturedPolygons) {
                    std::vector<std::vector<glm::vec2>> poly;
                    poly.push_back(item);
                    const auto triangleIndices = mapbox::earcut(poly);
                    res.texturedVertices.reserve(triangleIndices.size());
                    for (unsigned int triangleIndex: triangleIndices) {
                        const auto coord3d = planeConverter.convert2dTo3d(item[triangleIndex]);
                        const auto distToP1 = util::getDistanceBetweenPointAndPlane(plane1Ray, coord3d);
                        const auto distToP2 = util::getDistanceBetweenPointAndPlane(plane2Ray, coord3d);
                        glm::vec2 uv = {distToP1 / texWidth, distToP2 / texHeight};
                        res.texturedVertices.emplace_back(coord3d, uv);
                    }
                }
            }
        }

        {
            std::vector<std::vector<glm::vec2>> plainColoredPolygons;
            polyclip::Polygon triangleIn2dPolygon(trianglePointsIn2D);
            polyclip::Polygon texEndPointsPolygon(texEndPointsIn2D);

            polyclip::PolygonOperation::detectIntersection(triangleIn2dPolygon, texEndPointsPolygon);
            auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(triangleIn2dPolygon, texEndPointsPolygon, polyclip::MARK_DIFFERENTIATE);
            if (normalIntersection) {
                plainColoredPolygons = polyclip::PolygonOperation::extractDifferentiateResults(triangleIn2dPolygon);
            } else {
                plainColoredPolygons = possibleResult;
            }
            for (const auto& poly: plainColoredPolygons) {
                std::vector<std::vector<glm::vec2>> polyWrapper;
                polyWrapper.push_back(poly);
                const auto triangleIndices = mapbox::earcut(polyWrapper);

                size_t baseIndex = res.plainColorVertices.size();
                for (auto coord2d: poly) {
                    const glm::vec3 coord3d = planeConverter.convert2dTo3d(coord2d);
                    res.plainColorVertices.push_back({coord3d, triangleRay.direction});
                }
                for (const auto& idx: triangleIndices) {
                    res.plainColorIndices.push_back(baseIndex + idx);
                }
            }
        }

        return res;
    }
}
