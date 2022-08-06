#pragma once

#include "config.hpp"
#include "matrix.hpp"

#include <stdexcept>
#include <vector>

namespace tsp {

// template <typename config::value_type, typename Iter>
// auto calculate_value(const ds::heap_matrix<config::value_type>& matrix, Iter path_begin, Iter path_end) -> config::value_type
// {
//     config::value_type total_value {};

//     for (auto it = path_begin, next = it + 1; next < path_end; it = next++) {
//         auto&& from = *it;
//         auto&& to = *next;
//         auto&& value = matrix.at(from, to);

//         total_value += value;
//     }

//     return total_value;
// }

/**
 * @brief
 * UWAGA MA INNA KONWENCJE
 * @tparam config::value_type
 * @tparam Iter
 * @param matrix
 * @param begin
 * @param end
 * @param start iterator miasta z ktorego sie zaczyna
 * @param finish iterator miasta w ktorym sie konczy
 * @return config::value_type
 */
template <typename Iter>
auto calculate_value(const ds::heap_matrix<config::value_type>& matrix, Iter begin, Iter end, Iter start, Iter finish) -> config::value_type
{
    if (start < finish) {
        return calculate_value(matrix, start, finish + 1);
    }

    return calculate_value(matrix, start, end) + calculate_value(matrix, begin, finish + 1);
}

inline auto calculate_value(const ds::heap_matrix<config::value_type>& matrix, const config::path_type& path) -> config::value_type
{
    config::value_type total_value {};

    if (path.size() > 2) {
        auto from = path[0];
        for (std::size_t i = 1; i < path.size(); ++i) {
            auto to = path[i];
            auto&& value = matrix.at(from, to);

            from = to;
            total_value += value;
        }
    }

    return total_value;

    // return calculate_value(matrix, path.cbegin(), path.cend()); // wolniejsze??
    // return calculate_value(matrix, path.data(), path.data() + path.size()); // wolniejsze?? dlaczego pointery sa wolniejsze
}

inline auto calculate_prd(const config::value_type& value, const config::value_type& opt) -> double
{
    /*
    6. Obliczanie wartości PRD(x) dla rozwiązania x

    PRD(x) liczy się jako 100% * (f(x) - f(opt) / f(opt)

    gdzie f(x) to wartość funkcji celu dla rozwiązania x, a f(opt) to wartość funkcji celu rozwiązania optymalnego.
    */

    if (opt != config::value_type { 0 }) {
        return 100. * (static_cast<double>(value) - opt) / opt;
    }

    return 0.;
}

inline void assert_path(const config::path_type& path)
{
    std::vector<uint16_t> v(path.size());
    for (auto const& city : path) {
        ++v[city];
    }

    uint8_t _2 {};
    for (const auto& i : v) {
        if (i >= 2) {
            if (_2 > 0) {
                throw std::runtime_error { "assert path" };
            }
            ++_2;
        }
    }
}

}