#include "../../genetic.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Evaluate an individual using precomputed coverage lists.
   coverage[j] = array of town indices within RADIUS_HOSPITAL_KM of town j. */
void evaluate(Individual *ind, const Town *towns, int town_count,
              const int *insee_to_idx, int **coverage, const int *coverage_size,
              int total_inhabitants)
{
    int i, k;
    int distant_residents = 0;
    int distant_towns = 0;
    int uhc_count = 0;
    char *covered = calloc(town_count, 1);

    for (i = 0; i < ind->size; i++)
    {
        int j = insee_to_idx[ind->hospitals[i].insee];
        if (j < 0)
            continue;
        for (k = 0; k < coverage_size[j]; k++)
            covered[coverage[j][k]] = 1;
        if (towns[j].inhabitants_count > THRESHOLD_UHC)
            uhc_count++;
    }

    for (i = 0; i < town_count; i++)
    {
        if (!covered[i])
        {
            distant_residents += towns[i].inhabitants_count;
            distant_towns++;
        }
    }
    free(covered);

    ind->fitness.hospital_count = ind->size;
    ind->fitness.uhc_count = uhc_count;
    ind->fitness.distant_resident_count = distant_residents;
    ind->fitness.distant_town_count = distant_towns;
    ind->fitness.distant_resident_percent =
        (100.0 * distant_residents / total_inhabitants);
    ind->fitness.distant_town_percent =
        (100.0 * distant_towns / town_count);

    ind->fitness.fitness_score = calculate_fitness_score(
        total_inhabitants, distant_residents, ind->size, uhc_count);
}

/* Random individual: pick k random distinct towns as hospitals.
   k varies in [ratio/2 .. ratio*2] to seed diversity in the population. */
Individual create_individual_random(const Town *towns, int town_count)
{
    Individual ind;
    int i, k;

    k = (int)(town_count * INIT_HOSPITAL_RATIO * (0.5 + (double)rand() / RAND_MAX * 1.5));

    ind.hospitals = malloc(town_count * sizeof(Hospital));
    ind.size = 0;
    memset(&ind.fitness, 0, sizeof(Fitness));

    char *used = calloc(town_count, 1);
    while (ind.size < k)
    {
        i = rand() % town_count;
        if (!used[i])
        {
            used[i] = 1;
            ind.hospitals[ind.size].insee = towns[i].insee;
            ind.hospitals[ind.size].beds_count = 0;
            ind.size++;
        }
    }
    free(used);

    return ind;
}

/* Greedy stochastic individual: at each step, pick randomly from towns whose
   score (uncovered inhabitants they would cover) is >= 90% of the current best.
   Scores are updated lazily — O(town_count * avg_coverage) total. */
Individual create_individual_greedy(const Town *towns, int town_count,
                                    int **coverage, const int *coverage_size)
{
    Individual ind;
    int i, k, m;
    int target = (int)(town_count * INIT_HOSPITAL_RATIO * (0.5 + (double)rand() / RAND_MAX * 1.5));
    char *covered = calloc(town_count, 1);
    int *score = malloc(town_count * sizeof(int));

    ind.hospitals = malloc(town_count * sizeof(Hospital));
    ind.size = 0;
    memset(&ind.fitness, 0, sizeof(Fitness));

    /* Initial scores: inhabitants that each town's hospital would cover */
    for (i = 0; i < town_count; i++)
    {
        score[i] = 0;
        for (k = 0; k < coverage_size[i]; k++)
            score[i] += towns[coverage[i][k]].inhabitants_count;
    }

    while (ind.size < target)
    {
        /* Find the maximum score */
        int max_score = 0;
        for (i = 0; i < town_count; i++)
            if (score[i] > max_score)
                max_score = score[i];
        if (max_score == 0)
            break; /* All towns are covered */

        /* Pick randomly among towns scoring >= 90% of the best (stochastic greedy) */
        int threshold = max_score * 9 / 10;
        int n_candidates = 0;
        for (i = 0; i < town_count; i++)
            if (score[i] >= threshold)
                n_candidates++;

        int pick = rand() % n_candidates;
        int best = -1;
        for (i = 0; i < town_count; i++)
        {
            if (score[i] >= threshold && pick-- == 0)
            {
                best = i;
                break;
            }
        }

        ind.hospitals[ind.size].insee = towns[best].insee;
        ind.hospitals[ind.size].beds_count = 0;
        ind.size++;

        /* Lazy score update: remove the contribution of newly covered towns */
        for (k = 0; k < coverage_size[best]; k++)
        {
            int t = coverage[best][k];
            if (!covered[t])
            {
                covered[t] = 1;
                for (m = 0; m < coverage_size[t]; m++)
                    score[coverage[t][m]] -= towns[t].inhabitants_count;
            }
        }
    }

    free(covered);
    free(score);
    return ind;
}

