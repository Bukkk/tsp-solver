#pragma once

#include "matrix.hpp"
#include "path.hpp"
#include "config.hpp"

#include <vector>

namespace tsp::solver::surroundings {

struct surrounding_key {
    std::size_t l { 0 };
    std::size_t r { 1 };

    void next()
    {
        if (l + 1 < r) {
            ++l;
        } else {
            ++r;
            l = 0;
        }
    }
};

class asymetric_inverse {

    ds::heap_matrix<config::value_type> const& matrix_;

    config::path_type const& solution_;
    config::value_type const& solution_value_ [[maybe_unused]];

    config::path_type& surrounding_;
    config::value_type& surrounding_value_;

public:
    surrounding_key key {};

    asymetric_inverse(
        const ds::heap_matrix<config::value_type>& matrix,
        const config::path_type& current_path,
        const config::value_type& current_path_value,
        config::path_type& surrounding_space,
        config::value_type& current_value)
        : matrix_ { matrix }
        , solution_ { current_path }
        , solution_value_ { current_path_value }
        , surrounding_ { surrounding_space }
        , surrounding_value_ { current_value }
    {
        surrounding_ = solution_;
    }

    asymetric_inverse() = delete;

    void next()
    {
        key.next();
    }

    auto valid() -> bool
    {
        // ostatni w pathu to start
        return key.r < solution_.size() - 1;
    }

    void calculate()
    {
        auto&& fb = solution_.begin();
        auto&& fe = solution_.begin() + key.l;
        auto&& mb = solution_.rend() - 1 - key.r;
        auto&& me = solution_.rend() - key.l;
        auto&& lb = solution_.begin() + key.r + 1;
        auto&& le = solution_.end() - 1;

        // auto last_middle_value = calculate_value(matrix_, surrounding_.begin(), surrounding_.end(), fe - 1, lb);

        // auto beg =solution_.begin() ;

        // auto i = update_from_index;
        // while (i <= update_to_index && fb <= beg + i && beg + i < fe) {
        //     surrounding_[i] = solution_[i];
        //     ++i;
        // }
        // while (i <= update_to_index && mb_for <= beg + i && beg + i < me_for) {
        //     surrounding_[i] = solution_[r - (i - l)];
        //     ++i;
        // }
        // while (i <= update_to_index && lb < beg + i && beg < le) {
        //     surrounding_[i] = solution_[i];
        //     ++i;
        // }
        // surrounding_.back() = surrounding_.front();

        // taniej jest skopiowac wszystko niz przeprowadzic logike co skopiowac
        surrounding_.clear();
        surrounding_.insert(surrounding_.end(), fb, fe);
        surrounding_.insert(surrounding_.end(), mb, me);
        surrounding_.insert(surrounding_.end(), lb, le);
        surrounding_.push_back(surrounding_.front());

        // tamte sa zinvalidowane
        // auto&& fe_2 = solution_.begin() + l;
        // auto&& lb_2 = solution_.begin() + r + 1;
        // auto middle_value = calculate_value(matrix_, surrounding_.begin(), surrounding_.end(), fe_2 - 1, lb_2);

        // surrounding_value_ = surrounding_value_ - last_middle_value + middle_value;

        surrounding_value_ = calculate_value(matrix_, surrounding_);
    }
};

class symetric_inverse {

    ds::heap_matrix<config::value_type> const& matrix_;

    config::path_type const& solution_;
    config::value_type const& solution_value_;

    config::path_type& surrounding_;
    config::value_type& surrounding_value_;

public:
    surrounding_key key {};

    symetric_inverse(
        const ds::heap_matrix<config::value_type>& matrix,
        const config::path_type& current_path,
        const config::value_type& current_path_value,
        config::path_type& surrounding_space,
        config::value_type& current_value)
        : matrix_ { matrix }
        , solution_ { current_path }
        , solution_value_ { current_path_value }
        , surrounding_ { surrounding_space }
        , surrounding_value_ { current_value }
    {
        surrounding_ = solution_;
    }

    symetric_inverse() = delete;

    void next()
    {
        key.next();
    }

    auto valid() -> bool
    {
        // ostatni w pathu to start
        return key.r < solution_.size() - 1;
    }

