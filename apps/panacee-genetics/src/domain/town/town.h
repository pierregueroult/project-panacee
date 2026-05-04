#ifndef TOWN_H
#define TOWN_H

#include "../hospital/hospital.h"

typedef struct
{
    int insee;
    char name[64];
    char department_code[4];
    char department_name[48];
    double latitude;
    double longitude;
    int inhabitants_count;
} Town;

double haversine_km(double lat1, double lon1, double lat2, double lon2);
int inhabitant_count(const Town *towns, int size);
void precompute_coverage(const Town *towns, int town_count,
                         int ***coverage, int **coverage_size);

void nearest_hospital_per_town(const Hospital *hospitals, int hospital_count,
                               const Town *towns, int town_count,
                               const int *insee_to_idx,
                               int **coverage, const int *coverage_size,
                               int *nearest_idx,
                               double *nearest_dist_km);

#endif
