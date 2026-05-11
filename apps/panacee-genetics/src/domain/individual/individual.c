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

/* Create a child by always keeping the parents' intersection and filling the
   remaining slots with a random sample of the symmetric difference. */
Individual crossover(const Individual *a, const Individual *b, int town_count)
{
    int i;
    Individual child;
    /* INSEE codes are at most 5 digits (max 99999) */
    unsigned char *count = calloc(INSEE_MAX, 1);
    int min_size = a->size < b->size ? a->size : b->size;
    Hospital *intersection = malloc(min_size * sizeof(Hospital));
    Hospital *symdiff = malloc((a->size + b->size) * sizeof(Hospital));
    int int_n = 0;
    int sym_n = 0;
    int lo, hi, target_size, extras;

    for (i = 0; i < a->size; i++)
        count[a->hospitals[i].insee]++;
    for (i = 0; i < b->size; i++)
        count[b->hospitals[i].insee]++;

    for (i = 0; i < a->size; i++)
    {
        if (count[a->hospitals[i].insee] >= 2)
            intersection[int_n++] = a->hospitals[i];
        else
            symdiff[sym_n++] = a->hospitals[i];
    }
    for (i = 0; i < b->size; i++)
        if (count[b->hospitals[i].insee] == 1)
            symdiff[sym_n++] = b->hospitals[i];

    lo = a->size < b->size ? a->size : b->size;
    hi = a->size > b->size ? a->size : b->size;
    target_size = lo + rand() % (hi - lo + 1);

    if (target_size > int_n + sym_n)
        target_size = int_n + sym_n;
    if (target_size < int_n)
        target_size = int_n; /* always keep the full intersection */

    /* Shuffle symdiff so the picked extras are random */
    for (i = sym_n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Hospital tmp = symdiff[i];
        symdiff[i] = symdiff[j];
        symdiff[j] = tmp;
    }

    child.hospitals = malloc(town_count * sizeof(Hospital));
    child.size = target_size;
    for (i = 0; i < int_n; i++)
        child.hospitals[i] = intersection[i];
    extras = target_size - int_n;
    for (i = 0; i < extras; i++)
        child.hospitals[int_n + i] = symdiff[i];
    memset(&child.fitness, 0, sizeof(Fitness));

    free(count);
    free(intersection);
    free(symdiff);
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
    int op, i, k;

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
        /* Remove the hospital with the smallest exclusive coverage
           (fewest inhabitants that would become uncovered). */
        int *cover_count = calloc(town_count, sizeof(int));
        int worst = -1;
        int worst_exclusive = -1;
        int h;

        for (h = 0; h < ind->size; h++)
        {
            int j = insee_to_idx[ind->hospitals[h].insee];
            if (j >= 0)
                for (k = 0; k < coverage_size[j]; k++)
                    cover_count[coverage[j][k]]++;
        }

        for (h = 0; h < ind->size; h++)
        {
            int j = insee_to_idx[ind->hospitals[h].insee];
            int exclusive = 0;
            if (j >= 0)
                for (k = 0; k < coverage_size[j]; k++)
                    if (cover_count[coverage[j][k]] == 1)
                        exclusive += towns[coverage[j][k]].inhabitants_count;
            if (worst < 0 || exclusive < worst_exclusive)
            {
                worst_exclusive = exclusive;
                worst = h;
            }
        }

        if (worst >= 0)
            ind->hospitals[worst] = ind->hospitals[--ind->size];
        free(cover_count);
    }
    else
    {
        /* Move the least-exclusive hospital to the best uncovered town. */
        int *cover_count = calloc(town_count, sizeof(int));
        int worst = -1;
        int worst_exclusive = -1;
        int best_dest = -1;
        int best_gain = 0;
        int h;

        for (h = 0; h < ind->size; h++)
        {
            int j = insee_to_idx[ind->hospitals[h].insee];
            if (j >= 0)
                for (k = 0; k < coverage_size[j]; k++)
                    cover_count[coverage[j][k]]++;
        }

        for (h = 0; h < ind->size; h++)
        {
            int j = insee_to_idx[ind->hospitals[h].insee];
            int exclusive = 0;
            if (j >= 0)
                for (k = 0; k < coverage_size[j]; k++)
                    if (cover_count[coverage[j][k]] == 1)
                        exclusive += towns[coverage[j][k]].inhabitants_count;
            if (worst < 0 || exclusive < worst_exclusive)
            {
                worst_exclusive = exclusive;
                worst = h;
            }
        }

        for (i = 0; i < town_count; i++)
        {
            int gain = 0;
            if (cover_count[i] > 0)
                continue;
            for (k = 0; k < coverage_size[i]; k++)
                if (cover_count[coverage[i][k]] == 0)
                    gain += towns[coverage[i][k]].inhabitants_count;
            if (gain > best_gain)
            {
                best_gain = gain;
                best_dest = i;
            }
        }

        if (worst >= 0 && best_dest >= 0)
        {
            ind->hospitals[worst].insee = towns[best_dest].insee;
            ind->hospitals[worst].beds_count = 0;
        }
        free(cover_count);
    }
}

/* Compute beds_count for each hospital in the result.
   Each covered town is assigned to its nearest hospital.
   beds_count = floor(BEDS_PER_INHABITANT / 1000 * assigned_inhabitants).*/
void compute_beds(Individual *ind, const Town *towns, int town_count,
                  const int *insee_to_idx, int **coverage,
                  const int *coverage_size)
{
    int i, h;
    int *inhabitants_per_hospital = calloc(ind->size, sizeof(int));
    int *nearest_hospital = malloc(town_count * sizeof(int));

    nearest_hospital_per_town(ind->hospitals, ind->size,
                              towns, town_count,
                              insee_to_idx, coverage, coverage_size,
                              nearest_hospital, NULL);

    for (i = 0; i < town_count; i++)
        if (nearest_hospital[i] >= 0)
            inhabitants_per_hospital[nearest_hospital[i]] +=
                towns[i].inhabitants_count;

    for (h = 0; h < ind->size; h++)
        ind->hospitals[h].beds_count =
            (int)(BEDS_PER_INHABITANT / 1000.0 * inhabitants_per_hospital[h]);

    free(inhabitants_per_hospital);
    free(nearest_hospital);
}

