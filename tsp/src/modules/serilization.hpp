#pragma once

#include "solver/matrix.hpp"

#include <cstdint>
#include <ostream>
#include <vector>

template <typename T>
auto operator<<(std::ostream& ost, const ds::heap_matrix<T>& matrix) -> std::ostream&
{
    ost << "{\n";
    for (uint64_t y = 0; y < matrix.size(); ++y) {
        for (uint64_t x = 0; x < matrix.size(); ++x) {

            ost << matrix.at(x, y) << ' ';
        }

        ost << "\n";
    }
    ost << "}";

    return ost;
}

template <typename T>
auto operator<<(std::ostream& ost, const std::vector<T>& vec) -> std::ostream&
{
    ost << "[";

    bool first = true;
    for (auto& it : vec) {
        if (first) {
            first = false;

        } else {
            ost << ", ";
        }

        ost << it;
    }

    ost << "]";

    return ost;
}