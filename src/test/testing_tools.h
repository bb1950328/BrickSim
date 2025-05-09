#pragma once

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include <magic_enum/magic_enum.hpp>
#include <glm/gtx/io.hpp>

constexpr auto FLOAT_EPSILON = std::numeric_limits<float>::epsilon() * 100;

template<glm::length_t L, typename T, glm::qualifier Q>
class ApproxVec;

template<glm::length_t L, typename T, glm::qualifier Q>
class ApproxVec {
private:
    std::vector<Catch::Approx> approxes;
    glm::vec<L, T, Q> value;

public:
    explicit ApproxVec(const glm::vec<L, T, Q>& value) :
        value(value) {
        for (int i = 0; i < L; ++i) {
            approxes.push_back(Catch::Approx(value[i]));
        }
    }

    bool operator!=(const glm::vec<L, T, Q>& rhs) const {
        return !operator==(*this, rhs);
    }

    bool operator!=(const ApproxVec<L, T, Q>& rhs) const {
        return !operator==(*this, rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const ApproxVec& vec) {
        os << vec.value;
        return os;
    }

    template<glm::length_t Lx, typename Tx, glm::qualifier Qx>
    friend bool operator==(const ApproxVec<Lx, Tx, Qx>& lhs, const glm::vec<Lx, Tx, Qx>& rhs);

    template<glm::length_t Lx, typename Tx, glm::qualifier Qx>
    friend bool operator==(const ApproxVec<Lx, Tx, Qx>& lhs, const ApproxVec<Lx, Tx, Qx>& rhs);
};

template<glm::length_t L, typename T, glm::qualifier Q>
bool operator==(const ApproxVec<L, T, Q>& lhs, const glm::vec<L, T, Q>& rhs) {
    bool result = true;
    for (int i = 0; i < L; ++i) {
        result &= lhs.approxes[i] == rhs[i];
    }
    return result;
}

template<glm::length_t L, typename T, glm::qualifier Q>
bool operator==(const ApproxVec<L, T, Q>& lhs, const ApproxVec<L, T, Q>& rhs) {
    bool result = true;
    for (int i = 0; i < L; ++i) {
        result &= lhs.approxes[i] == rhs.value[i];
    }
    return result;
}

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
class ApproxMat;

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
class ApproxMat {
private:
    std::vector<Catch::Approx> approxes;
    glm::mat<C, R, T, Q> value;

public:
    explicit ApproxMat(const glm::mat<C, R, T, Q>& value) :
        value(value) {
        for (int i = 0; i < C; ++i) {
            for (int j = 0; j < R; ++j) {
                approxes.push_back(Catch::Approx(value[i][j]));
            }
        }
    }

    bool operator!=(const glm::mat<C, R, T, Q>& rhs) const {
        return !operator==(*this, rhs);
    }

    bool operator!=(const ApproxMat<C, R, T, Q>& rhs) const {
        return !operator==(*this, rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const ApproxMat& mat) {
        os << mat.value;
        return os;
    }

    template<glm::length_t Cx, glm::length_t Rx, typename Tx, glm::qualifier Qx>
    friend bool operator==(const ApproxMat<Cx, Rx, Tx, Qx>& lhs, const glm::mat<Cx, Rx, Tx, Qx>& rhs);
    template<glm::length_t Cx, glm::length_t Rx, typename Tx, glm::qualifier Qx>
    friend bool operator==(const ApproxMat<Cx, Rx, Tx, Qx>& lhs, const ApproxMat<Cx, Rx, Tx, Qx>& rhs);
};

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
bool operator==(const ApproxMat<C, R, T, Q>& lhs, const glm::mat<C, R, T, Q>& rhs) {
    bool result = true;
    for (int i = 0; i < C; ++i) {
        for (int j = 0; j < R; ++j) {
            result &= lhs.approxes[i * R + j] == rhs[i][j];
        }
    }
    return result;
}

template<glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
bool operator==(const ApproxMat<C, R, T, Q>& lhs, const ApproxMat<C, R, T, Q>& rhs) {
    bool result = true;
    for (int i = 0; i < C; ++i) {
        for (int j = 0; j < R; ++j) {
            result &= lhs.approxes[i * R + j] == rhs.value[i][j];
        }
    }
    return result;
}

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
    const auto it = std::min_element(input.begin(), input.end(), [](const auto& a, const auto& b) {
        float sumA = 1, sumB = 1;
        for (int i = 0; i < L; ++i) {
            sumA *= a[i];
            sumB *= b[i];
        }
        return sumA < sumB;
    });
    return reorderCircularList(input, it);
}

template<typename E>
struct MagicEnumGenerator : public Catch::Generators::IGenerator<E> {
    static_assert(magic_enum::enum_count<E>() > 0);

    const E& get() const override {
        return i;
    }

    bool next() override {
        const auto index = *magic_enum::enum_index(i) + 1ul;
        if (index >= magic_enum::enum_count<E>()) {
            return false;
        }
        i = magic_enum::enum_value<E>(index);
        return true;
    }

    [[nodiscard]] std::string stringifyImpl() const override {
        return std::string(magic_enum::enum_name(get()));
    }

protected:
    E i = magic_enum::enum_value<E>(0);
};

template<typename E>
Catch::Generators::GeneratorWrapper<E> enumGenerator() {
    return {Catch::Detail::make_unique<MagicEnumGenerator<E>>() /*new MagicEnumGenerator<E>()*/};
}
