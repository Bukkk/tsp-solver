#pragma once 

#include "config.hpp"

#include <cstdint>
#include <vector>
#include <ostream>

template<typename ValueType>
struct export_info {
    const std::vector<std::size_t>& path;
    const uint64_t& execution_time;
    const ValueType& total_value;
};

namespace python_export {
    void euclidean_visualization(
    const std::string& orgfile,
    const std::string& tofile,
    const export_info<config::value_type>& info);
}