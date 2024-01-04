#include "../helpers/geometry.h"
#include <catch2/catch_all.hpp>

#define GENERATE_RANDOM_VEC()                         \
    glm::vec2 {                                       \
        GENERATE(take(1, random(-1.f, 1.f))),         \
                GENERATE(take(1, random(-1.f, 1.f))), \
    }

namespace bricksim {
    TEST_CASE("triangle_clockwise_check") {
        BENCHMARK_ADVANCED("discr")
                           (Catch::Benchmark::Chronometer meter) {
                                       glm::vec2 p0 = GENERATE_RANDOM_VEC();
                                       glm::vec2 p1 = GENERATE_RANDOM_VEC();
                                       glm::vec2 p2 = GENERATE_RANDOM_VEC();
                                       meter.measure([&] { return geometry::is2dTriangleClockwise(p0, p1, p2); });
                                   };
        BENCHMARK_ADVANCED("determinant")
                                 (Catch::Benchmark::Chronometer meter) {
                                             glm::vec2 p0 = GENERATE_RANDOM_VEC();
                                             glm::vec2 p1 = GENERATE_RANDOM_VEC();
                                             glm::vec2 p2 = GENERATE_RANDOM_VEC();
                                             meter.measure([&] {
                                                 const auto det = glm::determinant(glm::mat3{
                                                         glm::vec3(p0, 1.f),
                                                         glm::vec3(p1, 1.f),
                                                         glm::vec3(p2, 1.f),
                                                 });
                                                 return det < 0;
                                             });
                                         };
    }
}
