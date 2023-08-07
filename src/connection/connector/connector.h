#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace bricksim::connection {
    enum class Gender {
        M,
        F,
    };

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

    protected:
        static std::pair<float, float> getRadiusAndLengthFactorFromTransformation(const glm::mat4& transformation, const glm::vec3& direction);
    };
}
namespace std {
    template<>
    struct hash<bricksim::connection::Connector> {
        std::size_t operator()(const bricksim::connection::Connector& value) const;
    };
}
