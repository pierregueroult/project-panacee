#ifndef FITNESS_H
#define FITNESS_H

typedef struct
{
    int hospital_count;
    int uhc_count;
    int distant_resident_count;
    double distant_resident_percent;
    int distant_town_count;
    double distant_town_percent;
    double fitness_score;
    int fitness_count;
    double fitness_average;
} Fitness;

double calculate_fitness_score(int total_inhabitants, int distant_residents, int hospitals_count, int uhc_count);
void export_fitness_csv(const Fitness *fitness, const char *path);

#endif
