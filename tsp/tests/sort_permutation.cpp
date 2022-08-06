#include "../src/solver/sort_permutation.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

int main()
{
    using namespace std::literals::string_literals;
    std::vector<uint32_t> w = { 1, 0, 5, 3, 4, 2 };
    std::vector<std::string> d = { "asd"s, "dfs"s, "dgh"s, "ghn"s, "dtjh"s, "dgh"s };

    std::vector<uint64_t> expected_w = { 1, 0, 5, 3, 4, 2 };
    std::vector<std::string> expected_d = { "dfs"s, "asd"s, "dgh"s, "ghn"s, "dtjh"s, "dgh"s };

    auto k = [&](const uint32_t& l, const uint32_t r) {
        return l < r;
    };
    auto perm = sort_permutation(w, k);

    auto sorted = apply_permutation(d, perm);

    assert(perm == expected_w);
    assert(sorted == expected_d);
}