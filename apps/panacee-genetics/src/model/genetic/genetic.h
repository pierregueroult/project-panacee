#ifndef GENETIC_H
#define GENETIC_H

#include "../context.h"
#include "../individual/individual.h"
#include "../town/town.h"

/* Build a lookup table mapping an INSEE code to its index in the towns array.
   Entries are -1 when no town owns that code. Caller frees the returned table.
   Runs before a Context exists, hence the explicit arguments. */
int *build_insee_to_idx(const Town *towns, int town_count);

/* Mark covered[t] = 1 for every town t within reach of one of the individual's
   hospitals. covered must hold at least ctx->town_count bytes. */
void compute_covered(char *covered, const Individual *ind, const Context *ctx);

/* Greedily add hospitals that cover still-uncovered towns whose population
   gain outweighs PENALTY_HOSPITAL, until no further improvement is found. */
void local_search(Individual *result, const Context *ctx);

#endif
