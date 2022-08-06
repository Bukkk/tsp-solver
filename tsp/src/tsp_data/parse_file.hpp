#pragma once

#include "../solver/matrix.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tsp_data {

namespace parsing {
    template <typename ValueType>
    void euclidean(std::istream& ss, ds::heap_matrix<ValueType>& matrix)
    {
        auto size = matrix.size();

        std::vector<double> xs {};
        std::vector<double> ys {};

        xs.reserve(size);
        ys.reserve(size);

        for (decltype(size) i = 0; i < size; ++i) {
            uint64_t id {};
            double x {}, y {};

            ss >> id >> x >> y;
            // jak pokolei id to moge je olac

            xs.push_back(x);
            ys.push_back(y);
        }

        for (decltype(size) to = 0; to < size; ++to) {
            for (decltype(size) from = 0; from < size; ++from) {
                auto dx = xs[to] - xs[from];
                auto dy = ys[to] - ys[from];

                ValueType distance { static_cast<ValueType>(std::sqrt(dx * dx + dy * dy) + 0.5) };

                matrix.at(from, to) = distance;
                // matrix.at(to, from) = distance; // myslenie jest trudne
            }
        }
    }

    template <typename ValueType>
    void lower_diag(std::istream& ss, ds::heap_matrix<ValueType>& matrix)
    {
        auto size = matrix.size();

        for (decltype(size) y = 0; y < size; ++y) {
            for (decltype(y) x = 0; x <= y; ++x) {
                ValueType val {};
                ss >> val;
                matrix.at(x, y) = val;
                matrix.at(y, x) = val;
            }
        }
    }

    template <typename ValueType>
    void full_matrix(std::istream& ss, ds::heap_matrix<ValueType>& matrix)
    {
        auto size = matrix.size();

        for (decltype(size) y = 0; y < size; ++y) {
            for (decltype(y) x = 0; x < size; ++x) {
                ValueType val {};
                ss >> val;
                matrix.at(x, y) = val;
            }
        }
    }
}

namespace exporting {
    template <typename ValueType>
    void full_matrix(ds::heap_matrix<ValueType> const& matrix, std::ostream& ost)
    {
        ost << "DIMENSION: " << std::to_string(matrix.size()) << '\n';
        ost << "EDGE_WEIGHT_TYPE EXPLICIT" << '\n';
        ost << "EDGE_WEIGHT_FORMAT FULL_MATRIX" << '\n';
        ost << "EDGE_WEIGHT_SECTION" << '\n';

        for (std::size_t y {}; y < matrix.size(); ++y) {
            for (std::size_t x {}; x < matrix.size(); ++x) {
                ost << std::to_string(matrix.at(x, y)) << ' ';
            }
            ost << "\n";
        }
    }
}

struct file_info {
    uint64_t dimension {};

    enum class format_type {
        error,
        euc2d,
        lower_diag,
        full_matrix
    } type { format_type::error };
};

template <typename ValueType>
auto parse_metadata(std::istream& ss) -> file_info
{
    file_info info {};

    while (true) {
        std::string line {};
        std::getline(ss, line, '\n');

        if (line.find("DIMENSION") != std::string::npos) {

            // czasem jest spacja przed : dlatego tak
            auto at_colon = std::find(line.begin(), line.end(), ':');
            if (at_colon != line.end()) {
                std::string num_string { at_colon + 1, line.end() };
                std::stringstream num_stream { num_string };
                num_stream >> info.dimension;
            }

        } else if (line.find("EDGE_WEIGHT_TYPE") != std::string::npos) {
            // w linijce z EDGE_WEIGHT_TYPE wystepuje EUC_2D lub EXPLICIT

            if (info.dimension == 0) {
                throw std::runtime_error { "parse:: nie wczytano wielkosci, ale natknieto sie na typ "
                                           "(czy w tym pliku wielkosc jest za typem?)" };
            }

            if (line.find("EUC_2D") != std::string::npos) {
                // w tym wypadku dane wystepuja po linijce z NODE_CORD_SECTION
                info.type = file_info::format_type::euc2d;

                while (true) {
                    std::getline(ss, line, '\n');
                    if (line.find("NODE_COORD_SECTION") != std::string::npos) {
                        // jezeli mam juz wymiary to nie potrzebuje eofa zeby wiedziec gdzie jest koniec
                        return info;
                    }
                }
            } else if (line.find("EXPLICIT") != std::string::npos) {
                // w przypadku EXPLICIT patrzymy na 6 linijke i chcemy mieć LOWER_DIAG_ROW lub FULL_MATRIX
                while (true) {
                    std::getline(ss, line, '\n');
                    if (line.find("EDGE_WEIGHT_FORMAT") != std::string::npos) {
                        break;
                    }
                }

                if (line.find("LOWER_DIAG_ROW") != std::string::npos) {
                    info.type = file_info::format_type::lower_diag;

                    while (true) {
                        std::getline(ss, line, '\n');
                        if (line.find("EDGE_WEIGHT_SECTION") != std::string::npos) {
                            break;
                        }
                    }
                    return info;

                } else if (line.find("FULL_MATRIX") != std::string::npos) {
                    info.type = file_info::format_type::full_matrix;

                    while (true) {
                        std::getline(ss, line, '\n');
                        if (line.find("EDGE_WEIGHT_SECTION") != std::string::npos) {
                            break;
                        }
                    }
                    return info;
                }
            } else {
                throw std::runtime_error { "parse:: nie znaleziono EUC_2D lub EXPLICIT w lnijce: " + line };
            }
        }
    }
}

template <typename ValueType>
auto parse(std::istream& ss) -> ds::heap_matrix<ValueType>
{

    file_info info = parse_metadata<ValueType>(ss);

    ds::heap_matrix<ValueType> matrix { info.dimension };
    switch (info.type) {
    case file_info::format_type::euc2d: {
        parsing::euclidean(ss, matrix);
    } break;
    case file_info::format_type::full_matrix: {
        parsing::full_matrix(ss, matrix);
    } break;
    case file_info::format_type::lower_diag: {
        parsing::lower_diag(ss, matrix);
    } break;

    default: {
        throw std::runtime_error { "parse:: nieznany typ danych pliku" };
    }
    }

    return matrix;
}
}