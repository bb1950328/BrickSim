#pragma once
#include <Eigen/Eigen>
#include <glm/ext/quaternion_common.hpp>
#include <glm/glm.hpp>

namespace bricksim {
    template<typename S, int rows, int cols>
    Eigen::Matrix<S, rows, cols> glm2eigen(glm::mat<rows, cols, S> value) {
        Eigen::Matrix<S, rows, cols> result;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                result(r, c) = value[c][r];
            }
        }
        return result;
    }
    template<typename S, int size>
    Eigen::Matrix<S, size, 1> glm2eigen(glm::vec<size, S> value) {
        Eigen::Matrix<S, size, 1> result;
        for (int i = 0; i < size; ++i) {
            result(i, 0) = value[i];
        }
        return result;
    }

    template<typename S>
    Eigen::Quaternion<S> glm2eigen(glm::qua<S> value) {
        return Eigen::Quaternion<S>(value.w, value.x, value.y, value.z);
    }

    template<typename S, int rows, int cols>
    glm::mat<rows, cols, S> eigen2glm(Eigen::Matrix<S, rows, cols> value) {
        glm::mat<rows, cols, S> result;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                result[c][r] = value(r, c);
            }
        }
        return result;
    }

    template<typename S, int size>
    glm::vec<size, S> eigen2glm(Eigen::Matrix<S, size, 1> value) {
        glm::vec<size, S> result;
        for (int i = 0; i < size; ++i) {
            result[i] = value(i, 0);
        }
        return result;
    }

    template<typename S>
    glm::qua<S> eigen2glm(Eigen::Quaternion<S> value) {
        return glm::qua(value.w(), value.x(), value.y(), value.z());
    }
}
