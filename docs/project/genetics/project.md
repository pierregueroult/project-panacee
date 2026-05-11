# Genetic Algorithm — Panacée

## 1. How a genetic algorithm works

A genetic algorithm is a metaheuristic inspired by Darwinian natural selection. It evolves a **population** of candidate solutions across successive **generations**, applying selection pressure, recombination, and random mutation to progressively improve solution quality.

The general loop is:

```
1. Initialize population
2. Evaluate fitness of each individual
3. While stopping criterion not met:
   a. Select parents (tournament selection)
   b. Recombine parents (crossover) → children
   c. Mutate children
   d. Remove redundant genes (pruning)
   e. Replace old population with new generation (with elitism)
   f. Evaluate new population
4. Post-process best individual (local search, bed count)
5. Export results
```

---

## 2. Problem statement

The algorithm solves a **coverage optimization problem** over metropolitan France:

> Given 34 437 towns with population data and GPS coordinates, place the minimum number of hospitals such that every town is covered by at least one hospital within a 10 km radius, while maximizing the total covered population.

---

## 3. Data structures

### Town

Represents a French municipality parsed from the INSEE dataset.

```c
typedef struct {
    int    insee;              /* INSEE code (unique town identifier)   */
    double latitude;
    double longitude;
    int    inhabitants_count;
} Town;
```

### Hospital

A hospital placed in a town. The `beds_count` is computed at the end of the algorithm based on how many inhabitants the hospital serves.

```c
typedef struct {
    int insee;       /* town where the hospital is located */
    int beds_count;  /* computed after optimization        */
} Hospital;
```

### Individual

One candidate solution: a set of towns selected to host a hospital.

```c
typedef struct {
    Hospital *hospitals;  /* array of selected hospital locations */
    int       size;       /* number of hospitals                  */
    Fitness   fitness;    /* evaluation result                    */
} Individual;
```

### Population

A collection of individuals evaluated together in the same generation.

```c
typedef struct {
    Individual *individuals;
    int         size;
} Population;
```

### Fitness

All metrics produced by evaluating one individual.

```c
typedef struct {
    int    hospital_count;
    int    uhc_count;               /* number of hospitals in towns > 80k inhabitants (CHRU) */
    int    distant_resident_count;  /* inhabitants not covered by any hospital               */
    double distant_resident_percent;
    int    distant_town_count;
    double distant_town_percent;
    double fitness_score;           /* main optimization objective                           */
    int    fitness_count;           /* population size at evaluation time                    */
    double fitness_average;         /* average score across the population                   */
} Fitness;
```

---

## 4. Fitness function

The fitness score drives the entire optimization. It rewards coverage and penalizes cost:

```
fitness = total_inhabitants
        − distant_residents          (uncovered population)
        − PENALITY_HOSPITAL × H      (cost per hospital)
        + BONUS_UHC × U              (bonus for CHRU-grade hospitals)
```

| Constant             | Value    | Meaning                                              |
|----------------------|----------|------------------------------------------------------|
| `PENALITY_HOSPITAL`  | 5 000    | Score deducted per hospital placed                   |
| `BONUS_UHC`          | 4 000    | Score added when a hospital is in a town > 80k pop.  |
| `TRESHOLD_UHC`       | 80 000   | Population threshold to qualify as a CHRU            |
| `RADIUS_HOSPITAL_KM` | 10 km    | Coverage radius of a hospital                        |

A higher score means fewer uncovered residents, fewer hospitals, and more CHRU-grade placements.

---

## 5. Coverage precomputation

Before running the genetic loop, the algorithm precomputes for every town the list of towns within 10 km (the **coverage map**). This is done once using the Haversine formula with a latitude bounding-box filter to skip obviously distant pairs.

```
coverage[i] = array of town indices within 10 km of town i
coverage_size[i] = number of such towns
```

This precomputation transforms the per-evaluation cost from O(N²) to O(N × avg_coverage), making the genetic loop fast enough to run hundreds of thousands of generations.

---

## 6. Algorithm steps

### 6.1 Initialization

The initial population mixes two strategies to balance quality and diversity:

- **80% greedy individuals** — a stochastic greedy heuristic places hospitals one by one, at each step picking randomly from towns whose coverage gain is within 90% of the current best. This produces well-spread, high-quality seeds.
- **20% random individuals** — towns are selected uniformly at random. This preserves genetic diversity.

Each individual starts with roughly `town_count × 5%` hospitals, with some variance to diversify genome sizes.

### 6.2 Evaluation

