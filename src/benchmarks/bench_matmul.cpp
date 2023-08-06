#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
#include <Eigen/Eigen>

#define GENERATE_RANDOM_VEC3()                        \
    glm::vec3 {                                       \
        GENERATE(take(1, random(-1.f, 1.f))),         \
                GENERATE(take(1, random(-1.f, 1.f))), \
                GENERATE(take(1, random(-1.f, 1.f))), \
    }
#define GENERATE_RANDOM_VEC4()                        \
    glm::vec4 {                                       \
        GENERATE(take(1, random(-1.f, 1.f))),         \
                GENERATE(take(1, random(-1.f, 1.f))), \
                GENERATE(take(1, random(-1.f, 1.f))), \
                GENERATE(take(1, random(-1.f, 1.f))), \
    }

namespace bricksim {
    TEST_CASE("matrix_multiplication") {
        BENCHMARK_ADVANCED("glm")
        (Catch::Benchmark::Chronometer meter) {
            glm::vec4 p0 = GENERATE_RANDOM_VEC4();
            glm::mat4 m0 = {
                    GENERATE_RANDOM_VEC4(),
                    GENERATE_RANDOM_VEC4(),
                    GENERATE_RANDOM_VEC4(),
                    GENERATE_RANDOM_VEC4(),
            };
            meter.measure([&] { return p0 * m0; });
        };
        BENCHMARK_ADVANCED("eigen")
        (Catch::Benchmark::Chronometer meter) {

            meter.measure([&] {
                return 0;
            });
        };
    }
}
