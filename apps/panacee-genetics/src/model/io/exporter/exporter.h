#ifndef EXPORTER_H
#define EXPORTER_H

/**
 * @file exporter.h
 * @brief CSV exporters for the final result.
 */

#include "../../fitness/fitness.h"
#include "../../individual/individual.h"
#include "../../town/town.h"

/**
 * @brief Export the fitness metrics of the final solution to a CSV file.
 * @param fitness Metrics to export.
 * @param path Output file path; parent directories are created if needed.
 */
void export_fitness_csv(const Fitness *fitness, const char *path);

/**
 * @brief Export the hospital list of the final solution to a CSV file.
 * @param result Final solution.
 * @param towns Array of towns.
 * @param insee_to_idx Lookup table from INSEE code to town index.
 * @param path Output file path; parent directories are created if needed.
 */
void export_result_csv(const Individual *result, const Town *towns, const int *insee_to_idx, const char *path);

/**
 * @brief Export the coverage status of every town to a CSV file.
 *
 * For each town, write its assigned hospital INSEE (-1 if desert). Assignment
 * uses the nearest hospital among those whose coverage disc contains the town.
 *
 * @param result Final solution.
 * @param towns Array of towns.
 * @param town_count Number of towns.
 * @param insee_to_idx Lookup table from INSEE code to town index.
 * @param coverage Precomputed coverage lists.
 * @param coverage_size Length of each coverage list.
 * @param path Output file path; parent directories are created if needed.
 */
void export_towns_status_csv(const Individual *result, const Town *towns, int town_count, const int *insee_to_idx,
                             int **coverage, const int *coverage_size, const char *path);

#endif
