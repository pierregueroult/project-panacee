#ifndef TOWN_H
#define TOWN_H

#define TOWN_COUNT 34437

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

#endif
