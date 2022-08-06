#pragma once

#include "config.hpp"
#include "path.hpp"
#include "solver/prdprinter.hpp"
#include "solver/sort_permutation.hpp"
#include "solver/surroundings.hpp"

#include "utils/thread_pool.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <random>
#include <set>
#include <stdexcept>
#include <thread>
#include <type_traits>

namespace tsp::solver::genetic {

namespace selection_operator {

    template <typename Rng>
    struct roulette_wheel_selection {
        static auto select(
            std::vector<config::path_type> const& population,
            std::vector<config::value_type> const& fitness_values,
            std::size_t choose_n,
            Rng& rng)
            -> std::vector<config::path_type const*>
        {
            std::vector<config::path_type const*> choosen_solutions {};
            choosen_solutions.resize(choose_n);

            std::size_t size = population.size();
            config::value_type sum {};
            for (std::size_t i {}; i < size; ++i) {
                sum += fitness_values[i];
            }

            std::vector<double> probs {};
            probs.resize(size);
            double probs_sum {};
            for (std::size_t i {}; i < size; ++i) {
                double prob = probs_sum + double(fitness_values[i]) / sum;
                probs[i] = prob;
                probs_sum += prob;
            }

            // wazone losowanie ze zwracaniem ?? czy moze lepiej nie zwracac
            std::uniform_real_distribution<> distr {};
            for (std::size_t n {}; n < choose_n; ++n) {
                double random = distr(rng);

                auto iter = std::lower_bound(probs.begin(), probs.end(), random);
                // musi znalezc bo zawiera wszystkie przedzialy z [0;1]
                assert(iter != probs.end());

                std::size_t elem_at = std::distance(probs.begin(), iter);
                choosen_solutions[n] = &population[elem_at];
            }

            return choosen_solutions;
        }
    };

    template <typename Rng, std::size_t TourneySize>
    struct tourney_selection {
        static auto select(
            std::vector<config::path_type> const& population,
            std::vector<config::value_type> const& fitness_values,
            std::size_t choose_n,
            Rng& rng)
            -> std::vector<config::path_type const*>
        {
            std::vector<config::path_type const*> selected_solutions(choose_n);

            auto size = population.size();
            std::uniform_int_distribution<std::size_t> distr(0, size - 1);
            for (decltype(choose_n) nth {}; nth < choose_n; ++nth) {
                std::array<std::size_t, TourneySize> inds {};
                for (std::size_t i {}; i < inds.size(); ++i) {
                    inds[i] = distr(rng);
                }

                std::size_t best = inds[0];
                for (std::size_t i = 1; i < inds.size(); ++i) {
                    if (fitness_values[best] < fitness_values[inds[i]]) {
                        best = inds[i];
                    }
                }

                selected_solutions[nth] = &population[best];
            }

            return selected_solutions;
        }
    };
}

template <typename MutationOperator>
inline auto create_initial_population(
    config::path_type const& solution,
    std::size_t population_size,
    auto& rng)
    -> std::vector<config::path_type>
{
    std::vector<config::path_type> population {};
    population.resize(population_size);

    for (std::size_t i {}; i < population_size; ++i) {
        population[i] = solution;
        MutationOperator::mutate(population[i], rng);
    }

    return population;
}

namespace crossover_operator {

    // struct partially_mapped {
    //     static auto crossover(const config::path_type& a, const config::path_type& b, auto& rng)
    //         -> const config::path_type
    //     {
    //         // zlozony obliczeniowo.. nie wiem czy chcemy...
    //     }
    // };

