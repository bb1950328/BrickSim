#include "texmap_projection.h"
#include "../helpers/earcut_hpp_with_glm.h"
#include "../helpers/geometry.h"
#include "../helpers/polygon_clipping.h"
#include "../ldr/file_repo.h"
#include "clipper2/clipper.h"

#include <glm/gtx/normal.hpp>

namespace bricksim::graphics::texmap_projection {
    namespace clipper2 = Clipper2Lib;

    template<typename T>
    clipper2::PathD convertPath(const std::vector<glm::vec<2, T>> &value) {
        clipper2::PathD result;
        result.reserve(value.size());
        for (const auto &vec: value) {
            result.emplace_back(vec.x, vec.y);
        }
        return result;
    }

    template<typename T>
    std::vector<glm::vec<2, T>> convertPath(const clipper2::PathD &value) {
        std::vector<glm::vec<2, T>> result;
        result.reserve(value.size());
        for (const auto &point: value) {
            result.emplace_back(point.x, point.y);
        }
        return result;
    }

    glm::vec2 getPlanarUVCoord(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, glm::vec3 point) {
        const glm::vec3 p1(startCommand->x1(), startCommand->y1(), startCommand->z1());
        const glm::vec3 p2(startCommand->x2(), startCommand->y2(), startCommand->z2());
        const glm::vec3 p3(startCommand->x3(), startCommand->y3(), startCommand->z3());
        const auto p1to2 = p2 - p1;
        const auto p1to3 = p3 - p1;
        const auto distToP1 = geometry::getDistanceBetweenPointAndPlane(Ray3(p1, p1to2), point);
        const auto distToP2 = geometry::getDistanceBetweenPointAndPlane(Ray3(p1, p1to3), point);
        return {distToP1 / glm::length(p1to2), distToP2 / glm::length(p1to3)};
    }

