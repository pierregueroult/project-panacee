#include "genetic.h"
#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int *build_insee_to_idx(const Town *towns, int town_count)
{
    int *table = malloc(INSEE_MAX * sizeof(int));
    int i;

    for (i = 0; i < INSEE_MAX; i++)
        table[i] = -1;
    for (i = 0; i < town_count; i++)
    {
        if (towns[i].insee < 0 || towns[i].insee >= INSEE_MAX)
        {
            fprintf(stderr,
                    "Warning: INSEE %d out of [0,%d), skipping\n",
                    towns[i].insee, INSEE_MAX);
            continue;
        }
        table[towns[i].insee] = i;
    }
    return table;
}

void compute_covered(char *covered,
                     const Individual *ind, int town_count,
                     const int *insee_to_idx,
                     int **coverage, const int *coverage_size)
{
    int i, k;
    memset(covered, 0, town_count);
    for (i = 0; i < ind->size; i++)
    {
        int j = insee_to_idx[ind->hospitals[i].insee];
        if (j < 0) continue;
        for (k = 0; k < coverage_size[j]; k++)
            covered[coverage[j][k]] = 1;
    }
}

void local_search(Individual *result, const Town *towns, int town_count,
                  const int *insee_to_idx,
                  int **coverage, const int *coverage_size)
{
    int improved = 1;
    int i, k;

    while (improved)
    {
        int *cover_count = calloc(town_count, sizeof(int));

        improved = 0;
        for (i = 0; i < result->size; i++)
        {
            int j = insee_to_idx[result->hospitals[i].insee];
            if (j >= 0)
                for (k = 0; k < coverage_size[j]; k++)
                    cover_count[coverage[j][k]]++;
        }

        for (i = 0; i < town_count; i++)
        {
            int gain;
            if (cover_count[i] > 0)
                continue;
            gain = 0;
            for (k = 0; k < coverage_size[i]; k++)
                if (cover_count[coverage[i][k]] == 0)
                    gain += towns[coverage[i][k]].inhabitants_count;
            if (gain > PENALTY_HOSPITAL)
            {
                result->hospitals[result->size].insee = towns[i].insee;
                result->hospitals[result->size].beds_count = 0;
                result->size++;
                for (k = 0; k < coverage_size[i]; k++)
                    cover_count[coverage[i][k]]++;
                improved = 1;
            }
        }
        free(cover_count);
    }
}
