#include "modules/demo.hpp"
#include "modules/python_export.hpp"

#include "solver/genetic.hpp"
#include "solver/matrix.hpp"
#include "solver/path.hpp"
#include "solver/solver.hpp"
#include "solver/surroundings.hpp"
#include "solver/taboo.hpp"
#include "tsp_data/parse_file.hpp"
#include "tsp_data/randomized.hpp"

#include "modules/serilization.hpp"

#include "utils/args_helper.hpp"
#include "utils/time_it.hpp"

#include "args.hpp"
#include "config.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

template <typename T>
auto initialize_matrix(const arguments& opts) -> ds::heap_matrix<T>
{
    auto parse_size = [](const std::string& size_repr) {
        uint64_t size {};
        if (!size_repr.empty()) {
            std::stringstream ss { size_repr };
            ss >> size;
        }

        return size;
    };

    if (opts.problem_ == "atsp") {
        auto size = parse_size(opts.problem_argument_);
        return tsp_data::randomized_atsp<T>(size, 5, 20);
        // TODO moze zakresy do generowania
    } else if (opts.problem_ == "tsp") {
        auto size = parse_size(opts.problem_argument_);
        return tsp_data::randomized_tsp<T>(size, 5, 20);
    } else if (opts.problem_ == "file") {
        std::stringstream content;

        std::ifstream file { opts.problem_argument_.c_str() };
        if (file.is_open()) {
            content << file.rdbuf();
            file.close();
        } else {
            throw std::runtime_error("initialize_matrix:: nie mozna otworzyc pliku!");
        }

        return tsp_data::parse<T>(content);
    } else {
        throw std::runtime_error(
            "initialize_matrix:: nie znany typ problemu \"" + opts.problem_ + "\"");
    }
}

template <typename T>
auto choose_primary_algorithm(const arguments& opts) -> std::function<std::vector<std::size_t>(ds::heap_matrix<T>)>
{
    using namespace std::placeholders;
    using namespace tsp;

    if (opts.algo_ == "k_random") {

        uint64_t k { 1 };
        {
            if (!opts.algo_option_.empty()) {
                std::stringstream ss { opts.algo_option_ };
                ss >> k;
            }
        }

        return std::bind(solver::k_random, _1, k);

    } else if (opts.algo_ == "nearest") {

        auto parse_position = [](const std::string& repr) {
            uint64_t pos { 0 };
            if (!repr.empty()) {
                std::stringstream ss { repr };
                ss >> pos;
            }

            return pos;
        };
        auto position = parse_position(opts.algo_option_);

        return std::bind(solver::nearest, _1, position);

    } else if (opts.algo_ == "nearest_ext") {
        return solver::nearest_ext;
    } else if (opts.algo_ == "2_opt" || opts.algo_ == "2_opt_sym" || opts.algo_ == "2_opt_swap") {

        auto&& algorithm_option = opts.algo_option_;
        auto choose_starting_path = [algorithm_option](ds::heap_matrix<T> const& matrix) -> std::vector<std::size_t> {
            if (algorithm_option == "asc" || algorithm_option == "") {
                return solver::example_path::monotonic(matrix);
            } else if (algorithm_option == "rand") {
                return solver::example_path::random(matrix);
            } else {
                throw std::runtime_error("nie znana opcja algorytmu 2_opt");
            }
        };

        if (opts.algo_ == "2_opt") {
            auto wrapper = [choose_starting_path](ds::heap_matrix<T> const& matrix) -> std::vector<std::size_t> {
                return solver::two_opt<solver::surroundings::asymetric_inverse>(matrix, choose_starting_path(matrix));
            };

            return wrapper;
        } else if (opts.algo_ == "2_opt_sym") {
            auto wrapper = [choose_starting_path](ds::heap_matrix<T> const& matrix) -> std::vector<std::size_t> {
                return solver::two_opt<solver::surroundings::symetric_inverse>(matrix, choose_starting_path(matrix));
            };

            return wrapper;
        } else if (opts.algo_ == "2_opt_swap") {
            auto wrapper = [choose_starting_path](ds::heap_matrix<T> const& matrix) -> std::vector<std::size_t> {
                return solver::two_opt<solver::surroundings::swap>(matrix, choose_starting_path(matrix));
            };

            return wrapper;
        }

    } else if (opts.algo_ == "monotonic") {
        return solver::monotonic;
    } else {
        throw std::runtime_error("niepoprawny algorytm!");
    }

    throw std::runtime_error("nie znaleziono algorytmu!");
}

