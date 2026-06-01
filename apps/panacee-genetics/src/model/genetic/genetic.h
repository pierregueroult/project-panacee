#ifndef GENETIC_H
#define GENETIC_H

#include "../individual/individual.h"
#include "../town/town.h"

/* Build a lookup table mapping an INSEE code to its index in the towns array.
   Entries are -1 when no town owns that code. Caller frees the returned table. */
int *build_insee_to_idx(const Town *towns, int town_count);

/* Mark covered[t] = 1 for every town t within reach of one of the individual's
   hospitals. covered must hold at least town_count bytes. */
void compute_covered(char *covered, const Individual *ind, int town_count,
                     const int *insee_to_idx,
                     int **coverage, const int *coverage_size);

/* Greedily add hospitals that cover still-uncovered towns whose population
   gain outweighs PENALTY_HOSPITAL, until no further improvement is found. */
void local_search(Individual *result, const Town *towns, int town_count,
                  const int *insee_to_idx,
                  int **coverage, const int *coverage_size);

#endif
