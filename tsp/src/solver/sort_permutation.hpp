#pragma once

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

template <typename T, typename Less>
inline auto sort_permutation(
    const std::vector<T>& sorting_key,
    Less& less)
    -> std::vector<std::size_t>
{
    std::vector<std::size_t> permutation(sorting_key.size());

    std::iota(permutation.begin(), permutation.end(), 0);
    std::sort(permutation.begin(), permutation.end(), [&](std::size_t i, std::size_t j) {
        return less(sorting_key[i], sorting_key[j]);
    });

    return permutation;
}

template <typename T, typename Less>
inline auto partial_sort_permutation(
    const std::vector<T>& sorting_key,
    std::size_t n,
    Less& less)
    -> std::vector<std::size_t>
{
    std::vector<std::size_t> permutation(sorting_key.size());

    std::iota(permutation.begin(), permutation.end(), 0);
    std::partial_sort(permutation.begin(), permutation.begin() + n, permutation.end(), [&](std::size_t i, std::size_t j) {
        return less(sorting_key[i], sorting_key[j]);
    });

    return permutation;
}

template <typename T>
auto apply_permutation(
    std::vector<T>& to_permutate,
    const std::vector<std::size_t>& p)
    -> std::vector<T>
{
    std::size_t size = p.size();
    std::vector<T> sorted_vec(size);

    // std::transform(p.begin(), p.end(), sorted_vec.begin(), [&](std::size_t i) {
    //     return std::move(to_permutate[i]);
    // });

    for (std::size_t i {}; i < size; ++i ) {
        std::size_t ind = p[i];
        sorted_vec[i] = std::move(to_permutate[ind]);
    }

    return sorted_vec;
}
