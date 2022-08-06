#pragma once

#include "path.hpp"
#include "surroundings.hpp"
#include "config.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <stack>
#include <vector>
#include <deque>

namespace tsp::solver::taboo_search {

struct city_pair {
    // miasta musza byc rozne wiec 2 takie same oznaczaja pusty wpis
    std::size_t first_ {};
    std::size_t second_ {};

    bool operator==(const city_pair& other)
    {
        return (first_ == other.first_ && second_ == other.second_)
            || (first_ == other.second_ && second_ == other.first_);
    }

    // ale chyba nie ma potrzeby pisania is_empty bo nigdzize tego nie sprawdzam
};

struct taboo_list {
    size_t max_entries_;
    std::deque<city_pair> raw_ {};

    taboo_list(size_t max_entries)
        : max_entries_ { max_entries }
    {
    }

    bool is_inside(const city_pair& p)
    {
        auto found = std::find(raw_.begin(), raw_.end(), p);
        return found != raw_.end();
    }

    auto add(const city_pair& p) { // nie ma po co move dla city_pair
        if (raw_.size() < max_entries_) {
            raw_.push_back(p);
        } else {
            raw_.pop_front();
            raw_.push_back(p);
        }
    }
};

struct tree_entry {
    config::path_type path_;
    config::value_type value_;
    taboo_list taboo_list_;
    city_pair move_;
};

struct parameters {
    size_t taboo_list_length_ { 7 };
    double ignore_ratio_ { 0.9 };
    size_t max_depth_ { 25 };
    size_t max_back_ { 5 };
};

template <typename Surrounding>
struct solver {

    const parameters params_;

    solver(const parameters& params)
        : params_ { params }
    {
    }

    auto operator()(const ds::heap_matrix<config::value_type>& matrix, const config::path_type& starting_path) const -> config::path_type
    {
        // TODO popracowac nad nazwami bo sa troche mylace

        config::path_type best = starting_path;
        config::value_type best_len = calculate_value(matrix, starting_path);

        taboo_list list(params_.taboo_list_length_);
        std::stack<tree_entry> tree {};


        tree.push(tree_entry {
            .path_ = best,
            .value_ = best_len,
            .taboo_list_ = list,
            .move_ = {} });

        for (size_t times_back {}; times_back < params_.max_back_; ++times_back) {
            size_t since_tree_update = 0;

            if (tree.size() == 0) {
                return best;
            }
            config::path_type current_path = tree.top().path_;
            config::value_type current_value = tree.top().value_;

            while (since_tree_update < params_.max_depth_) {
                config::path_type best_path {};
                best_path.reserve(starting_path.size());
                std::optional<config::value_type> best_value {};
                city_pair best_pair {};

                config::path_type surrounding_path {};
                surrounding_path.reserve(starting_path.size());
                config::value_type surrounding_value = current_value;

                Surrounding surrounding_generator {
                    matrix,
                    current_path,
                    current_value,
                    surrounding_path,
                    surrounding_value
                };
                for (; surrounding_generator.valid(); surrounding_generator.next()) {
                    surrounding_generator.calculate();

                    city_pair surrounding_pair = { current_path[surrounding_generator.key.l], current_path[surrounding_generator.key.r] };

                    config::value_type ignore_value = static_cast<config::value_type>(*best_value * params_.ignore_ratio_);
                    if (surrounding_value < ignore_value) {
                        best_path = surrounding_path;
                        best_value = surrounding_value;
                        best_pair = surrounding_pair;
                    } else if (list.is_inside(surrounding_pair)) {
                        continue;
                    } else if (!best_value || (best_value && surrounding_value < *best_value)) {
                        best_path = surrounding_path;
                        best_value = surrounding_value;
                        best_pair = surrounding_pair;
                    }
                }

                if (since_tree_update == 0) {
                    tree.top().move_ = best_pair;
                }

                if (best_value < tree.top().value_) {
                    tree.push(tree_entry { 
                        .path_ = best_path,
                        .value_ = *best_value,
                        .taboo_list_ = list,
                        .move_ = {} });

                    since_tree_update = 0;
                } else{
                    ++since_tree_update;
                }

                if (*best_value < best_len) {
                    best_len = *best_value;
                    best = best_path;
                }

                list.add(best_pair);
                current_path = best_path;
                current_value = *best_value;
            }

            auto&& top = tree.top();
            current_path = top.path_;
            current_value = top.value_;
            list = top.taboo_list_;
            list.add(top.move_);

            tree.pop();
        }
        return best;
    }
};

}