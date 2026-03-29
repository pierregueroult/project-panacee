#include "genetic.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <time.h>

double haversine_km(double lat1, double lon1, double lat2, double lon2)
{
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    double a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dlon / 2) * sin(dlon / 2);
    return EARTH_RADIUS_KM * 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
}

int inhabitant_count(Town *towns, int size)
{
    int result = 0;
    int i;
    for (i = 0; i < size; i++)
        result += towns[i].inhabitants_count;
    return result;
}

double calculate_fitness_score(int total_inhabitants,
                               int distant_residents,
                               int hospitals_count,
                               int uhc_count)
{
    return (double)total_inhabitants - distant_residents - (double)PENALITY_HOSPITAL * hospitals_count + (double)BONUS_UHC * uhc_count;
}

/* Evaluate an individual using precomputed coverage lists.
   coverage[j] = array of town indices within RADIUS_HOSPITAL_KM of town j. */
void evaluate(Individual *ind, Town *towns, int town_count,
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
        if (towns[j].inhabitants_count > TRESHOLD_UHC)
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

/* Evaluate all individuals in a population */
void evaluate_population(Population *pop, Town *towns, int town_count,
                         const int *insee_to_idx, int **coverage,
                         const int *coverage_size, int total_inhabitants)
{
    int i;
    double sum = 0.0;
    for (i = 0; i < pop->size; i++)
    {
        evaluate(&pop->individuals[i], towns, town_count,
                 insee_to_idx, coverage, coverage_size, total_inhabitants);
        sum += pop->individuals[i].fitness.fitness_score;
    }

    for (i = 0; i < pop->size; i++)
    {
        pop->individuals[i].fitness.fitness_count = pop->size;
        pop->individuals[i].fitness.fitness_average = sum / pop->size;
    }
}

/* Random individual: pick k random distinct towns as hospitals.
   k varies in [ratio/2 .. ratio*2] to seed diversity in the population. */
static Individual create_individual_random(Town *towns, int town_count)
{
    Individual ind;
    int i, k;

    k = (int)(town_count * INIT_HOSPITAL_RATIO * (0.5 + (double)rand() / RAND_MAX * 1.5));

    ind.hospitals = malloc(town_count * sizeof(Hospital));
    ind.size = 0;
    memset(&ind.fitness, 0, sizeof(Fitness));

    {
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
    }
    return ind;
}

/* Greedy stochastic individual: at each step, pick randomly from towns whose
   score (uncovered inhabitants they would cover) is >= 90% of the current best.
   Scores are updated lazily — O(town_count * avg_coverage) total. */
static Individual create_individual_greedy(Town *towns, int town_count,
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

/* Initialize population: 80% greedy individuals for a strong start,
   20% random for diversity. */
Population init_population(Town *towns, int town_count,
                           int **coverage, const int *coverage_size)
{
    int i;
    Population pop;
    int greedy_count = POPULATION_SIZE * 4 / 5;

    pop.size = POPULATION_SIZE;
    pop.individuals = malloc(POPULATION_SIZE * sizeof(Individual));

    for (i = 0; i < greedy_count; i++)
        pop.individuals[i] = create_individual_greedy(towns, town_count, coverage, coverage_size);
    for (i = greedy_count; i < POPULATION_SIZE; i++)
        pop.individuals[i] = create_individual_random(towns, town_count);

    return pop;
}

void free_population(Population *pop)
{
    int i;
    for (i = 0; i < pop->size; i++)
        free_individual(&pop->individuals[i]);
    free(pop->individuals);
    pop->individuals = NULL;
    pop->size = 0;
}

Individual tournament_select(const Population *pop)
{
    int i, index;
    int best_index = rand() % pop->size;
    for (i = 1; i < TOURNAMENT_K; i++)
    {
        index = rand() % pop->size;
        if (pop->individuals[index].fitness.fitness_score >
            pop->individuals[best_index].fitness.fitness_score)
            best_index = index;
    }
    return pop->individuals[best_index];
}

/* Create a child by merging two parents' hospital lists and sampling a random subset */
Individual crossover(const Individual *a, const Individual *b, int town_count)
{
    int i;
    Individual child;
    /* INSEE codes are at most 5 digits (max 99999) */
    char *present = calloc(100000, 1);
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

    {
        int lo = a->size < b->size ? a->size : b->size;
        int hi = a->size > b->size ? a->size : b->size;
        target_size = lo + rand() % (hi - lo + 1);
    }
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
static void remove_redundant(Individual *ind, int **coverage, const int *coverage_size,
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
void mutate(Individual *ind, Town *towns, int town_count, double mutation_rate,
            int **coverage, const int *coverage_size, const int *insee_to_idx)
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
static void compute_beds(Individual *ind, Town *towns, int town_count,
                         const int *insee_to_idx, int **coverage, const int *coverage_size)
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

static void export_result_csv(const Individual *result, Town *towns,
                              const int *insee_to_idx, const char *path)
{
    int i;
    FILE *f = fopen(path, "w");
    if (!f)
    {
        fprintf(stderr, "Warning: could not open %s for writing\n", path);
        return;
    }
    fprintf(f, "insee,beds_count\n");
    for (i = 0; i < result->size; i++)
    {
        int j = insee_to_idx[result->hospitals[i].insee];
        if (j >= 0)
            fprintf(f, "%d,%d\n",
                    towns[j].insee,
                    result->hospitals[i].beds_count);
    }
    fclose(f);
    printf("Results exported to %s (%d hospitals)\n", path, result->size);
}

static void export_fitness_csv(const Fitness *fitness, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        fprintf(stderr, "Warning: could not open %s for writing\n", path);
        return;
    }
    fprintf(f, "hospital_count,uhc_count,distant_resident_count,distant_resident_percent,"
               "distant_town_count,distant_town_percent,fitness_score,fitness_count,fitness_average\n");
    fprintf(f, "%d,%d,%d,%.2f,%d,%.2f,%.2f,%d,%.2f\n",
            fitness->hospital_count,
            fitness->uhc_count,
            fitness->distant_resident_count,
            fitness->distant_resident_percent,
            fitness->distant_town_count,
            fitness->distant_town_percent,
            fitness->fitness_score,
            fitness->fitness_count,
            fitness->fitness_average);
    fclose(f);
    printf("Fitness exported to %s\n", path);
}

Individual best_individual(const Population *pop)
{
    int i, best = 0;
    for (i = 1; i < pop->size; i++)
        if (pop->individuals[i].fitness.fitness_score >
            pop->individuals[best].fitness.fitness_score)
            best = i;
    return pop->individuals[best];
}

/* Precompute coverage lists: for each town, which towns are within radius.
   Uses a latitude bounding box to skip most pairs before calling haversine. */
void precompute_coverage(Town *towns, int town_count, int ***coverage, int **coverage_size)
{
    int i, j, cnt;
    double dlat_max = RADIUS_HOSPITAL_KM / 111.0;
    int *tmp = malloc(town_count * sizeof(int));

    *coverage = malloc(town_count * sizeof(int *));
    *coverage_size = malloc(town_count * sizeof(int));

    printf("Precomputing coverage map...\n");
    for (i = 0; i < town_count; i++)
    {
        cnt = 0;
        for (j = 0; j < town_count; j++)
        {
            if (fabs(towns[j].latitude - towns[i].latitude) > dlat_max)
                continue;
            if (haversine_km(towns[i].latitude, towns[i].longitude,
                             towns[j].latitude, towns[j].longitude) <= RADIUS_HOSPITAL_KM)
                tmp[cnt++] = j;
        }
        (*coverage)[i] = malloc(cnt * sizeof(int));
        memcpy((*coverage)[i], tmp, cnt * sizeof(int));
        (*coverage_size)[i] = cnt;
    }
    free(tmp);
    printf("Coverage map done.\n");
}

Individual run_genetic(Town *towns, int town_count)
{
    int gen, i;
    int stagnation = 0;
    double prev_best = -1;
    double current_mutation_rate = MUTATION_RATE;
    Individual result;
    Population pop, next_pop;
    int *insee_to_idx;
    int **coverage;
    int *coverage_size;
    int total_inhabitants;

    srand((unsigned int)time(NULL));

    /* Build a lookup table insee -> town array index, built once for all evaluations */
    insee_to_idx = malloc(100000 * sizeof(int));
    for (i = 0; i < 100000; i++)
        insee_to_idx[i] = -1;
    for (i = 0; i < town_count; i++)
        insee_to_idx[towns[i].insee] = i;

    precompute_coverage(towns, town_count, &coverage, &coverage_size);

    total_inhabitants = inhabitant_count(towns, town_count);

    pop = init_population(towns, town_count, coverage, coverage_size);
    evaluate_population(&pop, towns, town_count, insee_to_idx, coverage, coverage_size, total_inhabitants);

    for (gen = 0; gen < MAX_GENERATIONS; gen++)
    {
        Individual current_best = best_individual(&pop);
        double best_score = current_best.fitness.fitness_score;

        printf("Gen %4d | fitness: %.0f | hop: %d | CHRU: %d | desert: %d (%.1f%%)\n",
               gen,
               best_score,
               current_best.fitness.hospital_count,
               current_best.fitness.uhc_count,
               current_best.fitness.distant_resident_count,
               (double)current_best.fitness.distant_resident_percent);

        if (best_score > prev_best)
        {
            prev_best = best_score;
            stagnation = 0;
            current_mutation_rate = MUTATION_RATE; /* reset mutation rate on improvement */
        }
        else
        {
            stagnation++;

            /* Progressively increase mutation rate to escape local optima */
            if (stagnation % 10 == 0 && current_mutation_rate < 0.4)
            {
                current_mutation_rate = fmin(0.4, current_mutation_rate * 1.3);
                printf("Stagnation at gen %d -> mutation_rate = %.2f\n",
                       stagnation, current_mutation_rate);
            }
        }

        if (stagnation >= STAGNATION_LIMIT)
        {
            printf("Stop: stagnation over %d generations.\n", STAGNATION_LIMIT);
            break;
        }

        /* Build next generation */
        next_pop.size = POPULATION_SIZE;
        next_pop.individuals = malloc(POPULATION_SIZE * sizeof(Individual));

        /* Elitism: carry the best individual unchanged into the next generation */
        {
            Individual elite = best_individual(&pop);
            next_pop.individuals[0].hospitals = malloc(town_count * sizeof(Hospital));
            next_pop.individuals[0].size = elite.size;
            memcpy(next_pop.individuals[0].hospitals,
                   elite.hospitals,
                   elite.size * sizeof(Hospital));
            next_pop.individuals[0].fitness = elite.fitness;
        }

        for (i = 1; i < POPULATION_SIZE; i++)
        {
            Individual p1 = tournament_select(&pop);
            Individual p2 = tournament_select(&pop);
            next_pop.individuals[i] = crossover(&p1, &p2, town_count);
            mutate(&next_pop.individuals[i], towns, town_count, current_mutation_rate,
                   coverage, coverage_size, insee_to_idx);
            remove_redundant(&next_pop.individuals[i], coverage, coverage_size,
                             insee_to_idx, town_count);
        }

        free_population(&pop);
        pop = next_pop;
        evaluate_population(&pop, towns, town_count, insee_to_idx, coverage, coverage_size, total_inhabitants);
    }

    /* Copy the best solution before freeing the population */
    {
        Individual b = best_individual(&pop);
        result.hospitals = malloc(town_count * sizeof(Hospital)); /* town_count: upper bound for local search */
        result.size = b.size;
        result.fitness = b.fitness;
        memcpy(result.hospitals, b.hospitals, b.size * sizeof(Hospital));
    }
    free_population(&pop);

    /* Local search: greedily add hospitals as long as the coverage gain > penalty */
    printf("Running local search...\n");
    {
        int improved = 1;
        int i, k;
        while (improved)
        {
            improved = 0;
            int *cover_count = calloc(town_count, sizeof(int));

            for (i = 0; i < result.size; i++)
            {
                int j = insee_to_idx[result.hospitals[i].insee];
                if (j >= 0)
                    for (k = 0; k < coverage_size[j]; k++)
                        cover_count[coverage[j][k]]++;
            }

            for (i = 0; i < town_count; i++)
            {
                if (cover_count[i] > 0)
                    continue;
                /* Count inhabitants newly covered by a hospital at town i */
                int gain = 0;
                for (k = 0; k < coverage_size[i]; k++)
                    if (cover_count[coverage[i][k]] == 0)
                        gain += towns[coverage[i][k]].inhabitants_count;
                if (gain > PENALITY_HOSPITAL)
                {
                    result.hospitals[result.size].insee = towns[i].insee;
                    result.hospitals[result.size].beds_count = 0;
                    result.size++;
                    for (k = 0; k < coverage_size[i]; k++)
                        cover_count[coverage[i][k]]++;
                    improved = 1;
                }
            }
            free(cover_count);
        }
    }
    printf("Local search done.\n");

    /* Compute beds per hospital (post-optimization) */
    compute_beds(&result, towns, town_count, insee_to_idx, coverage, coverage_size);

    printf("\n=== Final result: %d hospitals ===\n", result.size);

    int total_beds = 0;
    int covered_inhabitants = total_inhabitants - result.fitness.distant_resident_count;
    for (i = 0; i < result.size; i++)
        total_beds += result.hospitals[i].beds_count;
    printf("Total beds: %d\n", total_beds);
    printf("Covered inhabitants: %d\n", covered_inhabitants);
    printf("Fitness Score : %f\n", result.fitness.fitness_score);

    if (covered_inhabitants > 0)
        printf("Beds per 1000 covered inhabitants: %.2f (target: %.2f)\n",
               (double)total_beds / (covered_inhabitants / 1000.0), BEDS_PER_INHABITANT);

    export_result_csv(&result, towns, insee_to_idx, "./src/results/hospitals.csv");
    export_fitness_csv(&result.fitness, "./src/results/fitness.csv");

    for (i = 0; i < town_count; i++)
        free(coverage[i]);
    free(coverage);
    free(coverage_size);
    free(insee_to_idx);
    return result;
}