    /**
     * @brief kopiuje srodek przedzzialu, a potem zachowujac kolejnosc b wypelnia niepowtarzajacymi sie
     *
     */
    template <typename Rng>
    struct order {
        static auto crossover(const config::path_type& a, const config::path_type& b, Rng& rng)
            -> config::path_type
        {
            config::path_type child {};

            // cut pointsy
            std::size_t size = a.size() - 1; // bo powtorka poczatku
            assert(a.size() == b.size());
            assert(size >= 2);

            child.resize(size + 1);
            std::uniform_int_distribution<std::size_t> distr(0, size - 1);

            // WTF segfault na release GCC jak nie sa jako zmienne??? to jest zdecydowanie poprawne jezykowo
            // na clangu dziala normalnie na releasie
            // auto [ind_a, ind_b] = std::minmax(distr(rng), distr(rng));
            auto po = distr(rng);
            auto poo = distr(rng);
            auto [ind_a, ind_b] = std::minmax(po, poo);

            // assert(ind_a < size);
            // assert(ind_b < size);

            std::set<config::path_type::value_type> skip {};
            for (auto ind = ind_a; ind < ind_b; ++ind) {
                skip.insert(a[ind]);
            }

            for (auto ind = ind_a; ind < ind_b; ++ind) {
                child[ind] = a[ind];
            }

            auto child_ptr = ind_b;
            for (std::size_t ind = ind_b; ind < size; ++ind) {
                config::path_type::value_type city = b[ind];
                if (!skip.contains(city)) {
                    child[child_ptr] = city;
                    ++child_ptr;
                    if (child_ptr == size) {
                        child_ptr = 0;
                    }
                }
            }
            for (std::size_t ind = 0; ind < ind_b; ++ind) {
                config::path_type::value_type city = b[ind];
                if (!skip.contains(city)) {
                    child[child_ptr] = city;
                    ++child_ptr;
                    if (child_ptr == size) {
                        child_ptr = 0;
                    }
                }
            }

            child.back() = child.front(); // napraw koncowke

            // assert_path(child);
            return child;
        }
    };

    template <typename Rng>
    struct random_crossover {
        static constexpr std::array<double, 1>
            probs_ = {
                1.0
            };

        static constexpr std::array<config::path_type (*)(const config::path_type& a, const config::path_type& b, Rng& rng), 1>
            mutations_ = {
                order<Rng>::crossover
            };

        static auto crossover(const config::path_type& a, const config::path_type& b, Rng& rng)
        {
            std::uniform_real_distribution<> distr {};
            double p = distr(rng);

            auto iter = std::lower_bound(probs_.begin(), probs_.end(), p);
            if (iter != probs_.end()) {
                std::size_t ind = std::distance(probs_.begin(), iter);

                return mutations_[ind](a, b, rng);
            }

            throw std::runtime_error { "crossover wychodzi poza skale" };
        }
    };

}

namespace mutation_operator {

    /**
     * @brief mutacja swapujaca 2 losowe miasta miejscami
     *
     */
    struct twor_swap {
        static void mutate(config::path_type& chromosome, auto& rng)
        {
            std::size_t size = chromosome.size() - 1;
            assert(size >= 2);
            std::uniform_int_distribution<std::size_t> distr(0, size - 1);

            std::size_t ind_a = distr(rng);
            std::size_t ind_b = distr(rng);

            std::swap(chromosome[ind_a], chromosome[ind_b]);

            chromosome.back() = chromosome.front(); // napraw koncowke
            // assert_path(chromosome);
        }
    };

    /**
     * @brief mutacja dzielaca chromosom na 2 czesci i odwracajaca kolejnosc w tych czesciach
     *
     */
    struct centre_inverse {
        static void mutate(config::path_type& chromosome, auto& rng)
        {
            std::size_t size = chromosome.size() - 1;
            assert(size >= 2);
            std::uniform_int_distribution<std::size_t> distr(0, size - 1);

            std::size_t ind = distr(rng); // poczatek drugiej sekwencji
            std::reverse(chromosome.begin(), chromosome.begin() + ind);
            std::reverse(chromosome.begin() + ind, chromosome.begin() + size);

            chromosome.back() = chromosome.front(); // napraw koncowke
            // assert_path(chromosome);
        }
    };

    /**
     * @brief mutacja odwracajaca kolejnosc jakiegos srodkowego obszaru
     *
     */
    struct reverse_sequence {
        static void mutate(config::path_type& chromosome, auto& rng)
        {
            std::size_t size = chromosome.size() - 1;
            assert(size >= 2);
            std::uniform_int_distribution<std::size_t> distr(0, size);

            auto [ind_a, ind_b] = std::minmax(distr(rng), distr(rng));
            std::reverse(chromosome.begin() + ind_a, chromosome.begin() + ind_b);

            chromosome.back() = chromosome.front(); // napraw koncowke
            // assert_path(chromosome);
        }
    };

    template <typename Rng>
    struct random_mutation {
        static constexpr std::array<double, 3>
            probs_ = {
                0.6,
                0.85,
                1
            };

        static constexpr std::array<void (*)(config::path_type& chromosome, Rng& rng), 3>
            mutations_ = {
                twor_swap::mutate,
                centre_inverse::mutate,
                reverse_sequence::mutate
            };

