#include "map.h"

BoundingBox getBoundingBox(Town *towns, int count)
{
    BoundingBox box;
    int i;

    if (count <= 0) {
        BoundingBox empty = {0, 0, 0, 0};
        return empty;
    }

    box.min_lat = towns[0].latitude;
    box.max_lat = towns[0].latitude;
    box.min_lon = towns[0].longitude;
    box.max_lon = towns[0].longitude;

    for (i = 1; i < count; i++) {
        if (towns[i].latitude < box.min_lat) box.min_lat = towns[i].latitude;
        else if (towns[i].latitude > box.max_lat) box.max_lat = towns[i].latitude;

        if (towns[i].longitude < box.min_lon) box.min_lon = towns[i].longitude;
        else if (towns[i].longitude > box.max_lon) box.max_lon = towns[i].longitude;
    }

    return box;
}
