#ifndef BRICKSIM_RAY_H
#define BRICKSIM_RAY_H

#include <glm/glm.hpp>

template <int N>
class Ray {
public:
    glm::vec<N, float, glm::defaultp> origin;
    glm::vec<N, float, glm::defaultp> direction;
    Ray(const glm::vec<N, float, glm::defaultp> &origin, const glm::vec<N, float, glm::defaultp> &direction) : origin(origin), direction(direction) {}
};

typedef Ray<2> Ray2;
typedef Ray<3> Ray3;

#endif //BRICKSIM_RAY_H
