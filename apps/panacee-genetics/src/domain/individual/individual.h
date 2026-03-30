#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include "../fitness/fitness.h"
#include "../hospital/hospital.h"
#include "../town/town.h"

typedef struct
{
    Hospital *hospitals;
    int size;
    Fitness fitness;
} Individual;

Individual create_individual_random(Town *towns, int town_count);
Individual create_individual_greedy(Town *towns, int town_count, int **coverage, const int *coverage_size);
void free_individual(Individual *ind);
Individual crossover(const Individual *a, const Individual *b, int town_count);
void remove_redundant(Individual *ind, int **coverage, const int *coverage_size, const int *insee_to_idx, int town_count);
void mutate(Individual *ind, Town *towns, int town_count, double mutation_rate, int **coverage, const int *coverage_size, const int *insee_to_idx);
void compute_beds(Individual *ind, Town *towns, int town_count, const int *insee_to_idx, int **coverage, const int *coverage_size);
void evaluate(Individual *ind, Town *towns, int town_count, const int *insee_to_idx, int **coverage, const int *coverage_size, int total_inhabitants);

#endif
