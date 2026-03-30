#ifndef POPULATION_H
#define POPULATION_H

#include "individual.h"

typedef struct
{
    Individual *individuals;
    int         size;
} Population;

#endif