        static void mutate(config::path_type& chromosome, Rng& rng)
        {
            std::uniform_real_distribution<> distr {};
            double p = distr(rng);

            auto iter = std::lower_bound(probs_.begin(), probs_.end(), p);
            if (iter != probs_.end()) {
                std::size_t ind = std::distance(probs_.begin(), iter);

                mutations_[ind](chromosome, rng);
            }
        }
    };
}

struct parameters {
    size_t population_size_ { 100 };
    uint64_t generations_number_ { 1000 }; // > 0
    double selection_factor_ { 0.5 };
    double elitysm_factor_ { 0.05 };
    size_t genetic_threads_ { 0 };
    double crossover_chance_ { 0.25 };
    double mutate_chance_ { 0.9 };
    double enchance_chance_ { 0.05 };
};

constexpr bool ignore_threads = false;

// TODO moze jakos wyprowadz je na zewnatrz tak aby dalo sie zmieniac je z argumentow
template </*typename MutationOperator, typename SelectionOperator, typename CrossoverOperator,*/ typename Rng>
struct solver {

    Rng rng_;

    using MutationOperator = mutation_operator::random_mutation<decltype(rng_)>;
    // using SelectionOperator = selection_operator::tourney_selection<decltype(rng_), 8>;
    using SelectionOperator = selection_operator::roulette_wheel_selection<decltype(rng_)>;
    using CrossoverOperator = crossover_operator::random_crossover<decltype(rng_)>;

    struct precomputed_parameters {
        size_t population_size_; // = reproduction_number_ + elitysm_number_
        uint64_t generations_;
        double selection_number_;
        double crossover_chance_;
        double mutation_chance_;
        double enchancement_chance_;
        size_t elitysm_number_;
        size_t reproduction_number_;
        size_t genetic_threads_;

        precomputed_parameters(parameters const& p)
        {
            // nie chce mi sie pisac ograniczania zakresu
            population_size_ = p.population_size_;
            generations_ = p.generations_number_;
            selection_number_ = p.population_size_ * p.selection_factor_;
            elitysm_number_ = p.population_size_ * p.elitysm_factor_;
            reproduction_number_ = population_size_ - elitysm_number_;
            genetic_threads_ = p.genetic_threads_ == 0 ? std::thread::hardware_concurrency() : p.genetic_threads_;
            crossover_chance_ = p.crossover_chance_;
            mutation_chance_ = p.mutate_chance_;
            enchancement_chance_ = p.enchance_chance_;
        }
    } const params_;

    solver(parameters const& params, Rng const& rng)
        : rng_ { rng }
        , params_(params)
    {
    }