    void calculate()
    {
        auto&& fb = solution_.begin();
        auto&& fe = solution_.begin() + key.l;
        auto&& mb = solution_.rend() - 1 - key.r;
        auto&& me = solution_.rend() - key.l;
        auto&& lb = solution_.begin() + key.r + 1;
        auto&& le = solution_.end() - 1;

        // taniej jest skopiowac wszystko niz przeprowadzic logike co skopiowac
        surrounding_.clear();
        surrounding_.insert(surrounding_.end(), fb, fe);
        surrounding_.insert(surrounding_.end(), mb, me);
        surrounding_.insert(surrounding_.end(), lb, le);
        surrounding_.push_back(surrounding_.front());

        auto sol_size = solution_.size() - 1;
        size_t l_from = key.l == 0 ? sol_size - 1 : key.l - 1;
        size_t l_to = key.l;
        size_t r_from = key.r;
        size_t r_to = key.r == sol_size ? 0 : key.r + 1;

        surrounding_value_ = solution_value_
            + matrix_.at(surrounding_[l_from], surrounding_[l_to])
            - matrix_.at(solution_[l_from], solution_[l_to])
            + matrix_.at(surrounding_[r_from], surrounding_[r_to])
            - matrix_.at(solution_[r_from], solution_[r_to]);
    }
};

class swap {
    surrounding_key last_key {};

    ds::heap_matrix<config::value_type> const& matrix_;

    config::path_type const& solution_;
    config::value_type const& solution_value_;

    config::path_type& surrounding_;
    config::value_type& surrounding_value_;

public:
    surrounding_key key {};
    
    swap(
        const ds::heap_matrix<config::value_type>& matrix,
        const config::path_type& current_path,
        const config::value_type& current_path_value,
        config::path_type& surrounding_space,
        config::value_type& current_value)
        : matrix_ { matrix }
        , solution_ { current_path }
        , solution_value_ { current_path_value }
        , surrounding_ { surrounding_space }
        , surrounding_value_ { current_value }
    {
        surrounding_ = solution_;
        std::swap(surrounding_[key.l], surrounding_[key.r]);
        surrounding_.back() = surrounding_.front();
    }

    swap() = delete;

    void next()
    {
        last_key = key;
        key.next();
    }

    auto valid() -> bool
    {
        // ostatni w pathu to start
        return key.r < solution_.size() - 1;
    }

    void calculate()
    {
        std::swap(surrounding_[last_key.l], surrounding_[last_key.r]);
        std::swap(surrounding_[key.l], surrounding_[key.r]);
        surrounding_.back() = surrounding_.front();

        auto l = key.l;
        auto r = key.r;

        auto sol_size = solution_.size() - 1;
        if (r - l > 1) {
            size_t l_left = l == 0 ? sol_size - 1 : l - 1;
            size_t l_right = l == sol_size ? 0 : l + 1;
            size_t r_left = r == 0 ? sol_size - 1 : r - 1;
            size_t r_right = r == sol_size ? 0 : r + 1;

            surrounding_value_ = solution_value_
                + matrix_.at(surrounding_[l_left], surrounding_[l])
                + matrix_.at(surrounding_[l], surrounding_[l_right])
                - matrix_.at(solution_[l_left], solution_[l])
                - matrix_.at(solution_[l], solution_[l_right])
                + matrix_.at(surrounding_[r_left], surrounding_[r])
                + matrix_.at(surrounding_[r], surrounding_[r_right])
                - matrix_.at(solution_[r_left], solution_[r])
                - matrix_.at(solution_[r], solution_[r_right]);
        } else {
            size_t l_left = l == 0 ? sol_size - 1 : l - 1;
            size_t r_right = r == sol_size ? 0 : r + 1;

            surrounding_value_ = solution_value_
                + matrix_.at(surrounding_[l_left], surrounding_[l])
                + matrix_.at(surrounding_[l], surrounding_[r])
                + matrix_.at(surrounding_[r], surrounding_[r_right])
                - matrix_.at(solution_[l_left], solution_[l])
                - matrix_.at(solution_[l], solution_[r])
                - matrix_.at(solution_[r], solution_[r_right]);
        }
    }
};

}