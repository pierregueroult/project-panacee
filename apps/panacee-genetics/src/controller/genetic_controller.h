#ifndef GENETIC_CONTROLLER_H
#define GENETIC_CONTROLLER_H

#include "../model/individual/individual.h"
#include "../model/town/town.h"

/* Run the full genetic optimisation: orchestrates the model (population,
   operators, local search) and the views (map, console, CSV export).
   Returns the best solution found; caller frees result.hospitals. */
Individual run_genetic(const Town *towns, int town_count);

#endif
