#include "meshes.h"
#include "magic_enum.hpp"
#include "spdlog/fmt/fmt.h"

namespace bricksim::connection::visualization {
    namespace {
        color::RGB getRGBFromSimpleLineColor(SimpleLineColor color) {
            switch (color) {
                case SimpleLineColor::RED:
                    return {0xff, 0, 0};
                case SimpleLineColor::GREEN:
                    return {0, 0xff, 0};
                case SimpleLineColor::BLUE:
                    return {0, 0, 0xff};
                case SimpleLineColor::CYAN:
                    return {0, 0xff, 0xff};
                case SimpleLineColor::MAGENTA:
                    return {0xff, 0, 0xff};
                case SimpleLineColor::YELLOW:
                    return {0xff, 0xff, 0};
                case SimpleLineColor::WHITE:
                    return {0xff, 0xff, 0xff};
                case SimpleLineColor::BLACK:
                    return {0, 0, 0};
            }
        }
    }

    LineSunNode::LineSunNode(const std::shared_ptr<Node>& parent, SimpleLineColor color, bool inverted, CylindricalShapeType shape) :
        GeneratedMeshNode(ldr::color_repo::getInstanceDummyColor(), parent), lineColor(color), inverted(inverted), shape(shape) {
    }

    std::string LineSunNode::getDescription() {
        return fmt::format("{}LineSun {} {}", inverted ? "Inverted" : "", magic_enum::enum_name(lineColor), magic_enum::enum_name(shape));
    }

    mesh_identifier_t LineSunNode::getMeshIdentifier() const {
        return util::combinedHash(constants::MESH_ID_LINE_SUN, lineColor, inverted, shape);
    }

    void LineSunNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh,
                                bool windingInversed,
                                const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        auto& lineData = mesh->getLineData();
        const auto color = getRGBFromSimpleLineColor(lineColor).asGlmVector();
        const auto basePoints = generateShape();
        for (int i1 = 0; i1 < basePoints.size(); ++i1) {
            const int i2 = (i1 + 1) % basePoints.size();

            //shape circumfence
            lineData.addVertex({{basePoints[i1].x, basePoints[i1].y, 0.f}, color});
            lineData.addVertex({{basePoints[i2].x, basePoints[i2].y, 0.f}, color});

            //rays
            const auto factor = inverted ? .85f : 1.15f;
            lineData.addVertex({{basePoints[i1].x, basePoints[i1].y, 0.f}, color});
            lineData.addVertex({{basePoints[i1].x * factor, basePoints[i1].y * factor, 0.f}, color});
        }

        /*        constexpr auto rCircle = .5f;
        const auto rRay = inverted ? .4f : .6f;
        for (int i1 = 0; i1 < NUM_CORNERS; ++i1) {
            const int i2 = (i1 + 1) % NUM_CORNERS;
            const auto a1 = M_PI * 2 * i1 / NUM_CORNERS;
            const auto a2 = M_PI * 2 * i2 / NUM_CORNERS;
            lineData.addVertex({{std::cos(a1) * rCircle, std::sin(a1) * rCircle, 0}, color});
            lineData.addVertex({{std::cos(a2) * rCircle, std::sin(a2) * rCircle, 0}, color});

            lineData.addVertex({{std::cos(a1) * rCircle, std::sin(a1) * rCircle, 0}, color});
            lineData.addVertex({{std::cos(a1) * rRay, std::sin(a1) * rRay, 0}, color});
        }*/
    }
    std::vector<glm::vec2> LineSunNode::generateShape() {
        std::vector<glm::vec2> result;
        switch (shape) {
            case CylindricalShapeType::ROUND:
                for (int i = 0; i < 12; ++i) {
                    float a = M_PIf * 2.f * i / 12;
                    result.push_back({std::cos(a) * .5f, std::sin(a) * .5f});
                }
                break;
            case CylindricalShapeType::SQUARE:
                result.push_back({-.5f, -.5f});
                result.push_back({.5f, -.5f});
                result.push_back({.5f, .5f});
                result.push_back({-.5f, .5f});
                break;
            case CylindricalShapeType::AXLE:
                for (int i = 0; i < 4; ++i) {
                    float a = M_PIf * 2.f * i / 4;
                    constexpr float offset = .35f;
                    result.push_back({std::cos(a - offset) * .5f, std::sin(a - offset) * .5f});
                    result.push_back({std::cos(a) * .5f, std::sin(a) * .5f});
                    result.push_back({std::cos(a + offset) * .5f, std::sin(a + offset) * .5f});
                    result.push_back({std::cos(a + M_PI_4f) * .235f, std::sin(a + M_PI_4f) * .235f});
                }
                break;
            default:
                break;
        }
        return result;
    }
}