void free_individual(Individual *ind)
{
    free(ind->hospitals);
    ind->hospitals = NULL;
    ind->size = 0;
}

/* Create a child by merging two parents' hospital lists and sampling a random subset */
Individual crossover(const Individual *a, const Individual *b, int town_count)
{
    int i;
    Individual child;
    /* INSEE codes are at most 5 digits (max 99999) */
    char *present = calloc(INSEE_MAX, 1);
    Hospital *pool = malloc((a->size + b->size) * sizeof(Hospital));
    int pool_n = 0;
    int target_size;

    for (i = 0; i < a->size; i++)
        if (!present[a->hospitals[i].insee])
        {
            present[a->hospitals[i].insee] = 1;
            pool[pool_n++] = a->hospitals[i];
        }
    for (i = 0; i < b->size; i++)
        if (!present[b->hospitals[i].insee])
        {
            present[b->hospitals[i].insee] = 1;
            pool[pool_n++] = b->hospitals[i];
        }

    int lo = a->size < b->size ? a->size : b->size;
    int hi = a->size > b->size ? a->size : b->size;
    target_size = lo + rand() % (hi - lo + 1);

    if (target_size > pool_n)
        target_size = pool_n;

    /* Shuffle pool and take the first target_size entries */
    for (i = pool_n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Hospital tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }

    child.hospitals = malloc(town_count * sizeof(Hospital));
    child.size = target_size;
    for (i = 0; i < target_size; i++)
        child.hospitals[i] = pool[i];
    memset(&child.fitness, 0, sizeof(Fitness));

    free(present);
    free(pool);
    return child;
}

/* Remove hospitals that cover no town exclusively (all their towns are also
   covered by at least one other hospital). */
void remove_redundant(Individual *ind, int **coverage, const int *coverage_size,
                      const int *insee_to_idx, int town_count)
{
    int h, k;
    int *cover_count = calloc(town_count, sizeof(int));

    /* Count how many hospitals cover each town */
    for (h = 0; h < ind->size; h++)
    {
        int j = insee_to_idx[ind->hospitals[h].insee];
        if (j < 0)
            continue;
        for (k = 0; k < coverage_size[j]; k++)
            cover_count[coverage[j][k]]++;
    }

    /* Remove hospitals that don't uniquely cover any town */
    for (h = 0; h < ind->size; h++)
    {
        int j = insee_to_idx[ind->hospitals[h].insee];
        int unique = 0;
        if (j >= 0)
            for (k = 0; k < coverage_size[j]; k++)
                if (cover_count[coverage[j][k]] == 1)
                {
                    unique = 1;
                    break;
                }

        if (!unique)
        {
            /* Update cover counts before removing */
            if (j >= 0)
                for (k = 0; k < coverage_size[j]; k++)
                    cover_count[coverage[j][k]]--;
            ind->hospitals[h--] = ind->hospitals[--ind->size];
        }
    }
    free(cover_count);
}

