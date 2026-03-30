#ifndef TOWN_H
#define TOWN_H

#define TOWN_COUNT 34437

typedef struct
{
    int insee;
    double latitude;
    double longitude;
    int inhabitants_count;
} Town;

double haversine_km(double lat1, double lon1, double lat2, double lon2);
int inhabitant_count(Town *towns, int size);
void precompute_coverage(Town *towns, int town_count, int ***coverage, int **coverage_size);

#endif