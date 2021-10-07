#include "../../graphics/texmap_projection.h"
#include "../../helpers/geometry.h"
#include "../testing_tools.h"
#include <catch2/catch.hpp>
#include <array>

using namespace bricksim;
using namespace bricksim::graphics::texmap_projection;

TEST_CASE("texmapProjection simple triangle") {
    //https://www.geogebra.org/calculator/mwwkxeha
    auto startCommand = std::make_shared<ldr::TexmapStartCommand>("!TEXMAP START PLANAR\t1 1 2\t2 2 2\t1 1 1\tx.png");
    auto uvCoord = getPlanarUVCoord(startCommand, {3.5156434799567f, -0.8608205892683f, 1.7220141478743f});
    CHECK(glm::vec2(0.3274114453442f, 0.2779858521257f) == ApproxVec(uvCoord));
}

TEST_CASE("texmapProjection simple triangle 2") {
    //https://www.geogebra.org/calculator/cbx6hkut
    auto startCommand = std::make_shared<ldr::TexmapStartCommand>("!TEXMAP START PLANAR\t1 1 2.8708286935\t2 4 1\t0.5 0 1\tx.png");
    auto uvCoord = getPlanarUVCoord(startCommand, {17.8011746347567f, -3.6976918214597f, 0.0527569087467f});
    CHECK(glm::vec2(0.591128053722f, 0.3303650650423f) == ApproxVec(uvCoord));
}

TEST_CASE("texmap_projection::splitPolygonBiggerThanTexturePlanar 1") {
    //3D: https://www.geogebra.org/calculator/sj7b7498
    //2D: https://www.geogebra.org/calculator/rghp7pex
    auto startCommand = std::make_shared<ldr::TexmapStartCommand>("!TEXMAP START PLANAR\t1 3 2\t3 2 4\t-1.3912508712418, 3.2948843844814, 4.5386930634825\tx.png");
    glm::vec3 p1(-2.6221650023338, -1.5270076241372, 2.3974746723825);
    glm::vec3 p2(2, -3, 2);
    glm::vec3 p3(3.5156379001008, -0.9796540449998, 6.2660564702739);
    auto [plainIndices, plainVertices, texturedVertices] = splitPolygonBiggerThanTexturePlanar(startCommand, {p1, p2, p3});
    REQUIRE(texturedVertices.size() == 9);
    geometry::Plane3dTo2dConverter planeConverter(texturedVertices[0].position, texturedVertices[1].position, texturedVertices[2].position);
    const glm::vec3 tp1(1.6646536716619f, -2.2085746189214f, 3.2310590188774f);
    const glm::vec3 is1(0.8218596994811f, -1.2198783016188f, 4.5682011497095f);
    const glm::vec3 is2(-2.0160054983872f, -1.4729518684723f, 2.7795295641511f);
    const glm::vec3 is3(-1.7266609394129f, -1.8123870202796f, 2.3204674292731f);
    const glm::vec3 is4(0.4610677426426f, -2.5095727909081f, 2.132337680395f);

    std::vector<std::array<glm::vec3, 3>> texturedVertexCoords;
    for (int i = 0; i < texturedVertices.size(); i += 3) {
        texturedVertexCoords.push_back({
                texturedVertices[i].position,
                texturedVertices[i + 1].position,
                texturedVertices[i + 2].position,
        });
    }
    auto texturedPolygons = geometry::joinTrianglesToPolygon(texturedVertexCoords);
    REQUIRE(texturedPolygons.size() == 1);
    CHECK_VEC_VECTOR(consistentStartOfCircularList<3>({tp1, is1, is2, is3, is4}), consistentStartOfCircularList(texturedPolygons[0]));
    //todo check UV coordinates

    std::vector<std::array<glm::vec3, 3>> plainVertexCoords;
    for (int i = 0; i < plainIndices.size(); i += 3) {
        plainVertexCoords.push_back({
                plainVertices[plainIndices[i]].position,
                plainVertices[plainIndices[i + 1]].position,
                plainVertices[plainIndices[i + 2]].position,
        });
    }
    auto plainPolygons = geometry::joinTrianglesToPolygon(plainVertexCoords);
    REQUIRE(plainPolygons.size() == 2);
    if (plainPolygons[0].size() < plainPolygons[1].size()) {
        std::swap(plainPolygons[0], plainPolygons[1]);
    }
    CHECK_VEC_VECTOR(consistentStartOfCircularList<3>({is4, p2, p3, is1, tp1}), consistentStartOfCircularList(plainPolygons[0]));
    CHECK_VEC_VECTOR(consistentStartOfCircularList<3>({p1, is3, is2}), consistentStartOfCircularList(plainPolygons[1]));
}