/* Random variation for an individual */
void mutate(Individual *ind, const Town *towns, int town_count,
            double mutation_rate, int **coverage, const int *coverage_size,
            const int *insee_to_idx)
{
    int op, idx, new_idx, i, k;

    if ((double)rand() / RAND_MAX > mutation_rate)
        return;

    op = rand() % 3;

    if (op == 0)
    {
        /* Smart add: place a hospital in the town that maximises newly covered inhabitants */
        char *covered = calloc(town_count, 1);
        for (i = 0; i < ind->size; i++)
        {
            int j = insee_to_idx[ind->hospitals[i].insee];
            if (j >= 0)
                for (k = 0; k < coverage_size[j]; k++)
                    covered[coverage[j][k]] = 1;
        }
        int best = -1;
        int best_gain = 0;
        for (i = 0; i < town_count; i++)
        {
            int gain = 0;
            for (k = 0; k < coverage_size[i]; k++)
                if (!covered[coverage[i][k]])
                    gain += towns[coverage[i][k]].inhabitants_count;
            if (gain > best_gain)
            {
                best_gain = gain;
                best = i;
            }
        }
        if (best >= 0)
        {
            ind->hospitals[ind->size].insee = towns[best].insee;
            ind->hospitals[ind->size].beds_count = 0;
            ind->size++;
        }
        free(covered);
    }
    else if (op == 1 && ind->size > 1)
    {
        /* Remove a random hospital */
        idx = rand() % ind->size;
        ind->hospitals[idx] = ind->hospitals[--ind->size];
    }
    else
    {
        /* Move a hospital to a random town */
        idx = rand() % ind->size;
        new_idx = rand() % town_count;

        /* If the destination already has a hospital, remove the current one instead */
        for (i = 0; i < ind->size; i++)
        {
            if (i != idx && ind->hospitals[i].insee == towns[new_idx].insee)
            {
                ind->hospitals[idx] = ind->hospitals[--ind->size];
                return;
            }
        }
        ind->hospitals[idx].insee = towns[new_idx].insee;
        ind->hospitals[idx].beds_count = 0;
    }
}

/* Compute beds_count for each hospital in the result.
   Each covered town is assigned to its nearest hospital.
   beds_count = floor(BEDS_PER_INHABITANT / 1000 * assigned_inhabitants).*/
void compute_beds(Individual *ind, const Town *towns, int town_count,
                  const int *insee_to_idx, int **coverage,
                  const int *coverage_size)
{
    int i, k, h;
    int *inhabitants_per_hospital = calloc(ind->size, sizeof(int));
    /* nearest_hospital[t] = index in ind->hospitals of the closest hospital covering town t */
    int *nearest_hospital = malloc(town_count * sizeof(int));
    double *nearest_dist = malloc(town_count * sizeof(double));

    for (i = 0; i < town_count; i++)
    {
        nearest_hospital[i] = -1;
        nearest_dist[i] = 1e18;
    }

    for (h = 0; h < ind->size; h++)
    {
        int j = insee_to_idx[ind->hospitals[h].insee];
        if (j < 0)
            continue;
        for (k = 0; k < coverage_size[j]; k++)
        {
            int t = coverage[j][k];
            double d = haversine_km(towns[j].latitude, towns[j].longitude,
                                    towns[t].latitude, towns[t].longitude);
            if (d < nearest_dist[t])
            {
                nearest_dist[t] = d;
                nearest_hospital[t] = h;
            }
        }
    }

    for (i = 0; i < town_count; i++)
        if (nearest_hospital[i] >= 0)
            inhabitants_per_hospital[nearest_hospital[i]] += towns[i].inhabitants_count;

    for (h = 0; h < ind->size; h++)
        ind->hospitals[h].beds_count =
            (int)(BEDS_PER_INHABITANT / 1000.0 * inhabitants_per_hospital[h]);

    free(inhabitants_per_hospital);
    free(nearest_hospital);
    free(nearest_dist);
}

