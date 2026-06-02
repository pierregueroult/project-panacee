#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSEE_COL 0
#define NAME_COL 1
#define DEPARTMENT_CODE_COL 4
#define DEPARTMENT_NAME_COL 5
#define INHABITANTS_COL 7
#define LATITUDE_COL 8
#define LONGITUDE_COL 9

#define LINE_BUFFER 1024
#define DEFAULT_CSV_PATH "../../data/input/communes-france-metrople-2025.csv"

static void copy_field(char *dst, const char *src, size_t cap)
{
    size_t n;
    if (cap == 0)
    {
        return;
    }
    n = strlen(src);
    if (n >= cap)
    {
        n = cap - 1;
    }
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static int count_lines(FILE *fptr)
{
    int lines = 0;
    int started = 0;
    char buffer[LINE_BUFFER];
    while (fgets(buffer, LINE_BUFFER, fptr))
    {
        size_t len = strlen(buffer);
        if (!started)
        {
            lines++;
            started = 1;
        }
        if (len > 0 && buffer[len - 1] == '\n')
        {
            started = 0;
        }
        else if (feof(fptr))
        {
            started = 0;
        }
        else
        {
            fprintf(stderr, "Warning: CSV row exceeds %d bytes (will be truncated)\n", LINE_BUFFER - 1);
        }
    }
    rewind(fptr);
    return lines;
}

static const char *resolve_path(const char *path)
{
    const char *env;
    if (path && *path)
    {
        return path;
    }
    env = getenv("PANACEE_CSV_PATH");
    if (env && *env)
    {
        return env;
    }
    return DEFAULT_CSV_PATH;
}

Town *parse(const char *path, int *count)
{
    const char *resolved = resolve_path(path);
    FILE *fptr;
    int total;
    Town *towns;
    char buffer[LINE_BUFFER];
    int i = 0;

    *count = 0;

    fptr = fopen(resolved, "r");
    if (!fptr)
    {
        fprintf(stderr, "fopen(\"%s\"): ", resolved);
        perror("");
        return NULL;
    }

    total = count_lines(fptr);
    if (total <= 0)
    {
        fclose(fptr);
        return NULL;
    }

    towns = malloc(total * sizeof(Town));

    while (fgets(buffer, LINE_BUFFER, fptr))
    {
        int column = 0;
        char *value;
        Town town;

        memset(&town, 0, sizeof town);
        value = strtok(buffer, ",\n");
        while (value)
        {
            switch (column)
            {
            case INSEE_COL:
                town.insee = atoi(value);
                break;
            case NAME_COL:
                copy_field(town.name, value, sizeof town.name);
                break;
            case DEPARTMENT_CODE_COL:
                copy_field(town.department_code, value, sizeof town.department_code);
                break;
            case DEPARTMENT_NAME_COL:
                copy_field(town.department_name, value, sizeof town.department_name);
                break;
            case INHABITANTS_COL:
                town.inhabitants_count = atoi(value);
                break;
            case LATITUDE_COL:
                town.latitude = atof(value);
                break;
            case LONGITUDE_COL:
                town.longitude = atof(value);
                break;
            default:
                break;
            }
            value = strtok(NULL, ",\n");
            column++;
        }
        towns[i++] = town;
    }

    fclose(fptr);
    *count = i;
    return towns;
}
