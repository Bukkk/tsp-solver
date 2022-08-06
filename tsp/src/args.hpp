#pragma once

#include "utils/args_helper.hpp"

#include <string>

struct arguments {
    // positionals
    std::string problem_ {};
    std::string problem_argument_ {};
    std::string algo_ {};

    // optionals
    bool print_matrix_ {};
    std::string runner_threads_ {};

    std::string demo_ {};
    std::string generate_file_ {};
    std::string f_opt_ {};
    std::string python_ {};
    std::string algo_option_ {};

    // taboo
    std::string execute_taboo_ {};
    std::string taboo_list_length_ {};
    std::string taboo_ignore_ratio_ {};
    std::string execute_max_depth_ {};
    std::string execute_max_back_ {};

    // genetic
    std::string execute_genetic_ {};
    std::string population_size_ {};
    std::string generations_number_ {};
    std::string selection_factor_ {};
    std::string genetic_threads_ {};
    std::string arch_btw_ {};
    std::string crossover_chance_ {};
    std::string mutate_chance_ {};
    std::string enchance_chance_ {};
};

namespace args {

inline auto init_parser(arguments& opts) -> utils::args_helper
{
    utils::args_helper parser {
        "TSP solver\n"
        "\n"
        "usage:\n"
        "tsp-solver problem_type problem_argument algorithm [options]\n"
        "problem_type:\n"
        "                          atsp - generate random -- problem_argument size\n"
        "                          tsp  - generate random -- problem_argument size\n"
        "                          file - read problem from file\n"
        "algorithms:\n"
        "                        k_random - best path from k random permutations, default k = 1\n"
        "                        nearest - greedy nearest neighbour algorithm. chooses always closest city not yet visited. option -x which city comes first\n"
        "                        nearest_ext - extended nearest neighbour algorithm.\n"
        "                        2_opt - k_opt where k = 2 :), inverse surrounding general for Asymetric TSP\n"
        "                        2_opt_sym - 2_opt with inverse surrounding optimised for SYMETRIC TSP (won't work with ATSP)\n"
        "                        2_opt_swap - 2-opt with swap surrounding (good for both STSP AND ATSP)\n"
        "\n"
        "options:\n"
        "  -x algoritm_option -> \n"
        "                        k_random: k - amount of permutations\n"
        "                        nearest: id - id of city which comes first (starting from 0)\n"
        "                        nearest_ext: <no options available>\n"
        "                        2_opt: one of {asc, rand} - asc (ascending) - 0 1 2 3 .. n-1, rand (random) - <random path :0>\n"
        "  -h,--help          -> show this help screen\n"
        "  --threads          -> run in parallel, printing values calculated for each, and best path\n"
        "                     -> 0: all threads available, 1: one thread only, 2 : 2 threads and so on\n"
        "  -d demo_type       -> demo (you should not care about this. it was used only in early development)\n"
        "                        demo_type: <check in code :)>\n"
        "  -o optimum_value   -> optimal value f(opt) for given problem. if you know it, program will print some additional statistics, how good are its solutions\n"
        "  -p file_path       -> export euclides data to python file (to make some nice visualisations :))\n"
        "                        file_path - python file to write\n"
        "  -m                 -> print problem matrix -- you can see what distances between cities look like\n"
        "  -g file_path       -> save problem matrix as atsp fullmatrix file -- useful for generating random problems with atsp tsp problem_type.\n"
        "\n"
        "  -t taboo_version   -> add taboosearch to algorithm pipeline\n"
        "                        taboo_asym -- same as in 2_opt\n"
        "                        taboo_sym  -- same as in 2_opt\n"
        "                        taboo_swap -- same as in 2_opt\n"
        "  --taboo_list_length        uint   -> taboo list max entries number\n"
        "  --taboo_ignore_ratio       d[0,1] -> ignore taboo entry if value_new is better than ratio*best_value_now\n"
        "  --taboo_max_depth          uint   -> how many iterations taboo can look for better solutions without finding better solution\n"
        "  --taboo_max_back           uint   -> how many times taboo can jump back in a row\n"
        "\n"
        "  -G genetic_version -> add genetic to algorithm pipeline\n"
        "                        rand_oper -- its the only implemented version :)\n"
        "  --genetic_population_size  uint   -> genotypes in population\n"
        "  --genetic_generations      uint   -> how may generations shall algorithm simulate until it ends\n"
        "  --genetic_selection_factor d[0,1] -> factor of population that has a chance to procreate :)\n"
        "  --genetic_elitysm_factor   d[0,1] -> factor of population of best genes that survives to next generation\n"
        "  --genetic_threads          uint   -> amount of threads that cooperate for generic algorithm. 0 for all available\n"
        "  --genetic_crossover_chance d[0,1] -> chance that a new genotype will be created as a crossover of two genotypes from procreation pool, instead of copy of one from the same pool\n"
        "  --genetic_mutate_chance    d[0,1] -> chance that newly created genotype has a mutation\n"
        "  --genetic_enchance_chance  d[0,1] -> chance that a new genotype will be magically enchanced :)\n"
    };

    parser.set_positional({ .write_to = opts.problem_ });

    parser.set_positional({ .write_to = opts.problem_argument_ });
    parser.set_positional({ .write_to = opts.algo_ });

    parser.set_boolean({ .write_to = opts.print_matrix_, .symbol = "-m" });

    parser.set_optional({ .write_to = opts.runner_threads_, .symbol = "--threads" });

    parser.set_optional({ .write_to = opts.demo_, .symbol = "-d" });
    parser.set_optional({ .write_to = opts.generate_file_, .symbol = "-g" });
    parser.set_optional({ .write_to = opts.f_opt_, .symbol = "-o" });
    parser.set_optional({ .write_to = opts.python_, .symbol = "-p" });
    parser.set_optional({ .write_to = opts.algo_option_, .symbol = "-x" });

    parser.set_optional({ .write_to = opts.execute_taboo_, .symbol = "-t" });
    parser.set_optional({ .write_to = opts.taboo_list_length_, .symbol = "--taboo_list_length" });
    parser.set_optional({ .write_to = opts.taboo_ignore_ratio_, .symbol = "--taboo_ignore_ratio" });
    parser.set_optional({ .write_to = opts.execute_max_depth_, .symbol = "--taboo_max_depth" });
    parser.set_optional({ .write_to = opts.execute_max_back_, .symbol = "--taboo_max_back" });

    parser.set_optional({ .write_to = opts.execute_genetic_, .symbol = "-G" });
    parser.set_optional({ .write_to = opts.population_size_, .symbol = "--genetic_population_size" });
    parser.set_optional({ .write_to = opts.generations_number_, .symbol = "--genetic_generations" });
    parser.set_optional({ .write_to = opts.selection_factor_, .symbol = "--genetic_selection_factor" });
    parser.set_optional({ .write_to = opts.genetic_threads_, .symbol = "--genetic_threads" });
    parser.set_optional({ .write_to = opts.arch_btw_, .symbol = "--genetic_elitysm_factor" });
    parser.set_optional({ .write_to = opts.crossover_chance_, .symbol = "--genetic_crossover_chance" });
    parser.set_optional({ .write_to = opts.mutate_chance_, .symbol = "--genetic_mutate_chance" });
    parser.set_optional({ .write_to = opts.enchance_chance_, .symbol = "--genetic_enchance_chance" });

    return parser;
};

}
