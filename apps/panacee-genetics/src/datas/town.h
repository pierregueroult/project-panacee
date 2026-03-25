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

#endif