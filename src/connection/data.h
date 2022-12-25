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
        glm::vec3 start;

        Connector(std::string group, const glm::vec3& start);

        virtual ~Connector() {}
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
        glm::vec3 direction;
        Gender gender;
        std::vector<CylindricalShapePart> parts;
        bool openStart;
        bool openEnd;
        bool slide;

        CylindricalConnector(std::string group,
                             const glm::vec3& start,
                             const glm::vec3& direction,
                             Gender gender,
                             std::vector<CylindricalShapePart> parts,
                             bool openStart,
                             bool openEnd,
                             bool slide);

        float getTotalLength() const;
    };

    class ClipConnector : public Connector {
    public:
        glm::vec3 direction;
        float radius;
        float width;
        bool slide;

        ClipConnector(const std::string& group,
                      const glm::vec3& start,
                      const glm::vec3& direction,
                      float radius,
                      float width,
                      bool slide);
    };

    class FingerConnector : public Connector {
    public:
        glm::vec3 direction;
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
