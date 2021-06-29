#pragma once

#include <glm/gtx/io.hpp>
#include <catch2/catch.hpp>

constexpr auto FLOAT_EPSILON = std::numeric_limits<float>::epsilon() * 100;

template<glm::length_t L, typename T, glm::qualifier Q>
class ApproxVec;

template<glm::length_t L, typename T, glm::qualifier Q>
class ApproxVec {
private:
    std::vector<Approx> approxes;
    glm::vec<L, T, Q> value;

public:
    explicit ApproxVec(const glm::vec<L, T, Q>& value) :
        value(value) {
        for (int i = 0; i < L; ++i) {
            approxes.push_back(Approx(value[i]));
        }
    }

    bool operator==(const glm::vec<L, T, Q>& rhs) const {
        bool result = true;
        for (int i = 0; i < L; ++i) {
            result &= approxes[i] == rhs[i];
        }
        return result;
    }

    bool operator!=(const glm::vec<L, T, Q>& rhs) const {
        return !(rhs == *this);
    }

    friend std::ostream& operator<<(std::ostream& os, const ApproxVec& vec) {
        os << vec.value;
        return os;
    }
};
