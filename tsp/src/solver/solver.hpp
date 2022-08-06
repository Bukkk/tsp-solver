#include "matrix.hpp"
#include "path.hpp"
#include "surroundings.hpp"
#include "config.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace tsp::solver {

namespace example_path {

    inline auto random(const ds::heap_matrix<config::value_type>& matrix) -> std::vector<std::size_t>
    {
        std::random_device device;
        std::mt19937 twister(device());

        std::vector<std::size_t> path {};
        path.reserve(matrix.size() + 1);

        for (std::size_t i {}; i < matrix.size(); ++i) {
            path.push_back(i);
        }

        std::shuffle(path.begin(), path.end(), twister);
        path.push_back(path[0]);

        return path;
    }

    inline auto monotonic(const ds::heap_matrix<config::value_type>& matrix) -> std::vector<std::size_t>
    {
        std::vector<std::size_t> path {};
        path.reserve(matrix.size() + 1);

        for (std::size_t i {}; i < matrix.size(); ++i) {
            path.push_back(i);
        }
        path.push_back(path[0]);

        return path;
    }

}

/**
 * @brief przykladowy algorytm (definiuje interfejs solverow)
 *
 * @tparam config::value_type
 * @param matrix
 * @return std::vector<config::value_type> trasa np {0, 1, 2, 3, 0};
 */
inline auto monotonic(const ds::heap_matrix<config::value_type>& matrix) -> std::vector<std::size_t>
{
    return example_path::monotonic(matrix);
}

inline auto k_random(const ds::heap_matrix<config::value_type>& matrix, uint64_t k) -> std::vector<std::size_t>
{
    std::random_device device;
    std::mt19937 twister(device());

    std::vector<std::size_t> indices {};
    indices.reserve(matrix.size() + 1);

    for (std::size_t i {}; i < matrix.size(); ++i) {
        indices.push_back(i);
    }

    decltype(indices) best {};
    std::optional<config::value_type> best_value {};
    for (std::size_t i {}; i < k; ++i) {
        auto path { indices }; // alokacja kopia i move vs 1 kopia i potencjalnie druga kopia -- tak chyba lepiej ??

        std::shuffle(path.begin(), path.end(), twister);
        path.push_back(path[0]);

        auto value = calculate_value(matrix, path);
        if ((best_value && value < *best_value) || !best_value) {
            best_value = value;
            best = std::move(path);
        }
    }

    if (!best_value) {
        throw std::runtime_error("nie znaleziono besta. nie chce mi sie pisac obslugi bledow");
    }
    return best;
}

inline auto nearest(const ds::heap_matrix<config::value_type>& matrix, const std::size_t starting_position) -> std::vector<std::size_t>
{
    if (!(starting_position < matrix.size())) {
        throw std::invalid_argument("taka pozycja startowa nie istnieje!");
    }

    std::vector<std::size_t> path {};
    path.reserve(matrix.size() + 1);
    path.push_back(starting_position);
    std::set<std::size_t> already_visited {};
    already_visited.insert(starting_position);

    auto position = starting_position;
    for (size_t num { 1 }; num < matrix.size(); ++num) {
        std::optional<config::value_type> closest_value {};
        std::size_t closest_position {};
        for (std::size_t i {}; i < matrix.size(); ++i) {
            if (i == position || already_visited.find(i) != already_visited.end()) { // tak jest 3x szybciej dla p654
                // if (i == position || std::find(path.begin(), path.end(), i) != path.end()) {
                continue;
            }

            auto value = matrix.at(position, i);
            if (!closest_value || (closest_value && value < *closest_value)) {
                closest_value = value;
                closest_position = i;
            }
        }

        path.push_back(closest_position);
        already_visited.insert(closest_position);
        position = closest_position;
    }

    path.push_back(starting_position);

    return path;
}

inline auto nearest_ext(const ds::heap_matrix<config::value_type>& matrix) -> std::vector<std::size_t>
{
    std::optional<config::value_type> best_value {};
    std::vector<std::size_t> best_path {};
    for (std::size_t i {}; i < matrix.size(); ++i) {

        auto path = nearest(matrix, i);
        auto value = calculate_value(matrix, path);

        if (!best_value || (best_value && value < *best_value)) {
            best_value = value;
            best_path = std::move(path);
        }
    }

    return best_path;
}

template <typename Surrounding>
auto two_opt(ds::heap_matrix<config::value_type> const& matrix, const std::vector<std::size_t>& starting_path) -> std::vector<std::size_t>
{
    config::path_type current_path = starting_path;
    config::value_type current_value = calculate_value(matrix, starting_path);

    config::path_type best_path {};
    best_path.reserve(starting_path.size());
    std::optional<config::value_type> best_value {};

    // memory resource -- zeby rzadziej allocowac
    config::path_type surrounding_path {};
    surrounding_path.reserve(starting_path.size());
    config::value_type surrounding_value = current_value;

    while (true) {

        Surrounding surrounding_generator { matrix, current_path, current_value, surrounding_path, surrounding_value };
        while (surrounding_generator.valid()) {
            surrounding_generator.calculate();

            if (!best_value || (best_value && surrounding_value < *best_value)) {
                best_value = surrounding_value;
                best_path = surrounding_path;
            }

            surrounding_generator.next();
        }

        if (best_value && *best_value < current_value) {
            current_path = best_path;
            current_value = *best_value;
        } else {
            return current_path;
        };
    }
}
}