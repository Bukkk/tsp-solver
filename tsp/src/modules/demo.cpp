#include "demo.hpp"

#include "tsp_data/randomized.hpp"
#include "serilization.hpp"

#include <iostream>

namespace demo {

void atsp()
{
    std::cout << "demo generowanie i printowanie atsp\n";
    auto const matrix = tsp_data::randomized_atsp<uint64_t>(10, 5, 20);
    std::cout << matrix << "\n";
}

void tsp()
{
    std::cout << "demo generowanie i printowanie tsp\n";
    auto const matrix = tsp_data::randomized_tsp<uint64_t>(10, 5, 20);
    std::cout << matrix << "\n";
}

void matrix()
{
    std::cout << "demo generowanie i printowanie macierzy odleglosci\n";

    ds::heap_matrix<uint64_t> matrix { 5 };

    matrix.at(0, 0) = 1;
    matrix.at(0, 2) = 2;
    matrix.at(3, 0) = 3;
    matrix.at(4, 4) = 4;

    std::cout << matrix << "\n";
}

void run(std::string const& demoarg)
{
    if (demoarg == "atsp") {
        demo::atsp();
    } else if (demoarg == "tsp") {
        demo::tsp();
    } else if (demoarg == "matrix") {
        demo::matrix();
    } else {
        throw std::runtime_error { "nie znana opcja demo!" };
    }
}

}