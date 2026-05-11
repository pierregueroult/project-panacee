#ifndef POPULATION_H
#define POPULATION_H

#include "../individual/individual.h"

typedef struct
{
    Individual *individuals;
    int         size;
} Population;

Population init_population(const Town *towns, int town_count,
                           int **coverage, const int *coverage_size);
void free_population(Population *pop);
Individual tournament_select(const Population *pop, int k);
Individual best_individual(const Population *pop);
void evaluate_population(Population *pop, const Town *towns, int town_count,
                         const int *insee_to_idx, int **coverage,
                         const int *coverage_size, int total_inhabitants);

#endif
