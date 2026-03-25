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
    {
        result += towns[i].inhabitants_count;
    }

    return result;
}

int is_covered(Town *town, const Town *towns,
               const int *hopitals, int town_count)
{
    int i;
    for (i = 0; i < town_count; i++)
    {
        if (!hopitals[i])
            continue;
        double d = haversine_km(town->latitude, town->longitude,
                                towns[i].latitude, towns[i].longitude);
        if (d <= RADIUS_HOSPITAL_KM)
            return 1;
    }
    return 0;
}

double calculate_fitness_score(int inhabitant_count, int distant_residents, int hopitals_count, int uhc_count)
{
    return (double)inhabitant_count - distant_residents - (double)PENALITY_HOSPITAL * hopitals_count + (double)BONUS_UHC * uhc_count;
}

/* Evaluate an individual */
void evaluate(Individual *ind, Town *towns, int town_count)
{
    int i;
    int distant_residents = 0;
    int distant_towns = 0;
    int uhc_count = 0;
    int inhabitants_count = inhabitant_count(towns, town_count);

    for (i = 0; i < ind->size; i++)
    {
        if (towns[ind->town_indexes[i]].inhabitants_count > TRESHOLD_UHC)
            uhc_count++;
    }

    for (i = 0; i < town_count; i++)
    {
        if (!is_covered(&towns[i], towns, ind->town_indexes, town_count))
        {
            distant_residents += towns[i].inhabitants_count;
            distant_towns++;
        }
    }

    ind->fitness.hospital_count = ind->size;
    ind->fitness.uhc_count = uhc_count;
    ind->fitness.distant_resident_count = distant_residents;
    ind->fitness.distant_town_count = distant_towns;
    ind->fitness.distant_resident_percent =
        (int)(100.0 * distant_residents / inhabitants_count);
    ind->fitness.distant_town_percent =
        (int)(100.0 * distant_towns / town_count);

    ind->fitness.fitness_score = calculate_fitness_score(inhabitants_count, distant_residents, ind->size, uhc_count);
}

/* Evaluate a population */
void evaluate_population(Population *pop, Town *towns, int town_count)
{
    printf("oui\n");

    int i;
    double sum = 0.0;
    for (i = 0; i < pop->size; i++)
    {
        evaluate(&pop->individuals[i], towns, town_count);
        sum += pop->individuals[i].fitness.fitness_score;
    }
    for (i = 0; i < pop->size; i++)
    {
        pop->individuals[i].fitness.fitness_count = pop->size;
        pop->individuals[i].fitness.fitness_average = sum / pop->size;
    }

    printf("oui\n");
}

Individual create_individual(int town_count)
{
    Individual ind;
    int i, k;

    k = (int)(town_count * INIT_HOSPITAL_RATIO);

    ind.town_indexes = malloc(town_count * sizeof(int)); /* max length because we are going to add hospitals later */
    ind.size = 0;

    /* Initialize all the value of the fitness structo to 0 */
    memset(&ind.fitness, 0, sizeof(Fitness));

    /* Random hospitals for the individual */
    {
        char *used = calloc(town_count, 1);
        while (ind.size < k)
        {
            i = rand() % town_count;
            if (!used[i])
            {
                used[i] = 1;
                ind.town_indexes[ind.size++] = i;
            }
        }
        free(used);
    }
    return ind;
}

void free_individual(Individual *ind)
{
    free(ind->town_indexes);
    ind->town_indexes = NULL;
    ind->size = 0;
}

