#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../element_tree.h"
#include "bounding.h"
#include "glm/glm.hpp"

namespace bricksim::connection {
    enum class Gender {
        M,
        F,
    };

    class Connector {
    public:
        std::string group;
        glm::mat4 location;
    };

    enum class CylindricalShapeType {
        ROUND,
        AXLE,
        SQUARE,
    };
    class CylindricalShapePart {
    public:
        CylindricalShapeType type;
        bool flexibleRadius;
        float radius;
        float length;
    };

    class CylindricalConnector : public Connector {
    public:
        Gender gender;
        std::vector<CylindricalShapePart> parts;
        bool openStart, openEnd;
        bool slide;
    };

    class ClipConnector : public Connector {
    public:
        float radius;
        float width;
        bool slide;
    };

    class FingerConnector : public Connector {
    public:
        Gender firstFingerGender;
        float radius;
        std::vector<float> fingerWidths;
    };

    class GenericConnector : public Connector {
    public:
        Gender gender;
        bounding_variant_t bounding;
    };

    class DegreesOfFreedom {
    public:
        std::vector<glm::vec3> slideDirections;
        std::vector<glm::vec3> rotationAxes;
    };

    class Connection {
    public:
        std::size_t connectorA;
        std::size_t connectorB;
        DegreesOfFreedom degreesOfFreedom;
        Connection(size_t connectorA, size_t connectorB, DegreesOfFreedom degreesOfFreedom);
    };

    class ConnectionGraph {
    public:
        typedef std::shared_ptr<etree::LdrNode> node_t;
        typedef std::shared_ptr<Connection> edge_t;
        uomap_t<node_t, uomap_t<node_t, std::vector<edge_t>>> adjacencyLists;

        void addConnection(const node_t& a, const node_t& b, const edge_t& edge);
        void removeConnection(const node_t& a, const node_t& b, const edge_t& edge);
        void removeAllConnections(const node_t& a, const node_t& b);
        const std::vector<edge_t>& getConnections(const node_t& a, const node_t& b);

    private:
        std::array<std::vector<edge_t>, 2> getBothVectors(const node_t& a, const node_t& b);
    };
}