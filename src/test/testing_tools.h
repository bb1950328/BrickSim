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

    bool operator==(const ApproxVec<L, T, Q>& rhs) const {
        bool result = true;
        for (int i = 0; i < L; ++i) {
            result &= approxes[i] == rhs.value[i];
        }
        return result;
    }

    bool operator!=(const glm::vec<L, T, Q>& rhs) const {
        return !this->operator==(rhs);
    }

    bool operator!=(const ApproxVec<L, T, Q>& rhs) const {
        return !this->operator==(rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const ApproxVec& vec) {
        os << vec.value;
        return os;
    }
};

template<glm::length_t L, typename T, glm::qualifier Q>
std::vector<ApproxVec<L, float, glm::defaultp>> convertToApproxVector(const std::vector<glm::vec<L, T, Q>>& a) {
    return std::vector<ApproxVec<L, float, glm::defaultp>>(a.begin(), a.end());
}

#define CHECK_VEC_VECTOR(a, b) CHECK(convertToApproxVector(a) == convertToApproxVector(b))

template<glm::length_t L>
std::vector<glm::vec<L, float, glm::defaultp>> reorderCircularList(const std::vector<glm::vec<L, float, glm::defaultp>>& input, const typename std::vector<glm::vec<L, float, glm::defaultp>>::const_iterator& it) {
    std::vector<glm::vec<L, float, glm::defaultp>> output;
    output.insert(output.end(), it, input.end());
    output.insert(output.end(), input.begin(), it);
    return output;
}

template<glm::length_t L>
std::vector<glm::vec<L, float, glm::defaultp>> reorderCircularList(const std::vector<glm::vec<L, float, glm::defaultp>>& input, long index) {
    return reorderCircularList(input, input.begin() + index);
}

template<glm::length_t L>
std::vector<glm::vec<L, float, glm::defaultp>> consistentStartOfCircularList(const std::vector<glm::vec<L, float, glm::defaultp>>& input) {
    const auto it = std::min_element(input.begin(), input.end(), [](const auto& a, const auto& b) { return glm::length2(a) > glm::length2(b); });
    return reorderCircularList(input, it);
}