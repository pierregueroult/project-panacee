/**
 * @file genetic.c
 * @brief Implementation of the genetic algorithm helpers (see genetic.h).
 */

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
    {
        table[i] = -1;
    }
    for (i = 0; i < town_count; i++)
    {
        if (towns[i].insee < 0 || towns[i].insee >= INSEE_MAX)
        {
            fprintf(stderr, "Warning: INSEE %d out of [0,%d), skipping\n", towns[i].insee, INSEE_MAX);
            continue;
        }
        table[towns[i].insee] = i;
    }
    return table;
}

void compute_covered(char *covered, const Individual *ind, const Context *ctx)
{
    int i, k;
    memset(covered, 0, ctx->town_count);
    for (i = 0; i < ind->size; i++)
    {
        int j = ctx->insee_to_idx[ind->hospitals[i].insee];
        if (j < 0)
        {
            continue;
        }
        for (k = 0; k < ctx->coverage_size[j]; k++)
        {
            covered[ctx->coverage[j][k]] = 1;
        }
    }
}

void local_search(Individual *result, const Context *ctx)
{
    int improved = 1;
    int i, k;

    while (improved)
    {
        int *cover_count = calloc(ctx->town_count, sizeof(int));

        improved = 0;
        for (i = 0; i < result->size; i++)
        {
            int j = ctx->insee_to_idx[result->hospitals[i].insee];
            if (j >= 0)
            {
                for (k = 0; k < ctx->coverage_size[j]; k++)
                {
                    cover_count[ctx->coverage[j][k]]++;
                }
            }
        }

        for (i = 0; i < ctx->town_count; i++)
        {
            int gain;
            if (cover_count[i] > 0)
            {
                continue;
            }
            gain = 0;
            for (k = 0; k < ctx->coverage_size[i]; k++)
            {
                if (cover_count[ctx->coverage[i][k]] == 0)
                {
                    gain += ctx->towns[ctx->coverage[i][k]].inhabitants_count;
                }
            }
            if (gain > PENALTY_HOSPITAL)
            {
                result->hospitals[result->size].insee = ctx->towns[i].insee;
                result->hospitals[result->size].beds_count = 0;
                result->size++;
                for (k = 0; k < ctx->coverage_size[i]; k++)
                {
                    cover_count[ctx->coverage[i][k]]++;
                }
                improved = 1;
            }
        }
        free(cover_count);
    }
}
