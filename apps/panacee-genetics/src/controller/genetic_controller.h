#ifndef GENETIC_CONTROLLER_H
#define GENETIC_CONTROLLER_H

/**
 * @file genetic_controller.h
 * @brief Controller orchestrating the genetic optimisation over model and views.
 */

#include "../model/individual/individual.h"
#include "../model/town/town.h"

/**
 * @brief Run the full genetic optimisation.
 *
 * Orchestrates the model (population, operators, local search) and the views
 * (map, console, CSV export).
 *
 * @param towns Array of towns.
 * @param town_count Number of towns.
 * @return The best solution found; caller frees result.hospitals.
 */
Individual run_genetic(const Town *towns, int town_count);

#endif
