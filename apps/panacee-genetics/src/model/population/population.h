#ifndef POPULATION_H
#define POPULATION_H

#include "../individual/individual.h"

typedef struct
{
    Individual *individuals;
    int         size;
} Population;

Population init_population(const Context *ctx);
void free_population(Population *pop);
Individual tournament_select(const Population *pop);
Individual best_individual(const Population *pop);
void evaluate_population(Population *pop, const Context *ctx);

#endif
