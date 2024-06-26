#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <ostream>

namespace bricksim {
    template<int N>
    class Ray {
    public:
        glm::vec<N, float, glm::defaultp> origin;
        glm::vec<N, float, glm::defaultp> direction;

        Ray(const glm::vec<N, float, glm::defaultp>& origin, const glm::vec<N, float, glm::defaultp>& direction) :
            origin(origin), direction(direction) {}

        Ray(const Ray& other) = default;
        Ray& operator=(const Ray& other) = default;

        friend std::ostream& operator<<(std::ostream& os, const Ray& ray) {
            return os << "Ray" << N << "(origin=" << ray.origin << ", direction=" << ray.direction << ")";
        }

        Ray<N> operator*(const glm::mat<N + 1, N + 1, glm::f32, glm::defaultp>& transformation) {
            Ray<N> result(*this);
            result *= transformation;
            return result;
        }

        Ray<N>& operator*=(const glm::mat<N + 1, N + 1, glm::f32, glm::defaultp>& transformation) {
            glm::vec<N + 1, float, glm::defaultp> originPlusOne{origin, 1.0f};
            glm::vec<N + 1, float, glm::defaultp> directionPlusOne{direction, 0.0f};
            origin = originPlusOne * transformation;
            direction = directionPlusOne * transformation;
            return *this;
        }

        void normalizeDirection() {
            direction = glm::normalize(direction);
        }
    };

    using Ray2 = Ray<2>;
    using Ray3 = Ray<3>;
}