Each individual is scored by:
1. Marking all towns covered by its hospitals (via the precomputed coverage map).
2. Counting uncovered towns and their population (`distant_residents`).
3. Counting CHRU-eligible hospitals (`uhc_count`).
4. Applying the fitness formula.

Population-level statistics (average score) are also computed.

### 6.3 Selection — Tournament

To select a parent, `TOURNAMENT_K = 3` individuals are drawn at random from the population. The one with the highest fitness score wins. This keeps selection pressure moderate and preserves diversity.

### 6.4 Crossover

Two parents are recombined to produce one child:
1. Their hospital lists are merged into a pool of unique towns (by INSEE code).
2. A `target_size` is drawn uniformly between `min(|p1|, |p2|)` and `max(|p1|, |p2|)`.
3. The pool is shuffled (Fisher-Yates) and the first `target_size` entries become the child's hospitals.

This operator preserves genes present in at least one parent while exploring different subsets.

### 6.5 Mutation

Applied with probability `MUTATION_RATE = 0.15` per individual. Three equally probable operations:

| Operation | Description |
|-----------|-------------|
| **Smart add** | Find the town that would cover the most currently uncovered inhabitants and add a hospital there. |
| **Remove** | Delete a randomly chosen hospital. |
| **Move** | Relocate a randomly chosen hospital to a random town. If the destination already has a hospital, the source is removed instead. |

### 6.6 Redundancy pruning

After crossover and mutation, hospitals that do not exclusively cover any town (every town they cover is also covered by another hospital) are removed. This keeps the genome lean and avoids wasting the hospital penalty.

### 6.7 Elitism

The best individual from the current generation is carried unchanged into the next generation (index 0). This guarantees that the best solution never degrades between generations.

### 6.8 Adaptive mutation

If the best fitness score does not improve for several consecutive generations (*stagnation*), the mutation rate increases progressively (up to 0.40) to escape local optima. The rate resets to `MUTATION_RATE` as soon as improvement is found.

### 6.9 Stopping criterion

The algorithm stops when either:
- `MAX_GENERATIONS = 200 000` generations are reached, or
- stagnation lasts `STAGNATION_LIMIT = 500` generations without improvement.

---

## 7. Post-optimization local search

After the genetic loop, a greedy local search refines the best solution:

> For every uncovered town, compute the number of inhabitants that would be newly covered by placing a hospital there. If this gain exceeds `PENALITY_HOSPITAL`, add the hospital.

This pass repeats until no improving addition is found. It ensures the final result is locally optimal with respect to single hospital insertions.

---

## 8. Bed count computation

Once the final placement is determined, each hospital is assigned a `beds_count`:

1. Every town is assigned to its **nearest** hospital (by Haversine distance) within coverage radius.
2. `beds_count = floor(BEDS_PER_INHABITANT / 1000 × assigned_inhabitants)`

The target ratio is **5.40 beds per 1 000 covered inhabitants** (`BEDS_PER_INHABITANT = 5.40`).

---

## 9. Hyperparameters summary

| Constant               | Value    | Role                                          |
|------------------------|----------|-----------------------------------------------|
| `POPULATION_SIZE`      | 50       | Number of individuals per generation          |
| `TOURNAMENT_K`         | 3        | Tournament size for parent selection          |
| `MUTATION_RATE`        | 0.15     | Base probability of mutation per individual   |
| `MAX_GENERATIONS`      | 200 000  | Hard generation limit                         |
| `STAGNATION_LIMIT`     | 500      | Consecutive non-improving generations before stop |
| `INIT_HOSPITAL_RATIO`  | 0.05     | ~5% of towns receive a hospital at init       |
| `RADIUS_HOSPITAL_KM`   | 10 km    | Hospital coverage radius                      |
| `PENALITY_HOSPITAL`    | 5 000    | Fitness cost per hospital                     |
| `BONUS_UHC`            | 4 000    | Fitness bonus per CHRU-eligible hospital      |
| `TRESHOLD_UHC`         | 80 000   | Population threshold for CHRU status          |
| `BEDS_PER_INHABITANT`  | 5.40     | Target beds per 1 000 covered inhabitants     |
| `EARTH_RADIUS_KM`      | 6 371 km | Used in Haversine distance formula            |

---

## 10. Output files

| File                          | Content                                                     |
|-------------------------------|-------------------------------------------------------------|
| `src/results/hospitals.csv`   | `insee,beds_count` — one row per hospital in the solution   |
| `src/results/fitness.csv`     | All fitness metrics of the final best individual            |