/* Initialize the population */
Population init_population(int town_count)
{
    int i;
    Population pop;
    pop.size = POPULATION_SIZE;
    pop.individuals = malloc(POPULATION_SIZE * sizeof(Individual));
    for (i = 0; i < POPULATION_SIZE; i++)
        pop.individuals[i] = create_individual(town_count);
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

/* Create a child individual by fusionning two individuals */
Individual crossover(const Individual *a, const Individual *b, int town_count)
{
    int i;
    Individual child;
    char *present = calloc(town_count, 1);
    int *pool = malloc((a->size + b->size) * sizeof(int));
    int pool_n = 0;
    int target_size;

    for (i = 0; i < a->size; i++)
        if (!present[a->town_indexes[i]])
        {
            present[a->town_indexes[i]] = 1;
            pool[pool_n++] = a->town_indexes[i];
        }
    for (i = 0; i < b->size; i++)
        if (!present[b->town_indexes[i]])
        {
            present[b->town_indexes[i]] = 1;
            pool[pool_n++] = b->town_indexes[i];
        }

    {
        int lo = a->size < b->size ? a->size : b->size;
        int hi = a->size > b->size ? a->size : b->size;
        target_size = lo + rand() % (hi - lo + 1);
    }
    if (target_size > pool_n)
        target_size = pool_n;

    for (i = pool_n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }

    child.town_indexes = malloc(town_count * sizeof(int));
    child.size = target_size;
    for (i = 0; i < target_size; i++)
        child.town_indexes[i] = pool[i];
    memset(&child.fitness, 0, sizeof(Fitness));

    free(present);
    free(pool);
    return child;
}

void mutate(Individual *ind, int town_count)
{
    int op, idx, new_town, i;

    if ((double)rand() / RAND_MAX > MUTATION_RATE)
        return;

    op = rand() % 3;

    if (op == 0)
    {
        /* Add */
        char *present = calloc(town_count, 1);
        for (i = 0; i < ind->size; i++)
            present[ind->town_indexes[i]] = 1;
        new_town = rand() % town_count;
        if (!present[new_town])
            ind->town_indexes[ind->size++] = new_town;
        free(present);
    }
    else if (op == 1 && ind->size > 1)
    {
        /* Remove */
        idx = rand() % ind->size;
        ind->town_indexes[idx] = ind->town_indexes[--ind->size];
    }
    else
    {
        /* Move */
        idx = rand() % ind->size;
        new_town = rand() % town_count;
        ind->town_indexes[idx] = new_town;

        for (i = 0; i < ind->size; i++)
        {
            if (i != idx && ind->town_indexes[i] == new_town)
            {
                ind->town_indexes[idx] = ind->town_indexes[--ind->size];
                break;
            }
        }
    }
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

Individual run_genetic(Town *towns, int town_count)
{
    int gen, i;
    int stagnation = 0;
    double prev_best = -1e18;
    Individual result;
    Population pop, next_pop;

    srand((unsigned int)time(NULL));

    pop = init_population(town_count);
    evaluate_population(&pop, towns, town_count);

    for (gen = 0; gen < MAX_GENERATIONS; gen++)
    {
        Individual current_best = best_individual(&pop);
        double best_score = current_best.fitness.fitness_score;

        printf("Gen %4d | fitness: %.0f | hop: %d | CHRU: %d | "
               "desert: %d (%.1f%%)\n",
               gen,
               best_score,
               current_best.fitness.hospital_count,
               current_best.fitness.uhc_count,
               current_best.fitness.distant_resident_count,
               (double)current_best.fitness.distant_resident_percent);

        /* Critère d'arrêt : stagnation */
        if (best_score > prev_best)
        {
            prev_best = best_score;
            stagnation = 0;
        }
        else
        {
            stagnation++;
        }
        if (stagnation >= STAGNATION_LIMIT)
        {
            printf("Stop : stagnation on %d generations.\n", STAGNATION_LIMIT);
            break;
        }

        /* Construire la prochaine génération */
        next_pop.size = POPULATION_SIZE;
        next_pop.individuals = malloc(POPULATION_SIZE * sizeof(Individual));

        /* Élitisme : conserver le meilleur individu tel quel */
        {
            Individual elite = best_individual(&pop);
            next_pop.individuals[0].town_indexes =
                malloc(town_count * sizeof(int));
            next_pop.individuals[0].size = elite.size;
            memcpy(next_pop.individuals[0].town_indexes,
                   elite.town_indexes,
                   elite.size * sizeof(int));
            next_pop.individuals[0].fitness = elite.fitness;
        }

        for (i = 1; i < POPULATION_SIZE; i++)
        {
            Individual p1 = tournament_select(&pop);
            Individual p2 = tournament_select(&pop);
            next_pop.individuals[i] = crossover(&p1, &p2, town_count);
            mutate(&next_pop.individuals[i], town_count);
        }

        free_population(&pop);
        pop = next_pop;
        evaluate_population(&pop, towns, town_count);
    }

    /* Copier la meilleure solution avant de libérer la population */
    {
        Individual b = best_individual(&pop);
        result.town_indexes = malloc(b.size * sizeof(int));
        result.size = b.size;
        result.fitness = b.fitness;
        memcpy(result.town_indexes, b.town_indexes, b.size * sizeof(int));
    }
    free_population(&pop);
    return result;
}
