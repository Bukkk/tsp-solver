#pragma once

#include "../solver/matrix.hpp"

#include <random>

namespace tsp_data {
template <typename ValueType>
auto randomized_atsp(const uint64_t size, const uint64_t min, const uint64_t max) -> ds::heap_matrix<ValueType>
{
    ds::heap_matrix<ValueType> matrix { size };

    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<> distribution(min, max);

    for (std::size_t y {}; y < matrix.size(); ++y) {
        for (decltype(y) x = 0; x < matrix.size(); ++x) {
            if (x != y) {
                matrix.at(x, y) = distribution(generator);
            } else {
                matrix.at(x, y) = 0;
            }
        }
    }

    return matrix;
}

template <typename ValueType>
auto randomized_tsp(const uint64_t size, const uint64_t min, const uint64_t max) -> ds::heap_matrix<ValueType>
{
    ds::heap_matrix<ValueType> matrix { size };

    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<> distribution(min, max);

    for (std::size_t y {}; y < matrix.size(); ++y) {
        for (decltype(y) x = 0; x < y; ++x) {
            auto val = distribution(generator);

            matrix.at(x, y) = val;
            matrix.at(y, x) = val;
        }
    }

    return matrix;
}
}