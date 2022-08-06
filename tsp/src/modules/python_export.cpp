#include "python_export.hpp"
#include "config.hpp"
#include "tsp_data/parse_file.hpp"

#include <fstream>

namespace python_export {

namespace {
    template <typename T>
    void write_euclidean(
        std::ostream& export_to,
        const tsp_data::file_info& file_info,
        std::istream& content,
        const export_info<T>& info)
    {
        std::vector<double> xs {};
        std::vector<double> ys {};

        xs.reserve(file_info.dimension);
        ys.reserve(file_info.dimension);

        for (decltype(file_info.dimension) i = 0; i < file_info.dimension; ++i) {
            uint64_t id {};
            double x {}, y {};

            content >> id >> x >> y;
            // jak pokolei id to moge je olac

            xs.push_back(x);
            ys.push_back(y);
        }

        {
            export_to << "[";

            bool first = true;
            for (auto i = decltype(xs.size()) {}; i < xs.size(); ++i) {
                if (first) {
                    first = false;
                } else {
                    export_to << ",";
                }

                export_to << "(" + std::to_string(xs[i]) + "," + std::to_string(ys[i]) + ")";
            }

            export_to << "]";
        }

        export_to << "\n";

        {
            export_to << "[";

            bool first = true;
            for (auto i = decltype(info.path.size()) {}; i < info.path.size(); ++i) {
                if (first) {
                    first = false;
                } else {
                    export_to << ",";
                }

                export_to << std::to_string(info.path[i]);
            }

            export_to << "]";
        }

        // python_file << path; // nie chce spacji w pliku dlatego rozpisuje

        export_to << "\n";

        export_to << std::to_string(info.total_value);

        export_to << "\n";

        export_to << std::to_string(info.execution_time);

        export_to << "\n";
    }
}

void euclidean_visualization(
    const std::string& orgfile,
    const std::string& tofile,
    const export_info<config::value_type>& info)
{
    std::ifstream euclid_file { orgfile.c_str() };
    if (!euclid_file.is_open()) {
        throw std::runtime_error("main:: nie mozna otworzyc pliku podczas eksportowania do pythona!");
    }

    using namespace tsp_data;
    file_info file_info = parse_metadata<config::value_type>(euclid_file);

    if (file_info.type == file_info::format_type::euc2d) {
        std::ofstream python_file { tofile.c_str() };
        if (!python_file.is_open()) {
            throw std::runtime_error { "eksport:: nie mozna otworzyc pliku eksportu: " + tofile };
        }

        python_export::write_euclidean(python_file, file_info, euclid_file, info);
    }
    
    throw std::runtime_error { "eksport:: nie mozna wyeksportowac do pythona pliku o formacie innym niz euc2d" };
}

}
