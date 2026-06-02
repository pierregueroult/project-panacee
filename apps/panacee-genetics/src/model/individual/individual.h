#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include "../context.h"
#include "../fitness/fitness.h"
#include "../hospital/hospital.h"
#include "../town/town.h"

typedef struct
{
    Hospital *hospitals;
    int size;
    Fitness fitness;
} Individual;

Individual create_individual_random(const Context *ctx);
Individual create_individual_greedy(const Context *ctx);
Individual clone_individual(const Individual *src, const Context *ctx);
void free_individual(Individual *ind);
Individual crossover(const Individual *a, const Individual *b, const Context *ctx);
void remove_redundant(Individual *ind, const Context *ctx);
void mutate(Individual *ind, const Context *ctx, double mutation_rate);
void compute_beds(Individual *ind, const Context *ctx);
void evaluate(Individual *ind, const Context *ctx);

#endif
