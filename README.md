# Solver Traveling Salesman Problem

##### quick start:

1. clone repository
2. ```cd uni-meta/tsp```
3. ```meson setup --buildtype release build``` -- to configure build
4. ```ninja -C build``` -- to compile

##### then you can run the program:
```./build/src/tsp-solver```

##### examples

```./build/src/tsp-solver file data/STSP-EUC_2D/pr439.tsp 2_opt_sym``` -- solve file pr439.tsp with algorithm 2_opt_sym

```./build/src/tsp-solver file data/STSP-EUC_2D/pr439.tsp 2_opt_sym -x rand --threads 0``` -- use all of the threads in your processor to calculate and return best 2_opt_sym path starting from random path permutation

```./build/src/tsp-solver file data/STSP-EUC_2D/pr439.tsp nearest_ext -t taboo_swap``` -- use taboo search

```./build/src/tsp-solver file data/STSP-EUC_2D/pr439.tsp nearest_ext -G rand_oper -o 107217 --genetic_generations 10000``` -- use genetic algorithm and print some statistics. dont forget to tune its options :)

##### to use prepared data read README inside data directory!