    PolygonSplittingResult splitPolygonBiggerThanTexturePlanar(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, const std::vector<glm::vec3>& points) {
        const glm::vec3 texP1(startCommand->x1(), startCommand->y1(), startCommand->z1());
        const glm::vec3 texP2(startCommand->x2(), startCommand->y2(), startCommand->z2());
        const glm::vec3 texP3(startCommand->x3(), startCommand->y3(), startCommand->z3());
        const glm::vec3 texP4 = texP2 + texP3 - texP1;//texP1 + 2.f * ((texP2 + texP3) / 2.f - texP1)

        const auto& p1 = points[0];
        const auto& p2 = points[1];
        const auto& p3 = points[2];

        const Ray3 polygonPlaneRay(p1, glm::triangleNormal(p1, p2, p3));
        const auto texturePlaneNormal = glm::triangleNormal(texP1, texP2, texP3);
        geometry::Plane3dTo2dConverter planeConverter(p1, p2, p3);

        PolygonSplittingResult res;

        if (std::abs(glm::dot(polygonPlaneRay.direction, texturePlaneNormal)) < 0.0001f) {
            //polygon plane is orthogonal to texture plane, cannot project texture plane onto polygon plane
            const std::vector<Ray3> planes{
                    {texP1, texP2 - texP1},
                    {texP2, texP1 - texP2},
                    {texP1, texP3 - texP1},
                    {texP3, texP1 - texP3},
            };

            std::vector<std::vector<glm::vec3>> splittedPolygons;
            splittedPolygons.push_back(points);
            for (const auto& pl: planes) {
                std::vector<std::vector<glm::vec3>> copy(splittedPolygons.begin(), splittedPolygons.end());
                splittedPolygons.clear();
                for (const auto& polyToSplit: copy) {
                    auto splitted = geometry::splitPolygonByPlane(polyToSplit, pl);
                    splittedPolygons.insert(splittedPolygons.end(), splitted.begin(), splitted.end());
                }
            }

            const auto p1to2 = texP2 - texP1;
            const auto p1to3 = texP3 - texP1;
            Ray3 plane1Ray(texP1, p1to2);
            Ray3 plane2Ray(texP1, p1to3);
            const auto texWidth = glm::length(p1to2);
            const auto texHeight = glm::length(p1to3);

            for (const auto& poly3d: splittedPolygons) {
                std::vector<glm::vec2> poly2d;
                poly2d.reserve(poly3d.size());
                std::transform(poly3d.begin(), poly3d.end(), std::back_inserter(poly2d), [&planeConverter](const auto& c3d) { return planeConverter.convert3dTo2d(c3d); });
                std::vector<std::vector<glm::vec2>> polyWrapper;
                polyWrapper.push_back(poly2d);
                const auto triangleIndices = mapbox::earcut(polyWrapper);

                for (size_t i = 0; i < triangleIndices.size(); i += 3) {
                    std::array<glm::vec2, 3> UVs{};
                    std::array<glm::vec3, 3> coords3d{};
                    bool allUVsInImage = true;
                    for (int j = 0; j < 3; ++j) {
                        coords3d[j] = poly3d[triangleIndices[i + j]];
                        const auto distToP1 = geometry::getDistanceBetweenPointAndPlane(plane1Ray, coords3d[j]);
                        const auto distToP2 = geometry::getDistanceBetweenPointAndPlane(plane2Ray, coords3d[j]);
                        UVs[j] = {distToP1 / texWidth, distToP2 / texHeight};
                        allUVsInImage &= util::isUvInsideImage(UVs[j]);
                    }
                    if (allUVsInImage) {
                        for (int j = 0; j < 3; ++j) {
                            res.texturedVertices.emplace_back(coords3d[j], UVs[j]);
                        }
                    } else {
                        for (const auto& c: coords3d) {
                            res.plainColorIndices.push_back(static_cast<unsigned int>(res.plainColorVertices.size()));
                            res.plainColorVertices.emplace_back(c, polygonPlaneRay.direction);
                        }
                    }
                }
            }

            //todo splitPolygonByPlanes(points, planes);

            std::vector<glm::vec2> cutPolygon2d;
            std::vector<glm::vec3> cutPolygon3d;
            /*for (size_t i1 = 0, i2 = 1; i1 < points.size(); ++i1, i2 = (i1 + 1) % points.size()) {
                const auto& ep1 = points[i1];
                const auto& ep2 = points[i2];
                cutPolygon2d.push_back(planeConverter.convert3dTo2d(ep1));
                cutPolygon3d.push_back(ep1);

                std::vector<glm::vec3> cutPoints;
                for (const auto& pl: planes) {
                    const auto intersectOpt = geometry::segmentPlaneIntersection(ep1, ep2, pl);
                    if (intersectOpt.has_value()) {
                        cutPoints.push_back(intersectOpt.value());
                    }
                }
                std::sort(cutPoints.begin(), cutPoints.end(), [&ep1](const glm::vec3& a, const glm::vec3& b) { return glm::length2(a - ep1) < glm::length2(b - ep1); });
                for (const auto &cp : cutPoints) {
                    cutPolygon2d.push_back(planeConverter.convert3dTo2d(cp));
                    cutPolygon3d.push_back(cp);
                }
            }*/

            //todo create 4 orthogonal planes around the texture plane
            // iterate over all edges of the polygon
            //   if the edges intersects one of the planes, insert a new point into the polygon there
            // convert the polygon to triangles using earcut.hpp
            // get uv coords for all triangles normally
            // iterate over the triangles
            //   if all uv coords are inside the image, add textured triangle vertex
            //   else add plain colored vertex}
        } else {
            std::vector<glm::vec3> texEndPointsOnTrianglePlane;
            std::vector<glm::vec2> texEndPointsIn2D;
            for (size_t i = 0; const auto& texP: {texP1, texP3, texP4, texP2}) {
                texEndPointsOnTrianglePlane.push_back(geometry::linePlaneIntersection(texP, texP + texturePlaneNormal, polygonPlaneRay).value());
                texEndPointsIn2D.push_back(planeConverter.convert3dTo2d(texEndPointsOnTrianglePlane[i]));
                ++i;
            }

            std::vector<glm::vec2> trianglePointsIn2D;
            for (size_t i = 0; const auto& trianglePoint: points) {
                trianglePointsIn2D.push_back(planeConverter.convert3dTo2d(trianglePoint));
                ++i;
            }

            if (!geometry::is2dPolygonClockwise(texEndPointsIn2D)) {
                std::reverse(texEndPointsOnTrianglePlane.begin(), texEndPointsOnTrianglePlane.end());
                std::reverse(texEndPointsIn2D.begin(), texEndPointsIn2D.end());
            }

            if (!geometry::is2dPolygonClockwise(trianglePointsIn2D)) {
                std::reverse(trianglePointsIn2D.begin(), trianglePointsIn2D.end());
            }

            {
                clipper2::PathD triangleIn2dPolygon = convertPath(trianglePointsIn2D);
                clipper2::PathD texEndPointsPolygon = convertPath(texEndPointsIn2D);
                const auto texturedPolygons = clipper2::Intersect({texEndPointsPolygon}, {triangleIn2dPolygon},
                                                                  clipper2::FillRule::NonZero);

                const auto p1to2 = texP2 - texP1;
                const auto p1to3 = texP3 - texP1;
                Ray3 plane1Ray(texP1, p1to2);
                Ray3 plane2Ray(texP1, p1to3);
                const auto texWidth = glm::length(p1to2);
                const auto texHeight = glm::length(p1to3);

                for (const auto &item: texturedPolygons) {
                    std::vector<std::vector<glm::vec2>> poly;
                    poly.push_back(convertPath<float>(item));
                    const auto triangleIndices = mapbox::earcut(poly);
                    res.texturedVertices.reserve(triangleIndices.size());
                    for (unsigned int triangleIndex: triangleIndices) {
                        const auto &point = item[triangleIndex];
                        const auto coord3d = planeConverter.convert2dTo3d({point.x, point.y});
                        const auto distToP1 = geometry::getDistanceBetweenPointAndPlane(plane1Ray, coord3d);
                        const auto distToP2 = geometry::getDistanceBetweenPointAndPlane(plane2Ray, coord3d);
                        glm::vec2 uv = {distToP1 / texWidth, distToP2 / texHeight};
                        res.texturedVertices.emplace_back(coord3d, uv);//todo add plainColor here as blendColor
                    }
                }
            }

            {
                clipper2::PathD triangleIn2dPolygon = convertPath(trianglePointsIn2D);
                clipper2::PathD texEndPointsPolygon = convertPath(texEndPointsIn2D);

                const auto plainColoredPolygons = clipper2::Difference({texEndPointsPolygon}, {triangleIn2dPolygon},
                                                                       clipper2::FillRule::NonZero);

                /*auto [normalIntersection, possibleResult] = polyclip::PolygonOperation::mark(triangleIn2dPolygon, texEndPointsPolygon, polyclip::MARK_DIFFERENTIATE);
                if (normalIntersection) {
                    plainColoredPolygons = polyclip::PolygonOperation::extractDifferentiateResults(triangleIn2dPolygon);
                } else {
                    if (possibleResult.size() == 2) {
                        //`texEndPointsIn2D` is fully inside `trianglePointsIn2D`
                        plainColoredPolygons.push_back(geometry::convertPolygonWithHoleToC(trianglePointsIn2D, texEndPointsIn2D));
                    } else {
                        plainColoredPolygons.push_back(possibleResult[0]);
                    }
                }*/
                for (const auto &poly: plainColoredPolygons) {
                    std::vector<std::vector<glm::vec2>> polyWrapper;
                    polyWrapper.push_back(convertPath<float>(poly));
                    const auto triangleIndices = mapbox::earcut(polyWrapper);

                    size_t baseIndex = res.plainColorVertices.size();
                    for (auto point: poly) {
                        const glm::vec3 coord3d = planeConverter.convert2dTo3d({point.x, point.y});
                        res.plainColorVertices.emplace_back(coord3d, polygonPlaneRay.direction);
                    }
                    for (const auto &idx: triangleIndices) {
                        res.plainColorIndices.push_back(static_cast<unsigned int>((baseIndex + idx)));
                    }
                }

                /*for (int i = 0; i < res.plainColorIndices.size(); i += 3) {
                    const auto xp0 = res.plainColorVertices[res.plainColorIndices[i]].position;
                    const auto xp1 = res.plainColorVertices[res.plainColorIndices[i + 1]].position;
                    const auto xp2 = res.plainColorVertices[res.plainColorIndices[i + 2]].position;
                    std::vector<float> dists = {glm::length(xp0 - xp1), glm::length(xp1 - xp2), glm::length(xp0 - xp2)};
                    std::sort(dists.begin(), dists.end());
                    if (std::fabs(32 - dists[0]) < 0.01 && std::fabs(48 - dists[1]) < 0.01) {
                        std::cout << dists[0] << ";" << dists[1] << ";" << dists[2] << std::endl;
                    }
                }*/
            }
        }

        return res;
    }