    // ale to jest shitcode -- az sie nie poznaje
    auto operator()(
        const ds::heap_matrix<config::value_type>& matrix,
        const config::path_type& starting_path)
        -> config::path_type
    {
        config::path_type best_solution {};
        std::optional<config::value_type> best_value_opt {};

        utils::thread_pool pool { params_.genetic_threads_ };

        // Step 1. Create an initial population of P chromosomes.
        std::vector<config::path_type> population = create_initial_population<MutationOperator>(starting_path, params_.population_size_, rng_);

        for (uint64_t generation {}; generation < params_.generations_; ++generation) {

            // Step 2. Evaluate the fitness of each chromosome.
            std::vector<config::value_type> fitness_values(params_.population_size_);
            {
                // std::vector<std::future<void>> futures {};
                // futures.reserve(params_.population_size_);
                for (size_t i {}; i < params_.population_size_; ++i) {
                    auto evaluate_nth = [&fitness_values, &population, &matrix, i]() {
                        fitness_values[i] = calculate_value(matrix, population[i]);
                    };

                    // nie oplaca sie dodawac 2 punktow synchronizacji miedzy threadami dla liczenia tego wielowatkowo
                    // wykonuje sie dluzej
                    // if constexpr (ignore_threads) {
                    evaluate_nth();
                    // } else {
                    //     futures.push_back(pool.queue(evaluate_nth));
                    // }
                }
                // for (auto& future : futures) {
                //     future.wait();
                // }
            }

            // NOTE czesciowo posortuje populacje od najlepszego i odpowiadajacy fitness -- optymalizacja dla elitaryzmu
            // i dzieki temu tez mamy darmowy best
            {
                auto less = [](decltype(fitness_values)::value_type const& l, decltype(fitness_values)::value_type const& r) {
                    return l < r;
                };

                auto permutation = partial_sort_permutation(fitness_values, params_.elitysm_number_, less);
                population = apply_permutation(population, permutation);
                fitness_values = apply_permutation(fitness_values, permutation);
            }
            {
                if (!best_value_opt || fitness_values[0] < *best_value_opt) {
                    best_solution = population[0];
                    best_value_opt = fitness_values[0];

                    auto instance = prd_printer::instance();
                    if (instance) {
                        instance->print(generation, *best_value_opt);
                    }
                }
            }

            // Step 3. Choose P/2 parents from the current population via proportional selection.
            auto selected = SelectionOperator::select(population, fitness_values, params_.selection_number_, rng_);
            // NOTE roulette_wheel_selection nie da sie urownoleglic, musi stworzyc lookup table z prawdopodobienstwami
            // FIXME mozna preallocowac selected aby oszczedzic na allokacji

            // Step 4. Randomly select two parents to create offspring using crossover operator.
            // Step 5. Apply mutation operators for minor changes in the results.
            std::vector<config::path_type> reproduced(params_.reproduction_number_);
            {
                std::vector<std::future<void>> futures {};
                futures.reserve(params_.reproduction_number_);

                for (size_t i {}; i < params_.reproduction_number_; ++i) {

                    Rng thread_rng { rng_() };
                    auto reproduce_nth = [&matrix, &reproduced, &selected, this, thread_rng, i]() mutable {
                        std::uniform_int_distribution<> index_distr(0, params_.selection_number_ - 1);
                        std::uniform_real_distribution<> distr {};
                        if (distr(rng_) < params_.crossover_chance_) {
                            auto ind_a = index_distr(thread_rng);
                            auto ind_b = index_distr(thread_rng);

                            reproduced[i] = CrossoverOperator::crossover(*selected[ind_a], *selected[ind_b], thread_rng);
                        } else {
                            auto ind = index_distr(thread_rng);
                            reproduced[i] = *selected[ind];
                        }

                        if (distr(rng_) < params_.mutation_chance_) {
                            MutationOperator::mutate(reproduced[i], thread_rng);
                        }

                        if (distr(rng_) < params_.enchancement_chance_) {
                            config::path_type& current_path = reproduced[i];
                            config::value_type current_value = calculate_value(matrix, current_path);

                            config::path_type surrounding_path {};
                            surrounding_path.reserve(current_path.size());
                            config::value_type surrounding_value = current_value;

                            config::path_type best_path {};
                            best_path.reserve(current_path.size());
                            std::optional<config::value_type> best_value {};

                            surroundings::swap surrounding_generator { matrix, current_path, current_value, surrounding_path, surrounding_value };
                            while (surrounding_generator.valid()) {
                                surrounding_generator.calculate();

                                if (!best_value || (best_value && surrounding_value < *best_value)) {
                                    best_value = surrounding_value;
                                    best_path = surrounding_path;
                                }

                                surrounding_generator.next();
                            }

                            current_path = best_path;
                            // NOTE ignoruje znaleziony value
                        }
                    };
                    if constexpr (ignore_threads) {
                        reproduce_nth();
                    } else {
                        futures.push_back(pool.queue(reproduce_nth));
                    }
                }
                for (auto& future : futures) {
                    future.wait();
                }
            }

            // Step 6. Repeat Steps  4 and 5 until all parents are selected and mated.
            // jescze dodam elityzm
            std::vector<config::path_type> elityst(params_.elitysm_number_);
            {
                // dzieki czesciowemu posortowaniu na poczatku wystarczy ze nie usuniemy kilku pierwszych i mamy elityzm, ale i tak zrobie movem dla czytelnosci
                for (size_t i {}; i < params_.elitysm_number_; ++i) {
                    elityst[i] = std::move(population[i]);
                    // TODO optymalizacja mozna jakos tez skopiowac fitness elitystow zeby go nie rekalkulowac
                }
            }

            for (size_t i {}; i < params_.elitysm_number_; ++i) {
                population[i] = std::move(elityst[i]);
            }
            for (size_t i {}; i < params_.reproduction_number_; ++i) {
                population[i + params_.elitysm_number_] = std::move(reproduced[i]);
            }
        }
        return best_solution;
    }
};
}