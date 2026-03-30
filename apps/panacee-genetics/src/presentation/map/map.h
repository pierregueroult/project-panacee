#ifndef MAP_H
#define MAP_H

#include "../../domain/town/town.h"

typedef struct {
    double min_lat;
    double max_lat;
    double min_lon;
    double max_lon;
} BoundingBox;

BoundingBox getBoundingBox(Town *towns, int count);

#endif