    std::shared_ptr<ldr::TexmapStartCommand> transformTexmapStartCommand(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand, glm::mat4 transformation) {
        auto result = std::make_shared<ldr::TexmapStartCommand>(*startCommand);
        transformation = glm::inverse(transformation);
        const auto tp1 = transformation * glm::vec4(result->x1(), result->y1(), result->z1(), 1.f);
        const auto tp2 = transformation * glm::vec4(result->x2(), result->y2(), result->z2(), 1.f);
        const auto tp3 = transformation * glm::vec4(result->x3(), result->y3(), result->z3(), 1.f);
        result->x1() = tp1.x;
        result->y1() = tp1.y;
        result->z1() = tp1.z;
        result->x2() = tp2.x;
        result->y2() = tp2.y;
        result->z2() = tp2.z;
        result->x3() = tp3.x;
        result->y3() = tp3.y;
        result->z3() = tp3.z;
        return result;
    }

    std::shared_ptr<Texture> getTexture(const std::shared_ptr<ldr::TexmapStartCommand>& startCommand) {
        auto flipVerticallyBackup = util::isStbiFlipVertically();
        util::setStbiFlipVertically(false);//todo I'm not 100% sure if this is right
        const auto& textureFile = ldr::file_repo::get().getBinaryFile(nullptr, startCommand->textureFilename, ldr::file_repo::BinaryFileSearchPath::TEXMAP);
        auto texture = graphics::Texture::getFromBinaryFileCached(textureFile);
        util::setStbiFlipVertically(flipVerticallyBackup);
        return texture;
    }
}
