#ifndef GENETIC_H
#define GENETIC_H

/**
 * @file genetic.h
 * @brief Helpers shared by the genetic algorithm: lookup table, coverage, local search.
 */

#include "../context.h"
#include "../individual/individual.h"
#include "../town/town.h"

/**
 * @brief Build a lookup table mapping an INSEE code to its index in the towns array.
 *
 * Entries are -1 when no town owns that code. Runs before a Context exists,
 * hence the explicit arguments.
 *
 * @param towns Array of towns.
 * @param town_count Number of towns.
 * @return Table of INSEE_MAX entries; caller frees it.
 */
int *build_insee_to_idx(const Town *towns, int town_count);

/**
 * @brief Mark covered[t] = 1 for every town t within reach of one of the
 * individual's hospitals.
 *
 * @param covered Output buffer; must hold at least ctx->town_count bytes.
 * @param ind Individual whose hospitals define the coverage.
 * @param ctx Problem environment.
 */
void compute_covered(char *covered, const Individual *ind, const Context *ctx);

/**
 * @brief Greedily add hospitals that cover still-uncovered towns whose population
 * gain outweighs PENALTY_HOSPITAL, until no further improvement is found.
 *
 * @param result Individual improved in place.
 * @param ctx Problem environment.
 */
void local_search(Individual *result, const Context *ctx);

#endif
