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

    constexpr static float PARALLELITY_ANGLE_TOLERANCE = .018f;//around 1°
    constexpr static float PARALLELITY_ANGLE_TOLERANCE_SQUARED = PARALLELITY_ANGLE_TOLERANCE * PARALLELITY_ANGLE_TOLERANCE;
    constexpr static float COLINEARITY_TOLERANCE_LDU = .1f;
    constexpr static float POSITION_TOLERANCE_LDU = .1f;
    constexpr static float CONNECTION_RADIUS_TOLERANCE = 1.f;

    class Connector {
    public:
        enum class Type {
            CYLINDRICAL,
            CLIP,
            FINGER,
            GENERIC,
        };

        const Type type;
        std::string group;
        glm::vec3 start;
        glm::vec3 direction;
        std::string sourceTrace;

        Connector(Type type,
                  std::string group,
                  const glm::vec3& start,
                  const glm::vec3& direction,
                  std::string sourceTrace);

        virtual std::shared_ptr<Connector> clone();
        virtual std::shared_ptr<Connector> transform(const glm::mat4& transformation);
        virtual std::string infoStr() const;
        virtual std::size_t hash() const;
        virtual ~Connector() = default;
        bool operator==(const Connector& rhs) const;
        bool operator!=(const Connector& rhs) const;
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
        bool operator==(const CylindricalShapePart& rhs) const;
        bool operator!=(const CylindricalShapePart& rhs) const;
        CylindricalShapePart(CylindricalShapeType type, bool flexibleRadius, float radius, float length);
    };

    class CylindricalConnector : public Connector {
    public:
        Gender gender;
        std::vector<CylindricalShapePart> parts;
        bool openStart;
        bool openEnd;
        bool slide;
        float totalLength;

        CylindricalConnector(const std::string& group,
                             const glm::vec3& start,
                             const glm::vec3& direction,
                             std::string sourceTrace,
                             Gender gender,
                             std::vector<CylindricalShapePart> parts,
                             bool openStart,
                             bool openEnd,
                             bool slide);

        /**
         * @param offsetFromStart
         * @return
         * if the offset is out of bounds, this function will just return the first or last shape part
         */
        [[nodiscard]] const CylindricalShapePart& getPartAt(float offsetFromStart) const;
        [[nodiscard]] float getRadiusAt(float offsetFromStart) const;
        std::shared_ptr<Connector> clone() override;
        std::shared_ptr<Connector> transform(const glm::mat4& transformation) override;
        std::string infoStr() const override;
        size_t hash() const override;
        bool operator==(const CylindricalConnector& rhs) const;
        bool operator!=(const CylindricalConnector& rhs) const;
    };

    class ClipConnector : public Connector {
    public:
        float radius;
        float width;
        bool slide;
        glm::vec3 openingDirection;

        ClipConnector(const std::string& group,
                      const glm::vec3& start,
                      const glm::vec3& direction,
                      std::string sourceTrace,
                      float radius,
                      float width,
                      bool slide,
                      const glm::vec3& openingDirection);
        std::shared_ptr<Connector> clone() override;
        std::shared_ptr<Connector> transform(const glm::mat4& transformation) override;
        std::string infoStr() const override;
        bool operator==(const ClipConnector& rhs) const;
        bool operator!=(const ClipConnector& rhs) const;
    };

    class FingerConnector : public Connector {
    public:
        Gender firstFingerGender;
        float radius;
        std::vector<float> fingerWidths;
        float totalWidth;

        FingerConnector(const std::string& group,
                        const glm::vec3& start,
                        const glm::vec3& direction,
                        std::string sourceTrace,
                        Gender firstFingerGender,
                        float radius,
                        const std::vector<float>& fingerWidths);
        std::shared_ptr<Connector> clone() override;
        std::shared_ptr<Connector> transform(const glm::mat4& transformation) override;
        std::string infoStr() const override;
        bool operator==(const FingerConnector& rhs) const;
        bool operator!=(const FingerConnector& rhs) const;
    };

    class GenericConnector : public Connector {
    public:
        Gender gender;
        bounding_variant_t bounding;

        GenericConnector(const std::string& group,
                         const glm::vec3& start,
                         const glm::vec3& direction,
                         std::string sourceTrace,
                         Gender gender,
                         const bounding_variant_t& bounding);
        std::shared_ptr<Connector> clone() override;
        std::shared_ptr<Connector> transform(const glm::mat4& transformation) override;
        std::string infoStr() const override;
        bool operator==(const GenericConnector& rhs) const;
        bool operator!=(const GenericConnector& rhs) const;
    };

    struct RotationPossibility {
        glm::vec3 origin;
        glm::vec3 axis;
        RotationPossibility(const glm::vec3& origin, const glm::vec3& axis);
        bool operator==(const RotationPossibility& rhs) const;
        bool operator!=(const RotationPossibility& rhs) const;
    };

    class DegreesOfFreedom {
    public:
        std::vector<glm::vec3> slideDirections;
        std::vector<RotationPossibility> rotationPossibilities;

        DegreesOfFreedom();
        DegreesOfFreedom(const std::vector<glm::vec3>& slideDirections,
                         const std::vector<RotationPossibility>& rotationPossibilities);
        bool operator==(const DegreesOfFreedom& rhs) const;
        bool operator!=(const DegreesOfFreedom& rhs) const;
    };

    class Connection {
    public:
        std::shared_ptr<Connector> connectorA;
        std::shared_ptr<Connector> connectorB;
        DegreesOfFreedom degreesOfFreedom;

        Connection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB);
        Connection(const std::shared_ptr<Connector>& connectorA, const std::shared_ptr<Connector>& connectorB, DegreesOfFreedom degreesOfFreedom);
        bool operator==(const Connection& rhs) const;
        bool operator!=(const Connection& rhs) const;
    };

    class ConnectionGraph {//todo move to separate file
    public:
        using node_t = std::shared_ptr<etree::MeshNode>;
        using edge_t = std::shared_ptr<Connection>;
        using adjacency_list_t = uomap_t<node_t, uomap_t<node_t, std::vector<edge_t>>>;

        void addConnection(const node_t& a, const node_t& b, const edge_t& edge);

        void removeConnection(const node_t& a, const node_t& b, const edge_t& edge);

        void removeAllConnections(const node_t& a, const node_t& b);
        void removeAllConnections(const node_t& a);
        void removeAllConnections(const uoset_t<node_t>& toRemove);

        [[nodiscard]] const adjacency_list_t& getAdjacencyLists() const;
        [[nodiscard]] const std::vector<edge_t>& getConnections(const node_t& a, const node_t& b) const;
        [[nodiscard]] const uomap_t<node_t, std::vector<edge_t>>& getConnections(const node_t& node) const;

        [[nodiscard]] std::size_t countTotalConnections() const;
        [[nodiscard]] std::vector<uoset_t<node_t>> findAllCliques() const;

    protected:
        adjacency_list_t adjacencyLists;
        std::mutex lock;

        void findRestOfClique(uoset_t<node_t>& nodes, const ConnectionGraph::node_t& current) const;

    private:
        /**
         * Not thread-safe on its own, make sure that `adjacencyLists` is locked!
         */
        std::array<std::reference_wrapper<std::vector<ConnectionGraph::edge_t>>, 2> getBothVectors(const node_t& a, const node_t& b);
    };
}

namespace std {
    template<>
    struct hash<bricksim::connection::Connector> {
        std::size_t operator()(const bricksim::connection::Connector& value) const {
            return value.hash();
        }
    };
}
