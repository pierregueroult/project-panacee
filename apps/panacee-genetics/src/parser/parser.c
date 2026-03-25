#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../datas/town.h"

#define INSEE_COL 0
#define INHABITANTS_COL 7
#define LATITUDE_COL 8
#define LONGITUDE_COL 9

int count_lines(FILE *fptr)
{
    int lines = 0;
    char buffer[1024];
    while (fgets(buffer, 1024, fptr))
        lines++;
    rewind(fptr);
    return lines;
}

Town *parse(int *count)
{
    FILE *fptr = fopen("../../data/communes-france-metrople-2025.csv", "r");
    if (!fptr)
    {
        perror("fopen");
        return NULL;
    }

    int total = count_lines(fptr);
    Town *towns = malloc(total * sizeof(Town));
    if (!towns)
    {
        fclose(fptr);
        return NULL;
    }

    char buffer[1024];
    int i = 0;
    while (fgets(buffer, 1024, fptr))
    {
        int column = 0;
        char *value = strtok(buffer, ",");
        Town town = {0};
        while (value)
        {
            if (column == INSEE_COL)
                town.insee = atoi(value);
            if (column == INHABITANTS_COL)
                town.inhabitants_count = atoi(value);
            if (column == LATITUDE_COL)
                town.latitude = atof(value);
            if (column == LONGITUDE_COL)
                town.longitude = atof(value);
            value = strtok(NULL, ",");
            column++;
        }
        towns[i++] = town;
    }

    fclose(fptr);
    *count = i;
    return towns;
}
