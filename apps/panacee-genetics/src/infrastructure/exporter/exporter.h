#ifndef EXPORTER_H
#define EXPORTER_H

#include "../../domain/fitness/fitness.h"
#include "../../domain/individual/individual.h"
#include "../../domain/town/town.h"

void export_fitness_csv(const Fitness *fitness, const char *path);
void export_result_csv(const Individual *result, const Town *towns,
                       const int *insee_to_idx, const char *path);
void export_towns_status_csv(const Individual *result, const Town *towns,
                             int town_count, const int *insee_to_idx,
                             int **coverage, const int *coverage_size,
                             const char *path);

#endif
