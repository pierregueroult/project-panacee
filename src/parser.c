#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datas/town.h"

void parse(Town *towns)
{

    FILE *fptr = fopen("./src/assets/communes-france-metrople-2025.csv", "r");
    if (!fptr)
        printf("Can't open file\n");

    char buffer[1024];

    int column = 0;

    int i = 0;

    while (fgets(buffer, 1024, fptr))
    {
        column = 0;
        char *value = strtok(buffer, ",");
        Town town;
        while (value)
        {
            if (column == 0)
                town.insee = atoi(value);

            if (column == 7)
                town.inhabitants_count = atoi(value);

            if (column == 8)
                town.latitude = atof(value);

            if (column == 9)
                town.longitude = atof(value);
            value = strtok(NULL, ",");
            column++;
        }
        towns[i] = town;
        i++;
    }
}