template <typename T>
auto choose_algorithm(const arguments& opts) -> std::function<std::vector<std::size_t>(ds::heap_matrix<T>)>
{
    using namespace tsp::solver;

    auto fun = choose_primary_algorithm<T>(opts);
    if (!opts.execute_taboo_.empty()) {
        taboo_search::parameters params {};

        if (!opts.taboo_list_length_.empty()) {
            std::stringstream ss { opts.taboo_list_length_ };
            ss >> params.taboo_list_length_;
        }

        if (!opts.taboo_ignore_ratio_.empty()) {
            std::stringstream ss { opts.taboo_ignore_ratio_ };
            ss >> params.ignore_ratio_;
        }

        if (!opts.execute_max_depth_.empty()) {
            std::stringstream ss { opts.execute_max_depth_ };
            ss >> params.max_depth_;
        }

        if (!opts.execute_max_back_.empty()) {
            std::stringstream ss { opts.execute_max_back_ };
            ss >> params.max_back_;
        }

        if (opts.execute_taboo_ == "taboo_asym") {
            return [fun, params](ds::heap_matrix<T> const& matrix) -> std::vector<std::size_t> {
                taboo_search::solver<surroundings::asymetric_inverse> algorithm { params };
                return algorithm(matrix, fun(matrix));
            };
        }
        if (opts.execute_taboo_ == "taboo_sym") {
            return [fun, params](ds::heap_matrix<T> const& matrix) -> std::vector<std::size_t> {
                taboo_search::solver<surroundings::symetric_inverse> algorithm { params };
                return algorithm(matrix, fun(matrix));
            };
        }
        if (opts.execute_taboo_ == "taboo_swap") {
            return [fun, params](ds::heap_matrix<T> const& matrix) -> std::vector<std::size_t> {
                taboo_search::solver<surroundings::swap> algorithm { params };
                return algorithm(matrix, fun(matrix));
            };
        }

        throw std::runtime_error { "nie znaleziono odpowiadajacego algo taboo!" };
    }
    if (!opts.execute_genetic_.empty()) {
        genetic::parameters params {};

        if (!opts.population_size_.empty()) {
            std::stringstream ss { opts.population_size_ };
            ss >> params.population_size_;
        }

        if (!opts.generations_number_.empty()) {
            std::stringstream ss { opts.generations_number_ };
            ss >> params.generations_number_;
        }

        if (!opts.selection_factor_.empty()) {
            std::stringstream ss { opts.selection_factor_ };
            ss >> params.selection_factor_;
        }

        if (!opts.arch_btw_.empty()) {
            std::stringstream ss { opts.arch_btw_ };
            ss >> params.elitysm_factor_;
        }

        if (!opts.genetic_threads_.empty()) {
            std::stringstream ss { opts.genetic_threads_ };
            ss >> params.genetic_threads_;
        }

        if (!opts.crossover_chance_.empty()) {
            std::stringstream ss { opts.crossover_chance_ };
            ss >> params.crossover_chance_;
        }

        if (!opts.mutate_chance_.empty()) {
            std::stringstream ss { opts.mutate_chance_ };
            ss >> params.mutate_chance_;
        }

        if (!opts.enchance_chance_.empty()) {
            std::stringstream ss { opts.enchance_chance_ };
            ss >> params.enchance_chance_;
        }

        std::random_device dev {};
        std::mt19937_64 rng { dev() };
        if (opts.execute_genetic_ == "rand_oper") {
            return [fun, params, rng](ds::heap_matrix<T> const& matrix) -> std::vector<std::size_t> {
                // static_assert(!std::is_const<decltype(rng)>::value, "debil");
                genetic::solver algorithm { params, rng };
                return algorithm(matrix, fun(matrix));
            };
        }

        throw std::runtime_error { "nie znaleziono odpowiadajacego algo genetic!" };
    }

    return fun;
}

