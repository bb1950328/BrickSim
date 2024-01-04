#include "meshes.h"
#include "magic_enum.hpp"
#include "spdlog/fmt/fmt.h"
#include <cmath>

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
                default:
                    return {0, 0, 0};
            }
        }
    }

    LineSunNode::LineSunNode(const std::shared_ptr<Node>& parent, SimpleLineColor color, bool inverted, CylindricalShapeType shape) :
        SimpleGeneratedLineNode(parent, color), inverted(inverted), shape(shape) {}

    std::string LineSunNode::getDescription() {
        return fmt::format("{}LineSun {} {}", inverted ? "Inverted" : "", magic_enum::enum_name(lineColor), magic_enum::enum_name(shape));
    }

    mesh_identifier_t LineSunNode::getMeshIdentifier() const {
        return util::combinedHash(constants::MESH_ID_LINE_SUN, lineColor, inverted, shape);
    }

    void LineSunNode::addToMesh(mesh::LineData& lineData, const glm::vec3& color) {
        const auto basePoints = generateShape();
        for (size_t i1 = 0; i1 < basePoints.size(); ++i1) {
            const auto i2 = (i1 + 1ul) % basePoints.size();

            //shape circumfence
            lineData.addVertex({{basePoints[i1].x, basePoints[i1].y, 0.f}, color});
            lineData.addVertex({{basePoints[i2].x, basePoints[i2].y, 0.f}, color});

            //rays
            const auto factor = inverted ? .85f : 1.15f;
            lineData.addVertex({{basePoints[i1].x, basePoints[i1].y, 0.f}, color});
            lineData.addVertex({{basePoints[i1].x * factor, basePoints[i1].y * factor, 0.f}, color});
        }
    }

    std::vector<glm::vec2> LineSunNode::generateShape() const {
        std::vector<glm::vec2> result;
        switch (shape) {
            case CylindricalShapeType::ROUND:
                for (int i = 0; i < 12; ++i) {
                    float a = static_cast<float>(M_PI) * 2.f * static_cast<float>(i) / 12;
                    result.emplace_back(std::cos(a) * .5f, std::sin(a) * .5f);
                }
                break;
            case CylindricalShapeType::SQUARE:
                result.emplace_back(-.5f, -.5f);
                result.emplace_back(.5f, -.5f);
                result.emplace_back(.5f, .5f);
                result.emplace_back(-.5f, .5f);
                break;
            case CylindricalShapeType::AXLE:
                for (int i = 0; i < 4; ++i) {
                    float a = static_cast<float>(M_PI) * 2.f * static_cast<float>(i) / 4;
                    constexpr float offset = .35f;
                    result.emplace_back(std::cos(a - offset) * .5f, std::sin(a - offset) * .5f);
                    result.emplace_back(std::cos(a) * .5f, std::sin(a) * .5f);
                    result.emplace_back(std::cos(a + offset) * .5f, std::sin(a + offset) * .5f);
                    result.emplace_back(std::cos(a + M_PI_4) * .235f, std::sin(a + M_PI_4) * .235f);
                }
                break;
            default:
                break;
        }
        return result;
    }

    PointMarkerNode::PointMarkerNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor) :
        SimpleGeneratedLineNode(parent, lineColor) {}

    mesh_identifier_t PointMarkerNode::getMeshIdentifier() const {
        return util::combinedHash(constants::MESH_ID_POINT_MARKER, lineColor);
    }

    void PointMarkerNode::addToMesh(mesh::LineData& lineData, const glm::vec3& color) {
        for (int axis = 0; axis < 3; ++axis) {
            for (int step = 0; step < 10; ++step) {
                double angle = step / 5.f * M_PI;
                const int a0 = axis == 0 ? 1 : 0;
                const int a1 = axis == 2 ? 1 : 2;
                glm::vec3 pos(0.f, 0.f, 0.f);
                lineData.addVertex({pos, color});
                pos[a0] = std::cos(angle);
                pos[a1] = std::sin(angle);
                lineData.addVertex({pos, color});
            }
        }
    }

    std::string PointMarkerNode::getDescription() {
        return "Point Marker";
    }

    LineUVSphereNode::LineUVSphereNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor) :
        SimpleGeneratedLineNode(parent, lineColor) {}

    std::string LineUVSphereNode::getDescription() {
        return "Line UV Sphere";
    }

    mesh_identifier_t LineUVSphereNode::getMeshIdentifier() const {
        return util::combinedHash(constants::MESH_ID_LINE_UV_SPHERE, lineColor);
    }

    void LineUVSphereNode::addToMesh(mesh::LineData& lineData, const glm::vec3& color) {
        for (int iLat0 = 0; iLat0 < steps - 1; ++iLat0) {
            const int iLat1 = iLat0 + 1;
            for (int iLon0 = 0; iLon0 < steps; ++iLon0) {
                const int iLon1 = (iLon0 + 1) % steps;
                glm::vec3 p00 = getPointOnSphere(iLat0, iLon0);
                glm::vec3 p10 = getPointOnSphere(iLat1, iLon0);

                //vertical line
                lineData.addVertex({p00, color});
                lineData.addVertex({p10, color});

                //horizontal line
                if (iLat0 > 0) {
                    glm::vec3 p01 = getPointOnSphere(iLat0, iLon1);
                    lineData.addVertex({p00, color});
                    lineData.addVertex({p01, color});
                }
            }
        }
    }

    glm::vec3 LineUVSphereNode::getPointOnSphere(int iLat, int iLon) {
        constexpr float stepToAngle = 2 * M_PI / steps;
        return {
                std::cos(iLon * stepToAngle) * std::sin(iLat * stepToAngle),
                std::sin(iLon * stepToAngle) * std::sin(iLat * stepToAngle),
                std::cos(iLat * stepToAngle),
        };
    }

    SimpleGeneratedLineNode::SimpleGeneratedLineNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor) :
        GeneratedMeshNode(ldr::color_repo::getInstanceDummyColor(), parent), lineColor(lineColor) {}

    void SimpleGeneratedLineNode::addToMesh(std::shared_ptr<mesh::Mesh> mesh, bool windingInversed, const std::shared_ptr<ldr::TexmapStartCommand>& texmap) {
        addToMesh(mesh->getLineData(), getRGBFromSimpleLineColor(lineColor).asGlmVector());
    }

    LineCylinderNode::LineCylinderNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor) :
        SimpleGeneratedLineNode(parent, lineColor) {}

    std::string LineCylinderNode::getDescription() {
        return "Line Cylinder";
    }

    mesh_identifier_t LineCylinderNode::getMeshIdentifier() const {
        return util::combinedHash(constants::MESH_ID_LINE_CYLINDER, lineColor);
    }

    void LineCylinderNode::addToMesh(mesh::LineData& lineData, const glm::vec3& color) {
        constexpr float radius = .5f;
        constexpr float halfLength = .5f;
        float x0 = 0.f;
        float z0 = radius;
        for (int i0 = 0; i0 < steps; ++i0) {
            const int i1 = (i0 + 1) % steps;
            const float angle1 = 2.f * i1 / steps * M_PI;
            const float x1 = radius * std::sin(angle1);
            const float z1 = radius * std::cos(angle1);

            //top line
            lineData.addVertex({{x0, halfLength, z0}, color});
            lineData.addVertex({{x1, halfLength, z1}, color});

            //vertical line
            lineData.addVertex({{x0, halfLength, z0}, color});
            lineData.addVertex({{x0, -halfLength, z0}, color});

            //bottom line
            lineData.addVertex({{x0, -halfLength, z0}, color});
            lineData.addVertex({{x1, -halfLength, z1}, color});

            x0 = x1;
            z0 = z1;
        }
    }

    LineBoxNode::LineBoxNode(const std::shared_ptr<Node>& parent, const SimpleLineColor lineColor) :
        SimpleGeneratedLineNode(parent, lineColor) {}

    std::string LineBoxNode::getDescription() {
        return "Line Box";
    }

    mesh_identifier_t LineBoxNode::getMeshIdentifier() const {
        return util::combinedHash(constants::MESH_ID_LINE_BOX, lineColor);
    }

    void LineBoxNode::addToMesh(mesh::LineData& lineData, const glm::vec3& color) {
        for (int mainAxis = 0; mainAxis < 3; ++mainAxis) {
            const int axis0 = mainAxis == 0 ? 1 : 0;
            const int axis1 = mainAxis == 2 ? 1 : 2;
            for (int i0 = -1; i0 < 2; i0 += 2) {
                for (int i1 = -1; i1 < 2; i1 += 2) {
                    glm::vec3 p;
                    p[mainAxis] = -.5;
                    p[axis0] = .5 * i0;
                    p[axis1] = .5 * i1;
                    lineData.addVertex({p, color});
                    p[mainAxis] = .5;
                    lineData.addVertex({p, color});
                }
            }
        }
    }
}