template <typename T>
auto create_runner(auto const& algorithm, const arguments& opts) -> std::function<std::vector<std::pair<std::vector<std::size_t>, T>>(ds::heap_matrix<T> const& matrix)>
{
    uint32_t thread_number { 1 };
    {
        std::stringstream ss { opts.runner_threads_ };
        ss >> thread_number;
    }

    if (thread_number == 0) {
        thread_number = std::thread::hardware_concurrency();
    }

    if (thread_number == 1) {
        return [&algorithm](ds::heap_matrix<T> const& matrix) -> std::vector<std::pair<std::vector<std::size_t>, T>> {
            std::vector<std::pair<std::vector<std::size_t>, T>> results;

            auto&& path = algorithm(matrix);
            auto value = tsp::calculate_value(matrix, path);

            results.emplace_back(path, value);

            return results;
        };
    }

    if (thread_number > 1) {
        return [&algorithm, thread_number](ds::heap_matrix<T> const& matrix) -> std::vector<std::pair<std::vector<std::size_t>, T>> {
            std::vector<std::pair<std::vector<std::size_t>, T>> results;
            results.resize(thread_number);

            std::vector<std::thread> threads {};

            for (uint32_t i {}; i < thread_number; ++i) {

                auto thread_function = [&, i]() {
                    auto&& path = algorithm(matrix);
                    auto value = tsp::calculate_value(matrix, path);

                    results[i] = { path, value };
                };

                threads.emplace_back(thread_function);
            }

            for (auto& thread : threads) {
                thread.join();
            }

            return results;
        };
    }

    throw std::runtime_error("threads hardware_concurency == 0");
}

template <typename T>
auto choose_best(std::vector<std::pair<std::vector<std::size_t>, T>> const& results) -> std::pair<std::vector<std::size_t>, T>
{
    auto mim_elem = std::min_element(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    return *mim_elem;
}

int main(int argc, char** argv)
{
    // std::random_device dev {};
    // std::srand(dev());

    arguments opts {};
    utils::args_helper parser = args::init_parser(opts);
    // tutaj bo help nie jest w optsach dziedziny programu
    bool help_request_ {};
    parser.set_boolean({ .write_to = help_request_, .symbol = "-h" });
    parser.set_boolean({ .write_to = help_request_, .symbol = "--help" });
    if (!parser.parse(argc, argv) || help_request_) {
        std::cout << parser.help_page() << "\n";
        return 0;
    }

    if (!opts.demo_.empty()) {
        demo::run(opts.demo_);

        return 0;
    }

    if (!opts.f_opt_.empty()) {
        config::value_type fopt_value {};
        std::stringstream ss { opts.f_opt_ };
        ss >> fopt_value;

        prd_printer::start(fopt_value);
    }

    const auto& matrix = initialize_matrix<config::value_type>(opts);
    if (opts.print_matrix_) {
        std::cout << "macierz odleglosci: " << matrix << "\n";
    }
    if (!opts.generate_file_.empty()) {
        std::ofstream file { opts.generate_file_ };
        if (!file.is_open()) {
            throw std::runtime_error { "nie mozna otworzyc pliku zapisu macierzy" };
        }
        tsp_data::exporting::full_matrix(matrix, file);
        return 0;
    }

    const auto algorithm = choose_algorithm<config::value_type>(opts);

    utils::time_it<std::chrono::milliseconds> timer {};

    auto runner = create_runner<config::value_type>(algorithm, opts);

    timer.set();
    auto results = runner(matrix);
    uint64_t execution_time = timer.measure();

    if (results.size() > 1) {
        std::cout << "szczegolowe wartosci dla threadow:\n";

        for (size_t i {}; i < results.size(); ++i) {
            std::cout << "thread " << i << " -> val = " << results[i].second << "\n";
        }
    }

    auto [path, value] = choose_best(results);

    std::cout << "obliczona trasa: " << path << "\n";
    std::cout << "czas obliczania trasy: " << execution_time << "ms"
              << "\n";
    std::cout << "obliczona funkcja celu: " << value << "\n";

    if (!opts.f_opt_.empty()) {
        config::value_type fopt_value {};
        std::stringstream ss { opts.f_opt_ };
        ss >> fopt_value;

        const auto prd = tsp::calculate_prd(value, fopt_value);
        std::cout << "obliczona wartosc PRD: " << prd << "\n";

        prd_printer::stop();
    }

    if (!opts.python_.empty() && opts.problem_ == "file") {
        python_export::euclidean_visualization(
            opts.problem_argument_, opts.python_,
            export_info<config::value_type> { path, execution_time, value });
    }
}

#include "solver/prdprinter.hpp"

void prd_printer::print(uint64_t iteration, config::value_type value)
{
    std::cout << iteration << ' ' << value << ' ' << tsp::calculate_prd(value, prd_) << '%' << std::endl;
}